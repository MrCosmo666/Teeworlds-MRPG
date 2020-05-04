/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#include <engine/shared/config.h>
#include <game/server/gamecontext.h>
#include <game/server/mmocore/GameEntities/Items/dropquest.h>

#include "QuestJob.h"

std::map < int , QuestJob::StructQuestData > QuestJob::QuestsData;
std::map < int , std::map < int , QuestJob::StructQuest > > QuestJob::Quests;

int QuestJob::GetState(int ClientID, int QuestID) const
{
	if(IsValidQuest(QuestID, ClientID))
		return Quests[ClientID][QuestID].State;
	return QuestState::QUEST_NO_ACCEPT;
}

bool QuestJob::IsComplectedQuest(int ClientID, int QuestID) const
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

const char *QuestJob::GetStateName(int Type) const
{
	switch (Type)
	{
	case QuestState::QUEST_NO_ACCEPT: return "Not active";
	case QuestState::QUEST_ACCEPT: return "Active";
	case QuestState::QUEST_FINISHED: return "Finished";
	}
	return "unknow";
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

// получить имя квеста
const char *QuestJob::GetQuestName(int QuestID) const
{
	if(IsValidQuest(QuestID))
		return QuestsData[QuestID].Name;
	return "Unknown";
}

// получить имя истории квеста
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
		if(QuestID == qb.second.QuestID && Progress == qb.second.Progress)
			return &qb.second;
	}
	return nullptr;
}

bool QuestJob::IsActiveQuestBot(int QuestID, int Progress) const
{
	for(int i = 0; i < MAX_PLAYERS; i++)
	{
		if(!IsValidQuest(QuestID, i)) 
			continue;

		// -- пропускаем завершеный квест
		int PlayerState = GetState(i, QuestID);
		if(PlayerState != QuestState::QUEST_ACCEPT || Quests[i][QuestID].Progress != Progress)
			continue;

		// -- сортируем все квесты игрока
		BotJob::QuestBotInfo *FindBot = GetQuestBot(QuestID, Progress);
		if(!FindBot)
			continue;

		return true;
	}
	return false;
}

void QuestJob::ShowQuestID(CPlayer *pPlayer, int QuestID)
{
	const int ClientID = pPlayer->GetCID();
	const int HideID = NUM_TAB_MENU + 10500 + QuestID;
	const StructQuestData &activeQuestData = QuestsData[QuestID];
	const int CountQuest = GetStoryCount(activeQuestData.StoryLine);
	const int LineQuest = GetStoryCount(activeQuestData.StoryLine, QuestID)+1;

	GS()->AVH(ClientID, HideID, LIGHT_GOLDEN_COLOR, "[{INT}/{INT} {STR}] {STR}", &LineQuest, &CountQuest, activeQuestData.StoryLine, activeQuestData.Name);	
	GS()->AVM(ClientID, "null", NOPE, HideID, "You will receive a reward");
	GS()->AVM(ClientID, "null", NOPE, HideID, "Gold: {INT} Exp: {INT}", &activeQuestData.Money, &activeQuestData.Exp);
	GS()->AVM(ClientID, "null", NOPE, HideID, " ");
}

// завершить квест
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

	GS()->Chat(-1, "{STR} completed [{STR}] - {STR}!", GS()->Server()->ClientName(ClientID), finishQuestData.StoryLine, finishQuestData.Name);
	GS()->ChatDiscord(DC_PLAYER_INFO, GS()->Server()->ClientName(ClientID), "Completed ({STR} : {STR})", finishQuestData.StoryLine, finishQuestData.Name);
	Job()->SaveAccount(pPlayer, SaveType::SAVE_STATS);

	if (!CheckNewStories(pPlayer, QuestID))
		GS()->Chat(ClientID, "You completed the chapter {STR}!", QuestsData[QuestID].StoryLine);

	Job()->WorldSwap()->CheckQuestingOpened(pPlayer, QuestID);
	Job()->Dungeon()->CheckQuestingOpened(pPlayer, QuestID);
}


void QuestJob::CollectItem(CPlayer* pPlayer, BotJob::QuestBotInfo& BotData)
{
	// забрать предмет
	bool antiStressing = false;
	for (int i = 0; i < 2; i++)
	{
		int ItemID = BotData.ItemSearch[i];
		int Count = BotData.ItemSearchCount[i];
		if (ItemID > 0 && Count > 0)
		{
			pPlayer->GetItem(ItemID).Remove(Count);
			GS()->Chat(pPlayer->GetCID(), "You used quest item {STR}x{INT}!", pPlayer->GetItem(ItemID).Info().GetName(pPlayer), &Count);
			antiStressing = (bool)(ItemID == BotData.ItemGives[0] || ItemID == BotData.ItemGives[1]);
		}

		if (antiStressing)
		{
			kurosio::kpause(10);
		}

		ItemID = BotData.ItemGives[i];
		Count = BotData.ItemGivesCount[i] - pPlayer->GetItem(ItemID).Count;
		if (ItemID > 0 && Count > 0)
		{
			pPlayer->GetItem(ItemID).Add(Count);
		}
	}
}

// проверить прогресс по предметам что требует бот
bool QuestJob::IsCollectItemComplete(CPlayer *pPlayer, BotJob::QuestBotInfo &BotData, bool Gived) const
{
	if(Gived)
	{

		// проверить выдавание предмета
		for(int i = 0; i < 2; i++)
		{
			int ItemID = BotData.ItemGives[i];
			int Count = BotData.ItemGivesCount[i];
			if(ItemID > 0 && pPlayer->GetItem(ItemID).Count < Count)
				return false;
		}
		return true;
	}

	// проверить предмет
	for(int i = 0; i < 2; i++)
	{
		int ItemID = BotData.ItemSearch[i];
		int Count = BotData.ItemSearchCount[i];
		if(ItemID <= 0 || Count <= 0)
			continue;

		if(pPlayer->GetItem(ItemID).Count < Count)
			return false;
	}
	return true;
}

// Разговор повышение прогресса
void QuestJob::AddProgress(CPlayer *pPlayer, int QuestID)
{
	int ClientID = pPlayer->GetCID();
	if(!IsValidQuest(QuestID, ClientID) || !pPlayer->GetCharacter()) 
		return;

	StructQuest &talkQuestPlayer = Quests[ClientID][QuestID];
	talkQuestPlayer.Progress++;
	talkQuestPlayer.MobProgress[0] = 0;
	talkQuestPlayer.MobProgress[1] = 0;
	bool FinishedProgress = (talkQuestPlayer.Progress >= QuestsData[QuestID].ProgressSize);
	if (FinishedProgress)
	{
		FinishQuest(pPlayer, QuestID);
		GS()->VResetVotes(ClientID, MenuList::MENU_ADVENTURE_JOURNAL_MAIN);
	}
	else
	{
		pPlayer->GetCharacter()->CreateQuestsStep(QuestID);
		SJK.UD("tw_accounts_quests", "Progress = '%d', Mob1Progress = '%d', Mob2Progress = '%d' WHERE QuestID = '%d' AND OwnerID = '%d'",
			talkQuestPlayer.Progress, talkQuestPlayer.MobProgress[0], talkQuestPlayer.MobProgress[1], QuestID, pPlayer->Acc().AuthID);
	}

	// очистка прогресса ботов
	int OldProgress = talkQuestPlayer.Progress - 1;
	int NewProgress = talkQuestPlayer.Progress;
	GS()->UpdateQuestsBot(QuestID, OldProgress);
	GS()->UpdateQuestsBot(QuestID, NewProgress);
}

// Принять квест с ID
bool QuestJob::AcceptQuest(int QuestID, CPlayer* pPlayer)
{
	if (!pPlayer || !IsValidQuest(QuestID) || GetState(pPlayer->GetCID(), QuestID) >= QuestState::QUEST_ACCEPT)
		return false;

	// принимаем квест
	const int ClientID = pPlayer->GetCID();
	Quests[ClientID][QuestID].Progress = 1;
	Quests[ClientID][QuestID].State = QuestState::QUEST_ACCEPT;
	GS()->UpdateQuestsBot(QuestID, Quests[ClientID][QuestID].Progress);
	SJK.ID("tw_accounts_quests", "(QuestID, OwnerID, Type) VALUES ('%d', '%d', '%d')", QuestID, pPlayer->Acc().AuthID, QuestState::QUEST_ACCEPT);

	int StorySize = GetStoryCount(QuestsData[QuestID].StoryLine);
	int StoryProgress = GetStoryCount(QuestsData[QuestID].StoryLine, QuestID + 1);
	GS()->Chat(ClientID, "Story perform [{STR}] - {STR} {INT}/{INT}!", QuestsData[QuestID].StoryLine, QuestsData[QuestID].Name, &StoryProgress, &StorySize);
	GS()->Chat(ClientID, "Receive a reward Gold {INT}, Experience {INT}", &QuestsData[QuestID].Money, &QuestsData[QuestID].Exp);
	pPlayer->GetCharacter()->CreateQuestsStep(QuestID);
	GS()->CreatePlayerSound(ClientID, SOUND_CTF_CAPTURE);
	return true;
}

// действие над квестом
bool QuestJob::InteractiveQuestNPC(CPlayer* pPlayer, BotJob::QuestBotInfo& BotData, bool LastDialog)
{
	if (!pPlayer || !pPlayer->GetCharacter())
		return false;

	// проверяем собрали предметы и убили ли всех ботов
	const int ClientID = pPlayer->GetCID();
	const int QuestID = BotData.QuestID;
	if (!IsCollectItemComplete(pPlayer, BotData, false) || !IsDefeatMobsComplete(ClientID, QuestID))
	{
		GS()->Chat(ClientID, "Not all criteria to complete!");
		return false;
	}

	if (!LastDialog)
	{
		GS()->CreatePlayerSound(ClientID, SOUND_CTF_CAPTURE);
		return true;
	}

	// проверяем и выдаем потом
	CollectItem(pPlayer, BotData);
	GS()->VResetVotes(ClientID, MenuList::MENU_ADVENTURE_JOURNAL_MAIN);
	GS()->Mmo()->Quest()->AddProgress(pPlayer, QuestID);
	return true;
}

// Прогресс мобов
void QuestJob::AddMobProgress(CPlayer* pPlayer, int BotID)
{
	if (!pPlayer || !Job()->BotsData()->IsDataBotValid(BotID))
		return;

	// Ищим в активных квестах Моба что требуется для квеста
	const int ClientID = pPlayer->GetCID();
	for (auto& qp : Quests[ClientID])
	{
		// если квест является принятым
		if (qp.second.State != QuestState::QUEST_ACCEPT)
			continue;

		// получаем активного нпс
		int questID = qp.first;
		int playerProgress = Quests[ClientID][questID].Progress;
		BotJob::QuestBotInfo *FindBot = GetQuestBot(questID, playerProgress);
		if (!FindBot)
			continue;

		// ищим нужен ли такой Моб
		for (int i = 0; i < 2; i++)
		{
			// проверяем если равен и прогресс меньше чем требуется
			if (BotID != FindBot->NeedMob[i] || qp.second.MobProgress[i] >= FindBot->NeedMobCount[i])
				continue;

			qp.second.MobProgress[i]++;
			if (qp.second.MobProgress[i] >= FindBot->NeedMobCount[i])
				GS()->Chat(ClientID, "You killed {STR} the required amount for NPC {STR}", BotJob::DataBot[BotID].NameBot, FindBot->Name);

			SJK.UD("tw_accounts_quests", "Mob1Progress = '%d', Mob2Progress = '%d' WHERE QuestID = '%d' AND OwnerID = '%d'",
				qp.second.MobProgress[0], qp.second.MobProgress[1], questID, pPlayer->Acc().AuthID);
			break;
		}
	}
}

// обновить стрелки направление
void QuestJob::UpdateArrowStep(int ClientID)
{
	CPlayer* pPlayer = GS()->GetPlayer(ClientID, true, true);
	if (!pPlayer)
		return;

	for (const auto& qp : Quests[ClientID])
	{
		if (qp.second.State != QuestState::QUEST_ACCEPT)
			continue;

		pPlayer->GetCharacter()->CreateQuestsStep(qp.first);
	}
}

// проверить новые Stories
bool QuestJob::CheckNewStories(CPlayer *pPlayer, int CheckQuestID)
{
	int ClientID = pPlayer->GetCID();
	if (CheckQuestID > 0)
	{
		int NextQuestID = CheckQuestID + 1;
		bool ActiveNextStories = (bool)(IsValidQuest(NextQuestID) && str_comp(QuestsData[CheckQuestID].StoryLine, QuestsData[NextQuestID].StoryLine) == 0);
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

// показать квесты
void QuestJob::ShowQuestList(CPlayer* pPlayer, int StateQuest)
{
	char storyLineSave[32];
	bool foundQuests = false;

	int ClientID = pPlayer->GetCID();
	pPlayer->m_Colored = GOLDEN_COLOR;
	GS()->AVL(ClientID, "null", "★ {STR} quests", GetStateName(StateQuest));

	for (const auto& qd : QuestsData)
	{
		int questID = qd.first;
		if (GetState(ClientID, questID) != StateQuest)
			continue;

		if (StateQuest != QuestState::QUEST_FINISHED)
		{
			if (str_comp(storyLineSave, qd.second.StoryLine) == 0)
				continue;

			str_copy(storyLineSave, qd.second.StoryLine, sizeof(storyLineSave));
		}
		ShowQuestID(pPlayer, questID);
		foundQuests = true;
	}

	if (!foundQuests)
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
	int ClientID = pPlayer->GetCID();
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
	GS()->AVM(ClientID, "MENU", MenuList::MENU_ADVENTURE_JOURNAL_FINISHED, NOPE, "List of completed quests");
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
		int HideID = (NUM_TAB_MENU + 12500 + BotInfo->QuestID);
		int PosX = BotInfo->PositionX / 32, PosY = BotInfo->PositionY / 32;
		GS()->AVH(clientID, HideID, LIGHT_BLUE_COLOR, "[{STR}] {STR} {STR}(x:{INT} y:{INT})", GetStoryName(qq.first), BotInfo->Name, GS()->Server()->GetWorldName(BotInfo->WorldID), &PosX, &PosY);

		// проверяем требуемые мобы
		bool interactiveNeed = false;
		for (int i = 0; i < 2; i++)
		{
			{
				int botID = BotInfo->NeedMob[i];
				int killNeed = BotInfo->NeedMobCount[i];
				if (botID > 0 && killNeed <= 0 && !Job()->BotsData()->IsDataBotValid(botID))
				{
					GS()->AVM(clientID, "null", NOPE, HideID, "- Defeat {STR} [{INT}/{INT}]", BotJob::DataBot[botID].NameBot, &qq.second.MobProgress[i], &killNeed);
					interactiveNeed = true;
				}
			}
			{
				int itemID = BotInfo->ItemSearch[i];
				int numNeed = BotInfo->ItemSearchCount[i];
				if (itemID > 0 && numNeed > 0)
				{
					ItemJob::ItemPlayer searchItem = pPlayer->GetItem(itemID);
					int ownCount = clamp(searchItem.Count, 0, numNeed);

					GS()->AVMI(clientID, searchItem.Info().GetIcon(), "null", NOPE, HideID, "- Item {STR} [{INT}/{INT}]", searchItem.Info().GetName(pPlayer), &ownCount, &numNeed);
					interactiveNeed = true;
				}
			}
			{
				int itemID = BotInfo->ItemGives[i];
				int getCount = BotInfo->ItemGivesCount[i];
				if (itemID > 0 && getCount > 0)
				{
					ItemJob::ItemInformation GivedInfItem = GS()->GetItemInfo(itemID);
					GS()->AVMI(clientID, GivedInfItem.GetIcon(), "null", NOPE, HideID, "- Gives {STR}x{INT}", GivedInfItem.GetName(pPlayer), &getCount);
					interactiveNeed = true;
				}
			}
		}

		// если не нашли ничего что он делает
		if (!interactiveNeed)
		{
			GS()->AVM(clientID, "null", NOPE, HideID, "You just need to talk.");
		}
		activeNPC = true;
	}
	return activeNPC;
}

void QuestJob::QuestTableShowRequired(CPlayer *pPlayer, BotJob::QuestBotInfo &BotData, const char* TextTalk)
{
	if(!pPlayer || !pPlayer->GetCharacter()) 
		return;

	// показываем текст завершения квеста
	int ClientID = pPlayer->GetCID();
	if (GS()->CheckClient(ClientID))
	{
		QuestTableShowRequired(pPlayer, BotData);
		return;
	}

	// показываем по информация о предметах
	dynamic_string Buffer;
	int QuestID = BotData.QuestID;
	bool ShowItemNeeded = false;
	{
		for(int i = 0; i < 2; i++) 
		{
			int ItemID = BotData.ItemSearch[i];
			int Count = BotData.ItemSearchCount[i];
			if (ItemID <= 0 || Count <= 0)
				continue;

			char aBuf[48];
			ItemJob::ItemPlayer& PlQuestItem = pPlayer->GetItem(ItemID);
			str_format(aBuf, sizeof(aBuf), "\n- Item %s [%d/%d]", PlQuestItem.Info().GetName(pPlayer), PlQuestItem.Count, Count);
			Buffer.append_at(Buffer.length(), aBuf);
			ShowItemNeeded = true;
		}
	}

	// показываем текст по информации о мобах
	{
		for(int i = 0; i < 2; i++)
		{
			int BotID = BotData.NeedMob[i];
			int Count = BotData.NeedMobCount[i];
			if(BotID <= 0 || Count <= 0 || !Job()->BotsData()->IsDataBotValid(BotID)) 
				continue;

			char aBuf[48];
			str_format(aBuf, sizeof(aBuf), "\n- Defeat %s [%d/%d]", BotJob::DataBot[BotID].NameBot, Quests[ClientID][QuestID].MobProgress[i], Count);
			Buffer.append_at(Buffer.length(), aBuf);
		}
	}

	// если у бота рандомное принятие предмета
	if(BotData.InterRandom[0] > 1)
	{
		double Chance = BotData.InterRandom[0] <= 0 ? 100.0f : (1.0f / (double)BotData.InterRandom[0]) * 100;

		char aBuf[64];
		str_format(aBuf, sizeof(aBuf), "\nChance that item he'll like [%0.2f%%]", Chance);
		Buffer.append_at(Buffer.length(), aBuf);
	}

	// показываем все информацию
	GS()->Motd(ClientID, "[Quest NPC] {STR}\n\n {STR}{STR}\n\n", TextTalk, (ShowItemNeeded ? "- - - - - - - I will need" : "\0"), Buffer.buffer());
	Buffer.clear();
	pPlayer->ClearFormatQuestText();
}

void QuestJob::QuestTableShowRequired(CPlayer* pPlayer, BotJob::QuestBotInfo& BotData)
{
	if (!pPlayer || !pPlayer->GetCharacter())
		return;

	int ClientID = pPlayer->GetCID();
	for (int i = 0; i < 2; i++)
	{
		int ItemID = BotData.ItemSearch[i];
		int Count = BotData.ItemSearchCount[i];
		if (ItemID <= 0 || Count <= 0)
			continue;

		char aBuf[128];
		str_format(aBuf, sizeof(aBuf), "%s", pPlayer->GetItem(ItemID).Info().GetName(pPlayer));
		if (BotData.InterRandom[0] > 1)
		{
			char aChanceBuf[128];
			double Chance = BotData.InterRandom[0] <= 0 ? 100.0f : (1.0f / (double)BotData.InterRandom[0]) * 100;
			str_format(aChanceBuf, sizeof(aChanceBuf), "%s [takes %0.2f%%]", aBuf, Chance);
		}
		GS()->Mmo()->Quest()->QuestTableAddItem(ClientID, aBuf, Count, ItemID, false);
	}

	// показываем текст по информации о мобах
	for (int i = 0; i < 2; i++)
	{
		int BotID = BotData.NeedMob[i];
		int Count = BotData.NeedMobCount[i];
		if (BotID <= 0 || Count <= 0 || !GS()->Mmo()->BotsData()->IsDataBotValid(BotID))
			continue;

		char aBuf[128];
		str_format(aBuf, sizeof(aBuf), "Defeat %s", BotJob::DataBot[BotID].NameBot);
		GS()->Mmo()->Quest()->QuestTableAddInfo(ClientID, aBuf, Count, QuestJob::Quests[ClientID][BotData.QuestID].MobProgress[i]);
	}

	for (int i = 0; i < 2; i++)
	{
		int ItemID = BotData.ItemGives[i];
		int Count = BotData.ItemGivesCount[i];
		if (ItemID <= 0 || Count <= 0)
			continue;

		char aBuf[128];
		str_format(aBuf, sizeof(aBuf), "Receive %s", pPlayer->GetItem(ItemID).Info().GetName(pPlayer));
		GS()->Mmo()->Quest()->QuestTableAddItem(ClientID, aBuf, Count, ItemID, true);
	}

}

void QuestJob::QuestTableAddItem(int ClientID, const char* pText, int Requires, int ItemID, bool GivingTable)
{
	CPlayer* pPlayer = GS()->GetPlayer(ClientID, true);
	if (!pPlayer || ItemID < itMoney || !GS()->CheckClient(ClientID))
		return;

	ItemJob::ItemPlayer &pSelectedItem = pPlayer->GetItem(ItemID);

	CNetMsg_Sv_AddQuestingProcessing Msg;
	Msg.m_pText = pText;
	Msg.m_pRequiresNum = Requires;
	Msg.m_pHaveNum = clamp(pSelectedItem.Count, 0, Requires);
	Msg.m_pGivingTable = GivingTable;
	StrToInts(Msg.m_pIcon, 4, pSelectedItem.Info().GetIcon());
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
	// поиск всех активных нпс
	int ClientID = pPlayer->GetCID();
	ItemJob::ItemPlayer searchItem = pPlayer->GetItem(ItemID);
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
			int needItemID = BotInfo->ItemSearch[i];
			int numNeed = BotInfo->ItemSearchCount[i];
			if (needItemID <= 0 || numNeed <= 0 || ItemID != needItemID)
				continue;

			int AvailableCount = clamp(searchItem.Count - numNeed, 0, searchItem.Count);
			return AvailableCount;
		}
	}
	return searchItem.Count;
}

void QuestJob::CreateQuestingItems(CPlayer *pPlayer, BotJob::QuestBotInfo &BotData)
{
	if (!pPlayer || !pPlayer->GetCharacter() || BotData.InterRandom[1] <= 0)
		return;

	const int ClientID = pPlayer->GetCID();
	for (CQuestItem* pHh = (CQuestItem*)GS()->m_World.FindFirst(CGameWorld::ENTTYPE_DROPQUEST); pHh; pHh = (CQuestItem*)pHh->TypeNext())
	{
		if (pHh->m_OwnerID == ClientID && pHh->m_QuestBot.QuestID == BotData.QuestID)
			return;
	}

	int Count = BotData.ItemSearch[0];
	vec2 Pos = vec2(BotData.PositionX, BotData.PositionY);
	for (int i = 0; i < Count * 2; i++)
	{
		vec2 Vel = vec2(frandom() * 40.0f - frandom() * 80.0f, frandom() * 40.0f - frandom() * 80.0f);
		float AngleForce = Vel.x * (0.15f + frandom() * 0.1f);
		new CQuestItem(&GS()->m_World, Pos, Vel, AngleForce, BotData, ClientID);
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
		QuestsData[QUID].ProgressSize = 1;

		// load talking progress size
		for (const auto& qb : BotJob::QuestBot)
		{
			if (qb.second.QuestID == QUID)
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
		int QuestID = RES->getInt("QuestID");
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

bool QuestJob::OnMessage(int MsgID, void* pRawMsg, int ClientID)
{
	CPlayer* pPlayer = GS()->m_apPlayers[ClientID];
	if (MsgID == NETMSGTYPE_CL_TALKINTERACTIVE)
	{
		if (pPlayer->m_PlayerTick[TickState::LastDialog] && pPlayer->m_PlayerTick[TickState::LastDialog] > GS()->Server()->Tick())
			return true;

		pPlayer->m_PlayerTick[TickState::LastDialog] = GS()->Server()->Tick() + (GS()->Server()->TickSpeed() / 4);
		pPlayer->SetTalking(pPlayer->GetTalkedID(), true);
		return true;
	}
	return false;
}

bool QuestJob::OnHandleMenulist(CPlayer* pPlayer, int Menulist, bool ReplaceMenu)
{
	int ClientID = pPlayer->GetCID();
	if (ReplaceMenu)
	{
		CCharacter* pChr = pPlayer->GetCharacter();
		if (!pChr || !pChr->IsAlive())
			return false;

		return false;
	}

	if (Menulist == MenuList::MENU_ADVENTURE_JOURNAL_FINISHED)
	{
		pPlayer->m_LastVoteMenu = MenuList::MENU_ADVENTURE_JOURNAL_MAIN;
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