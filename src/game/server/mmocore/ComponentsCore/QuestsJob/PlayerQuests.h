/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#ifndef GAME_SERVER_PLAYER_QUESTS_H
#define GAME_SERVER_PLAYER_QUESTS_H

#include "DataQuests.h"
#include "StepQuestBot.h"

class CPlayerQuest
{
	void Finish();

public:
	int m_QuestID;
	CPlayer* m_pPlayer;

	int m_State;
	int m_Step;

	// steps with array bot data on active step
	std::map < int, CPlayerStepQuestBot > m_StepsQuestBot;

	void CheckaAvailableNewStep();
	bool Accept();

	CDataQuest& Info() const;
	bool IsComplected() const { return m_State == QuestState::QUEST_FINISHED; }
	int GetState() const { return m_State; }
};

#endif