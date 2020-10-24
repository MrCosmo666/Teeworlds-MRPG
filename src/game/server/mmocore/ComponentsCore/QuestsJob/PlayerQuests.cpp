/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#include <engine/shared/config.h>
#include <teeother/system/string.h>

#include <game/server/gamecontext.h>
#include "PlayerQuests.h"

CDataQuest& CPlayerQuest::Info() const { return QuestJob::ms_aDataQuests[m_QuestID]; }

void CPlayerQuest::CheckaAvailableNewStep()
{
	// check complected bot steps
	for(auto& pStepBot : m_StepsQuestBot)
	{
		if(pStepBot.second.m_Bot->m_Step == m_Step && !pStepBot.second.m_StepComplete)
			return;
	}

	m_Step++;

	// check finished all steps bots
	CGS* pGS = m_pPlayer->GS();
	int StepsBotComplected = 0;
	for(auto& pStepBot : m_StepsQuestBot)
	{
		if(pStepBot.second.m_StepComplete)
			StepsBotComplected++;

		pStepBot.second.UpdateBot(pGS);
		pStepBot.second.CreateStepArrow(m_pPlayer);
	}

	// check new step or finish quest
	const bool IsFinalStep = (StepsBotComplected >= (int)QuestJob::ms_aDataQuests[m_QuestID].m_StepsQuestBot.size());
	if(IsFinalStep)
	{
		Finish();
		const int ClientID = m_pPlayer->GetCID();
		pGS->StrongUpdateVotes(ClientID, MenuList::MENU_JOURNAL_MAIN);
		pGS->StrongUpdateVotes(ClientID, MenuList::MAIN_MENU);
	}
	else
	{
		SJK.UD("tw_accounts_quests", "Step = '%d' WHERE QuestID = '%d' AND OwnerID = '%d'", m_Step, m_QuestID, m_pPlayer->Acc().m_AuthID);
	}
}

bool CPlayerQuest::Accept()
{
	if(m_State != QuestState::QUEST_NO_ACCEPT)
		return false;

	// init quest
	m_Step = 1;
	m_State = QuestState::QUEST_ACCEPT;
	m_StepsQuestBot = Info().CopySteps();

	// accept quest
	CGS* pGS = m_pPlayer->GS();
	for(auto& pStepBot : m_StepsQuestBot)
	{
		pStepBot.second.m_MobProgress[0] = 0;
		pStepBot.second.m_MobProgress[1] = 0;
		pStepBot.second.m_StepComplete = false;
		pStepBot.second.m_ClientQuitting = false;
		SJK.ID("tw_accounts_quests_bots_step", "(QuestID, SubBotID, OwnerID) VALUES ('%d', '%d', '%d')", 
			pStepBot.second.m_Bot->m_QuestID, pStepBot.second.m_Bot->m_SubBotID, m_pPlayer->Acc().m_AuthID);
		pStepBot.second.UpdateBot(pGS);
		pStepBot.second.CreateStepArrow(m_pPlayer);
	}
	SJK.ID("tw_accounts_quests", "(QuestID, OwnerID, Type) VALUES ('%d', '%d', '%d')", m_QuestID, m_pPlayer->Acc().m_AuthID, QuestState::QUEST_ACCEPT);

	// information
	const int ClientID = m_pPlayer->GetCID();
	const int StorySize = Info().GetStoryCount();
	const int StoryProgress = Info().GetStoryCount(m_QuestID + 1);
	pGS->Chat(ClientID, "Quest: {STR} - {STR} {INT}/{INT}!", Info().GetStory(), Info().GetName(), &StoryProgress, &StorySize);
	pGS->Chat(ClientID, "Receive a reward Gold {INT}, Experience {INT}", &Info().m_Gold, &Info().m_Exp);
	//pPlayer->GetCharacter()->CreateQuestsStep(QuestID);
	pGS->CreatePlayerSound(ClientID, SOUND_CTF_CAPTURE);
	return true;
}

void CPlayerQuest::Finish()
{
	CGS* pGS = m_pPlayer->GS();
	const int ClientID = m_pPlayer->GetCID();
	if(m_State != QUEST_ACCEPT)
		return;

	// set quest state to finished
	m_State = QuestState::QUEST_FINISHED;
	SJK.UD("tw_accounts_quests", "Type = '%d' WHERE QuestID = '%d' AND OwnerID = '%d'", m_State, m_QuestID, m_pPlayer->Acc().m_AuthID);

	// clear tempdata
	m_StepsQuestBot.clear();
	SJK.DD("tw_accounts_quests_bots_step", "WHERE OwnerID = '%d' AND QuestID = '%d'", m_pPlayer->Acc().m_AuthID, m_QuestID);

	// awards and write about completion
	m_pPlayer->AddMoney(Info().m_Gold);
	m_pPlayer->AddExp(Info().m_Exp);
	pGS->Chat(-1, "{STR} completed: {STR} - {STR}!", pGS->Server()->ClientName(ClientID), Info().m_aStoryLine, Info().m_aName);
	pGS->ChatDiscord(DC_PLAYER_INFO, pGS->Server()->ClientName(ClientID), "Completed ({STR} - {STR})", Info().m_aStoryLine, Info().m_aName);

	// check whether the after quest has opened something new
	pGS->Mmo()->WorldSwap()->CheckQuestingOpened(m_pPlayer, m_QuestID);
	pGS->Mmo()->Dungeon()->CheckQuestingOpened(m_pPlayer, m_QuestID);

	// save player stats and accept next story quest
	pGS->Mmo()->SaveAccount(m_pPlayer, SaveType::SAVE_STATS);
	pGS->Mmo()->Quest()->AcceptNextStoryQuestStep(m_pPlayer, m_QuestID);
}
