/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#include <engine/shared/config.h>
#include <teeother/system/string.h>

#include <game/server/gamecontext.h>
#include "DataQuests.h"

std::string CDataQuest::GetJsonFileName(int AuthID) const { return "server_data/quest_tmp/" + std::to_string(m_QuestID) + "-" + std::to_string(AuthID) + ".json"; }

int CDataQuest::GetQuestStoryPosition() const
{
	// get position of quests storyline
	return (int)std::count_if(QuestJob::ms_aDataQuests.begin(), QuestJob::ms_aDataQuests.end(), [this](std::pair< const int, CDataQuest>& pItem)
	{	return str_comp(pItem.second.m_aStoryLine, m_aStoryLine) == 0 && m_QuestID >= pItem.first; });
}

int CDataQuest::GetQuestStorySize() const
{
	// get size of quests storyline
	return (int)std::count_if(QuestJob::ms_aDataQuests.begin(), QuestJob::ms_aDataQuests.end(), [this](std::pair< const int, CDataQuest>& pItem)
		{	return str_comp(pItem.second.m_aStoryLine, m_aStoryLine) == 0; });
}