/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#include <engine/shared/config.h>
#include <game/server/gamecontext.h>
#include <game/server/mmocore/items/dropquest.h>

#include "questing.h"

std::map < int , QuestBase::StructQuestData > QuestBase::QuestsData;
std::map < int , std::map < int , QuestBase::StructQuest > > QuestBase::Quests;

// Инициализация класса
void QuestBase::OnInitGlobal() 
{
	// загрузить все квесты
	boost::scoped_ptr<ResultSet> RES(SJK.SD("*", "tw_quests_list", "WHERE ID > '0'"));
	while(RES->next())
	{
		// получить все текстовые данные
		int QUID = RES->getInt("ID");
		str_copy(QuestsData[ QUID ].Name, RES->getString("Name").c_str(), sizeof(QuestsData[ QUID ].Name));
		str_copy(QuestsData[ QUID ].Location, RES->getString("Location").c_str(), sizeof(QuestsData[ QUID ].Location));
		str_copy(QuestsData[ QUID ].StoryLine, RES->getString("StoryLine").c_str(), sizeof(QuestsData[ QUID ].StoryLine));

		/// установить целочисленные данные
		QuestsData[ QUID ].Level = (int)RES->getInt("Level");
		QuestsData[ QUID ].Money = (int)RES->getInt("Money");
		QuestsData[ QUID ].Exp = (int)RES->getInt("Exp");

		sscanf(RES->getString("ItemRewardID").c_str(), "%d %d %d", &QuestsData[ QUID ].ItemRewardID[0], 
			&QuestsData[ QUID ].ItemRewardID[1], &QuestsData[ QUID ].ItemRewardID[2]);
		sscanf(RES->getString("ItemRewardCount").c_str(), "%d %d %d", &QuestsData[ QUID ].ItemRewardCount[0], 
			&QuestsData[ QUID ].ItemRewardCount[1], &QuestsData[ QUID ].ItemRewardCount[2]);

		// загружаем кол-во разговоров
		int ProgressSize = 1;
		for(const auto& qb : ContextBots::QuestBot)
		{
			if(qb.second.QuestID != QUID) 
				continue;

			ProgressSize++;
		}
		QuestsData[ QUID ].ProgressSize = ProgressSize;
	}
}

// Загрузка данных игрока
void QuestBase::OnInitAccount(CPlayer *pPlayer)
{
	int ClientID = pPlayer->GetCID();
	boost::scoped_ptr<ResultSet> RES(SJK.SD("*", "tw_accounts_quests", "WHERE OwnerID = '%d'", pPlayer->Acc().AuthID));
	while(RES->next())
	{
		int QuestID = RES->getInt("QuestID");
		if (!IsValidQuest(QuestID))
			continue;
		
		Quests[ ClientID ][ QuestID ].State = (int)RES->getInt("Type");
		Quests[ ClientID ][ QuestID ].Progress = (int)RES->getInt("Progress");
		Quests[ ClientID ][ QuestID ].MobProgress[0] = (int)RES->getInt("Mob1Progress");
		Quests[ ClientID ][ QuestID ].MobProgress[1] = (int)RES->getInt("Mob2Progress");
		GS()->UpdateQuestsBot(QuestID, Quests[ ClientID ][ QuestID ].Progress);
	}
}

void QuestBase::OnResetClientData(int ClientID)
{
	if (Quests.find(ClientID) != Quests.end())
	{
		std::map < int, int > m_talkcheck;
		for (const auto& qp : Quests[ClientID])
		{
			if (qp.second.State == QuestState::QUEST_FINISHED)
				continue;
			m_talkcheck[qp.first] = qp.second.Progress;
		}
		Quests.erase(ClientID);

		for (const auto& qst : m_talkcheck)
			GS()->UpdateQuestsBot(qst.first, qst.second);
	}
}

/* #########################################################################
	GET CHECK QUESTING 
######################################################################### */
// получить статистику квеста
int QuestBase::GetState(int ClientID, int QuestID) const
{
	if(IsValidQuest(QuestID, ClientID))
		return Quests[ClientID][QuestID].State;
	return QuestState::QUEST_NO_ACCEPT;
}

// получить количество story quests
int QuestBase::GetStoryCount(const char *StoryName, int QuestID) const
{
	int Count = 0;
	for(const auto& qd : QuestsData)
	{
		if(str_comp(qd.second.StoryLine, StoryName) == 0)
			Count++;
	}
	
	if(QuestID > 0)
	{
		for (auto qquest = QuestsData.find(QuestID); qquest != QuestsData.end(); qquest++)
		{
			if(str_comp(qquest->second.StoryLine, StoryName) == 0)
				Count--;
		}
	}
	return Count;
}

// получить имя типа квестов
const char *QuestBase::GetStateName(int Type) const
{
	if(Type == QuestState::QUEST_ACCEPT)
		return "Active";
	else if(Type == QuestState::QUEST_FINISHED)
		return "Finished";
	else if(Type == QuestState::QUEST_NO_ACCEPT)
		return "Not active";
	return "Unknown";
}

// Проверить выполнен ли сбор мобов в квесте
bool QuestBase::IsDefeatComplete(int ClientID, int QuestID)
{
	if(!IsValidQuest(QuestID, ClientID)) 
		return false;
	
	// получаем активного бота и проверяем убили ли всех мобов или нет
	const int playerProgress = Quests[ClientID][QuestID].Progress;
	ContextBots::QuestBotInfo FindBot = GetQuestBot(QuestID, playerProgress);
	if (!FindBot.IsActive())
		return false;

	for(int i = 4; i < 6; i++)
	{
		const int MobID = FindBot.Interactive[i];
		const int Count = FindBot.InterCount[i];
		if(MobID <= 0 || Count <= 0) 
			continue;

		if(Quests[ClientID][QuestID].MobProgress[i-4] < Count)
			return false;
	}
	return true;
}

// получить имя квеста
const char *QuestBase::GetQuestName(int QuestID) const
{
	if(IsValidQuest(QuestID))
		return QuestsData[QuestID].Name;
	return "Unknown";
}

// получить имя истории квеста
const char *QuestBase::GetStoryName(int QuestID) const
{
	if(IsValidQuest(QuestID))
		return QuestsData[QuestID].StoryLine;
	return "Unknown";
}

// Получить активного бота в квесте
ContextBots::QuestBotInfo &QuestBase::GetQuestBot(int QuestID, int Progress)
{
	for(auto& qb : ContextBots::QuestBot)
	{
		if(QuestID != qb.second.QuestID || Progress != qb.second.Progress)
			continue;

		return qb.second;
	}
	return ContextBots::QuestBot[-1];
}

// Узнать активный ли бот
bool QuestBase::IsActiveQuestBot(int QuestID, int Progress)
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
		ContextBots::QuestBotInfo FindBot = GetQuestBot(QuestID, Progress);
		if(!FindBot.IsActive()) 
			continue;

		return true;
	}
	return false;
}

/* #########################################################################
	FUNCTIONS QUESTING 
######################################################################### */
void QuestBase::ShowQuestID(CPlayer *pPlayer, int QuestID)
{
	int ClientID = pPlayer->GetCID();
	int HideID = NUMHIDEMENU + 10500 + QuestID;

	StructQuestData activeQuestData = QuestsData[QuestID];
	int CountQuest = GetStoryCount(activeQuestData.StoryLine);
	int LineQuest = GetStoryCount(activeQuestData.StoryLine, QuestID)+1;
	GS()->AVH(ClientID, HideID, LIGHT_GOLDEN_COLOR, "[{INT}/{INT} {STR}] {STR}",
		&LineQuest, &CountQuest, activeQuestData.StoryLine, activeQuestData.Name);	
	GS()->AVM(ClientID, "null", NOPE, HideID, "Location: {STR}", activeQuestData.Location);
	GS()->AVM(ClientID, "null", NOPE, HideID, "You will receive a reward");
	GS()->AVM(ClientID, "null", NOPE, HideID, "Gold: {INT} Exp: {INT}", &activeQuestData.Money, &activeQuestData.Exp);

	dynamic_string Buffer;
	for(int i = 0; i < 3; i++)
	{
		if(activeQuestData.ItemRewardID[i] > 0 && activeQuestData.ItemRewardCount[i] > 0) 
		{
			char aBuf[32];
			str_format(aBuf, sizeof(aBuf), "%sx%d ", GS()->GetItemInfo(activeQuestData.ItemRewardID[i]).GetName(pPlayer), activeQuestData.ItemRewardCount[i]);
			Buffer.append_at(Buffer.length(), aBuf);
		}
	}
	if(Buffer.length()) 
		GS()->AVM(ClientID, "null", NOPE, HideID, "{STR}", Buffer.buffer()); 

	Buffer.clear();
	GS()->AVM(ClientID, "null", NOPE, HideID, " ");
}

// завершить квест
void QuestBase::FinishQuest(CPlayer *pPlayer, int QuestID)
{
	int ClientID = pPlayer->GetCID();
	if(!IsValidQuest(QuestID, ClientID))
		return;
	
	// установить статистику квеста
	Quests[ClientID][QuestID].State = QuestState::QUEST_FINISHED;
	SJK.UD("tw_accounts_quests", "Type = '%d' WHERE QuestID = '%d' AND OwnerID = '%d'", Quests[ClientID][QuestID].State, QuestID, pPlayer->Acc().AuthID);

	// выдать награды и написать о завершении
	StructQuestData finishQuestData = QuestsData[QuestID];
	for(int i = 0; i < 3; i++)
	{
		if(finishQuestData.ItemRewardID[i] <= 0 || finishQuestData.ItemRewardCount[i] <= 0) 
			continue;
		
		pPlayer->GetItem(finishQuestData.ItemRewardID[i]).Add(finishQuestData.ItemRewardCount[i]);
	}
	pPlayer->AddMoney(finishQuestData.Money);
	pPlayer->AddExp(finishQuestData.Exp);

	GS()->Chat(-1, "{STR} completed quest [{STR} {STR}]", GS()->Server()->ClientName(ClientID), finishQuestData.StoryLine, finishQuestData.Name);
	GS()->ChatDiscord(false, DC_PLAYER_INFO, GS()->Server()->ClientName(ClientID), "Completed quest [{STR} {STR}]", finishQuestData.StoryLine, finishQuestData.Name);
	Job()->SaveAccount(pPlayer, SAVESTATS);
}

// проверить прогресс по предметам что требует бот
bool QuestBase::IsCollectItemComplete(CPlayer *pPlayer, ContextBots::QuestBotInfo &BotData, bool Gived, bool Interactive)
{
	if(!pPlayer || !BotData.IsActive())
		return false;

	// проверить выданые предметы или выдать
	if(Gived)
	{
		// выдать предмет
		if(Interactive)
		{
			for(int i = 2; i < 4; i++)
			{
				int ItemID = BotData.Interactive[i];
				int Count = BotData.InterCount[i] - pPlayer->GetItem(ItemID).Count;
				if(ItemID > 0 && Count > 0) 
					pPlayer->GetItem(ItemID).Add(Count);
			}
			return true;
		}

		// проверить выдавание предмета
		for(int i = 2; i < 4; i++)
		{
			int ItemID = BotData.Interactive[i];
			int Count = BotData.InterCount[i];
			if(ItemID > 0 && pPlayer->GetItem(ItemID).Count < Count)
				return false;
		}
		return true;
	}

	// забрать предмет
	if(Interactive)
	{
		for(int i = 0; i < 2; i++)
		{
			int ItemID = BotData.Interactive[i];
			int Count = BotData.InterCount[i];
			if(ItemID > 0 && Count > 0) 
				pPlayer->GetItem(ItemID).Remove(Count);
		}
		return true;
	}

	// проверить предмет
	for(int i = 0; i < 2; i++)
	{
		if(BotData.Interactive[i] <= 0 || BotData.InterCount[i] <= 0) continue;
		if(pPlayer->GetItem(BotData.Interactive[i]).Count < BotData.InterCount[i])
			return false;
	}
	return true;
}

// Разговор повышение прогресса
void QuestBase::AddProgress(CPlayer *pPlayer, int QuestID)
{
	int ClientID = pPlayer->GetCID();
	if(!IsValidQuest(QuestID, ClientID) || !pPlayer->GetCharacter()) 
		return;

	StructQuest &talkQuestPlayer = Quests[ClientID][QuestID];
	talkQuestPlayer.Progress += 1;
	talkQuestPlayer.MobProgress[0] = 0;
	talkQuestPlayer.MobProgress[1] = 0;
	bool FinishedProgress = (talkQuestPlayer.Progress >= QuestsData[QuestID].ProgressSize);
	if (FinishedProgress)
	{
		AutoStartNextQuest(pPlayer, QuestID);
		GS()->VResetVotes(ClientID, ADVENTUREJOURNAL);
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
bool QuestBase::AcceptQuest(int QuestID, CPlayer* pPlayer)
{
	if (!pPlayer || !IsValidQuest(QuestID) || GetState(pPlayer->GetCID(), QuestID) >= QuestState::QUEST_ACCEPT)
		return false;

	// принимаем квест
	const int ClientID = pPlayer->GetCID();
	Quests[ClientID][QuestID].Progress = 1;
	Quests[ClientID][QuestID].State = QuestState::QUEST_ACCEPT;
	GS()->UpdateQuestsBot(QuestID, Quests[ClientID][QuestID].Progress);

	SJK.ID("tw_accounts_quests", "(QuestID, OwnerID, Type) VALUES ('%d', '%d', '%d')", QuestID, pPlayer->Acc().AuthID, QuestState::QUEST_ACCEPT);
	GS()->Chat(ClientID, "Accepted the quest [{STR}]", QuestsData[QuestID].Name);
	GS()->Chat(ClientID, "You will receive a reward Gold {INT}, Experience {INT}", &QuestsData[QuestID].Money, &QuestsData[QuestID].Exp);
	pPlayer->GetCharacter()->CreateQuestsStep(QuestID);
	return true;
}

// действие над квестом
bool QuestBase::InteractiveQuestNPC(CPlayer* pPlayer, ContextBots::QuestBotInfo& BotData, bool LastDialog)
{
	if (!pPlayer || !pPlayer->GetCharacter() || !BotData.IsActive())
		return false;

	// проверяем собрали предметы и убили ли всех ботов
	const int ClientID = pPlayer->GetCID();
	const int QuestID = BotData.QuestID;

	if (!IsCollectItemComplete(pPlayer, BotData, false) || !IsDefeatComplete(ClientID, QuestID) || pPlayer->Acc().Level < QuestsData[QuestID].Level)
	{
		GS()->Chat(ClientID, "Not all criteria to complete!");
		return false;
	}

	// интерактив рандомно понравиться ли предмет даст или нет
	int RandomGetItem = BotData.InterRandom[0];
	if (RandomGetItem > 1 && random_int() % RandomGetItem != 0)
	{
		// забираем предмет
		GS()->Chat(ClientID, "[{STR} NPC] I didn't like it, bring me another one!", BotData.Name);
		IsCollectItemComplete(pPlayer, BotData, false, true);
		return false;
	}

	if (!LastDialog)
		return true;

	// проверяем и выдаем потом
	IsCollectItemComplete(pPlayer, BotData, true, true);

	// забираем проверяем не является ли тип рандомно взять предмет
	if (BotData.InterRandom[2] <= 0 || RandomGetItem > 0)
		IsCollectItemComplete(pPlayer, BotData, false, true);

	GS()->VResetVotes(ClientID, ADVENTUREJOURNAL);
	GS()->Mmo()->Quest()->AddProgress(pPlayer, QuestID);
	return true;
}

// Прогресс мобов
void QuestBase::AddMobProgress(CPlayer* pPlayer, int BotID)
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
		ContextBots::QuestBotInfo FindBot = GetQuestBot(questID, playerProgress);
		if (!FindBot.IsActive())
			continue;

		// ищим нужен ли такой Моб
		for (int i = 4; i < 6; i++)
		{
			// проверяем если равен и прогресс меньше чем требуется
			if (BotID != FindBot.Interactive[i] || qp.second.MobProgress[i - 4] >= FindBot.InterCount[i])
				continue;

			// проверяем дабы чтобы писать о прогрессе всегда а не только 1 раз при перезаходах
			qp.second.MobProgress[i - 4]++;
			if (qp.second.MobProgress[i - 4] >= FindBot.InterCount[i])
				GS()->Chat(ClientID, "You killed {STR} the required amount for NPC {STR}", ContextBots::DataBot[BotID].NameBot, FindBot.Name);

			// обновляем таблицу
			SJK.UD("tw_accounts_quests", "Mob1Progress = '%d', Mob2Progress = '%d' WHERE QuestID = '%d' AND OwnerID = '%d'",
				qp.second.MobProgress[0], qp.second.MobProgress[1], questID, pPlayer->Acc().AuthID);
			break;
		}
	}
}

// автозавершение и автопринятие
void QuestBase::AutoStartNextQuest(CPlayer* pPlayer, int QuestID)
{
	// если пассивный то без автозавершения
	int clientID = pPlayer->GetCID();

	// завершаем квест
	FinishQuest(pPlayer, QuestID);

	// проверяем если есть следующий квест то продолжаем сюжетку только сюжетные квесты
	int NextQuestID = QuestID + 1;
	if (!IsValidQuest(NextQuestID) || str_comp(QuestsData[QuestID].StoryLine, QuestsData[NextQuestID].StoryLine) != 0)
	{
		GS()->Chat(clientID, "At the moment there is no continuation of '{STR}' story", QuestsData[QuestID].StoryLine);
		return;
	}

	// прнимаем квест и пишем информацию о следующем квесте
	AcceptQuest(NextQuestID, pPlayer);
	GS()->Chat(clientID, "You can see the details in vote 'Adventure Journal'");
	GS()->VResetVotes(clientID, ADVENTUREJOURNAL);
}

// обновить стрелки направление
void QuestBase::UpdateArrowStep(int ClientID)
{
	CPlayer* pPlayer = GS()->GetPlayer(ClientID, true, true);
	if (!pPlayer)
		return;

	for (const auto qp : Quests[ClientID])
	{
		if (qp.second.State != QuestState::QUEST_ACCEPT)
			continue;

		pPlayer->GetCharacter()->CreateQuestsStep(qp.first);
	}
}

// показать квесты
void QuestBase::ShowQuestList(CPlayer* pPlayer, int StateQuest)
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
void QuestBase::ShowFullQuestLift(CPlayer* pPlayer)
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
	GS()->AVM(ClientID, "MENU", FINISHQUESTMENU, NOPE, "List of completed quests");
}

// Адвентур активные нпс показ информации
bool QuestBase::ShowAdventureActiveNPC(CPlayer* pPlayer)
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
		ContextBots::QuestBotInfo& BotInfo = GetQuestBot(qq.first, qq.second.Progress);
		if (!BotInfo.IsActive())
			continue;

		// если нашли выводим информацию
		int HideID = (NUMHIDEMENU + 12500 + BotInfo.QuestID);
		int PosX = BotInfo.PositionX / 32, PosY = BotInfo.PositionY / 32;
		GS()->AVH(clientID, HideID, LIGHT_BLUE_COLOR, "[{STR}] {STR} {STR}(x:{INT} y:{INT})", GetStoryName(qq.first), BotInfo.Name, GS()->Server()->GetWorldName(BotInfo.WorldID), &PosX, &PosY);

		// проверяем требуемые мобы
		bool interactiveNeed = false;
		for (int i = 4; i < 6; i++)
		{
			int botID = BotInfo.Interactive[i], killNeed = BotInfo.InterCount[i];
			if (botID <= 0 || killNeed <= 0 || !Job()->BotsData()->IsDataBotValid(botID))
				continue;

			GS()->AVM(clientID, "null", NOPE, HideID, "- Defeat {STR} [{INT}/{INT}]", ContextBots::DataBot[botID].NameBot, &qq.second.MobProgress[i - 4], &killNeed);
			interactiveNeed = true;
		}

		// проверяем требуемые предметы
		for (int i = 0; i < 2; i++)
		{
			int itemID = BotInfo.Interactive[i], numNeed = BotInfo.InterCount[i];
			if (itemID <= 0 || numNeed <= 0)
				continue;

			ItemSql::ItemPlayer searchItem = pPlayer->GetItem(itemID);
			int ownCount = clamp(searchItem.Count, 0, numNeed);

			GS()->AVMI(clientID, searchItem.Info().GetIcon(), "null", NOPE, HideID, "- Item {STR} [{INT}/{INT}]", searchItem.Info().GetName(pPlayer), &ownCount, &numNeed);
			interactiveNeed = true;
		}

		// проверяем что даст
		for (int i = 2; i < 4; i++)
		{
			int itemID = BotInfo.Interactive[i], getCount = BotInfo.InterCount[i];
			if (itemID <= 0 || getCount <= 0)
				continue;

			ItemSql::ItemInformation GivedInfItem = GS()->GetItemInfo(itemID);
			GS()->AVMI(clientID, GivedInfItem.GetIcon(), "null", NOPE, HideID, "- Gives {STR}x{INT}", GivedInfItem.GetName(pPlayer), &getCount);
			interactiveNeed = true;
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

// Показать разговор информацию как motd
void QuestBase::ShowQuestRequired(CPlayer *pPlayer, ContextBots::QuestBotInfo &BotData, const char* TextTalk)
{
	if(!pPlayer || !pPlayer->GetCharacter() || !BotData.IsActive()) 
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
			int ItemID = BotData.Interactive[i];
			int Count = BotData.InterCount[i];
			if(ItemID <= 0 || Count <= 0) 
				continue;

			char aBuf[48];
			ItemSql::ItemPlayer &PlQuestItem = pPlayer->GetItem(ItemID);
			str_format(aBuf, sizeof(aBuf), "\n- Item %s [%d/%d]", PlQuestItem.Info().GetName(pPlayer), PlQuestItem.Count, Count);
			Buffer.append_at(Buffer.length(), aBuf);
			ShowItemNeeded = true;
		}
	}

	// показываем текст по информации о мобах
	{
		for(int i = 4; i < 6; i++)
		{
			int BotID = BotData.Interactive[i];
			int Count = BotData.InterCount[i];
			if(BotID <= 0 || Count <= 0 || !Job()->BotsData()->IsDataBotValid(BotID)) 
				continue;

			char aBuf[48];
			str_format(aBuf, sizeof(aBuf), "\n- Defeat %s [%d/%d]", ContextBots::DataBot[BotID].NameBot, Quests[ClientID][QuestID].MobProgress[i-4], Count);
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

	// маленький уровень
	if(pPlayer->Acc().Level < QuestsData[QuestID].Level)
	{
		char aBuf[64];
		str_format(aBuf, sizeof(aBuf), "\n\n# You are too weak, for continuation\n# Required level: %dLVL", QuestsData[QuestID].Level);
		Buffer.append_at(Buffer.length(), aBuf);
	}

	// показываем все информацию
	GS()->Motd(ClientID, "[Quest NPC] {STR}\n\n {STR}{STR}\n\n", TextTalk, (ShowItemNeeded ? "- - - - - - - I will need" : "\0"), Buffer.buffer());
	Buffer.clear();
	pPlayer->ClearFormatQuestText();
}

// Показать разговор информацию как motd
void QuestBase::QuestTableShowRequired(CPlayer* pPlayer, ContextBots::QuestBotInfo& BotData)
{
	if (!pPlayer || !pPlayer->GetCharacter() || !BotData.IsActive())
		return;

	int ClientID = pPlayer->GetCID();
	for (int i = 0; i < 2; i++)
	{
		int ItemID = BotData.Interactive[i];
		int Count = BotData.InterCount[i];
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
		GS()->Mmo()->Quest()->QuestTableAddItem(ClientID, aBuf, Count, ItemID);
	}

	// показываем текст по информации о мобах
	for (int i = 4; i < 6; i++)
	{
		int BotID = BotData.Interactive[i];
		int Count = BotData.InterCount[i];
		if (BotID <= 0 || Count <= 0 || !GS()->Mmo()->BotsData()->IsDataBotValid(BotID))
			continue;

		char aBuf[128];
		str_format(aBuf, sizeof(aBuf), "Defeat %s", ContextBots::DataBot[BotID].NameBot);
		GS()->Mmo()->Quest()->QuestTableAddInfo(ClientID, aBuf, Count,
			QuestBase::Quests[ClientID][BotData.QuestID].MobProgress[i - 4]);
	}
}

// Парсинг голосованний 
bool QuestBase::OnParseVotingMenu(CPlayer *pPlayer, const char *CMD, const int VoteID, const int VoteID2, int Get, const char *GetText)
{
	return false;
}

bool QuestBase::OnMessage(int MsgID, void *pRawMsg, int ClientID)
{
	CPlayer *pPlayer = GS()->m_apPlayers[ClientID];
	if (MsgID == NETMSGTYPE_CL_TALKINTERACTIVE)
	{
		int TalkedID = pPlayer->GetTalkedID();
		pPlayer->SetTalking(TalkedID, true);

		return true;
	}
	return false;
}

void QuestBase::QuestTableAddItem(int ClientID, const char* pText, int Requires, int ItemID)
{
	CPlayer* pPlayer = GS()->GetPlayer(ClientID, true);
	if (!pPlayer || ItemID < itMoney || !GS()->CheckClient(ClientID))
		return;

	ItemSql::ItemPlayer SelectedItem = pPlayer->GetItem(ItemID);

	CNetMsg_Sv_AddQuestingProcessing Msg;
	Msg.m_pText = pText;
	Msg.m_pRequiresNum = Requires;
	Msg.m_pHaveNum = clamp(SelectedItem.Count, 0, Requires);
	StrToInts(Msg.m_pIcon, 4, SelectedItem.Info().GetIcon());
	GS()->Server()->SendPackMsg(&Msg, MSGFLAG_VITAL, ClientID);
}

void QuestBase::QuestTableAddInfo(int ClientID, const char *pText, int Requires, int Have)
{
	if (ClientID < 0 || ClientID >= MAX_PLAYERS || !GS()->CheckClient(ClientID))
		return;

	CNetMsg_Sv_AddQuestingProcessing Msg;
	Msg.m_pText = pText;
	Msg.m_pRequiresNum = Requires;
	Msg.m_pHaveNum = clamp(Have, 0, Requires);
	StrToInts(Msg.m_pIcon, 4, "hammer");
	GS()->Server()->SendPackMsg(&Msg, MSGFLAG_VITAL, ClientID);
}

void QuestBase::QuestTableClear(int ClientID)
{
	if (ClientID < 0 || ClientID >= MAX_PLAYERS || !GS()->CheckClient(ClientID))
		return;
	
	CNetMsg_Sv_ClearQuestingProcessing Msg;
	GS()->Server()->SendPackMsg(&Msg, MSGFLAG_VITAL, ClientID);
}

int QuestBase::QuestingAllowedItemsCount(CPlayer *pPlayer, int ItemID)
{
	// поиск всех активных нпс
	int ClientID = pPlayer->GetCID();
	ItemSql::ItemPlayer searchItem = pPlayer->GetItem(ItemID);
	for (const auto& qq : Quests[ClientID])
	{
		if (qq.second.State != QuestState::QUEST_ACCEPT)
			continue;

		// проверяем бота есть или нет активный по квесту
		ContextBots::QuestBotInfo& BotInfo = GetQuestBot(qq.first, qq.second.Progress);
		if (!BotInfo.IsActive())
			continue;

		// проверяем требуемые предметы
		for (int i = 0; i < 2; i++)
		{
			int needItemID = BotInfo.Interactive[i], numNeed = BotInfo.InterCount[i];
			if (needItemID <= 0 || numNeed <= 0 || ItemID != needItemID)
				continue;

			int AvailableCount = clamp(searchItem.Count - numNeed, 0, searchItem.Count);
			return AvailableCount;
		}
	}
	return searchItem.Count;
}

void QuestBase::CreateQuestingItems(CPlayer *pPlayer, ContextBots::QuestBotInfo &BotData)
{
	if (!pPlayer || !pPlayer->GetCharacter() || !BotData.IsActive() || BotData.InterRandom[1] <= 0)
		return;

	// проверяем собрали предметы и убили ли всех ботов
	const int ClientID = pPlayer->GetCID();
	for (CQuestItem* pHh = (CQuestItem*)GS()->m_World.FindFirst(CGameWorld::ENTTYPE_DROPQUEST); pHh; pHh = (CQuestItem*)pHh->TypeNext())
	{
		if (pHh->m_OwnerID != ClientID || pHh->m_QuestBot.QuestID != BotData.QuestID)
			continue;
		return;
	}

	// создаем предметы
	int Count = BotData.InterCount[0];
	vec2 Pos = vec2(BotData.PositionX, BotData.PositionY);
	for (int i = 0; i < Count * 2; i++)
	{
		vec2 Vel = vec2(frandom() * 40.0f - frandom() * 80.0f, frandom() * 40.0f - frandom() * 80.0f);
		float AngleForce = Vel.x * (0.15f + frandom() * 0.1f);
		new CQuestItem(&GS()->m_World, Pos, Vel, AngleForce, BotData, ClientID);
	}
}