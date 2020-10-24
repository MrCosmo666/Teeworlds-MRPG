/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#ifndef GAME_SERVER_STEP_QUEST_H
#define GAME_SERVER_STEP_QUEST_H

// information steps data
class CStepQuestBot
{
public:
	BotJob::QuestBotInfo* m_Bot;

	void UpdateBot(CGS* pGS);
	bool IsActiveStep(CGS* pGS) const;
};

class CPlayerStepQuestBot : public CStepQuestBot
{
public:
	int m_MobProgress[2];
	bool m_StepComplete;
	bool m_ClientQuitting;

	bool IsCompleteItems(CPlayer* pPlayer) const;
	bool IsCompleteMobs(CPlayer* pPlayer) const;
	bool Finish(CPlayer* pPlayer, bool LastDialog);
	void DoCollectItem(CPlayer* pPlayer);
	void AddMobProgress(CPlayer* pPlayer, int BotID);
	void CreateStepArrow(CPlayer* pPlayer);
	void CreateQuestingItems(CPlayer* pPlayer);
	void ShowRequired(CPlayer* pPlayer, const char* TextTalk);
};

#endif