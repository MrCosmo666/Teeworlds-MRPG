/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#ifndef GAME_SERVER_ENTITIES_QUESTITEM_H
#define GAME_SERVER_ENTITIES_QUESTITEM_H

class CDropQuestItem : public CEntity
{
	enum 
	{
		NUM_IDS = 3
	};
	int m_IDs[NUM_IDS];

	vec2 m_Vel;
	float m_Angle;
	float m_AngleForce;
	bool m_Flashing;
	int m_LifeSpan;
	int m_FlashTimer;

public:
	CDropQuestItem(CGameWorld *pGameWorld, vec2 Pos, vec2 Vel, float AngleForce, BotJob::QuestBotInfo BotData, int OwnerID);
	virtual ~CDropQuestItem();

	int m_OwnerID;
	BotJob::QuestBotInfo m_QuestBot;

	virtual void Tick();
	virtual void Snap(int SnappingClient);

};

#endif
