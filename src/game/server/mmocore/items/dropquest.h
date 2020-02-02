/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#ifndef GAME_SERVER_ENTITIES_QUESTITEM_H
#define GAME_SERVER_ENTITIES_QUESTITEM_H

class CQuestItem : public CEntity
{
	vec2 m_ActualPos;
	vec2 m_ActualDir;
	vec2 m_Direction;
	int m_StartTick;

	int m_OwnerID;
	ContextBots::QuestBotInfo m_QuestBot;
	bool m_Collide;

	vec2 GetTimePos(float Time);
public:
	CQuestItem(CGameWorld *pGameWorld, vec2 Pos, vec2 Dir, ContextBots::QuestBotInfo BotData, int OwnerID);
	virtual ~CQuestItem();

	virtual void Tick();
	virtual void TickPaused(); 
	virtual void Snap(int SnappingClient);

};

#endif
