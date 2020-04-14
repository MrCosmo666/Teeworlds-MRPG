/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#include <engine/shared/config.h>
#include <game/server/gamecontext.h>
#include "questing.h"

std::map < int , QuestBase::StructQuestData > QuestBase::QuestsData;
std::map < int , std::map < int , QuestBase::StructQuest > > QuestBase::Quests;

// Если завершен квест
bool QuestBase::IsComplecte(int ClientID, int QuestID) const
{
	if(IsValidQuest(QuestID, ClientID) &&
		Quests[ClientID][QuestID].TalkProgress == (QuestsData[QuestID].TalkCount - 1))
		return true;
	return false;
}

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
		int TalkCount = 1;
		for(const auto& qb : ContextBots::QuestBot)
		{
			if(qb.second.QuestID != QUID) 
				continue;

			TalkCount++;
		}
		QuestsData[ QUID ].TalkCount = TalkCount;
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
		if(IsValidQuest(QuestID))
		{
			Quests[ ClientID ][ QuestID ].Type = (int)RES->getInt("Type");
			Quests[ ClientID ][ QuestID ].TalkProgress = (int)RES->getInt("TalkProgress");
			Quests[ ClientID ][ QuestID ].MobProgress[0] = (int)RES->getInt("Mob1Progress");
			Quests[ ClientID ][ QuestID ].MobProgress[1] = (int)RES->getInt("Mob2Progress");
			GS()->ClearQuestsBot(QuestID, Quests[ ClientID ][ QuestID ].TalkProgress);
		}
	}
}

/* #########################################################################
	GET CHECK QUESTING 
######################################################################### */
// получить статистику квеста
int QuestBase::GetQuestState(int ClientID, int QuestID) const
{
	if(IsValidQuest(QuestID, ClientID))
		return Quests[ClientID][QuestID].Type;

	return QUESTNOACCEPT;
}

// получить количество story quests
int QuestBase::GetStoryCountQuest(const char *StoryName, int QuestID) const
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
const char *QuestBase::QuestState(int Type) const
{
	if(Type == QUESTACCEPT) return "Active";
	else if(Type == QUESTFINISHED) return "Finished";
	else if(Type == QUESTNOACCEPT) return "Not active";
	else return "Unknown";
}

// Проверить выполнен ли сбор мобов в квесте
bool QuestBase::CheckMobProgress(int ClientID, int QuestID)
{
	if(!IsValidQuest(QuestID, ClientID)) 
		return false;
	
	// получаем активного бота и проверяем убили ли всех мобов или нет
	int playerTalkProgress = Quests[ClientID][QuestID].TalkProgress;
	ContextBots::QuestBotInfo FindBot = GetQuestBot(QuestID, playerTalkProgress);
	if(FindBot.IsActive())
	{
		for(int i = 4; i < 6; i++)
		{
			const int MobID = FindBot.Interactive[i], Count = FindBot.InterCount[i];
			if(MobID <= 0 || Count <= 0) 
				continue;

			if(Quests[ClientID][QuestID].MobProgress[i-4] < Count)
				return false;
		}
	}
	return true;
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
bool QuestBase::CheckActiveBot(int QuestID, int Progress)
{
	for(int i = 0; i < MAX_PLAYERS; i++)
	{
		if(!IsValidQuest(QuestID, i)) 
			continue;

		// -- пропускаем завершеный квест		
		if(Quests[i][QuestID].Type != QUESTACCEPT || Quests[i][QuestID].TalkProgress != Progress) 
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
// показать квест по id
void QuestBase::ShowQuestID(CPlayer *pPlayer, int QuestID, bool Passive)
{
	int ClientID = pPlayer->GetCID();
	int HideID = NUMHIDEMENU + 10500 + QuestID;
	StructQuestData activeQuestData = QuestsData[QuestID];

	// тип пасивный или история
	if(Passive)
	{
		GS()->AVH(ClientID, HideID, vec3(18,3,35), "Basic {STR} {STR}", QuestsData[QuestID].Name, (IsComplecte(ClientID, QuestID) ?  "[OK]" : "\0"));	
	}
	else
	{
		int CountQuest = GetStoryCountQuest(activeQuestData.StoryLine);
		int LineQuest = GetStoryCountQuest(activeQuestData.StoryLine, QuestID)+1;
		GS()->AVH(ClientID, HideID, vec3(18,3,35), "[{INT}/{INT} {STR}] {STR} {STR}", 
			&LineQuest, &CountQuest, activeQuestData.StoryLine, activeQuestData.Name, (IsComplecte(ClientID, QuestID) ?  "[OK]" : "\0"));	
	}

	// информация о квесте
	GS()->AVM(ClientID, "null", NOPE, HideID, "Location: {STR}", activeQuestData.Location);

	{ // подсчет предметов награды
		GS()->AVM(ClientID, "null", NOPE, HideID, "You will receive a reward");
	
		char aBuf[32];
		dynamic_string Buffer;
		for(int i = 0; i < 3; i++)
		{
			if(activeQuestData.ItemRewardID[i] > 0 && activeQuestData.ItemRewardCount[i] > 0) {
				str_format(aBuf, sizeof(aBuf), "%sx%d ", GS()->GetItemInfo(activeQuestData.ItemRewardID[i]).GetName(pPlayer), activeQuestData.ItemRewardCount[i]);
				Buffer.append_at(Buffer.length(), aBuf);
			}
		}
		if(Buffer.length()) 
		{	
			GS()->AVM(ClientID, "null", NOPE, HideID, "{STR}", Buffer.buffer()); 
		}
		Buffer.clear();

		GS()->AVM(ClientID, "null", NOPE, HideID, "Gold: {INT} Exp: {INT}", &activeQuestData.Money, &activeQuestData.Exp);
	}

	// если квест не принят
	if(GetQuestState(ClientID, QuestID) == QUESTNOACCEPT)
	{
		{ // либо принять либо маленький уровень
			int Level = activeQuestData.Level;
			if(pPlayer->Acc().Level >= Level)
				GS()->AVM(ClientID, "ACCEPTQUEST", QuestID, HideID, "Accept {STR}", activeQuestData.Name);			
			else
				GS()->AVM(ClientID, "null", NOPE, HideID, "☒ Level required {INT}", &Level);
		}
	}
	GS()->AVM(ClientID, "null", NOPE, HideID, " ");
}

// завершить квест
void QuestBase::FinishQuest(CPlayer *pPlayer, int QuestID)
{
	int ClientID = pPlayer->GetCID();
	if(!IsValidQuest(QuestID, ClientID))
		return;
	
	StructQuestData finishQuestData = QuestsData[QuestID];
	StructQuest &finishQuestPlayer = Quests[ClientID][QuestID];

	// установить статистику квеста
	finishQuestPlayer.Type = QUESTFINISHED;
	SJK.UD("tw_accounts_quests", "Type = '%d' WHERE QuestID = '%d' AND OwnerID = '%d'", QUESTFINISHED, QuestID, pPlayer->Acc().AuthID);

	// выдать награды и написать о завершении
	for(int i = 0; i < 3; i++)
	{
		if(finishQuestData.ItemRewardID[i] <= 0 || finishQuestData.ItemRewardCount[i] <= 0) continue;
		pPlayer->GetItem(finishQuestData.ItemRewardID[i]).Add(finishQuestData.ItemRewardCount[i]);
	}

	pPlayer->AddMoney(finishQuestData.Money);
	pPlayer->AddExp(finishQuestData.Exp);
	GS()->Chat(-1, "{STR} completed quest [{STR} {STR}]", finishQuestData.StoryLine, GS()->Server()->ClientName(ClientID), finishQuestData.Name);			
	GS()->ChatDiscord(false, DC_PLAYER_INFO, GS()->Server()->ClientName(ClientID), "Completed quest [{STR} {STR}]", finishQuestData.StoryLine, finishQuestData.Name);

	Job()->SaveAccount(pPlayer, SAVESTATS);

	if(pPlayer->GetCharacter()) 
		pPlayer->GetCharacter()->FinishQuestStep(QuestID);
}

// проверить прогресс по предметам что требует бот
bool QuestBase::MultiQuestNPC(CPlayer *pPlayer, ContextBots::QuestBotInfo &BotData, bool Gived, bool Interactive)
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
				if(ItemID > 0 && Count > 0) pPlayer->GetItem(ItemID).Add(Count);
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
			if(ItemID > 0 && Count > 0) pPlayer->GetItem(ItemID).Remove(Count);
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
void QuestBase::TalkProgress(CPlayer *pPlayer, int QuestID)
{
	int ClientID = pPlayer->GetCID();
	if(!IsValidQuest(QuestID, ClientID)) 
		return;

	StructQuest &talkQuestPlayer = Quests[ClientID][QuestID];
	talkQuestPlayer.TalkProgress += 1;
	talkQuestPlayer.MobProgress[0] = 0;
	talkQuestPlayer.MobProgress[1] = 0;

	{ // очистка прогресса ботов
		int OldTalkProgress = talkQuestPlayer.TalkProgress - 1;
		int NewTalkProgress = talkQuestPlayer.TalkProgress;

		GS()->ClearQuestsBot(QuestID, OldTalkProgress);
		GS()->ClearQuestsBot(QuestID, NewTalkProgress);
	}

	bool LastTalkingProgress = (talkQuestPlayer.TalkProgress >= QuestsData[QuestID].TalkCount);
	if(!LastTalkingProgress)
	{
		SJK.UD("tw_accounts_quests", "TalkProgress = '%d', Mob1Progress = '%d', Mob2Progress = '%d' WHERE QuestID = '%d' AND OwnerID = '%d'",
			talkQuestPlayer.TalkProgress, talkQuestPlayer.MobProgress[0], 
			talkQuestPlayer.MobProgress[1], QuestID, pPlayer->Acc().AuthID);
	}
	Job()->Quest()->CheckQuest(pPlayer);
	
	// обновить направление поиск пути к квесту
	pPlayer->GetCharacter()->FinishQuestStep(QuestID);
	pPlayer->GetCharacter()->CreateQuestsSteps();
}

// показать квесты
void QuestBase::ShowQuestList(CPlayer *pPlayer, int StateQuest)
{ 
	char storyLineSave[32];
	bool foundQuests = false;

	int ClientID = pPlayer->GetCID();
	pPlayer->m_Colored = { 10,8,50 };
	GS()->AVL(ClientID, "null", "★ {STR} quests", QuestState(StateQuest));
	
	for(const auto& qd : QuestsData)
	{
		int questID = qd.first;
		if(GetQuestState(ClientID, questID) != StateQuest)
			continue;
			
		if(str_comp(qd.second.StoryLine, "Passive") == 0)
		{
			ShowQuestID(pPlayer, questID, true);
			foundQuests = true;
			continue;
		}
		
		if(StateQuest != QUESTFINISHED)
		{
			if(str_comp(storyLineSave, qd.second.StoryLine) == 0) 
				continue;
			
			str_copy(storyLineSave, qd.second.StoryLine, sizeof(storyLineSave));
		}
		ShowQuestID(pPlayer, questID);
		foundQuests = true;
	}
	
	if(!foundQuests)
	{	
		pPlayer->m_Colored = { 9,2,17 };
		GS()->AV(ClientID, "null", "This list is empty");		
	}
	GS()->AV(ClientID, "null", "");
}

// позать все квесты весь список
void QuestBase::ShowFullQuestLift(CPlayer *pPlayer)
{
	// показываем всех нпс активных
	int ClientID = pPlayer->GetCID();
	if(!ShowAdventureActiveNPC(pPlayer))
	{
		pPlayer->m_Colored = { 15, 35, 10 };
		GS()->AVM(ClientID, "null", NOPE, NOPE, "In current quests there is no interaction with NPC");
	}
	GS()->AV(ClientID, "null", "");

	// показываем лист квестов
	ShowQuestList(pPlayer, QUESTACCEPT);
	ShowQuestList(pPlayer, QUESTNOACCEPT);

	// показываем меню завершенных
	pPlayer->m_Colored = { 8,8,40 };
	GS()->AVM(ClientID, "MENU", FINISHQUESTMENU, NOPE, "List of completed quests");
}

// проверяем выполнение квеста
void QuestBase::CheckQuest(CPlayer *pPlayer)
{
	int ClientID = pPlayer->GetCID();
	for(const auto& qData : QuestsData)
	{
		// проверяем прогресс квеста по разговорам с нпс
		// получаем данные для справнения требуемое с тем что собрали
		int QuestID = qData.first;
		if(!IsValidQuest(QuestID, ClientID)) 
			continue;

		if(Quests[ClientID][QuestID].Type != QUESTACCEPT)
			continue;

		// создаем дроп элементов если имеются
		int playerTalkProgress = Quests[ClientID][QuestID].TalkProgress;
		ContextBots::QuestBotInfo &FindBot = GetQuestBot(QuestID, playerTalkProgress);
		if(FindBot.IsActive()) 
			GS()->CreateDropQuest(FindBot, ClientID); 

		if(qData.second.TalkCount > 1 && qData.second.TalkCount != Quests[ClientID][QuestID].TalkProgress) 
			continue;

		// автозавершение и автостарт нового квеста
		AutoStartNextQuest(pPlayer, QuestID);
		GS()->VResetVotes(ClientID, ADVENTUREJOURNAL);
	}
}

// Показать разговор информацию как motd
void QuestBase::ShowQuestInformation(CPlayer *pPlayer, ContextBots::QuestBotInfo &BotData, const char* TextTalk)
{
	if(!pPlayer || !pPlayer->GetCharacter() || !BotData.IsActive()) 
		return;

	// показываем текст завершения квеста
	int ClientID = pPlayer->GetCID();
	int QuestID = BotData.QuestID;

	// перекидываем на клиент если проверен
	if (GS()->CheckClient(ClientID))
	{
		QuestTableShowInformation(pPlayer, BotData);
		return;
	}

	// показываем по информация о предметах
	dynamic_string Buffer;
	bool ShowItemNeeded = false;
	{
		for(int i = 0; i < 2; i++) 
		{
			int ItemID = BotData.Interactive[i];
			int Count = BotData.InterCount[i];
			if(ItemID <= 0 || Count <= 0) continue;

			char aBuf[48];
			ItemSql::ItemPlayer &PlQuestItem = pPlayer->GetItem(ItemID);
			str_format(aBuf, sizeof(aBuf), "\n- Item %s [%d/%d]", PlQuestItem.Info().GetName(pPlayer), PlQuestItem.Count, Count);
			Buffer.append_at(Buffer.length(), aBuf);
			ShowItemNeeded = true;
		}
	}

	// показываем текст по информации о мобах
	bool ShowMobNeeded = false;
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
			ShowMobNeeded = true;
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
	GS()->Motd(ClientID, "[Quest NPC] {STR}\n\n {STR}{STR}\n\n", 
		TextTalk, (ShowItemNeeded ? "- - - - - - - I will need" : "\0"), Buffer.buffer());
	Buffer.clear();
	pPlayer->ClearFormatQuestText();
	GS()->SBL(ClientID, 99999, 8, _("PRESS (F4) FOR CONTINUE TALK!"), NULL);
}

// Принять квест с ID
bool QuestBase::AcceptQuest(int QuestID, CPlayer *pPlayer)
{
	// проверяем квест можно ли принять более одного или нет
	const int ClientID = pPlayer->GetCID();
	for(const auto& qp : Quests[ClientID])
	{
		int QuestSort = qp.first;
		if(qp.second.Type != QUESTACCEPT) 
			continue;
		
		if(str_comp(QuestsData[QuestSort].StoryLine, "Passive") != 0 && 
			str_comp(QuestsData[QuestSort].StoryLine, QuestsData[QuestID].StoryLine) == 0)
		{
			GS()->Chat(ClientID, "You can only take one '{STR}' quest.", QuestsData[QuestSort].StoryLine);	
			return false;
		}
	}

	// принимаем квест
	Quests[ClientID][QuestID].TalkProgress = 1;
	Quests[ClientID][QuestID].Type = QUESTACCEPT;
	GS()->ClearQuestsBot(QuestID, Quests[ClientID][QuestID].TalkProgress);

	SJK.ID("tw_accounts_quests", "(QuestID, OwnerID, Type) VALUES ('%d', '%d', '%d')", QuestID, pPlayer->Acc().AuthID, QUESTACCEPT);
	GS()->Chat(ClientID, "Accepted the quest [{STR}]", QuestsData[QuestID].Name);
	GS()->Chat(ClientID, "You will receive a reward Gold {INT}, Experience {INT}", &QuestsData[QuestID].Money, &QuestsData[QuestID].Exp);
	CheckQuest(pPlayer);
	return true;
}

// действие над квестом
bool QuestBase::InteractiveQuestNPC(CPlayer *pPlayer, ContextBots::QuestBotInfo &BotData, bool LastDialog)
{
	if(!pPlayer || !pPlayer->GetCharacter() || !BotData.IsActive()) 
		return false;

	// очистить интерактив
	const int ClientID = pPlayer->GetCID();

	// проверяем собрали предметы и убили ли всех ботов
	const int QuestID = BotData.QuestID;
	if(!MultiQuestNPC(pPlayer, BotData, false) || !CheckMobProgress(ClientID, QuestID) || pPlayer->Acc().Level < QuestsData[QuestID].Level)
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
		MultiQuestNPC(pPlayer, BotData, false, true);
		return false;
	}

	if (!LastDialog)
		return true;

	// проверяем и выдаем потом
	MultiQuestNPC(pPlayer, BotData, true, true);
	
	// забираем проверяем не является ли тип рандомно взять предмет
	if(BotData.InterRandom[2] <= 0 || RandomGetItem > 0)
		MultiQuestNPC(pPlayer, BotData, false, true);

	GS()->VResetVotes(ClientID, ADVENTUREJOURNAL);
	GS()->Mmo()->Quest()->TalkProgress(pPlayer, QuestID);
	return true;
}

// Прогресс мобов
void QuestBase::MobProgress(CPlayer *pPlayer, int BotID)
{
	if(!pPlayer || !Job()->BotsData()->IsDataBotValid(BotID)) 
		return;

	// Ищим в активных квестах Моба что требуется для квеста
	const int ClientID = pPlayer->GetCID();
	for(auto& qp : Quests[ClientID])
	{
		// если квест является принятым
		if(qp.second.Type != QUESTACCEPT) 
			continue;

		// получаем активного нпс
		int questID = qp.first;
		int playerTalkProgress = Quests[ClientID][questID].TalkProgress;
		ContextBots::QuestBotInfo FindBot = GetQuestBot(questID, playerTalkProgress);
		if(!FindBot.IsActive()) 
			continue;

		// ищим нужен ли такой Моб
		for(int i = 4; i < 6; i++)
		{
			// проверяем если равен и прогресс меньше чем требуется
			if(BotID != FindBot.Interactive[i] || qp.second.MobProgress[i-4] >= FindBot.InterCount[i]) 
				continue;

			// проверяем дабы чтобы писать о прогрессе всегда а не только 1 раз при перезаходах
			qp.second.MobProgress[i-4]++;
			if(qp.second.MobProgress[i-4] >= FindBot.InterCount[i])
			{
				GS()->Chat(ClientID, "You killed {STR} the required amount for NPC {STR}", ContextBots::DataBot[BotID].NameBot, FindBot.Name);
			}

			// обновляем таблицу
			SJK.UD("tw_accounts_quests", "Mob1Progress = '%d', Mob2Progress = '%d' WHERE QuestID = '%d' AND OwnerID = '%d'", 
				qp.second.MobProgress[0], qp.second.MobProgress[1], questID, pPlayer->Acc().AuthID);
			break;
		}
	}
}

// автозавершение и автопринятие
void QuestBase::AutoStartNextQuest(CPlayer *pPlayer, int QuestID)
{
	// если пассивный то без автозавершения
	int clientID = pPlayer->GetCID();
	if(str_comp(QuestsData[QuestID].StoryLine, "Passive") == 0)
	{
		GS()->Chat(clientID, "See 'Adventure Journal' for complete quest [{STR}]", QuestsData[QuestID].Name);
		return;
	}

	// завершаем квест
	FinishQuest(pPlayer, QuestID);

	// проверяем если есть следующий квест то продолжаем сюжетку только сюжетные квесты
	int NextQuestID = QuestID + 1;
	
	if(!IsValidQuest(NextQuestID) || str_comp(QuestsData[QuestID].StoryLine, QuestsData[NextQuestID].StoryLine) != 0)
	{
		GS()->Chat(clientID, "At the moment there is no continuation of '{STR}' story", QuestsData[QuestID].StoryLine);			
		return;
	}

	// прнимаем квест и пишем информацию о следующем квесте
	AcceptQuest(NextQuestID, pPlayer);
	GS()->Chat(clientID, "You can see the details in vote 'Adventure Journal'");
	if(pPlayer->GetCharacter()) pPlayer->GetCharacter()->CreateQuestsSteps();
	
	// все остальное
	GS()->VResetVotes(clientID, ADVENTUREJOURNAL);
}

// Адвентур активные нпс показ информации
bool QuestBase::ShowAdventureActiveNPC(CPlayer *pPlayer)
{
	bool activeNPC = false;
	const int clientID = pPlayer->GetCID();

	pPlayer->m_Colored = {30, 56, 20};
	GS()->AVM(clientID, "null", NOPE, NOPE, "Active NPC for current quests");

	// поиск всех активных нпс
	for(const auto& qq : Quests[clientID])
	{
		if(qq.second.Type != QUESTACCEPT) 
			continue;

		// проверяем бота есть или нет активный по квесту
		ContextBots::QuestBotInfo &BotInfo = GetQuestBot(qq.first, qq.second.TalkProgress);
		if(!BotInfo.IsActive()) 
			continue;

		// если нашли выводим информацию
		int HideID = (NUMHIDEMENU + 12500 + BotInfo.QuestID);
		int PosX = BotInfo.PositionX / 32, PosY = BotInfo.PositionY / 32;
		GS()->AVH(clientID, HideID, vec3(15,35,10), "[Active {STR}] {STR} {STR}(x:{INT} y:{INT})", GetStoryName(qq.first), BotInfo.Name, GS()->Server()->GetWorldName(BotInfo.WorldID), &PosX, &PosY);

		// проверяем требуемые мобы
		bool interactiveNeed = false;
		for(int i = 4; i < 6; i++)
		{
			int botID = BotInfo.Interactive[i], killNeed = BotInfo.InterCount[i];
			if(botID <= 0 || killNeed <= 0 || !Job()->BotsData()->IsDataBotValid(botID)) 
				continue;

			GS()->AVM(clientID, "null", NOPE, HideID, "- Defeat {STR} [{INT}/{INT}]", ContextBots::DataBot[botID].NameBot, &qq.second.MobProgress[i-4], &killNeed);
			interactiveNeed = true;	
		}

		// проверяем требуемые предметы
		for(int i = 0; i < 2; i++)
		{
			int itemID = BotInfo.Interactive[i], numNeed = BotInfo.InterCount[i];
			if(itemID <= 0 || numNeed <= 0) 
				continue;

			ItemSql::ItemPlayer searchItem = pPlayer->GetItem(itemID);
			int ownCount = clamp(searchItem.Count, 0, numNeed);

			GS()->AVMI(clientID, searchItem.Info().GetIcon(), "null", NOPE, HideID, "- Item {STR} [{INT}/{INT}]", searchItem.Info().GetName(pPlayer), &ownCount, &numNeed);
			interactiveNeed = true;	
		}

		// проверяем что даст
		for(int i = 2; i < 4; i++)
		{
			int itemID = BotInfo.Interactive[i], getCount = BotInfo.InterCount[i];
			if(itemID <= 0 || getCount <= 0) 
				continue;

			ItemSql::ItemInformation GivedInfItem = GS()->GetItemInfo(itemID);
			GS()->AVMI(clientID, GivedInfItem.GetIcon(), "null", NOPE, HideID, "- Gives {STR}x{INT}", GivedInfItem.GetName(pPlayer), &getCount);
			interactiveNeed = true;	
		}

		// если не нашли ничего что он делает
		if(!interactiveNeed) 
		{
			GS()->AVM(clientID, "null", NOPE, HideID, "You just need to talk.");
		}
		activeNPC = true;
	}
	return activeNPC;
}

// Парсинг голосованний 
bool QuestBase::OnParseVotingMenu(CPlayer *pPlayer, const char *CMD, const int VoteID, const int VoteID2, int Get, const char *GetText)
{
	int ClientID = pPlayer->GetCID();

	// принять квест
	if(PPSTR(CMD, "ACCEPTQUEST") == 0)
	{
		if(!AcceptQuest(VoteID, pPlayer)) 
			return true;

		GS()->ResetVotes(ClientID, pPlayer->m_OpenVoteMenu);
		
		if(pPlayer->GetCharacter()) 
			pPlayer->GetCharacter()->CreateQuestsSteps();
		return true;
	}
	return false;
}

// Найти квестового бота с ID
int QuestBase::FindQuestingBotClientID(int MobID)
{
	// for(int i = MAX_PLAYERS; i < MAX_CLIENTS; i++)
	// {
	// 	if(!GS()->m_apPlayers[i] || 
	// 		GS()->m_apPlayers[i]->GetSpawnBot() != SPAWNQUESTNPC ||
	// 		GS()->m_apPlayers[i]->GetBotSub() != MobID)
	// 		continue;

	// 	return i;
	// }
	return -1;
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
	if (!GS()->CheckClient(ClientID))
		return;

	CPlayer* pPlayer = GS()->GetPlayer(ClientID, true);
	if (ItemID >= itMoney && ItemID <= ItemSql::ItemsInfo.size() && pPlayer)
	{
		ItemSql::ItemPlayer SelectedItem = pPlayer->GetItem(ItemID);

		CNetMsg_Sv_AddQuestingProcessing Msg;
		Msg.m_pText = pText;
		Msg.m_pRequiresNum = Requires;
		Msg.m_pHaveNum = clamp(SelectedItem.Count, 0, Requires);
		StrToInts(Msg.m_pIcon, 4, SelectedItem.Info().GetIcon());
		GS()->Server()->SendPackMsg(&Msg, MSGFLAG_VITAL, ClientID);
	}
}

void QuestBase::QuestTableAddInfo(int ClientID, const char *pText, int Requires, int Have)
{
	if(!GS()->CheckClient(ClientID))
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
	if(!GS()->CheckClient(ClientID))
		return;
	
	CNetMsg_Sv_ClearQuestingProcessing Msg;
	GS()->Server()->SendPackMsg(&Msg, MSGFLAG_VITAL, ClientID);
}

// Показать разговор информацию как motd
void QuestBase::QuestTableShowInformation(CPlayer* pPlayer, ContextBots::QuestBotInfo& BotData)
{
	if (!pPlayer || !pPlayer->GetCharacter() || !BotData.IsActive())
		return;

	// показываем текст завершения квеста
	int ClientID = pPlayer->GetCID();	
	int QuestID = BotData.QuestID;

	// показываем по информация о предметах
	for (int i = 0; i < 2; i++)
	{
		int ItemID = BotData.Interactive[i];
		int Count = BotData.InterCount[i];
		if (ItemID <= 0 || Count <= 0)
			continue;

		char aBuf[128];
		str_format(aBuf, sizeof(aBuf), "I need (%s)", pPlayer->GetItem(ItemID).Info().GetName(pPlayer));
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
		str_format(aBuf, sizeof(aBuf), "Defeat (%s)", ContextBots::DataBot[BotID].NameBot);
		GS()->Mmo()->Quest()->QuestTableAddInfo(ClientID, aBuf, Count,
			QuestBase::Quests[ClientID][BotData.QuestID].MobProgress[i - 4]);
	}

	// если у бота рандомное принятие предмета
	/*if (BotData.InterRandom[0] > 1)
	{
		double Chance = BotData.InterRandom[0] <= 0 ? 100.0f : (1.0f / (double)BotData.InterRandom[0]) * 100;

		char aBuf[64];
		str_format(aBuf, sizeof(aBuf), "\nChance that item he'll like [%0.2f%%]", Chance);
		Buffer.append_at(Buffer.length(), aBuf);
	}*/

	// маленький уровень
	if (pPlayer->Acc().Level < QuestsData[QuestID].Level)
	{
		GS()->SBL(ClientID, 100, 100, "Required level: {INT}LVL", QuestsData[QuestID].Level);
	}
}