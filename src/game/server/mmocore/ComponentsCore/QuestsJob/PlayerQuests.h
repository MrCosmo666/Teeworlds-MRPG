/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#ifndef GAME_SERVER_PLAYER_QUESTS_H
#define GAME_SERVER_PLAYER_QUESTS_H

#include "DataQuests.h"
#include "StepQuestBot.h"

class CPlayerQuest
{

public:
	int m_QuestID;
	CPlayer* m_pPlayer;

	int m_State;
	int m_Step;

	std::string GetJsonName() const;
	CDataQuest& Info() const;
	bool IsComplected() const { return m_State == QuestState::QUEST_FINISHED; }
	int GetState() const { return m_State; }

	// steps
	void InitSteps();
	void LoadSteps();
	void SaveSteps();
	void ClearSteps();
	std::map < int, CPlayerStepQuestBot > m_StepsQuestBot;

	// main
	void CheckaAvailableNewStep();
	bool Accept();
private:
	void Finish();
};

#endif