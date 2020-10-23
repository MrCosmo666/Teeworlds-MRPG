/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#ifndef GAME_SERVER_ENTITIES_QUESTAI_H
#define GAME_SERVER_ENTITIES_QUESTAI_H

#include <game/server/entity.h>

class CQuestPathFinder : public CEntity
{
	int m_ClientID;
	int m_QuestID;
	int m_QuestProgress;
	bool m_MainScenario;

public:
	CQuestPathFinder(CGameWorld* pGameWorld, vec2 Pos, int ClientID, int QuestID, int QuestProgress, vec2 TargetPos);

	virtual void Tick();
	virtual void Snap(int SnappingClient);

	vec2 m_TargetPos;
	int GetQuestID() const { return m_QuestID; }
	int GetClientID() const { return m_ClientID; }
};

#endif
