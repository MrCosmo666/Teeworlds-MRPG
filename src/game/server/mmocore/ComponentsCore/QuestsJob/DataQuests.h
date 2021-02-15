/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#ifndef GAME_SERVER_DATA_QUESTS_H
#define GAME_SERVER_DATA_QUESTS_H

#include "StepQuestBot.h"

class CDataQuest
{
public:
	char m_aName[24];
	char m_aStoryLine[24];
	int m_QuestID;
	int m_Gold;
	int m_Exp;

	std::string GetJsonName(int AuthID) const;
	const char* GetName() const { return m_aName; }
	const char* GetStory() const { return m_aStoryLine; }
	int GetQuestStoryPosition() const;
	int GetQuestStorySize() const;

	std::map<int, CPlayerStepQuestBot> CopyBasicSteps()
	{
		std::map<int, CPlayerStepQuestBot> m_Copy;
		for(auto& pStepBot : m_StepsQuestBot)
			m_Copy[pStepBot.first] = (CPlayerStepQuestBot&)pStepBot.second;

		return m_Copy;
	}

	// steps with array bot data on active step
	std::map < int , CStepQuestBot > m_StepsQuestBot;
};

#endif