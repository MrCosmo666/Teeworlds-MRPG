/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#include <engine/shared/config.h>
#include <game/server/gamecontext.h>
#include "BotJob.h"

using namespace sqlstr;

// Структуры ботов
const char *BotJob::DescDataBot::Name(CPlayer *pPlayer) const
{
	if(!pPlayer) 
		return "(invalid)";

	int ClientID = pPlayer->GetCID();
	if (ClientID >= MAX_PLAYERS)
	{
		CPlayerBot* BotPlayer = static_cast<CPlayerBot*>(pPlayer);
		if (BotPlayer->GetSpawnBot() == SpawnBot::SPAWN_MOBS)
		{
			int SubID = BotPlayer->GetBotSub();
			return BotJob::MobBot[SubID].Name;
		}
		int BotID = BotPlayer->GetBotID();
		return BotJob::DataBot[BotID].NameBot;
	}
	return pPlayer->GS()->Server()->ClientName(ClientID);
}

std::map < int , BotJob::DescDataBot > BotJob::DataBot;
std::map < int , BotJob::QuestBotInfo > BotJob::QuestBot;
std::map < int , BotJob::NpcBotInfo > BotJob::NpcBot;
std::map < int , BotJob::MobBotInfo > BotJob::MobBot;

// Загрузка всех скинов и мобов что потом использовать для свзяей с другими ботами
void BotJob::LoadGlobalBots()
{
	// загружаем всю информацию о ботах
	boost::scoped_ptr<ResultSet> RES(SJK.SD("*", "tw_bots_world", "WHERE ID > '0'"));
	while(RES->next())
	{
		int BotID = (int)RES->getInt("ID");
		str_copy(DataBot[BotID].NameBot, RES->getString("BotName").c_str(), sizeof(DataBot[BotID].NameBot));

		if (!sscanf(RES->getString("SkinName").c_str(), "%s %s %s %s %s %s",
			DataBot[BotID].SkinNameBot[SKINPART_BODY], DataBot[BotID].SkinNameBot[SKINPART_MARKING],
			DataBot[BotID].SkinNameBot[SKINPART_DECORATION], DataBot[BotID].SkinNameBot[SKINPART_HANDS],
			DataBot[BotID].SkinNameBot[SKINPART_FEET], DataBot[BotID].SkinNameBot[SKINPART_EYES]))
			dbg_msg("Error", "Mised bots information");

		if(!sscanf(RES->getString("SkinColor").c_str(), "%d %d %d %d %d %d", 
			&DataBot[BotID].SkinColorBot[SKINPART_BODY], &DataBot[BotID].SkinColorBot[SKINPART_MARKING], 
			&DataBot[BotID].SkinColorBot[SKINPART_DECORATION], &DataBot[BotID].SkinColorBot[SKINPART_HANDS], 
			&DataBot[BotID].SkinColorBot[SKINPART_FEET], &DataBot[BotID].SkinColorBot[SKINPART_EYES]))
			dbg_msg("Error", "Mised bots information");

		for(int j = SKINPART_BODY; j < NUM_SKINPARTS; j ++) {
			if(DataBot[BotID].SkinColorBot[j] != 0)
				DataBot[BotID].UseCustomBot[j] = true;
		}
		DataBot[BotID].EquipSlot[EQUIP_WINGS] = RES->getInt("SlotWings");
		DataBot[BotID].EquipSlot[EQUIP_HAMMER] = RES->getInt("SlotHammer");
		DataBot[BotID].EquipSlot[EQUIP_GUN] = RES->getInt("SlotGun");
		DataBot[BotID].EquipSlot[EQUIP_SHOTGUN] = RES->getInt("SlotShotgun");
		DataBot[BotID].EquipSlot[EQUIP_GRENADE] = RES->getInt("SlotGrenade");
		DataBot[BotID].EquipSlot[EQUIP_RIFLE] = RES->getInt("SlotRifle");
	}

	// загружаем связанных ботов
	LoadQuestBots();
	LoadNpcBots();
	LoadMobsBots();
}

// Загрузка Quest Bots
void BotJob::LoadQuestBots()
{
	boost::scoped_ptr<ResultSet> RES(SJK.SD("*", "tw_bots_quest"));
	while(RES->next())
	{
		int MotID = (int)RES->getInt("ID");
		int WorldID = RES->getInt("WorldID");
		if(WorldID != GS()->GetWorldID()) continue;

		QuestBot[MotID].SubBotID = MotID;
		QuestBot[MotID].BotID = RES->getInt("BotID");
		QuestBot[MotID].WorldID = WorldID;
		QuestBot[MotID].PositionX = RES->getInt("pos_x");
		QuestBot[MotID].PositionY = RES->getInt("pos_y")+1;
		QuestBot[MotID].QuestID = RES->getInt("QuestID");
		str_copy(QuestBot[MotID].Name, DataBot[QuestBot[MotID].BotID].NameBot, sizeof(QuestBot[MotID].Name));

		QuestBot[MotID].Interactive[0] = RES->getInt("it_need_0");
		QuestBot[MotID].Interactive[1] = RES->getInt("it_need_1");
		QuestBot[MotID].Interactive[2] = RES->getInt("it_reward_0");
		QuestBot[MotID].Interactive[3] = RES->getInt("it_reward_1");
		QuestBot[MotID].Interactive[4] = RES->getInt("mob_0");
		QuestBot[MotID].Interactive[5] = RES->getInt("mob_1");

		sscanf(RES->getString("it_count").c_str(), "|%d|%d|%d|%d|%d|%d|", 
			&QuestBot[MotID].InterCount[0], &QuestBot[MotID].InterCount[1], &QuestBot[MotID].InterCount[2],
			&QuestBot[MotID].InterCount[3], &QuestBot[MotID].InterCount[4], &QuestBot[MotID].InterCount[5]);

		sscanf(RES->getString("it_random").c_str(), "|%d|%d|%d|%d|%d|%d|", 
			&QuestBot[MotID].InterRandom[0], &QuestBot[MotID].InterRandom[1], &QuestBot[MotID].InterRandom[2],
			&QuestBot[MotID].InterRandom[3], &QuestBot[MotID].InterRandom[4], &QuestBot[MotID].InterRandom[5]);

		// загрузить разговоры NPC
		boost::scoped_ptr<ResultSet> RES(SJK.SD("*", "tw_talk_quest_npc", "WHERE MobID = '%d'", MotID));
		while(RES->next())
		{
			TalkingData LoadTalk;
			LoadTalk.m_Emote = RES->getInt("TalkingEmote");
			LoadTalk.m_Style = RES->getInt("Style");
			LoadTalk.m_PlayerTalked = RES->getBoolean("PlayerTalked");
			LoadTalk.m_RequestComplete = RES->getBoolean("RequestComplete");
			str_copy(LoadTalk.m_TalkingText, RES->getString("TalkText").c_str(), sizeof(LoadTalk.m_TalkingText));
			QuestBot[MotID].m_Talk.push_back(LoadTalk);
		}
	}

	for (auto& qupdate : QuestBot)
		qupdate.second.Progress = Job()->Quest()->GetBotQuestProgress(qupdate.second.QuestID, qupdate.first);
}

// Загрузка обычных NPC
void BotJob::LoadNpcBots()
{
	boost::scoped_ptr<ResultSet> RES(SJK.SD("*", "tw_bots_npc"));
	while(RES->next())
	{
		int MotID = (int)RES->getInt("ID");
		int WorldID = RES->getInt("WorldID");
		if(WorldID != GS()->GetWorldID()) continue;
		bool CreateBot = !IsNpcBotValid(MotID);

		NpcBot[MotID].WorldID = WorldID;
		NpcBot[MotID].Static = RES->getBoolean("Static");
		NpcBot[MotID].PositionX = RES->getInt("PositionX");
		NpcBot[MotID].PositionY = (NpcBot[MotID].Static ? RES->getInt("PositionY")+1 : RES->getInt("PositionY"));
		NpcBot[MotID].Emote = RES->getInt("Emote");
		NpcBot[MotID].BotID = RES->getInt("BotID");
		str_copy(NpcBot[MotID].Name, DataBot[NpcBot[MotID].BotID].NameBot, sizeof(NpcBot[MotID].Name));
	
		// пропуск если не создаем ботов
		if(!CreateBot) continue;

		int CountMobs = RES->getInt("Count");
		for(int c = 0; c < CountMobs; c++)
			GS()->CreateBot(SpawnBot::SPAWN_NPC, NpcBot[MotID].BotID, MotID);

		// загрузить разговоры NPC
		boost::scoped_ptr<ResultSet> RES(SJK.SD("*", "tw_talk_other_npc", "WHERE MobID = '%d'", MotID));
		while(RES->next())
		{
			TalkingData LoadTalk;
			LoadTalk.m_Emote = RES->getInt("TalkingEmote");
			LoadTalk.m_Style = RES->getInt("Style");
			LoadTalk.m_PlayerTalked = RES->getBoolean("PlayerTalked");
			LoadTalk.m_GivingQuest = RES->getInt("GivingQuest");
			str_copy(LoadTalk.m_TalkingText, RES->getString("TalkText").c_str(), sizeof(LoadTalk.m_TalkingText));
			NpcBot[MotID].m_Talk.push_back(LoadTalk);
		}
	}
}

// Загрузка мобов
void BotJob::LoadMobsBots()
{
	boost::scoped_ptr<ResultSet> RES(SJK.SD("*", "tw_bots_mobs"));
	while(RES->next())
	{
		int WorldID = RES->getInt("WorldID");
		if(WorldID != GS()->GetWorldID()) continue;

		int MotID = (int)RES->getInt("ID");
		int BotID = RES->getInt("BotID");
		bool CreateBot = !IsMobBotValid(MotID);

		MobBot[MotID].WorldID = WorldID;
		MobBot[MotID].PositionX = RES->getInt("PositionX");
		MobBot[MotID].PositionY = RES->getInt("PositionY");
		MobBot[MotID].Power = RES->getInt("Power");
		MobBot[MotID].Spread = RES->getInt("Spread");
		MobBot[MotID].Boss = RES->getBoolean("Boss");
		MobBot[MotID].Level = RES->getInt("Level");
		MobBot[MotID].RespawnTick = RES->getInt("Respawn");
		MobBot[MotID].BotID = BotID;
		str_copy(MobBot[MotID].Name, DataBot[BotID].NameBot, sizeof(MobBot[MotID].Name));

		char aBuf[32];
		for(int i = 0; i < 6; i ++)
		{
			str_format(aBuf, sizeof(aBuf), "it_drop_%d", i);
			MobBot[MotID].DropItem[i] = RES->getInt(aBuf);
		}

		sscanf(RES->getString("it_count").c_str(), "|%d|%d|%d|%d|%d|%d|", 
			&MobBot[MotID].CountItem[0], &MobBot[MotID].CountItem[1], &MobBot[MotID].CountItem[2],
			&MobBot[MotID].CountItem[3], &MobBot[MotID].CountItem[4], &MobBot[MotID].CountItem[5]);

		sscanf(RES->getString("it_random").c_str(), "|%d|%d|%d|%d|%d|%d|", 
			&MobBot[MotID].RandomItem[0], &MobBot[MotID].RandomItem[1], &MobBot[MotID].RandomItem[2],
			&MobBot[MotID].RandomItem[3], &MobBot[MotID].RandomItem[4], &MobBot[MotID].RandomItem[5]);


		// пропуск если не создаем ботов
		if(!CreateBot) continue;

		int CountMobs = RES->getInt("Count");
		for(int c = 0; c < CountMobs; c++)
			GS()->CreateBot(SpawnBot::SPAWN_MOBS, BotID, MotID);
	}	
}

// добавить нового бота
void BotJob::ConAddCharacterBot(int ClientID, const char *pCharacter)
{
	// если нет игрока то не продолжаем
	CPlayer *pPlayer = GS()->GetPlayer(ClientID);
	if(!pPlayer) return;

	// собираем данные со скина игрока
	char SkinPart[256], SkinColor[256];
	str_format(SkinPart, sizeof(SkinPart), "%s %s %s %s %s %s", pPlayer->Acc().m_aaSkinPartNames[0], pPlayer->Acc().m_aaSkinPartNames[1], 
		pPlayer->Acc().m_aaSkinPartNames[2], pPlayer->Acc().m_aaSkinPartNames[3], pPlayer->Acc().m_aaSkinPartNames[4], pPlayer->Acc().m_aaSkinPartNames[5]);

	str_format(SkinColor, sizeof(SkinColor), "%d %d %d %d %d %d", pPlayer->Acc().m_aSkinPartColors[0], pPlayer->Acc().m_aSkinPartColors[1], 
		pPlayer->Acc().m_aSkinPartColors[2], pPlayer->Acc().m_aSkinPartColors[3], pPlayer->Acc().m_aSkinPartColors[4], pPlayer->Acc().m_aSkinPartColors[5]);

	// проверяем ник если есть обновим нет добавим
	CSqlString<16> cNick = CSqlString<16>(pCharacter);
	boost::scoped_ptr<ResultSet> RES(SJK.SD("*", "tw_bots_world", "WHERE BotName = '%s' AND ID > '0'", cNick.cstr()));
	if(RES->next())
	{
		// если ник не верен из базы данных
		int ID = RES->getInt("ID");
		SJK.UD("tw_bots_world", "SkinName = '%s', SkinColor = '%s' WHERE ID = '%d'", SkinPart, SkinColor, ID);
		GS()->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "parseskin", "Updated character bot!");
		return;	
	}

	// добавляем нового бота
	SJK.ID("tw_bots_world", "(BotName, SkinName, SkinColor) VALUES ('%s', '%s', '%s')", cNick.cstr(), SkinPart, SkinColor);
	GS()->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "parseskin", "Added new character bot!");
}

void BotJob::ProcessingTalkingNPC(int OwnID, int TalkingID, bool PlayerTalked, const char *Message, int Style, int TalkingEmote)
{
	if(GS()->CheckClient(OwnID))
	{
		GS()->SendTalkText(OwnID, TalkingID, PlayerTalked, Message, Style, TalkingEmote);
		return;
	}

	GS()->Motd(OwnID, Message);
}

bool BotJob::TalkingBotNPC(CPlayer* pPlayer, int MobID, int Progress, int TalkedID, const char *pText)
{
	int ClientID = pPlayer->GetCID();
	if (!IsNpcBotValid(MobID) || Progress >= (int)NpcBot[MobID].m_Talk.size())
	{
		GS()->ClearTalkText(ClientID);
		return false;
	}

	char reformTalkedText[512];
	int sizeTalking = NpcBot[MobID].m_Talk.size();
	if (str_comp_nocase(pText, "empty") != 0)
	{
		str_format(reformTalkedText, sizeof(reformTalkedText), "(Discussion %d of %d .. ) - %s", 1 + Progress, sizeTalking, pText);
		GS()->Mmo()->BotsData()->ProcessingTalkingNPC(ClientID, TalkedID, 0, reformTalkedText, 0, EMOTE_BLINK);
		return true;
	}

	int BotID = NpcBot[MobID].BotID;
	pPlayer->FormatTextQuest(BotID, NpcBot[MobID].m_Talk.at(Progress).m_TalkingText);
	str_format(reformTalkedText, sizeof(reformTalkedText), "(Discussion %d of %d .. ) - %s", 1 + Progress, sizeTalking, pPlayer->FormatedTalkedText());
	pPlayer->ClearFormatQuestText();

	GS()->Mmo()->BotsData()->ProcessingTalkingNPC(ClientID, TalkedID,
		NpcBot[MobID].m_Talk.at(Progress).m_PlayerTalked, reformTalkedText,
		NpcBot[MobID].m_Talk.at(Progress).m_Style, NpcBot[MobID].m_Talk.at(Progress).m_Emote);
	return true;
}

bool BotJob::TalkingBotQuest(CPlayer* pPlayer, int MobID, int Progress, int TalkedID)
{
	int ClientID = pPlayer->GetCID();
	if (!IsQuestBotValid(MobID) || Progress >= (int)QuestBot[MobID].m_Talk.size())
	{
		GS()->ClearTalkText(ClientID);
		return false;
	}

	if (!GS()->CheckClient(ClientID))
		GS()->SBL(ClientID, BroadcastPriority::BROADCAST_GAME_INFORMATION, 50, "Press 'F4' to continue the dialog!");

	int BotID = QuestBot[MobID].BotID;
	char reformTalkedText[512];
	int sizeTalking = QuestBot[MobID].m_Talk.size();
	pPlayer->FormatTextQuest(BotID, QuestBot[MobID].m_Talk.at(Progress).m_TalkingText);
	str_format(reformTalkedText, sizeof(reformTalkedText), "(Discussion %d of %d .. ) - %s", 1 + Progress, sizeTalking, pPlayer->FormatedTalkedText());
	pPlayer->ClearFormatQuestText();

	GS()->Mmo()->BotsData()->ProcessingTalkingNPC(ClientID, TalkedID,
		QuestBot[MobID].m_Talk.at(Progress).m_PlayerTalked, reformTalkedText,
		QuestBot[MobID].m_Talk.at(Progress).m_Style, QuestBot[MobID].m_Talk.at(Progress).m_Emote);
	return true;
}

void BotJob::ShowBotQuestTaskInfo(CPlayer* pPlayer, int MobID, int Progress)
{
	int ClientID = pPlayer->GetCID();
	if (!IsQuestBotValid(MobID) || Progress >= (int)QuestBot[MobID].m_Talk.size())
	{
		GS()->ClearTalkText(ClientID);
		return;
	}

	// vanila clients
	int BotID = BotJob::QuestBot[MobID].BotID;
	int sizeTalking = BotJob::QuestBot[MobID].m_Talk.size();
	if (!GS()->CheckClient(ClientID))
	{
		char reformTalkedText[512];
		pPlayer->FormatTextQuest(BotID, BotJob::QuestBot[MobID].m_Talk.at(Progress).m_TalkingText);
		str_format(reformTalkedText, sizeof(reformTalkedText), "(Discussion %d of %d .. ) - %s", 1 + Progress, sizeTalking, pPlayer->FormatedTalkedText());
		pPlayer->ClearFormatQuestText();

		GS()->Mmo()->Quest()->ShowQuestRequired(pPlayer, BotJob::QuestBot[MobID], reformTalkedText);
		return;
	}

	// mmo clients
	GS()->Mmo()->Quest()->ShowQuestRequired(pPlayer, BotJob::QuestBot[MobID], "\0");
}
