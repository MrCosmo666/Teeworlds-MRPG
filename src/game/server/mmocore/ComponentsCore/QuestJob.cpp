/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#include <engine/shared/config.h>
#include <teeother/system/string.h>

#include <game/server/gamecontext.h>
#include "QuestJob.h"

#include <game/server/mmocore/GameEntities/Items/drop_quest_items.h>

std::map < int , QuestJob::StructQuestData > QuestJob::ms_aQuestsData;
std::map < int , std::map < int , QuestJob::StructQuest > > QuestJob::ms_aQuests;

static const char* GetStateName(int Type)
{
	switch(Type)
	{
		case QuestState::QUEST_ACCEPT: return "Active";
		case QuestState::QUEST_FINISHED: return "Finished";
		default: return "Not active";
	}
}

int QuestJob::GetState(int ClientID, int QuestID) const
{
	if(IsValidQuest(QuestID, ClientID))
		return ms_aQuests[ClientID][QuestID].m_State;
	return QuestState::QUEST_NO_ACCEPT;
}

bool QuestJob::IsCompletedQuest(int ClientID, int QuestID) const
{
	return (bool)(GetState(ClientID, QuestID) == QuestState::QUEST_FINISHED);
}

int QuestJob::GetStoryCount(const char *StoryName, int CountFromQuestID) const
{
	int Count = 0;
	for(const auto& qd : ms_aQuestsData)
	{
		if(str_comp(qd.second.m_aStoryLine, StoryName) == 0)
			Count++;
	}
	
	if(CountFromQuestID > 0)
	{
		for (auto qquest = ms_aQuestsData.find(CountFromQuestID); qquest != ms_aQuestsData.end(); qquest++)
		{
			if(str_comp(qquest->second.m_aStoryLine, StoryName) == 0)
				Count--;
		}
	}
	return Count;
}

bool QuestJob::IsDefeatMobsComplete(int ClientID, int QuestID) const
{
	if(!IsValidQuest(QuestID, ClientID)) 
		return false;

	const int playerProgress = ms_aQuests[ClientID][QuestID].m_Progress;
	BotJob::QuestBotInfo* pFindBot = GetQuestBot(QuestID, playerProgress);
	if (!pFindBot)
		return false;

	for (int i = 0; i < 2; i++)
	{
		const int MobID = pFindBot->m_aNeedMob[i];
		const int Count = pFindBot->m_aNeedMobCount[i];
		if(MobID <= 0 || Count <= 0) 
			continue;

		if(ms_aQuests[ClientID][QuestID].m_aMobProgress[i] < Count)
			return false;
	}
	return true;
}

const char *QuestJob::GetQuestName(int QuestID) const
{
	if(IsValidQuest(QuestID))
		return ms_aQuestsData[QuestID].m_aName;
	return "Unknown";
}

const char *QuestJob::GetStoryName(int QuestID) const
{
	if(IsValidQuest(QuestID))
		return ms_aQuestsData[QuestID].m_aStoryLine;
	return "Unknown";
}

BotJob::QuestBotInfo *QuestJob::GetQuestBot(int QuestID, int Progress) const
{
	for(auto& qb : BotJob::ms_aQuestBot)
	{
		// skip decorative quest npc
		if(qb.second.m_NextEqualProgress)
			continue;

		if(QuestID == qb.second.m_QuestID && Progress == qb.second.m_Progress)
			return &qb.second;
	}
	return nullptr;
}

bool QuestJob::IsActiveQuestBot(int QuestID, int Progress) const
{
	for(int i = 0; i < MAX_PLAYERS; i++)
	{
		const int PlayerState = GetState(i, QuestID);
		if(PlayerState != QuestState::QUEST_ACCEPT || ms_aQuests[i][QuestID].m_Progress != Progress)
			continue;
		return true;
	}
	return false;
}

void QuestJob::ShowQuestID(CPlayer *pPlayer, int QuestID)
{
	const int ClientID = pPlayer->GetCID();
	const int HideID = NUM_TAB_MENU + 10500 + QuestID;
	const int CountQuest = GetStoryCount(ms_aQuestsData[QuestID].m_aStoryLine);
	const int LineQuest = GetStoryCount(ms_aQuestsData[QuestID].m_aStoryLine, QuestID)+1;

	GS()->AVH(ClientID, HideID, LIGHT_GOLDEN_COLOR, "{INT}/{INT} {STR}: {STR}", &LineQuest, &CountQuest, ms_aQuestsData[QuestID].m_aStoryLine, ms_aQuestsData[QuestID].m_aName);
	GS()->AVM(ClientID, "null", NOPE, HideID, "You will receive a reward");
	GS()->AVM(ClientID, "null", NOPE, HideID, "Gold: {INT} Exp: {INT}", &ms_aQuestsData[QuestID].m_Gold, &ms_aQuestsData[QuestID].m_Exp);
	GS()->AVM(ClientID, "null", NOPE, HideID, " ");
}

void QuestJob::FinishQuest(CPlayer *pPlayer, int QuestID)
{
	const int ClientID = pPlayer->GetCID();
	if(GetState(ClientID, QuestID) != QUEST_ACCEPT)
		return;
	
	// quest stats
	ms_aQuests[ClientID][QuestID].m_State = QuestState::QUEST_FINISHED;
	SJK.UD("tw_accounts_quests", "Type = '%d' WHERE QuestID = '%d' AND OwnerID = '%d'", ms_aQuests[ClientID][QuestID].m_State, QuestID, pPlayer->Acc().m_AuthID);

	// issue awards and write about completion
	const StructQuestData finishQuestData = ms_aQuestsData[QuestID];
	pPlayer->AddMoney(finishQuestData.m_Gold);
	pPlayer->AddExp(finishQuestData.m_Exp);

	GS()->Chat(-1, "{STR} completed: {STR} - {STR}!", GS()->Server()->ClientName(ClientID), finishQuestData.m_aStoryLine, finishQuestData.m_aName);
	GS()->ChatDiscord(DC_PLAYER_INFO, GS()->Server()->ClientName(ClientID), "Completed ({STR} - {STR})", finishQuestData.m_aStoryLine, finishQuestData.m_aName);
	Job()->SaveAccount(pPlayer, SaveType::SAVE_STATS);
	CheckNewStories(pPlayer, QuestID);

	Job()->WorldSwap()->CheckQuestingOpened(pPlayer, QuestID);
	Job()->Dungeon()->CheckQuestingOpened(pPlayer, QuestID);
}

void QuestJob::CollectItem(CPlayer* pPlayer, BotJob::QuestBotInfo& BotData)
{
	// anti stressing with double thread sql result what work one (item)
	bool antiStressing = false;
	for (int i = 0; i < 2; i++)
	{
		const int ItemID = BotData.m_aItemSearch[i];
		const int Count = BotData.m_aItemSearchCount[i];
		if (ItemID > 0 && Count > 0)
		{
			GS()->Chat(pPlayer->GetCID(), "You used quest item {STR}x{INT}!", pPlayer->GetItem(ItemID).Info().GetName(pPlayer), &Count);
			antiStressing = (bool)(ItemID == BotData.m_aItemGives[0] || ItemID == BotData.m_aItemGives[1]);
			pPlayer->GetItem(ItemID).Remove(Count);
		}
	}

	for(int i = 0; i < 2; i++)
	{
		const int ItemID = BotData.m_aItemGives[i];
		const int Count = BotData.m_aItemGivesCount[i];
		if(ItemID > 0 && Count > 0)
		{
			if(antiStressing)
			{
				Job()->Item()->AddItemSleep(pPlayer->Acc().m_AuthID, ItemID, Count, 300);
				continue;
			}

			if(pPlayer->GetItem(ItemID).Info().IsEnchantable() && pPlayer->GetItem(ItemID).m_Count >= 1)
			{
				GS()->SendInbox(pPlayer->GetCID(), "No place for item", "You already have this item, but we can't put it in inventory", ItemID, 1);
				continue;
			}

			pPlayer->GetItem(ItemID).Add(Count);
		}
	}	
}

bool QuestJob::IsCollectItemComplete(CPlayer *pPlayer, BotJob::QuestBotInfo &BotData) const
{
	for(int i = 0; i < 2; i++)
	{
		const int ItemID = BotData.m_aItemSearch[i];
		const int Count = BotData.m_aItemSearchCount[i];
		if(ItemID <= 0 || Count <= 0)
			continue;

		if(pPlayer->GetItem(ItemID).m_Count < Count)
			return false;
	}

	return true;
}

void QuestJob::AddProgress(CPlayer *pPlayer, int QuestID)
{
	const int ClientID = pPlayer->GetCID();
	if(!IsValidQuest(QuestID, ClientID) || !pPlayer->GetCharacter()) 
		return;

	StructQuest &talkQuestPlayer = ms_aQuests[ClientID][QuestID];
	talkQuestPlayer.m_Progress++;
	talkQuestPlayer.m_aMobProgress[0] = 0;
	talkQuestPlayer.m_aMobProgress[1] = 0;
	const bool FinishedProgress = (talkQuestPlayer.m_Progress >= ms_aQuestsData[QuestID].m_ProgressSize);
	if (FinishedProgress)
	{
		FinishQuest(pPlayer, QuestID);
		GS()->UpdateVotes(ClientID, MenuList::MENU_JOURNAL_MAIN);
		GS()->UpdateVotes(ClientID, MenuList::MAIN_MENU);
	}
	else
	{
		pPlayer->GetCharacter()->CreateQuestsStep(QuestID);
		SJK.UD("tw_accounts_quests", "Progress = '%d', Mob1Progress = '%d', Mob2Progress = '%d' WHERE QuestID = '%d' AND OwnerID = '%d'",
			talkQuestPlayer.m_Progress, talkQuestPlayer.m_aMobProgress[0], talkQuestPlayer.m_aMobProgress[1], QuestID, pPlayer->Acc().m_AuthID);
	}

	const int OldProgress = talkQuestPlayer.m_Progress - 1;
	const int NewProgress = talkQuestPlayer.m_Progress;
	GS()->UpdateQuestsBot(QuestID, OldProgress);
	GS()->UpdateQuestsBot(QuestID, NewProgress);
}

bool QuestJob::AcceptQuest(int QuestID, CPlayer* pPlayer)
{
	if (!pPlayer || GetState(pPlayer->GetCID(), QuestID) != QuestState::QUEST_NO_ACCEPT)
		return false;

	const int ClientID = pPlayer->GetCID();
	ms_aQuests[ClientID][QuestID].m_Progress = 1;
	ms_aQuests[ClientID][QuestID].m_State = QuestState::QUEST_ACCEPT;
	GS()->UpdateQuestsBot(QuestID, ms_aQuests[ClientID][QuestID].m_Progress);
	SJK.ID("tw_accounts_quests", "(QuestID, OwnerID, Type) VALUES ('%d', '%d', '%d')", QuestID, pPlayer->Acc().m_AuthID, QuestState::QUEST_ACCEPT);

	const int StorySize = GetStoryCount(ms_aQuestsData[QuestID].m_aStoryLine);
	const int StoryProgress = GetStoryCount(ms_aQuestsData[QuestID].m_aStoryLine, QuestID + 1);
	GS()->Chat(ClientID, "Quest: {STR} - {STR} {INT}/{INT}!", ms_aQuestsData[QuestID].m_aStoryLine, ms_aQuestsData[QuestID].m_aName, &StoryProgress, &StorySize);
	GS()->Chat(ClientID, "Receive a reward Gold {INT}, Experience {INT}", &ms_aQuestsData[QuestID].m_Gold, &ms_aQuestsData[QuestID].m_Exp);
	pPlayer->GetCharacter()->CreateQuestsStep(QuestID);
	GS()->CreatePlayerSound(ClientID, SOUND_CTF_CAPTURE);
	return true;
}

bool QuestJob::InteractiveQuestNPC(CPlayer* pPlayer, BotJob::QuestBotInfo& BotData, bool LastDialog)
{
	if (!pPlayer || !pPlayer->GetCharacter())
		return false;

	if(BotData.m_NextEqualProgress)
		return true;

	const int ClientID = pPlayer->GetCID();
	const int QuestID = BotData.m_QuestID;
	if (!IsCollectItemComplete(pPlayer, BotData) || !IsDefeatMobsComplete(ClientID, QuestID))
	{
		GS()->Chat(ClientID, "Task has not been completed yet!");
		return false;
	}

	if (!LastDialog)
	{
		GS()->CreatePlayerSound(ClientID, SOUND_CTF_CAPTURE);
		return true;
	}

	if (!InteractiveTypeQuest(pPlayer, BotData))
		return false;

	CollectItem(pPlayer, BotData);
	GS()->UpdateVotes(ClientID, MenuList::MENU_JOURNAL_MAIN);
	GS()->Mmo()->Quest()->AddProgress(pPlayer, QuestID);

	BotJob::ms_aDataBot[BotData.m_BotID].m_aAlreadySnapQuestBot[ClientID] = false;
	return true;
}

bool QuestJob::InteractiveTypeQuest(CPlayer* pPlayer, BotJob::QuestBotInfo& BotData)
{
	const int ClientID = pPlayer->GetCID();
	if (BotData.m_InteractiveType == (int)QuestInteractive::INTERACTIVE_RANDOM_ACCEPT_ITEM && BotData.m_InteractiveTemp > 0)
	{
		const bool Succesful = random_int() % BotData.m_InteractiveTemp == 0;
		for(int i = 0; i < 2; i++)
		{
			const int ItemID = BotData.m_aItemSearch[i];
			const int Count = BotData.m_aItemSearchCount[i];
			if(ItemID > 0 && Count > 0)
				pPlayer->GetItem(ItemID).Remove(Count);
		}

		if(!Succesful)
			GS()->Chat(ClientID, "{STR} don't like the item.", BotData.GetName());

		return Succesful;
	}

	return true;
}

void QuestJob::AddMobProgress(CPlayer* pPlayer, int BotID)
{
	if (!pPlayer || !Job()->BotsData()->IsDataBotValid(BotID))
		return;

	const int ClientID = pPlayer->GetCID();
	for (auto& qp : ms_aQuests[ClientID])
	{
		// if the quest is accepted
		if (qp.second.m_State != QuestState::QUEST_ACCEPT)
			continue;

		// get active npc
		const int questID = qp.first;
		const int playerProgress = qp.second.m_Progress;
		BotJob::QuestBotInfo *FindBot = GetQuestBot(questID, playerProgress);
		if (!FindBot)
			continue;

		// looking for a Mob
		for (int i = 0; i < 2; i++)
		{
			if (BotID != FindBot->m_aNeedMob[i] || qp.second.m_aMobProgress[i] >= FindBot->m_aNeedMobCount[i])
				continue;

			qp.second.m_aMobProgress[i]++;
			if(qp.second.m_aMobProgress[i] >= FindBot->m_aNeedMobCount[i])
			{
				GS()->Chat(ClientID, "You killed {STR} required amount for NPC {STR}", BotJob::ms_aDataBot[BotID].m_aNameBot, FindBot->GetName());
			}
			SJK.UD("tw_accounts_quests", "Mob1Progress = '%d', Mob2Progress = '%d' WHERE QuestID = '%d' AND OwnerID = '%d'",
				qp.second.m_aMobProgress[0], qp.second.m_aMobProgress[1], questID, pPlayer->Acc().m_AuthID);
			break;
		}
	}
}

void QuestJob::UpdateArrowStep(int ClientID)
{
	CPlayer* pPlayer = GS()->GetPlayer(ClientID, true, true);
	if (!pPlayer)
		return;

	for (const auto& qp : ms_aQuests[ClientID])
	{
		if (qp.second.m_State == QuestState::QUEST_ACCEPT)
			pPlayer->GetCharacter()->CreateQuestsStep(qp.first);
	}
}

bool QuestJob::CheckNewStories(CPlayer *pPlayer, int CheckQuestID)
{
	const int ClientID = pPlayer->GetCID();
	if (CheckQuestID > 0)
	{
		const int NextQuestID = CheckQuestID + 1;
		const bool ActiveNextStories = (bool)(IsValidQuest(NextQuestID) && str_comp(ms_aQuestsData[CheckQuestID].m_aStoryLine, ms_aQuestsData[NextQuestID].m_aStoryLine) == 0);
		if(ActiveNextStories)
			AcceptQuest(NextQuestID, pPlayer);
		
		return ActiveNextStories;
	}

	bool ActiveNextStories = false;
	for (const auto& qp : ms_aQuests[ClientID])
	{
		if (qp.second.m_State != QuestState::QUEST_FINISHED)
			continue;

		int NextQuestID = qp.first + 1, QuestID = qp.first;
		if (!IsValidQuest(NextQuestID) || ms_aQuests[ClientID][NextQuestID].m_State != QuestState::QUEST_NO_ACCEPT)
			continue;

		if (str_comp(ms_aQuestsData[QuestID].m_aStoryLine, ms_aQuestsData[NextQuestID].m_aStoryLine) == 0)
		{
			AcceptQuest(NextQuestID, pPlayer);
			ActiveNextStories = true;
			continue;
		}
	}
	return ActiveNextStories;
}

void QuestJob::ShowQuestsTabList(CPlayer* pPlayer, int StateQuest)
{
	char aStoryLineSave[32];
	bool FoundQuests = false;
	const int ClientID = pPlayer->GetCID();

	pPlayer->m_Colored = GOLDEN_COLOR;
	GS()->AVL(ClientID, "null", "★ {STR} quests", GetStateName(StateQuest));

	for (const auto& qd : ms_aQuestsData)
	{
		int questID = qd.first;
		if (GetState(ClientID, questID) != StateQuest)
			continue;

		if (StateQuest != QuestState::QUEST_FINISHED)
		{
			if (str_comp(aStoryLineSave, qd.second.m_aStoryLine) == 0)
				continue;

			str_copy(aStoryLineSave, qd.second.m_aStoryLine, sizeof(aStoryLineSave));
		}
		ShowQuestID(pPlayer, questID);
		FoundQuests = true;
	}

	if (!FoundQuests)
	{
		pPlayer->m_Colored = LIGHT_GOLDEN_COLOR;
		GS()->AV(ClientID, "null", "This list is empty");
	}
	GS()->AV(ClientID, "null");
}

// post all quests the whole list
void QuestJob::ShowQuestsMainList(CPlayer* pPlayer)
{
	// show all active npc
	const int ClientID = pPlayer->GetCID();
	if (!ShowQuestsActiveNPC(pPlayer))
	{
		pPlayer->m_Colored = LIGHT_BLUE_COLOR;
		GS()->AVM(ClientID, "null", NOPE, NOPE, "In current quests there is no interaction with NPC");
	}
	GS()->AV(ClientID, "null");

	// show the questsheet
	ShowQuestsTabList(pPlayer, QuestState::QUEST_ACCEPT);
	ShowQuestsTabList(pPlayer, QuestState::QUEST_NO_ACCEPT);

	// show the completed menu
	pPlayer->m_Colored = BLUE_COLOR;
	GS()->AVM(ClientID, "MENU", MenuList::MENU_JOURNAL_FINISHED, NOPE, "List of completed quests");
}

// Adventure active npc information display
bool QuestJob::ShowQuestsActiveNPC(CPlayer* pPlayer)
{
	bool activeNPC = false;
	const int clientID = pPlayer->GetCID();
	pPlayer->m_Colored = BLUE_COLOR;
	GS()->AVM(clientID, "null", NOPE, NOPE, "Active NPC for current quests");

	for (const auto& qq : ms_aQuests[clientID])
	{
		if (qq.second.m_State != QuestState::QUEST_ACCEPT)
			continue;

		BotJob::QuestBotInfo *BotInfo = GetQuestBot(qq.first, qq.second.m_Progress);
		if (!BotInfo)
			continue;

		const int HideID = (NUM_TAB_MENU + 12500 + BotInfo->m_QuestID);
		const int PosX = BotInfo->m_PositionX / 32, PosY = BotInfo->m_PositionY / 32;
		GS()->AVH(clientID, HideID, LIGHT_BLUE_COLOR, "[{STR}] {STR} {STR}(x{INT} y{INT})", GetStoryName(qq.first), BotInfo->GetName(), GS()->Server()->GetWorldName(BotInfo->m_WorldID), &PosX, &PosY);

		bool JustTalk = true;
		for (int i = 0; i < 2; i++)
		{
			const int botID = BotInfo->m_aNeedMob[i];
			const int killNeed = BotInfo->m_aNeedMobCount[i];
			if(botID > 0 && killNeed > 0 && Job()->BotsData()->IsDataBotValid(botID))
			{
				GS()->AVMI(clientID, "broken_h", "null", NOPE, HideID, "- Defeat {STR} [{INT}/{INT}]", BotJob::ms_aDataBot[botID].m_aNameBot, &qq.second.m_aMobProgress[i], &killNeed);
				JustTalk = false;
			}

			const int itemID = BotInfo->m_aItemSearch[i];
			const int itemCount = BotInfo->m_aItemSearchCount[i];
			if(itemID > 0 && itemCount > 0)
			{
				InventoryItem PlayerSearchItem = pPlayer->GetItem(itemID);
				int ownCount = clamp(PlayerSearchItem.m_Count, 0, itemCount);

				GS()->AVMI(clientID, PlayerSearchItem.Info().GetIcon(), "null", NOPE, HideID, "- Item {STR} [{INT}/{INT}]", PlayerSearchItem.Info().GetName(pPlayer), &ownCount, &itemCount);
				JustTalk = false;
			}
		}
		for(int i = 0; i < 2; i++)
		{
			const int itemID = BotInfo->m_aItemGives[i];
			const int itemCount = BotInfo->m_aItemGivesCount[i];
			if(itemID > 0 && itemCount > 0)
			{
				ItemInformation GivedInfItem = GS()->GetItemInfo(itemID);
				GS()->AVMI(clientID, GivedInfItem.GetIcon(), "null", NOPE, HideID, "- Receive {STR}x{INT}", GivedInfItem.GetName(pPlayer), &itemCount);
				JustTalk = false;
			}
		}

		if (JustTalk)
			GS()->AVM(clientID, "null", NOPE, HideID, "You just need to talk.");

		activeNPC = true;
	}
	return activeNPC;
}

void QuestJob::QuestTableShowRequired(CPlayer *pPlayer, BotJob::QuestBotInfo &BotData, const char* TextTalk)
{
	if(BotData.m_NextEqualProgress)
		return;

	const int ClientID = pPlayer->GetCID();
	if (GS()->IsMmoClient(ClientID))
	{
		QuestTableShowRequired(pPlayer, BotData);
		return;
	}

	char aBuf[64];
	dynamic_string Buffer;
	bool IsActiveTask = false;
	const int QuestID = BotData.m_QuestID;

	// search item's and mob's
	for(int i = 0; i < 2; i++)
	{
		const int BotID = BotData.m_aNeedMob[i];
		const int CountMob = BotData.m_aNeedMobCount[i];
		if(BotID > 0 && CountMob > 0 && Job()->BotsData()->IsDataBotValid(BotID))
		{
			str_format(aBuf, sizeof(aBuf), "\n- Defeat %s [%d/%d]", BotJob::ms_aDataBot[BotID].m_aNameBot, ms_aQuests[ClientID][QuestID].m_aMobProgress[i], CountMob);
			Buffer.append_at(Buffer.length(), aBuf);
			IsActiveTask = true;
		}

		const int ItemID = BotData.m_aItemSearch[i];
		const int CountItem = BotData.m_aItemSearchCount[i];
		if(ItemID > 0 && CountItem > 0)
		{
			InventoryItem PlayerQuestItem = pPlayer->GetItem(ItemID);
			str_format(aBuf, sizeof(aBuf), "\n- Need %s [%d/%d]", PlayerQuestItem.Info().GetName(pPlayer), PlayerQuestItem.m_Count, CountItem);
			Buffer.append_at(Buffer.length(), aBuf);
			IsActiveTask = true;
		}
	}

	// type random accept item's
	if(BotData.m_InteractiveType == (int)QuestInteractive::INTERACTIVE_RANDOM_ACCEPT_ITEM)
	{
		const double Chance = BotData.m_InteractiveTemp <= 0 ? 100.0f : (1.0f / (double)BotData.m_InteractiveTemp) * 100;
		str_format(aBuf, sizeof(aBuf), "\nChance that item he'll like [%0.2f%%]\n", Chance);
		Buffer.append_at(Buffer.length(), aBuf);
	}

	// reward item's
	for(int i = 0; i < 2; i++)
	{
		const int ItemID = BotData.m_aItemGives[i];
		const int CountItem = BotData.m_aItemGivesCount[i];
		if(ItemID > 0 && CountItem > 0)
		{
			str_format(aBuf, sizeof(aBuf), "\n- Receive %s [%d]", GS()->GetItemInfo(ItemID).GetName(pPlayer), CountItem);
			Buffer.append_at(Buffer.length(), aBuf);
		}
	}

	GS()->Motd(ClientID, "{STR}\n\n{STR}{STR}\n\n", TextTalk, (IsActiveTask ? "### Task" : "\0"), Buffer.buffer());
	pPlayer->ClearFormatQuestText();
	Buffer.clear();
}

void QuestJob::QuestTableShowRequired(CPlayer* pPlayer, BotJob::QuestBotInfo& BotData)
{
	char aBuf[64];
	const int ClientID = pPlayer->GetCID();

	// search item's
	for (int i = 0; i < 2; i++)
	{
		const int ItemID = BotData.m_aItemSearch[i];
		const int CountItem = BotData.m_aItemSearchCount[i];
		if(ItemID <= 0 || CountItem <= 0)
			continue;

		if(BotData.m_InteractiveType == (int)QuestInteractive::INTERACTIVE_RANDOM_ACCEPT_ITEM)
		{
			const float Chance = BotData.m_InteractiveTemp <= 0 ? 100.0f : (1.0f / (float)BotData.m_InteractiveTemp) * 100;
			str_format(aBuf, sizeof(aBuf), "%s [takes %0.2f%%]", aBuf, Chance);
		}
		else
			str_format(aBuf, sizeof(aBuf), "%s", pPlayer->GetItem(ItemID).Info().GetName(pPlayer));

		GS()->Mmo()->Quest()->QuestTableAddItem(ClientID, aBuf, CountItem, ItemID, false);
	}

	// search mob's
	for (int i = 0; i < 2; i++)
	{
		const int BotID = BotData.m_aNeedMob[i];
		const int CountMob = BotData.m_aNeedMobCount[i];
		if (BotID <= 0 || CountMob <= 0 || !GS()->Mmo()->BotsData()->IsDataBotValid(BotID))
			continue;

		str_format(aBuf, sizeof(aBuf), "Defeat %s", BotJob::ms_aDataBot[BotID].m_aNameBot);
		GS()->Mmo()->Quest()->QuestTableAddInfo(ClientID, aBuf, CountMob, QuestJob::ms_aQuests[ClientID][BotData.m_QuestID].m_aMobProgress[i]);
	}

	// reward item's
	for (int i = 0; i < 2; i++)
	{
		const int ItemID = BotData.m_aItemGives[i];
		const int CountItem = BotData.m_aItemGivesCount[i];
		if (ItemID <= 0 || CountItem <= 0)
			continue;

		str_format(aBuf, sizeof(aBuf), "Receive %s", pPlayer->GetItem(ItemID).Info().GetName(pPlayer));
		GS()->Mmo()->Quest()->QuestTableAddItem(ClientID, aBuf, CountItem, ItemID, true);
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

void QuestJob::QuestTableAddInfo(int ClientID, const char *pText, int Requires, int Have)
{
	if (ClientID < 0 || ClientID >= MAX_PLAYERS || !GS()->IsMmoClient(ClientID))
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
	if (ClientID < 0 || ClientID >= MAX_PLAYERS || !GS()->IsMmoClient(ClientID))
		return;
	
	CNetMsg_Sv_ClearQuestingProcessing Msg;
	GS()->Server()->SendPackMsg(&Msg, MSGFLAG_VITAL, ClientID);
}

int QuestJob::QuestingAllowedItemsCount(CPlayer *pPlayer, int ItemID)
{
	const int ClientID = pPlayer->GetCID();
	const InventoryItem PlayerSearchItem = pPlayer->GetItem(ItemID);
	for (const auto& qq : ms_aQuests[ClientID])
	{
		if (qq.second.m_State != QuestState::QUEST_ACCEPT)
			continue;

		BotJob::QuestBotInfo *BotInfo = GetQuestBot(qq.first, qq.second.m_Progress);
		if (!BotInfo)
			continue;

		for (int i = 0; i < 2; i++)
		{
			const int needItemID = BotInfo->m_aItemSearch[i];
			const int numNeed = BotInfo->m_aItemSearchCount[i];
			if (needItemID <= 0 || numNeed <= 0 || ItemID != needItemID)
				continue;

			const int AvailableCount = clamp(PlayerSearchItem.m_Count - numNeed, 0, PlayerSearchItem.m_Count);
			return AvailableCount;
		}
	}
	return PlayerSearchItem.m_Count;
}

void QuestJob::CreateQuestingItems(CPlayer *pPlayer, BotJob::QuestBotInfo &BotData)
{
	if (!pPlayer || !pPlayer->GetCharacter() || BotData.m_InteractiveType != (int)QuestInteractive::INTERACTIVE_DROP_AND_TAKE_IT)
		return;

	const int ClientID = pPlayer->GetCID();
	for (CDropQuestItem* pHh = (CDropQuestItem*)GS()->m_World.FindFirst(CGameWorld::ENTTYPE_DROPQUEST); pHh; pHh = (CDropQuestItem*)pHh->TypeNext())
	{
		if (pHh->m_OwnerID == ClientID && pHh->m_QuestBot.m_QuestID == BotData.m_QuestID)
			return;
	}

	const int Count = 3 + BotData.m_aItemSearchCount[0];
	const vec2 Pos = vec2(BotData.m_PositionX, BotData.m_PositionY);
	for (int i = 0; i < Count; i++)
	{
		const vec2 Vel = vec2(frandom() * 40.0f - frandom() * 80.0f, frandom() * 40.0f - frandom() * 80.0f);
		const float AngleForce = Vel.x * (0.15f + frandom() * 0.1f);
		new CDropQuestItem(&GS()->m_World, Pos, Vel, AngleForce, BotData, ClientID);
	}
}

void QuestJob::OnInit()
{
	std::shared_ptr<ResultSet> RES(SJK.SD("*", "tw_quests_list"));
	while (RES->next())
	{
		const int QUID = RES->getInt("ID");
		str_copy(ms_aQuestsData[QUID].m_aName, RES->getString("Name").c_str(), sizeof(ms_aQuestsData[QUID].m_aName));
		str_copy(ms_aQuestsData[QUID].m_aStoryLine, RES->getString("StoryLine").c_str(), sizeof(ms_aQuestsData[QUID].m_aStoryLine));
		ms_aQuestsData[QUID].m_Gold = (int)RES->getInt("Money");
		ms_aQuestsData[QUID].m_Exp = (int)RES->getInt("Exp");

		// load talking progress size
		ms_aQuestsData[QUID].m_ProgressSize = 1;
		for (const auto& qb : BotJob::ms_aQuestBot)
		{
			if (qb.second.m_QuestID == QUID && !qb.second.m_NextEqualProgress)
				ms_aQuestsData[QUID].m_ProgressSize++;
		}
	}
}

void QuestJob::OnInitAccount(CPlayer* pPlayer)
{
	const int ClientID = pPlayer->GetCID();
	std::shared_ptr<ResultSet> RES(SJK.SD("*", "tw_accounts_quests", "WHERE OwnerID = '%d'", pPlayer->Acc().m_AuthID));
	while (RES->next())
	{
		const int QuestID = RES->getInt("QuestID");
		ms_aQuests[ClientID][QuestID].m_State = (int)RES->getInt("Type");
		ms_aQuests[ClientID][QuestID].m_Progress = (int)RES->getInt("Progress");
		ms_aQuests[ClientID][QuestID].m_aMobProgress[0] = (int)RES->getInt("Mob1Progress");
		ms_aQuests[ClientID][QuestID].m_aMobProgress[1] = (int)RES->getInt("Mob2Progress");
		GS()->UpdateQuestsBot(QuestID, ms_aQuests[ClientID][QuestID].m_Progress);
	}
}

void QuestJob::OnResetClient(int ClientID)
{
	if (ms_aQuests.find(ClientID) != ms_aQuests.end())
	{
		std::map < int, int > m_talkcheck;
		for (const auto& qp : ms_aQuests[ClientID])
		{
			if (qp.second.m_State != QuestState::QUEST_FINISHED)
				m_talkcheck[qp.first] = qp.second.m_Progress;
		}
		ms_aQuests.erase(ClientID);

		for (const auto& qst : m_talkcheck)
			GS()->UpdateQuestsBot(qst.first, qst.second);
	}
}

void QuestJob::OnMessage(int MsgID, void* pRawMsg, int ClientID)
{
	CPlayer* pPlayer = GS()->m_apPlayers[ClientID];
	if (MsgID == NETMSGTYPE_CL_TALKINTERACTIVE)
	{
		if (pPlayer->m_PlayerTick[TickState::LastDialog] && pPlayer->m_PlayerTick[TickState::LastDialog] > GS()->Server()->Tick())
			return;

		pPlayer->m_PlayerTick[TickState::LastDialog] = GS()->Server()->Tick() + (GS()->Server()->TickSpeed() / 4);
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

bool QuestJob::OnVotingMenu(CPlayer* pPlayer, const char* CMD, const int VoteID, const int VoteID2, int Get, const char* GetText)
{
	return false;
}