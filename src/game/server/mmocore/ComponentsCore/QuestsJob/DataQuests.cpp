/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#include <engine/shared/config.h>
#include <teeother/system/string.h>

#include <game/server/gamecontext.h>
#include "DataQuests.h"

std::string CDataQuest::GetJsonName(int AuthID) const { return "server_data/quest_tmp/" + std::to_string(m_QuestID) + "-" + std::to_string(AuthID) + ".json"; }

int CDataQuest::GetStoryCount(int CountFromQuestID) const
{
	// get total number of quests storyline
	int Count = 0;
	for(const auto& qd : QuestJob::ms_aDataQuests)
	{
		if(str_comp(qd.second.m_aStoryLine, m_aStoryLine) == 0)
			Count++;
	}

	// get the number of quests storyline from the quest
	if(CountFromQuestID > 0)
	{
		for(auto qquest = QuestJob::ms_aDataQuests.find(CountFromQuestID); qquest != QuestJob::ms_aDataQuests.end(); qquest++)
		{
			if(str_comp(qquest->second.m_aStoryLine, m_aStoryLine) == 0)
				Count--;
		}
	}
	return Count;
}