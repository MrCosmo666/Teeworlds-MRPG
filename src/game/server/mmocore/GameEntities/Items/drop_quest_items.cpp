/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#include <base/math.h>
#include <base/vmath.h>
#include <generated/protocol.h>
#include <game/server/gamecontext.h>

#include "drop_quest_items.h"

CDropQuestItem::CDropQuestItem(CGameWorld *pGameWorld, vec2 Pos, vec2 Vel, float AngleForce, const BotJob::QuestBotInfo BotData, int OwnerID)
: CEntity(pGameWorld, CGameWorld::ENTTYPE_DROPQUEST, Pos, 24.0f)
{
	m_Pos = Pos;
	m_Vel = Vel;
	m_Angle = 0.0f;
	m_AngleForce = AngleForce;

	m_OwnerID = OwnerID;
	m_QuestBot = BotData;
	m_Flashing = false;
	m_LifeSpan = Server()->TickSpeed() * 60;
	m_StartTick = Server()->Tick();

	GameWorld()->InsertEntity(this);
	for(int i=0; i<NUM_IDS; i++)
	{
		m_IDs[i] = Server()->SnapNewID();
	}
}

CDropQuestItem::~CDropQuestItem()
{
	for(int i=0; i<NUM_IDS; i++)
	{
		Server()->SnapFreeID(m_IDs[i]);
	}
 }

void CDropQuestItem::Tick()
{
	// life time dk
	m_LifeSpan--;
	if (m_LifeSpan < 0 || !GS()->m_apPlayers[m_OwnerID] || GS()->Mmo()->Quest()->GetState(m_OwnerID, m_QuestBot.m_QuestID) != QuestState::QUEST_ACCEPT)
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


	// physic
	m_Vel.y += 0.5f;
	static const float CheckSize = (GetProximityRadius()/2.0f);
	m_Collide = (bool)GS()->Collision()->CheckPoint(m_Pos.x - CheckSize, m_Pos.y + CheckSize + 5) 
		|| GS()->Collision()->CheckPoint(m_Pos.x + CheckSize, m_Pos.y + CheckSize + 5);
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


	// interactive
	const int Count = m_QuestBot.m_aItemSearchCount[0];
	const int QuestID = m_QuestBot.m_QuestID;
	CPlayer* pOwnerPlayer = GS()->m_apPlayers[m_OwnerID];
	InventoryItem& pPlayerQuestItem = pOwnerPlayer->GetItem(m_QuestBot.m_aItemSearch[0]);
	if (QuestJob::ms_aQuests[m_OwnerID][QuestID].m_Progress != m_QuestBot.m_Progress || pPlayerQuestItem.m_Count >= Count)
	{
		GS()->m_World.DestroyEntity(this);
		return;
	}

	if (m_Collide && pOwnerPlayer->GetCharacter() && distance(m_Pos, pOwnerPlayer->GetCharacter()->m_Core.m_Pos) < 32.0f)
	{
		// item selection text
		GS()->SBL(m_OwnerID, BroadcastPriority::BROADCAST_GAME_INFORMATION, 10, "Press 'Fire' for pick Quest Item");
		if (pOwnerPlayer->GetCharacter()->m_ReloadTimer)
		{
			GS()->CreatePlayerSound(m_OwnerID, SOUND_ITEM_PICKUP);
			pOwnerPlayer->GetCharacter()->m_ReloadTimer = 0;
			pPlayerQuestItem.Add(1);
			GS()->Chat(m_OwnerID, "You pick {STR} for {STR}!", pPlayerQuestItem.Info().GetName(pOwnerPlayer), m_QuestBot.GetName());
			GS()->m_World.DestroyEntity(this);
			return;
		}
	}
}


void CDropQuestItem::Snap(int SnappingClient)
{
	if(m_Flashing || !m_Collide || m_OwnerID != SnappingClient || NetworkClipped(SnappingClient))
		return;

	// mrpg
	if (GS()->IsMmoClient(SnappingClient))
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

	// vanilla
	CNetObj_Projectile* pProj = static_cast<CNetObj_Projectile*>(Server()->SnapNewItem(NETOBJTYPE_PROJECTILE, GetID(), sizeof(CNetObj_Projectile)));
	if (pProj)
	{
		pProj->m_X = (int)m_Pos.x;
		pProj->m_Y = (int)m_Pos.y;
		pProj->m_VelX = 0;
		pProj->m_VelY = 0;
		pProj->m_StartTick = Server()->Tick();
		pProj->m_Type = WEAPON_HAMMER;
	}

	static const float Radius = 16.0f;
	const float AngleStep = 2.0f * pi / CDropQuestItem::NUM_IDS;
	const float AngleStart = (pi / CDropQuestItem::NUM_IDS) + (2.0f * pi * m_Angle);
	for(int i = 0; i < CDropQuestItem::NUM_IDS; i++)
	{
		CNetObj_Laser *pRifleObj = static_cast<CNetObj_Laser *>(Server()->SnapNewItem(NETOBJTYPE_LASER, m_IDs[i], sizeof(CNetObj_Laser)));
		if(!pRifleObj)
			return;

		vec2 Pos = m_Pos + vec2(Radius * cos(AngleStart + AngleStep * i), Radius * sin(AngleStart + AngleStep * i));
		vec2 PosTo = m_Pos + vec2(Radius * cos(AngleStart + AngleStep * (i+1)), Radius * sin(AngleStart + AngleStep * (i+1)));
		pRifleObj->m_X = (int)Pos.x;
		pRifleObj->m_Y = (int)Pos.y;
		pRifleObj->m_FromX = (int)PosTo.x;
		pRifleObj->m_FromY = (int)PosTo.y;
		pRifleObj->m_StartTick = Server()->Tick() - 4;
	}
} 