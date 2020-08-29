/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#include <engine/shared/config.h>
#include <teeother/system/string.h>

#include <game/server/gamecontext.h>
#include "QuestJob.h"

#include <game/server/mmocore/GameEntities/Items/drop_quest_items.h>

std::map < int , QuestJob::StructQuestData > QuestJob::QuestsData;
std::map < int , std::map < int , QuestJob::StructQuest > > QuestJob::Quests;

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
		return Quests[ClientID][QuestID].State;
	return QuestState::QUEST_NO_ACCEPT;
}

bool QuestJob::IsCompletedQuest(int ClientID, int QuestID) const
{
	return (bool)(GetState(ClientID, QuestID) == QuestState::QUEST_FINISHED);
}

int QuestJob::GetStoryCount(const char *StoryName, int CountFromQuestID) const
{
	int Count = 0;
	for(const auto& qd : QuestsData)
	{
		if(str_comp(qd.second.StoryLine, StoryName) == 0)
			Count++;
	}
	
	if(CountFromQuestID > 0)
	{
		for (auto qquest = QuestsData.find(CountFromQuestID); qquest != QuestsData.end(); qquest++)
		{
			if(str_comp(qquest->second.StoryLine, StoryName) == 0)
				Count--;
		}
	}
	return Count;
}

bool QuestJob::IsDefeatMobsComplete(int ClientID, int QuestID) const
{
	if(!IsValidQuest(QuestID, ClientID)) 
		return false;

	const int playerProgress = Quests[ClientID][QuestID].Progress;
	BotJob::QuestBotInfo* pFindBot = GetQuestBot(QuestID, playerProgress);
	if (!pFindBot)
		return false;

	for (int i = 0; i < 2; i++)
	{
		const int MobID = pFindBot->NeedMob[i];
		const int Count = pFindBot->NeedMobCount[i];
		if(MobID <= 0 || Count <= 0) 
			continue;

		if(Quests[ClientID][QuestID].MobProgress[i] < Count)
			return false;
	}
	return true;
}

const char *QuestJob::GetQuestName(int QuestID) const
{
	if(IsValidQuest(QuestID))
		return QuestsData[QuestID].Name;
	return "Unknown";
}

const char *QuestJob::GetStoryName(int QuestID) const
{
	if(IsValidQuest(QuestID))
		return QuestsData[QuestID].StoryLine;
	return "Unknown";
}

BotJob::QuestBotInfo *QuestJob::GetQuestBot(int QuestID, int Progress) const
{
	for(auto& qb : BotJob::QuestBot)
	{
		// skip decorative quest npc
		if(qb.second.NextEqualProgress)
			continue;

		if(QuestID == qb.second.QuestID && Progress == qb.second.Progress)
			return &qb.second;
	}
	return nullptr;
}

bool QuestJob::IsActiveQuestBot(int QuestID, int Progress) const
{
	for(int i = 0; i < MAX_PLAYERS; i++)
	{
		const int PlayerState = GetState(i, QuestID);
		if(PlayerState != QuestState::QUEST_ACCEPT || Quests[i][QuestID].Progress != Progress)
			continue;
		return true;
	}
	return false;
}

void QuestJob::ShowQuestID(CPlayer *pPlayer, int QuestID)
{
	const int ClientID = pPlayer->GetCID();
	const int HideID = NUM_TAB_MENU + 10500 + QuestID;
	const int CountQuest = GetStoryCount(QuestsData[QuestID].StoryLine);
	const int LineQuest = GetStoryCount(QuestsData[QuestID].StoryLine, QuestID)+1;

	GS()->AVH(ClientID, HideID, LIGHT_GOLDEN_COLOR, "{INT}/{INT} {STR}: {STR}", &LineQuest, &CountQuest, QuestsData[QuestID].StoryLine, QuestsData[QuestID].Name);
	GS()->AVM(ClientID, "null", NOPE, HideID, "You will receive a reward");
	GS()->AVM(ClientID, "null", NOPE, HideID, "Gold: {INT} Exp: {INT}", &QuestsData[QuestID].Money, &QuestsData[QuestID].Exp);
	GS()->AVM(ClientID, "null", NOPE, HideID, " ");
}

void QuestJob::FinishQuest(CPlayer *pPlayer, int QuestID)
{
	const int ClientID = pPlayer->GetCID();
	if(GetState(ClientID, QuestID) != QUEST_ACCEPT)
		return;
	
	// установить статистику квеста
	Quests[ClientID][QuestID].State = QuestState::QUEST_FINISHED;
	SJK.UD("tw_accounts_quests", "Type = '%d' WHERE QuestID = '%d' AND OwnerID = '%d'", Quests[ClientID][QuestID].State, QuestID, pPlayer->Acc().AuthID);

	// выдать награды и написать о завершении
	const StructQuestData finishQuestData = QuestsData[QuestID];
	pPlayer->AddMoney(finishQuestData.Money);
	pPlayer->AddExp(finishQuestData.Exp);

	GS()->Chat(-1, "{STR} completed: {STR} - {STR}!", GS()->Server()->ClientName(ClientID), finishQuestData.StoryLine, finishQuestData.Name);
	GS()->ChatDiscord(DC_PLAYER_INFO, GS()->Server()->ClientName(ClientID), "Completed ({STR} - {STR})", finishQuestData.StoryLine, finishQuestData.Name);
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
		const int ItemID = BotData.ItemSearch[i];
		const int Count = BotData.ItemSearchCount[i];
		if (ItemID > 0 && Count > 0)
		{
			GS()->Chat(pPlayer->GetCID(), "You used quest item {STR}x{INT}!", pPlayer->GetItem(ItemID).Info().GetName(pPlayer), &Count);
			antiStressing = (bool)(ItemID == BotData.ItemGives[0] || ItemID == BotData.ItemGives[1]);
			pPlayer->GetItem(ItemID).Remove(Count);
		}
	}

	for(int i = 0; i < 2; i++)
	{
		const int ItemID = BotData.ItemGives[i];
		const int Count = BotData.ItemGivesCount[i];
		if(ItemID > 0 && Count > 0)
		{
			if(antiStressing)
			{
				Job()->Item()->AddItemSleep(pPlayer->Acc().AuthID, ItemID, Count, 300);
				continue;
			}

			if(pPlayer->GetItem(ItemID).Info().IsEnchantable() && pPlayer->GetItem(ItemID).Count >= 1)
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
		const int ItemID = BotData.ItemSearch[i];
		const int Count = BotData.ItemSearchCount[i];
		if(ItemID <= 0 || Count <= 0)
			continue;
		if(pPlayer->GetItem(ItemID).Count < Count)
			return false;
	}
	return true;
}

void QuestJob::AddProgress(CPlayer *pPlayer, int QuestID)
{
	const int ClientID = pPlayer->GetCID();
	if(!IsValidQuest(QuestID, ClientID) || !pPlayer->GetCharacter()) 
		return;

	StructQuest &talkQuestPlayer = Quests[ClientID][QuestID];
	talkQuestPlayer.Progress++;
	talkQuestPlayer.MobProgress[0] = 0;
	talkQuestPlayer.MobProgress[1] = 0;
	const bool FinishedProgress = (talkQuestPlayer.Progress >= QuestsData[QuestID].ProgressSize);
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
			talkQuestPlayer.Progress, talkQuestPlayer.MobProgress[0], talkQuestPlayer.MobProgress[1], QuestID, pPlayer->Acc().AuthID);
	}

	const int OldProgress = talkQuestPlayer.Progress - 1;
	const int NewProgress = talkQuestPlayer.Progress;
	GS()->UpdateQuestsBot(QuestID, OldProgress);
	GS()->UpdateQuestsBot(QuestID, NewProgress);

}

bool QuestJob::AcceptQuest(int QuestID, CPlayer* pPlayer)
{
	if (!pPlayer || GetState(pPlayer->GetCID(), QuestID) != QuestState::QUEST_NO_ACCEPT)
		return false;

	const int ClientID = pPlayer->GetCID();
	Quests[ClientID][QuestID].Progress = 1;
	Quests[ClientID][QuestID].State = QuestState::QUEST_ACCEPT;
	GS()->UpdateQuestsBot(QuestID, Quests[ClientID][QuestID].Progress);
	SJK.ID("tw_accounts_quests", "(QuestID, OwnerID, Type) VALUES ('%d', '%d', '%d')", QuestID, pPlayer->Acc().AuthID, QuestState::QUEST_ACCEPT);

	const int StorySize = GetStoryCount(QuestsData[QuestID].StoryLine);
	const int StoryProgress = GetStoryCount(QuestsData[QuestID].StoryLine, QuestID + 1);
	GS()->Chat(ClientID, "Quest: {STR} - {STR} {INT}/{INT}!", QuestsData[QuestID].StoryLine, QuestsData[QuestID].Name, &StoryProgress, &StorySize);
	GS()->Chat(ClientID, "Receive a reward Gold {INT}, Experience {INT}", &QuestsData[QuestID].Money, &QuestsData[QuestID].Exp);
	pPlayer->GetCharacter()->CreateQuestsStep(QuestID);
	GS()->CreatePlayerSound(ClientID, SOUND_CTF_CAPTURE);
	return true;
}

bool QuestJob::InteractiveQuestNPC(CPlayer* pPlayer, BotJob::QuestBotInfo& BotData, bool LastDialog)
{
	if (!pPlayer || !pPlayer->GetCharacter())
		return false;

	// запретить мобам для красоты запрашивать что-либо
	if(BotData.NextEqualProgress)
		return true;

	const int ClientID = pPlayer->GetCID();
	const int QuestID = BotData.QuestID;
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

	// сбросить рисовку между (Quest NPC / NPC)
	BotJob::DataBot[BotData.BotID].AlreadySnapQuestBot[ClientID] = false;
	return true;
}

bool QuestJob::InteractiveTypeQuest(CPlayer* pPlayer, BotJob::QuestBotInfo& BotData)
{
	const int ClientID = pPlayer->GetCID();
	if (BotData.InteractiveType == (int)QuestInteractive::QUEST_INT_RANDOM_ACCEPT_ITEM && BotData.InteractiveTemp > 0)
	{
		const bool Succesful = random_int() % BotData.InteractiveTemp == 0;
		for(int i = 0; i < 2; i++)
		{
			const int ItemID = BotData.ItemSearch[i];
			const int Count = BotData.ItemSearchCount[i];
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
	for (auto& qp : Quests[ClientID])
	{
		// если квест является принятым
		if (qp.second.State != QuestState::QUEST_ACCEPT)
			continue;

		// получаем активного нпс
		const int questID = qp.first;
		const int playerProgress = qp.second.Progress;
		BotJob::QuestBotInfo *FindBot = GetQuestBot(questID, playerProgress);
		if (!FindBot)
			continue;

		// ищим нужен ли такой Моб
		for (int i = 0; i < 2; i++)
		{
			if (BotID != FindBot->NeedMob[i] || qp.second.MobProgress[i] >= FindBot->NeedMobCount[i])
				continue;

			qp.second.MobProgress[i]++;
			if(qp.second.MobProgress[i] >= FindBot->NeedMobCount[i])
			{
				GS()->Chat(ClientID, "You killed {STR} required amount for NPC {STR}", BotJob::DataBot[BotID].NameBot, FindBot->GetName());
			}
			SJK.UD("tw_accounts_quests", "Mob1Progress = '%d', Mob2Progress = '%d' WHERE QuestID = '%d' AND OwnerID = '%d'",
				qp.second.MobProgress[0], qp.second.MobProgress[1], questID, pPlayer->Acc().AuthID);
			break;
		}
	}
}

void QuestJob::UpdateArrowStep(int ClientID)
{
	CPlayer* pPlayer = GS()->GetPlayer(ClientID, true, true);
	if (!pPlayer)
		return;

	for (const auto& qp : Quests[ClientID])
	{
		if (qp.second.State == QuestState::QUEST_ACCEPT)
			pPlayer->GetCharacter()->CreateQuestsStep(qp.first);
	}
}

bool QuestJob::CheckNewStories(CPlayer *pPlayer, int CheckQuestID)
{
	const int ClientID = pPlayer->GetCID();
	if (CheckQuestID > 0)
	{
		const int NextQuestID = CheckQuestID + 1;
		const bool ActiveNextStories = (bool)(IsValidQuest(NextQuestID) && str_comp(QuestsData[CheckQuestID].StoryLine, QuestsData[NextQuestID].StoryLine) == 0);
		if(ActiveNextStories)
			AcceptQuest(NextQuestID, pPlayer);
		
		return ActiveNextStories;
	}

	bool ActiveNextStories = false;
	for (const auto& qp : Quests[ClientID])
	{
		if (qp.second.State != QuestState::QUEST_FINISHED)
			continue;

		int NextQuestID = qp.first + 1, QuestID = qp.first;
		if (!IsValidQuest(NextQuestID) || Quests[ClientID][NextQuestID].State != QuestState::QUEST_NO_ACCEPT)
			continue;

		if (str_comp(QuestsData[QuestID].StoryLine, QuestsData[NextQuestID].StoryLine) == 0)
		{
			AcceptQuest(NextQuestID, pPlayer);
			ActiveNextStories = true;
			continue;
		}
	}
	return ActiveNextStories;
}

void QuestJob::ShowQuestList(CPlayer* pPlayer, int StateQuest)
{
	char aStoryLineSave[32];
	bool FoundQuests = false;
	const int ClientID = pPlayer->GetCID();

	pPlayer->m_Colored = GOLDEN_COLOR;
	GS()->AVL(ClientID, "null", "★ {STR} quests", GetStateName(StateQuest));

	for (const auto& qd : QuestsData)
	{
		int questID = qd.first;
		if (GetState(ClientID, questID) != StateQuest)
			continue;

		if (StateQuest != QuestState::QUEST_FINISHED)
		{
			if (str_comp(aStoryLineSave, qd.second.StoryLine) == 0)
				continue;

			str_copy(aStoryLineSave, qd.second.StoryLine, sizeof(aStoryLineSave));
		}
		ShowQuestID(pPlayer, questID);
		FoundQuests = true;
	}

	if (!FoundQuests)
	{
		pPlayer->m_Colored = LIGHT_GOLDEN_COLOR;
		GS()->AV(ClientID, "null", "This list is empty");
	}
	GS()->AV(ClientID, "null", "");
}

// позать все квесты весь список
void QuestJob::ShowFullQuestLift(CPlayer* pPlayer)
{
	// показываем всех нпс активных
	const int ClientID = pPlayer->GetCID();
	if (!ShowAdventureActiveNPC(pPlayer))
	{
		pPlayer->m_Colored = LIGHT_BLUE_COLOR;
		GS()->AVM(ClientID, "null", NOPE, NOPE, "In current quests there is no interaction with NPC");
	}
	GS()->AV(ClientID, "null", "");

	// показываем лист квестов
	ShowQuestList(pPlayer, QuestState::QUEST_ACCEPT);
	ShowQuestList(pPlayer, QuestState::QUEST_NO_ACCEPT);

	// показываем меню завершенных
	pPlayer->m_Colored = BLUE_COLOR;
	GS()->AVM(ClientID, "MENU", MenuList::MENU_JOURNAL_FINISHED, NOPE, "List of completed quests");
}

// Адвентур активные нпс показ информации
bool QuestJob::ShowAdventureActiveNPC(CPlayer* pPlayer)
{
	bool activeNPC = false;
	const int clientID = pPlayer->GetCID();
	pPlayer->m_Colored = BLUE_COLOR;
	GS()->AVM(clientID, "null", NOPE, NOPE, "Active NPC for current quests");

	// поиск всех активных нпс
	for (const auto& qq : Quests[clientID])
	{
		if (qq.second.State != QuestState::QUEST_ACCEPT)
			continue;

		// проверяем бота есть или нет активный по квесту
		BotJob::QuestBotInfo *BotInfo = GetQuestBot(qq.first, qq.second.Progress);
		if (!BotInfo)
			continue;

		// если нашли выводим информацию
		const int HideID = (NUM_TAB_MENU + 12500 + BotInfo->QuestID);
		const int PosX = BotInfo->PositionX / 32, PosY = BotInfo->PositionY / 32;
		GS()->AVH(clientID, HideID, LIGHT_BLUE_COLOR, "[{STR}] {STR} {STR}(x{INT} y{INT})", GetStoryName(qq.first), BotInfo->GetName(), GS()->Server()->GetWorldName(BotInfo->WorldID), &PosX, &PosY);

		bool JustTalk = true;
		for (int i = 0; i < 2; i++)
		{
			const int botID = BotInfo->NeedMob[i];
			const int killNeed = BotInfo->NeedMobCount[i];
			if(botID > 0 && killNeed > 0 && Job()->BotsData()->IsDataBotValid(botID))
			{
				GS()->AVMI(clientID, "broken_h", "null", NOPE, HideID, "- Defeat {STR} [{INT}/{INT}]", BotJob::DataBot[botID].NameBot, &qq.second.MobProgress[i], &killNeed);
				JustTalk = false;
			}

			const int itemID = BotInfo->ItemSearch[i];
			const int itemCount = BotInfo->ItemSearchCount[i];
			if(itemID > 0 && itemCount > 0)
			{
				ItemJob::InventoryItem PlayerSearchItem = pPlayer->GetItem(itemID);
				int ownCount = clamp(PlayerSearchItem.Count, 0, itemCount);

				GS()->AVMI(clientID, PlayerSearchItem.Info().GetIcon(), "null", NOPE, HideID, "- Item {STR} [{INT}/{INT}]", PlayerSearchItem.Info().GetName(pPlayer), &ownCount, &itemCount);
				JustTalk = false;
			}
		}
		for(int i = 0; i < 2; i++)
		{
			const int itemID = BotInfo->ItemGives[i];
			const int itemCount = BotInfo->ItemGivesCount[i];
			if(itemID > 0 && itemCount > 0)
			{
				ItemJob::ItemInformation GivedInfItem = GS()->GetItemInfo(itemID);
				GS()->AVMI(clientID, GivedInfItem.GetIcon(), "null", NOPE, HideID, "- Receive {STR}x{INT}", GivedInfItem.GetName(pPlayer), &itemCount);
				JustTalk = false;
			}
		}

		// если не нашли ничего что он делает
		if (JustTalk)
			GS()->AVM(clientID, "null", NOPE, HideID, "You just need to talk.");

		activeNPC = true;
	}
	return activeNPC;
}

void QuestJob::QuestTableShowRequired(CPlayer *pPlayer, BotJob::QuestBotInfo &BotData, const char* TextTalk)
{
	// запретить мобам для красоты запрашивать что-либо
	if(BotData.NextEqualProgress)
		return;

	// показываем текст завершения квеста
	const int ClientID = pPlayer->GetCID();
	if (GS()->CheckClient(ClientID))
	{
		QuestTableShowRequired(pPlayer, BotData);
		return;
	}

	char aBuf[64];
	dynamic_string Buffer;
	bool ShowItemNeeded = false;
	const int QuestID = BotData.QuestID;

	// search item's and mob's
	for(int i = 0; i < 2; i++)
	{
		const int BotID = BotData.NeedMob[i];
		const int CountMob = BotData.NeedMobCount[i];
		if(BotID > 0 && CountMob > 0 && Job()->BotsData()->IsDataBotValid(BotID))
		{
			str_format(aBuf, sizeof(aBuf), "\n- Defeat %s [%d/%d]", BotJob::DataBot[BotID].NameBot, Quests[ClientID][QuestID].MobProgress[i], CountMob);
			Buffer.append_at(Buffer.length(), aBuf);
		}

		const int ItemID = BotData.ItemSearch[i];
		const int CountItem = BotData.ItemSearchCount[i];
		if(ItemID > 0 && CountItem > 0)
		{
			ItemJob::InventoryItem PlayerQuestItem = pPlayer->GetItem(ItemID);
			str_format(aBuf, sizeof(aBuf), "\n- Need %s [%d/%d]", PlayerQuestItem.Info().GetName(pPlayer), PlayerQuestItem.Count, CountItem);
			Buffer.append_at(Buffer.length(), aBuf);
			ShowItemNeeded = true;
		}
	}

	// type random accept item's
	if(BotData.InteractiveType == (int)QuestInteractive::QUEST_INT_RANDOM_ACCEPT_ITEM)
	{
		const double Chance = BotData.InteractiveTemp <= 0 ? 100.0f : (1.0f / (double)BotData.InteractiveTemp) * 100;
		str_format(aBuf, sizeof(aBuf), "\nChance that item he'll like [%0.2f%%]\n", Chance);
		Buffer.append_at(Buffer.length(), aBuf);
	}

	// reward item's
	for(int i = 0; i < 2; i++)
	{
		const int ItemID = BotData.ItemGives[i];
		const int CountItem = BotData.ItemGivesCount[i];
		if(ItemID > 0 && CountItem > 0)
		{
			str_format(aBuf, sizeof(aBuf), "\n- Receive %s [%d]", GS()->GetItemInfo(ItemID).GetName(pPlayer), CountItem);
			Buffer.append_at(Buffer.length(), aBuf);
		}
	}

	// показываем все информацию
	GS()->Motd(ClientID, "{STR}\n\n{STR}{STR}\n\n", TextTalk, (ShowItemNeeded ? "### Task" : "\0"), Buffer.buffer());
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
		const int ItemID = BotData.ItemSearch[i];
		const int CountItem = BotData.ItemSearchCount[i];
		if(ItemID <= 0 || CountItem <= 0)
			continue;

		if(BotData.InteractiveType == (int)QuestInteractive::QUEST_INT_RANDOM_ACCEPT_ITEM)
		{
			const float Chance = BotData.InteractiveTemp <= 0 ? 100.0f : (1.0f / (float)BotData.InteractiveTemp) * 100;
			str_format(aBuf, sizeof(aBuf), "%s [takes %0.2f%%]", aBuf, Chance);
		}
		else
			str_format(aBuf, sizeof(aBuf), "%s", pPlayer->GetItem(ItemID).Info().GetName(pPlayer));

		GS()->Mmo()->Quest()->QuestTableAddItem(ClientID, aBuf, CountItem, ItemID, false);
	}

	// search mob's
	for (int i = 0; i < 2; i++)
	{
		const int BotID = BotData.NeedMob[i];
		const int CountMob = BotData.NeedMobCount[i];
		if (BotID <= 0 || CountMob <= 0 || !GS()->Mmo()->BotsData()->IsDataBotValid(BotID))
			continue;

		str_format(aBuf, sizeof(aBuf), "Defeat %s", BotJob::DataBot[BotID].NameBot);
		GS()->Mmo()->Quest()->QuestTableAddInfo(ClientID, aBuf, CountMob, QuestJob::Quests[ClientID][BotData.QuestID].MobProgress[i]);
	}

	// reward item's
	for (int i = 0; i < 2; i++)
	{
		const int ItemID = BotData.ItemGives[i];
		const int CountItem = BotData.ItemGivesCount[i];
		if (ItemID <= 0 || CountItem <= 0)
			continue;

		str_format(aBuf, sizeof(aBuf), "Receive %s", pPlayer->GetItem(ItemID).Info().GetName(pPlayer));
		GS()->Mmo()->Quest()->QuestTableAddItem(ClientID, aBuf, CountItem, ItemID, true);
	}
}

void QuestJob::QuestTableAddItem(int ClientID, const char* pText, int Requires, int ItemID, bool GivingTable)
{
	CPlayer* pPlayer = GS()->GetPlayer(ClientID, true);
	if (!pPlayer || ItemID < itGold || !GS()->CheckClient(ClientID))
		return;

	const ItemJob::InventoryItem PlayerSelectedItem = pPlayer->GetItem(ItemID);

	CNetMsg_Sv_AddQuestingProcessing Msg;
	Msg.m_pText = pText;
	Msg.m_pRequiresNum = Requires;
	Msg.m_pHaveNum = clamp(PlayerSelectedItem.Count, 0, Requires);
	Msg.m_pGivingTable = GivingTable;
	StrToInts(Msg.m_pIcon, 4, PlayerSelectedItem.Info().GetIcon());
	GS()->Server()->SendPackMsg(&Msg, MSGFLAG_VITAL, ClientID);
}

void QuestJob::QuestTableAddInfo(int ClientID, const char *pText, int Requires, int Have)
{
	if (ClientID < 0 || ClientID >= MAX_PLAYERS || !GS()->CheckClient(ClientID))
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
	if (ClientID < 0 || ClientID >= MAX_PLAYERS || !GS()->CheckClient(ClientID))
		return;
	
	CNetMsg_Sv_ClearQuestingProcessing Msg;
	GS()->Server()->SendPackMsg(&Msg, MSGFLAG_VITAL, ClientID);
}

int QuestJob::QuestingAllowedItemsCount(CPlayer *pPlayer, int ItemID)
{
	const int ClientID = pPlayer->GetCID();
	const ItemJob::InventoryItem PlayerSearchItem = pPlayer->GetItem(ItemID);
	for (const auto& qq : Quests[ClientID])
	{
		if (qq.second.State != QuestState::QUEST_ACCEPT)
			continue;

		// проверяем бота есть или нет активный по квесту
		BotJob::QuestBotInfo *BotInfo = GetQuestBot(qq.first, qq.second.Progress);
		if (!BotInfo)
			continue;

		// проверяем требуемые предметы
		for (int i = 0; i < 2; i++)
		{
			const int needItemID = BotInfo->ItemSearch[i];
			const int numNeed = BotInfo->ItemSearchCount[i];
			if (needItemID <= 0 || numNeed <= 0 || ItemID != needItemID)
				continue;

			const int AvailableCount = clamp(PlayerSearchItem.Count - numNeed, 0, PlayerSearchItem.Count);
			return AvailableCount;
		}
	}
	return PlayerSearchItem.Count;
}

void QuestJob::CreateQuestingItems(CPlayer *pPlayer, BotJob::QuestBotInfo &BotData)
{
	if (!pPlayer || !pPlayer->GetCharacter() || BotData.InteractiveType != (int)QuestInteractive::QUEST_INT_DROP_AND_TAKE_IT)
		return;

	const int ClientID = pPlayer->GetCID();
	for (CDropQuestItem* pHh = (CDropQuestItem*)GS()->m_World.FindFirst(CGameWorld::ENTTYPE_DROPQUEST); pHh; pHh = (CDropQuestItem*)pHh->TypeNext())
	{
		if (pHh->m_OwnerID == ClientID && pHh->m_QuestBot.QuestID == BotData.QuestID)
			return;
	}

	const int Count = 3 + BotData.ItemSearchCount[0];
	const vec2 Pos = vec2(BotData.PositionX, BotData.PositionY);
	for (int i = 0; i < Count; i++)
	{
		const vec2 Vel = vec2(frandom() * 40.0f - frandom() * 80.0f, frandom() * 40.0f - frandom() * 80.0f);
		const float AngleForce = Vel.x * (0.15f + frandom() * 0.1f);
		new CDropQuestItem(&GS()->m_World, Pos, Vel, AngleForce, BotData, ClientID);
	}
}

void QuestJob::OnInit()
{
	boost::scoped_ptr<ResultSet> RES(SJK.SD("*", "tw_quests_list"));
	while (RES->next())
	{
		const int QUID = RES->getInt("ID");
		str_copy(QuestsData[QUID].Name, RES->getString("Name").c_str(), sizeof(QuestsData[QUID].Name));
		str_copy(QuestsData[QUID].StoryLine, RES->getString("StoryLine").c_str(), sizeof(QuestsData[QUID].StoryLine));
		QuestsData[QUID].Money = (int)RES->getInt("Money");
		QuestsData[QUID].Exp = (int)RES->getInt("Exp");

		// load talking progress size
		QuestsData[QUID].ProgressSize = 1;
		for (const auto& qb : BotJob::QuestBot)
		{
			if (qb.second.QuestID == QUID && !qb.second.NextEqualProgress)
				QuestsData[QUID].ProgressSize++;
		}
	}
}

void QuestJob::OnInitAccount(CPlayer* pPlayer)
{
	const int ClientID = pPlayer->GetCID();
	boost::scoped_ptr<ResultSet> RES(SJK.SD("*", "tw_accounts_quests", "WHERE OwnerID = '%d'", pPlayer->Acc().AuthID));
	while (RES->next())
	{
		const int QuestID = RES->getInt("QuestID");
		Quests[ClientID][QuestID].State = (int)RES->getInt("Type");
		Quests[ClientID][QuestID].Progress = (int)RES->getInt("Progress");
		Quests[ClientID][QuestID].MobProgress[0] = (int)RES->getInt("Mob1Progress");
		Quests[ClientID][QuestID].MobProgress[1] = (int)RES->getInt("Mob2Progress");
		GS()->UpdateQuestsBot(QuestID, Quests[ClientID][QuestID].Progress);
	}
}

void QuestJob::OnResetClient(int ClientID)
{
	if (Quests.find(ClientID) != Quests.end())
	{
		std::map < int, int > m_talkcheck;
		for (const auto& qp : Quests[ClientID])
		{
			if (qp.second.State != QuestState::QUEST_FINISHED)
				m_talkcheck[qp.first] = qp.second.Progress;
		}
		Quests.erase(ClientID);

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
		ShowQuestList(pPlayer, QuestState::QUEST_FINISHED);
		GS()->AddBack(ClientID);
		return true;
	}

	return false;
}

bool QuestJob::OnVotingMenu(CPlayer* pPlayer, const char* CMD, const int VoteID, const int VoteID2, int Get, const char* GetText)
{
	return false;
}