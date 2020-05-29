/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#include <base/math.h>
#include <base/vmath.h>
#include <generated/protocol.h>

#include <game/server/gamecontext.h>
#include "quest_path_finder.h"

CQuestPathFinder::CQuestPathFinder(CGameWorld *pGameWorld, vec2 Pos, int ClientID, int QuestID, int QuestProgress, vec2 TargetPos)
: CEntity(pGameWorld, CGameWorld::ENTTYPE_FINDQUEST, Pos)
{
	m_QuestID = QuestID;
	m_QuestProgress = QuestProgress;
	m_ClientID = ClientID;
	m_TargetPos = TargetPos;
	GameWorld()->InsertEntity(this);
}

void CQuestPathFinder::Tick() 
{
	CPlayer* pPlayer = GS()->GetPlayer(m_ClientID, true, true);
	if (!pPlayer || QuestJob::Quests[m_ClientID][m_QuestID].Progress != m_QuestProgress || QuestJob::Quests[m_ClientID][m_QuestID].State != QuestState::QUEST_ACCEPT)
	{
		GS()->m_World.DestroyEntity(this);
		return;
	}
	vec2 Direction = normalize(GS()->m_apPlayers[m_ClientID]->GetCharacter()->m_Core.m_Pos - m_TargetPos);
	m_Pos = GS()->m_apPlayers[m_ClientID]->GetCharacter()->m_Core.m_Pos - Direction * 90;
}

void CQuestPathFinder::Finish()
{
	GS()->CreateDeath(m_Pos, m_ClientID);
	GS()->m_World.DestroyEntity(this);
	return;
}

void CQuestPathFinder::Snap(int SnappingClient)
{
	if(NetworkClipped(SnappingClient) || m_TargetPos == vec2(0.0f, 0.0f) || SnappingClient != m_ClientID)
		return;

	// �������� ������� ���� ������ ������ �� ������
	if (GS()->CheckClient(SnappingClient))
	{
		vec2 Direction = normalize(m_Pos - m_TargetPos);
		CNetObj_MmoPickup* pObj = static_cast<CNetObj_MmoPickup*>(Server()->SnapNewItem(NETOBJTYPE_MMOPICKUP, GetID(), sizeof(CNetObj_MmoPickup)));
		if (!pObj)
			return;

		pObj->m_X = (int)m_Pos.x;
		pObj->m_Y = (int)m_Pos.y;
		pObj->m_Type = MMO_PICKUP_ARROW;
		pObj->m_Angle = (int)(angle(vec2(Direction.x, Direction.y)) * 256.0f);
		return;
	}

	CNetObj_Pickup *pP = static_cast<CNetObj_Pickup *>(Server()->SnapNewItem(NETOBJTYPE_PICKUP, GetID(), sizeof(CNetObj_Pickup)));
	if(!pP)
		return;

	pP->m_X = (int)m_Pos.x;
	pP->m_Y = (int)m_Pos.y;
	pP->m_Type = PICKUP_ARMOR;
} 