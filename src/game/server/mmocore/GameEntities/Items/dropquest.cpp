/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#include <engine/shared/config.h>

#include <base/math.h>
#include <base/vmath.h>
#include <generated/protocol.h>
#include <game/server/gamecontext.h>

#include "dropquest.h"

CQuestItem::CQuestItem(CGameWorld *pGameWorld, vec2 Pos, vec2 Vel, float AngleForce, const BotJob::QuestBotInfo BotData, int OwnerID)
: CEntity(pGameWorld, CGameWorld::ENTTYPE_DROPQUEST, Pos)
{
	m_Pos = Pos;
	m_Vel = Vel;
	m_Angle = 0.0f;
	m_AngleForce = AngleForce;

	m_OwnerID = OwnerID;
	m_QuestBot = BotData;
	m_Flashing = false;
	m_LifeSpan = Server()->TickSpeed() * 15;
	m_StartTick = Server()->Tick();

	GameWorld()->InsertEntity(this);
}

CQuestItem::~CQuestItem(){ }

void CQuestItem::Tick()
{
	if (!GS()->m_apPlayers[m_OwnerID] || QuestJob::Quests[m_OwnerID].find(m_QuestBot.QuestID) == QuestJob::Quests[m_OwnerID].end())
	{
		GS()->m_World.DestroyEntity(this);
		return;
	}

	m_LifeSpan--;
	if (m_LifeSpan < 0)
	{
		GS()->m_World.DestroyEntity(this);
		return;
	}
	if (m_LifeSpan < 150)
	{
		m_FlashTimer--;
		if (m_FlashTimer > 5)
			m_Flashing = true;
		else
		{
			m_Flashing = false;
			if (m_FlashTimer <= 0)
				m_FlashTimer = 10;
		}
	}


	m_Vel.y += 0.5f;
	m_Collide = (bool)GS()->Collision()->CheckPoint(m_Pos.x - 12, m_Pos.y + 12 + 5) || GS()->Collision()->CheckPoint(m_Pos.x + 12, m_Pos.y + 12 + 5);
	if (m_Collide)
	{
		m_AngleForce += (m_Vel.x - 0.74f * 6.0f - m_AngleForce) / 2.0f;
		m_Vel.x *= 0.8f;
	}
	else
	{
		m_Angle += clamp(m_AngleForce * 0.04f, -0.6f, 0.6f);
		m_Vel.x *= 0.99f;
	}
	GS()->Collision()->MoveBox(&m_Pos, &m_Vel, vec2(24.0f, 24.0f), 0.4f);


	int Count = m_QuestBot.ItemSearchCount[0];
	int QuestID = m_QuestBot.QuestID;
	CPlayer* pPlayer = GS()->m_apPlayers[m_OwnerID];
	ItemJob::InventoryItem& PlItemForQuest = pPlayer->GetItem(m_QuestBot.ItemSearch[0]);

	// проверяем если прогресс не равен данному / проверяем завершен ли квест / проверяем если предметов больше чем требуется
	if (QuestJob::Quests[m_OwnerID][QuestID].Progress != m_QuestBot.Progress || PlItemForQuest.Count >= Count)
	{
		GS()->m_World.DestroyEntity(this);
		return;
	}

	// подбор предмета
	if (pPlayer->GetCharacter() && distance(m_Pos, pPlayer->GetCharacter()->m_Core.m_Pos) < 32)
	{
		// текст подбора предмета
		GS()->SBL(m_OwnerID, BroadcastPriority::BROADCAST_GAME_INFORMATION, 10, "Press 'Fire' for pick Quest Item");

		// если кнопка нажата
		if (pPlayer->GetCharacter()->m_ReloadTimer)
		{
			GS()->CreatePlayerSound(m_OwnerID, SOUND_ITEM_PICKUP);
			pPlayer->GetCharacter()->m_ReloadTimer = 0;
			PlItemForQuest.Add(1);
			GS()->Chat(m_OwnerID, "You pick {STR} for {STR}!", PlItemForQuest.Info().GetName(pPlayer), m_QuestBot.GetName());
			GS()->m_World.DestroyEntity(this);
			return;
		}
	}
}

void CQuestItem::TickPaused()
{
	m_StartTick++;
}

void CQuestItem::Snap(int SnappingClient)
{
	if(m_Flashing || !m_Collide || m_OwnerID != SnappingClient || NetworkClipped(SnappingClient))
		return;

	// проверка клиента если чекнут дальше не рисуем
	if (GS()->CheckClient(SnappingClient))
	{
		CNetObj_MmoPickup* pObj = static_cast<CNetObj_MmoPickup*>(Server()->SnapNewItem(NETOBJTYPE_MMOPICKUP, GetID(), sizeof(CNetObj_MmoPickup)));
		if (!pObj)
			return;

		pObj->m_X = (int)m_Pos.x;
		pObj->m_Y = (int)m_Pos.y;
		pObj->m_Type = MMO_PICKUP_DROP;
		pObj->m_Angle = (int)(m_Angle * 256.0f);
		return;
	}

	CNetObj_Pickup *pP = static_cast<CNetObj_Pickup *>(Server()->SnapNewItem(NETOBJTYPE_PICKUP, GetID(), sizeof(CNetObj_Pickup)));
	if(!pP)
		return;

	pP->m_X = (int)m_Pos.x;
	pP->m_Y = (int)m_Pos.y;
	pP->m_Type = 0;
} 