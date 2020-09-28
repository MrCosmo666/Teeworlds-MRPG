/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
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
	m_MainScenario = str_startswith(GS()->Mmo()->Quest()->GetStoryName(m_QuestID), "Main:") != nullptr;
	GameWorld()->InsertEntity(this);
}

void CQuestPathFinder::Tick() 
{
	CPlayer* pPlayer = GS()->GetPlayer(m_ClientID, true, true);
	if (m_TargetPos == vec2(0.0f, 0.0f) || !pPlayer || QuestJob::ms_aQuests[m_ClientID][m_QuestID].m_Progress != m_QuestProgress || QuestJob::ms_aQuests[m_ClientID][m_QuestID].m_State != QuestState::QUEST_ACCEPT)
	{
		Finish();
		return;
	}
	vec2 Direction = normalize(GS()->m_apPlayers[m_ClientID]->GetCharacter()->m_Core.m_Pos - m_TargetPos);
	m_Pos = GS()->m_apPlayers[m_ClientID]->GetCharacter()->m_Core.m_Pos - Direction * 90;
}

void CQuestPathFinder::Finish()
{
	if(GS()->GetPlayer(m_ClientID, true, true))
	{
		GS()->CreateDeath(m_Pos, m_ClientID);
	}
	GS()->m_World.DestroyEntity(this);
	return;
}

void CQuestPathFinder::Snap(int SnappingClient)
{
	if(NetworkClipped(SnappingClient) || SnappingClient != m_ClientID)
		return;

	if (GS()->CheckClient(SnappingClient))
	{
		vec2 Direction = normalize(m_Pos - m_TargetPos);
		CNetObj_MmoPickup* pMmoP = static_cast<CNetObj_MmoPickup*>(Server()->SnapNewItem(NETOBJTYPE_MMOPICKUP, GetID(), sizeof(CNetObj_MmoPickup)));
		if (!pMmoP)
			return;

		pMmoP->m_X = (int)m_Pos.x;
		pMmoP->m_Y = (int)m_Pos.y;
		pMmoP->m_Type = (m_MainScenario ? (int)MMO_PICKUP_MAIN_ARROW : (int)MMO_PICKUP_SIDE_ARROW);
		pMmoP->m_Angle = (int)(angle(vec2(Direction.x, Direction.y)) * 256.0f);
		return;
	}

	CNetObj_Pickup *pP = static_cast<CNetObj_Pickup *>(Server()->SnapNewItem(NETOBJTYPE_PICKUP, GetID(), sizeof(CNetObj_Pickup)));
	if(!pP)
		return;

	pP->m_X = (int)m_Pos.x;
	pP->m_Y = (int)m_Pos.y;
	pP->m_Type = (m_MainScenario ? (int)PICKUP_HEALTH : (int)PICKUP_ARMOR);
} 