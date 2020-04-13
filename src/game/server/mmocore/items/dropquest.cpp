/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#include <engine/shared/config.h>

#include <base/math.h>
#include <base/vmath.h>
#include <generated/protocol.h>
#include <game/server/gamecontext.h>

#include "dropquest.h"

CQuestItem::CQuestItem(CGameWorld *pGameWorld, vec2 Pos, vec2 Dir, const ContextBots::QuestBotInfo BotData, int OwnerID)
: CEntity(pGameWorld, CGameWorld::ENTTYPE_DROPITEM, Pos), m_Direction(vec2(Dir.x, Dir.y+2/rand()%9))
{
	m_Pos = Pos;
	m_ActualPos = Pos;
	m_ActualDir = Dir;
	m_Direction = Dir;

	// other var
	m_OwnerID = OwnerID;
	m_QuestBot = BotData;
	m_StartTick = Server()->Tick();

	// create object
	GameWorld()->InsertEntity(this);
}

CQuestItem::~CQuestItem(){ }

vec2 CQuestItem::GetTimePos(float Time)
{
	float Curvature = 1.25f;
	float Speed = 2750.0f;

	return CalcPos(m_Pos, m_Direction, Curvature, Speed, Time);
}

void CQuestItem::Tick()
{
	// проверяем есть игрок / если бота нет / если квеста нет
	if(!GS()->m_apPlayers[m_OwnerID] ||
		QuestBase::Quests[m_OwnerID].find(m_QuestBot.QuestID) == QuestBase::Quests[m_OwnerID].end())
	{
		GS()->m_World.DestroyEntity(this);
		return;		
	}

	int Count = m_QuestBot.InterCount[0];
	int QuestID = m_QuestBot.QuestID;
	CPlayer *pPlayer = GS()->m_apPlayers[m_OwnerID];
	ItemSql::ItemPlayer &PlItemForQuest = pPlayer->GetItem(m_QuestBot.Interactive[0]);

	// проверяем если прогресс не равен данному / проверяем завершен ли квест / проверяем если предметов больше чем требуется
	if(QuestBase::Quests[m_OwnerID][QuestID].TalkProgress != m_QuestBot.Progress || PlItemForQuest.Count >= Count)
	{
		GS()->m_World.DestroyEntity(this);
		return;			
	}

	// подбор предмета
	if(pPlayer->GetCharacter() && distance(m_ActualPos, pPlayer->GetCharacter()->m_Core.m_Pos) < 32)
	{
		// текст подбора предмета
		GS()->SBL(m_OwnerID, 1000, 10, "Press 'Fire' for pick Quest Item");

		// если кнопка нажата
		if(pPlayer->GetCharacter()->m_ReloadTimer)
		{
			pPlayer->GetCharacter()->m_ReloadTimer = 0;
			PlItemForQuest.Add(1);
			GS()->Chat(m_OwnerID, "You pick {STR} for {STR}!", PlItemForQuest.Info().GetName(pPlayer), m_QuestBot.Name);
			GS()->m_World.DestroyEntity(this);
			return;
		}
	}

	// эфект рандомного гравитации ляля flagball
	float Pt = (Server()->Tick()-m_StartTick-1)/(float)Server()->TickSpeed();
	float Ct = (Server()->Tick()-m_StartTick)/(float)Server()->TickSpeed();
	vec2 PrevPos = GetTimePos(Pt), CurPos = GetTimePos(Ct);
	m_ActualPos = CurPos;
	m_ActualDir = normalize(CurPos - PrevPos);

	vec2 LastPos;
	m_Collide = GS()->Collision()->IntersectLine(PrevPos, CurPos, NULL, &LastPos);
	if(m_Collide)
	{			
		m_Pos = LastPos;
		m_ActualPos = m_Pos;
		m_Direction.x *= (100 - 50) / 100.0f;
		m_Direction.y *= (100 - 50) / 65.0f;
		m_StartTick = Server()->Tick();
		m_ActualDir = normalize(m_Direction);
	}
}

void CQuestItem::TickPaused()
{
	m_StartTick++;
}

void CQuestItem::Snap(int SnappingClient)
{
	float Ct = (Server()->Tick()-m_StartTick)/(float)Server()->TickSpeed();
	
	if(!m_Collide || m_OwnerID != SnappingClient || NetworkClipped(SnappingClient, GetTimePos(Ct)))
		return;

	CNetObj_Pickup *pP = static_cast<CNetObj_Pickup *>(Server()->SnapNewItem(NETOBJTYPE_PICKUP, GetID(), sizeof(CNetObj_Pickup)));
	if(!pP)
		return;

	pP->m_X = (int)m_ActualPos.x;
	pP->m_Y = (int)m_ActualPos.y;
	pP->m_Type = 0;
} 