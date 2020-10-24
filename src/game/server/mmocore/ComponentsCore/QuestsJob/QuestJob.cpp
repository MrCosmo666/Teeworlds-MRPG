/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#include <engine/shared/config.h>
#include <teeother/system/string.h>

#include <game/server/gamecontext.h>
#include "QuestJob.h"

/*
	Task list:
	- Resume arrow for quest npc
	- More clear structures quests
	- Resume task board quest npc
	- Clear data steps after finished quest (done not full)
*/

std::map < int, std::map <int, CPlayerQuest > > QuestJob::ms_aPlayerQuests;
std::map < int, CDataQuest > QuestJob::ms_aDataQuests;

static const char* GetStateName(int Type)
{
	switch(Type)
	{
		case QuestState::QUEST_ACCEPT: return "Active";
		case QuestState::QUEST_FINISHED: return "Finished";
		default: return "Not active";
	}
}

void QuestJob::ShowQuestsMainList(CPlayer* pPlayer)
{
	// show the quest sheet
	ShowQuestsTabList(pPlayer, QuestState::QUEST_ACCEPT);
	ShowQuestsTabList(pPlayer, QuestState::QUEST_NO_ACCEPT);

	// show the completed menu
	pPlayer->m_Colored = BLUE_COLOR;
	GS()->AVM(pPlayer->GetCID(), "MENU", MenuList::MENU_JOURNAL_FINISHED, NOPE, "List of completed quests");
}

void QuestJob::ShowQuestsTabList(CPlayer* pPlayer, int StateQuest)
{
	const int ClientID = pPlayer->GetCID();
	pPlayer->m_Colored = GOLDEN_COLOR;
	GS()->AVL(ClientID, "null", "★ {STR} quests", GetStateName(StateQuest));

	// check first quest story step
	bool IsEmptyList = true;
	pPlayer->m_Colored = LIGHT_GOLDEN_COLOR;
	std::list < std::string /*stories was checked*/ > StoriesChecked;
	for(const auto& pDataQuest : ms_aDataQuests)
	{
		if(pPlayer->GetQuest(pDataQuest.first).GetState() != StateQuest)
			continue;

		if(StateQuest == QuestState::QUEST_FINISHED)
		{
			ShowQuestID(pPlayer, pDataQuest.first);
			IsEmptyList = false;
			continue;
		}

		const auto& IsAlreadyChecked = std::find_if(StoriesChecked.begin(), StoriesChecked.end(), [=](const std::string& stories)
		{ return (str_comp_nocase(ms_aDataQuests[pDataQuest.first].m_aStoryLine, stories.c_str()) == 0); });
		if(IsAlreadyChecked == StoriesChecked.end())
		{
			StoriesChecked.emplace_back(ms_aDataQuests[pDataQuest.first].m_aStoryLine);
			ShowQuestID(pPlayer, pDataQuest.first);
			IsEmptyList = false;
		}
	}

	// if the quest list is empty
	if(IsEmptyList)
	{
		pPlayer->m_Colored = LIGHT_GOLDEN_COLOR;
		GS()->AV(ClientID, "null", "This list is empty");
	}
	GS()->AV(ClientID, "null");
}

void QuestJob::ShowQuestID(CPlayer *pPlayer, int QuestID)
{
	CDataQuest pData = pPlayer->GetQuest(QuestID).Info();
	const int ClientID = pPlayer->GetCID();
	const int CountQuest = pData.GetStoryCount();
	const int LineQuest = pData.GetStoryCount(QuestID) + 1;

	// TODO: REMOVE IT
	GS()->AVCALLBACK(ClientID, "MENU", "\0", QuestID, NOPE, NOPE, [](CVoteOptionsCallback Callback)
	{
		CPlayer* pPlayer = Callback.pPlayer;
		const int ClientID = pPlayer->GetCID();
		const int QuestID = Callback.VoteID;
		CDataQuest pData = pPlayer->GetQuest(QuestID).Info();

		pPlayer->GS()->ClearVotes(ClientID);
		pPlayer->GS()->Mmo()->Quest()->ShowQuestsActiveNPC(pPlayer, QuestID);
		pPlayer->GS()->AV(ClientID, "null");

		pPlayer->m_Colored = GOLDEN_COLOR;
		pPlayer->GS()->AVL(ClientID, "null", "{STR} : Reward", pData.GetName());
		pPlayer->m_Colored = LIGHT_GOLDEN_COLOR;
		pPlayer->GS()->AVL(ClientID, "null", "Gold: {INT} Exp: {INT}", &pData.m_Gold, &pData.m_Exp);

		pPlayer->m_LastVoteMenu = MenuList::MENU_JOURNAL_MAIN;
		pPlayer->GS()->AddBackpage(ClientID);
	}, "{INT}/{INT} {STR}: {STR}", &LineQuest, &CountQuest, pData.GetStory(), pData.GetName());
}

// active npc information display
void QuestJob::ShowQuestsActiveNPC(CPlayer* pPlayer, int QuestID)
{
	CPlayerQuest& pPlayerQuest = pPlayer->GetQuest(QuestID);
	const int ClientID = pPlayer->GetCID();
	pPlayer->m_Colored = BLUE_COLOR;
	GS()->AVM(ClientID, "null", NOPE, NOPE, "Active NPC for current quests");

	for(auto& pStepBot : ms_aDataQuests[QuestID].m_StepsQuestBot)
	{
		// header
		BotJob::QuestBotInfo* pBotInfo = pStepBot.second.m_Bot;
		const int HideID = (NUM_TAB_MENU + 12500 + pBotInfo->m_SubBotID);
		const int PosX = pBotInfo->m_PositionX / 32, PosY = pBotInfo->m_PositionY / 32;
		const char* pSymbol = (((pPlayerQuest.GetState() == QUEST_ACCEPT && pPlayerQuest.m_StepsQuestBot[pStepBot.first].m_StepComplete) || pPlayerQuest.GetState() == QuestState::QUEST_FINISHED) ? "✔ " : "\0");
		GS()->AVH(ClientID, HideID, LIGHT_BLUE_COLOR, "{STR}Step {INT}. {STR} {STR}(x{INT} y{INT})", pSymbol, &pBotInfo->m_Step, pBotInfo->GetName(), GS()->Server()->GetWorldName(pBotInfo->m_WorldID), &PosX, &PosY);

		// skipped non accepted task list
		if(pPlayerQuest.GetState() != QUEST_ACCEPT)
		{
			GS()->AVM(ClientID, "null", NOPE, HideID, "Quest been completed, or not accepted!");
			continue;
		}

		// need for bot
		bool NeedOnlyTalk = true;
		for(int i = 0; i < 2; i++)
		{
			const int NeedKillMobID = pBotInfo->m_aNeedMob[i];
			const int KillNeed = pBotInfo->m_aNeedMobCount[i];
			if(NeedKillMobID > 0 && KillNeed > 0 && Job()->BotsData()->IsDataBotValid(NeedKillMobID))
			{
				GS()->AVMI(ClientID, "broken_h", "null", NOPE, HideID, "- Defeat {STR} [{INT}/{INT}]",
					BotJob::ms_aDataBot[NeedKillMobID].m_aNameBot, &pPlayerQuest.m_StepsQuestBot[pStepBot.first].m_MobProgress[i], &KillNeed);
				NeedOnlyTalk = false;
			}

			const int NeedItemID = pBotInfo->m_aItemSearch[i];
			const int NeedCount = pBotInfo->m_aItemSearchCount[i];
			if(NeedItemID > 0 && NeedCount > 0)
			{
				InventoryItem PlayerItem = pPlayer->GetItem(NeedItemID);
				int ClapmItem = clamp(PlayerItem.m_Count, 0, NeedCount);
				GS()->AVMI(ClientID, PlayerItem.Info().GetIcon(), "null", NOPE, HideID, "- Item {STR} [{INT}/{INT}]", PlayerItem.Info().GetName(pPlayer), &ClapmItem, &NeedCount);
				NeedOnlyTalk = false;
			}
		}

		// reward from bot after can move to up
		for(int i = 0; i < 2; i++)
		{
			const int RewardItemID = pBotInfo->m_aItemGives[i];
			const int RewardCount = pBotInfo->m_aItemGivesCount[i];
			if(RewardItemID > 0 && RewardCount > 0)
			{
				ItemInformation RewardItem = GS()->GetItemInfo(RewardItemID);
				GS()->AVMI(ClientID, RewardItem.GetIcon(), "null", NOPE, HideID, "- Receive {STR}x{INT}", RewardItem.GetName(pPlayer), &RewardCount);
			}
		}

		if(NeedOnlyTalk)
			GS()->AVM(ClientID, "null", NOPE, HideID, "You just need to talk.");
	}
}

void QuestJob::QuestShowRequired(CPlayer* pPlayer, BotJob::QuestBotInfo& pBot, const char* TextTalk)
{
	const int QuestID = pBot.m_QuestID;
	CPlayerQuest& pPlayerQuest = pPlayer->GetQuest(QuestID);
	if(pPlayerQuest.m_StepsQuestBot.find(pBot.m_SubBotID) != pPlayerQuest.m_StepsQuestBot.end())
		pPlayerQuest.m_StepsQuestBot[pBot.m_SubBotID].ShowRequired(pPlayer, TextTalk);
}

void QuestJob::QuestTableAddInfo(int ClientID, const char* pText, int Requires, int Have)
{
	if(ClientID < 0 || ClientID >= MAX_PLAYERS || !GS()->IsMmoClient(ClientID))
		return;

	CNetMsg_Sv_AddQuestingProcessing Msg;
	Msg.m_pText = pText;
	Msg.m_pRequiresNum = Requires;
	Msg.m_pHaveNum = clamp(Have, 0, Requires);
	Msg.m_pGivingTable = false;
	StrToInts(Msg.m_pIcon, 4, "hammer");
	GS()->Server()->SendPackMsg(&Msg, MSGFLAG_VITAL, ClientID);
}

void QuestJob::QuestTableClear(int ClientID)
{
	if(ClientID < 0 || ClientID >= MAX_PLAYERS || !GS()->IsMmoClient(ClientID))
		return;

	CNetMsg_Sv_ClearQuestingProcessing Msg;
	GS()->Server()->SendPackMsg(&Msg, MSGFLAG_VITAL, ClientID);
}

bool QuestJob::InteractiveQuestNPC(CPlayer* pPlayer, BotJob::QuestBotInfo& pBot, bool LastDialog)
{
	const int QuestID = pBot.m_QuestID;
	CPlayerQuest& pPlayerQuest = pPlayer->GetQuest(QuestID);
	if(pPlayerQuest.m_StepsQuestBot.find(pBot.m_SubBotID) != pPlayerQuest.m_StepsQuestBot.end())
		return pPlayerQuest.m_StepsQuestBot[pBot.m_SubBotID].Finish(pPlayer, LastDialog);
	return false;
}

void QuestJob::DoStepDropTakeItems(CPlayer* pPlayer, BotJob::QuestBotInfo& pBot)
{
	const int QuestID = pBot.m_QuestID;
	CPlayerQuest& pPlayerQuest = pPlayer->GetQuest(QuestID);
	if(pPlayerQuest.m_StepsQuestBot.find(pBot.m_SubBotID) != pPlayerQuest.m_StepsQuestBot.end())
		pPlayerQuest.m_StepsQuestBot[pBot.m_SubBotID].CreateStepDropTakeItems(pPlayer);
}

void QuestJob::AddMobProgressQuests(CPlayer* pPlayer, int BotID)
{
	// TODO Optimize algoritm check complected steps
	const int ClientID = pPlayer->GetCID();
	for(auto& pPlayerQuest : QuestJob::ms_aPlayerQuests[ClientID])
	{
		if(pPlayerQuest.second.m_State != QuestState::QUEST_ACCEPT)
			continue;

		for(auto& pStepBot : pPlayerQuest.second.m_StepsQuestBot)
			pStepBot.second.AddMobProgress(pPlayer, BotID);
	}
}

void QuestJob::UpdateArrowStep(CPlayer *pPlayer)
{
	// TODO Optimize algoritm check complected steps
	const int ClientID = pPlayer->GetCID();
	for (auto& pPlayerQuest : ms_aPlayerQuests[ClientID])
	{
		if(pPlayerQuest.second.m_State != QuestState::QUEST_ACCEPT)
			continue;

		for(auto& pStepBot : pPlayerQuest.second.m_StepsQuestBot)
			pStepBot.second.CreateStepArrow(pPlayer);
	}
}

void QuestJob::AcceptNextStoryQuestStep(CPlayer *pPlayer, int CheckQuestID)
{
	const CDataQuest CheckingQuest = ms_aDataQuests[CheckQuestID];
	for (auto pQuestData = ms_aDataQuests.find(CheckQuestID); pQuestData != ms_aDataQuests.end(); pQuestData++)
	{
		// search next quest story step
		if(str_comp_nocase(CheckingQuest.m_aStoryLine, pQuestData->second.m_aStoryLine) == 0)
		{
			// skip all if a quest story is found that is still active
			if(pPlayer->GetQuest(pQuestData->first).GetState() == QUEST_ACCEPT)
				break;

			// accept next quest step
			if(!IsValidQuest(pQuestData->first, pPlayer->GetCID()) || pPlayer->GetQuest(pQuestData->first).Accept())
				break;
		}
	}
}

void QuestJob::AcceptNextStoryQuestStep(CPlayer* pPlayer)
{
	// check first quest story step search active quests
	std::list < std::string /*stories was checked*/ > StoriesChecked;
	for(const auto& pPlayerQuest : ms_aPlayerQuests[pPlayer->GetCID()])
	{
		// allow accept next story quest only for complected some quest on story
		if(pPlayerQuest.second.GetState() != QuestState::QUEST_FINISHED)
			continue;

		// accept next story quest
		const auto& IsAlreadyChecked = std::find_if(StoriesChecked.begin(), StoriesChecked.end(), [=](const std::string &stories)
		{ return (str_comp_nocase(ms_aDataQuests[pPlayerQuest.first].m_aStoryLine, stories.c_str()) == 0); });
		if(IsAlreadyChecked == StoriesChecked.end())
		{
			StoriesChecked.emplace_back(ms_aDataQuests[pPlayerQuest.first].m_aStoryLine);
			AcceptNextStoryQuestStep(pPlayer, pPlayerQuest.first);
		}
	}
}
void QuestJob::QuestTableAddItem(int ClientID, const char* pText, int Requires, int ItemID, bool GivingTable)
{
	CPlayer* pPlayer = GS()->GetPlayer(ClientID, true);
	if (!pPlayer || ItemID < itGold || !GS()->IsMmoClient(ClientID))
		return;

	const InventoryItem PlayerSelectedItem = pPlayer->GetItem(ItemID);

	CNetMsg_Sv_AddQuestingProcessing Msg;
	Msg.m_pText = pText;
	Msg.m_pRequiresNum = Requires;
	Msg.m_pHaveNum = clamp(PlayerSelectedItem.m_Count, 0, Requires);
	Msg.m_pGivingTable = GivingTable;
	StrToInts(Msg.m_pIcon, 4, PlayerSelectedItem.Info().GetIcon());
	GS()->Server()->SendPackMsg(&Msg, MSGFLAG_VITAL, ClientID);
}

int QuestJob::GetUnfrozenItemCount(CPlayer *pPlayer, int ItemID)
{
	const int ClientID = pPlayer->GetCID();
	int AvailableCount = pPlayer->GetItem(ItemID).m_Count;
	for (const auto& pPlayerQuest : ms_aPlayerQuests[ClientID])
	{
		if(pPlayerQuest.second.m_State != QuestState::QUEST_ACCEPT)
			continue;

		for(auto& pStepBot : pPlayerQuest.second.m_StepsQuestBot)
		{
			if(!pStepBot.second.m_StepComplete)
				AvailableCount -= pStepBot.second.GetCountBlockedItem(pPlayer, ItemID);
		}
	}
	return max(AvailableCount, 0);
}

void QuestJob::OnInit()
{
	std::shared_ptr<ResultSet> RES(SJK.SD("*", "tw_quests_list"));
	while (RES->next())
	{
		const int QUID = RES->getInt("ID");
		str_copy(ms_aDataQuests[QUID].m_aName, RES->getString("Name").c_str(), sizeof(ms_aDataQuests[QUID].m_aName));
		str_copy(ms_aDataQuests[QUID].m_aStoryLine, RES->getString("StoryLine").c_str(), sizeof(ms_aDataQuests[QUID].m_aStoryLine));
		ms_aDataQuests[QUID].m_Gold = (int)RES->getInt("Money");
		ms_aDataQuests[QUID].m_Exp = (int)RES->getInt("Exp");
		// init steps bots run on BotJob::Init
	}
}

void QuestJob::OnInitAccount(CPlayer* pPlayer)
{
	const int ClientID = pPlayer->GetCID();
	std::shared_ptr<ResultSet> RES(SJK.SD("*", "tw_accounts_quests", "WHERE OwnerID = '%d'", pPlayer->Acc().m_AuthID));
	while (RES->next())
	{
		const int QuestID = RES->getInt("QuestID");
		ms_aPlayerQuests[ClientID][QuestID].m_State = (int)RES->getInt("Type");
		ms_aPlayerQuests[ClientID][QuestID].m_Step = (int)RES->getInt("Step");
		ms_aPlayerQuests[ClientID][QuestID].m_StepsQuestBot = ms_aDataQuests[QuestID].CopySteps();
	}

	// init data steps players
	std::shared_ptr<ResultSet> PlayerData(SJK.SD("*", "tw_accounts_quests_bots_step", "WHERE OwnerID = '%d' ", pPlayer->Acc().m_AuthID));
	while(PlayerData->next())
	{
		const int SubBotID = PlayerData->getInt("SubBotID"); // is a unique value
		const int QuestID = BotJob::ms_aQuestBot[SubBotID].m_QuestID;
		ms_aPlayerQuests[ClientID][QuestID].m_StepsQuestBot[SubBotID].m_Bot = &BotJob::ms_aQuestBot[SubBotID];
		ms_aPlayerQuests[ClientID][QuestID].m_StepsQuestBot[SubBotID].m_MobProgress[0] = PlayerData->getInt("Mob1Progress");
		ms_aPlayerQuests[ClientID][QuestID].m_StepsQuestBot[SubBotID].m_MobProgress[1] = PlayerData->getInt("Mob1Progress");
		ms_aPlayerQuests[ClientID][QuestID].m_StepsQuestBot[SubBotID].m_StepComplete = PlayerData->getBoolean("Completed");
		ms_aPlayerQuests[ClientID][QuestID].m_StepsQuestBot[SubBotID].m_ClientQuitting = false;
		ms_aPlayerQuests[ClientID][QuestID].m_StepsQuestBot[SubBotID].UpdateBot(GS());
	}
}

void QuestJob::OnResetClient(int ClientID)
{
	for(auto& qp : ms_aPlayerQuests[ClientID])
	{
		for(auto& pStepBot : qp.second.m_StepsQuestBot)
		{
			pStepBot.second.m_ClientQuitting = true;
			pStepBot.second.UpdateBot(GS());
		}
	}
	ms_aPlayerQuests.erase(ClientID);
}

void QuestJob::OnMessage(int MsgID, void* pRawMsg, int ClientID)
{
	CPlayer* pPlayer = GS()->m_apPlayers[ClientID];
	if (MsgID == NETMSGTYPE_CL_TALKINTERACTIVE)
	{
		if (pPlayer->m_aPlayerTick[TickState::LastDialog] && pPlayer->m_aPlayerTick[TickState::LastDialog] > GS()->Server()->Tick())
			return;

		pPlayer->m_aPlayerTick[TickState::LastDialog] = GS()->Server()->Tick() + (GS()->Server()->TickSpeed() / 4);
		pPlayer->SetTalking(pPlayer->GetTalkedID(), true);
	}
}

bool QuestJob::OnHandleMenulist(CPlayer* pPlayer, int Menulist, bool ReplaceMenu)
{
	const int ClientID = pPlayer->GetCID();
	if (ReplaceMenu)
	{
		CCharacter* pChr = pPlayer->GetCharacter();
		if (!pChr || !pChr->IsAlive())
			return false;

		return false;
	}

	if (Menulist == MenuList::MENU_JOURNAL_FINISHED)
	{
		pPlayer->m_LastVoteMenu = MenuList::MENU_JOURNAL_MAIN;
		ShowQuestsTabList(pPlayer, QuestState::QUEST_FINISHED);
		GS()->AddBackpage(ClientID);
		return true;
	}

	return false;
}

bool QuestJob::OnParsingVoteCommands(CPlayer* pPlayer, const char* CMD, const int VoteID, const int VoteID2, int Get, const char* GetText)
{
	return false;
}
