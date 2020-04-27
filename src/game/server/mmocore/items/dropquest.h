/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#ifndef GAME_SERVER_ENTITIES_QUESTITEM_H
#define GAME_SERVER_ENTITIES_QUESTITEM_H

class CQuestItem : public CEntity
{
	vec2 m_Vel;
	float m_Angle;
	float m_AngleForce;

	int m_StartTick;

	bool m_Collide;
	bool m_Flashing;
	int m_LifeSpan;
	int m_FlashTimer;

public:
	CQuestItem(CGameWorld *pGameWorld, vec2 Pos, vec2 Vel, float AngleForce, ContextBots::QuestBotInfo BotData, int OwnerID);
	virtual ~CQuestItem();

	int m_OwnerID;
	ContextBots::QuestBotInfo m_QuestBot;

	virtual void Tick();
	virtual void TickPaused(); 
	virtual void Snap(int SnappingClient);

};

#endif
