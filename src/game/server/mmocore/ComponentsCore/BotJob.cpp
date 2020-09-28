/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#include <game/server/gamecontext.h>
#include "BotJob.h"

#include <game/server/mmocore/PathFinder.h>

using namespace sqlstr;

// bot structures
std::map <int, BotJob::DescDataBot> BotJob::ms_aDataBot;
std::map <int, BotJob::QuestBotInfo> BotJob::ms_aQuestBot;
std::map <int, BotJob::NpcBotInfo> BotJob::ms_aNpcBot;
std::map <int, BotJob::MobBotInfo> BotJob::ms_aMobBot;

// loading of all skins and mobs to use for connection with other bots
void BotJob::OnInitWorld(const char* pWhereLocalWorld)
{
	LoadMainInformationBots();
	LoadQuestBots(pWhereLocalWorld);
	LoadNpcBots(pWhereLocalWorld);
	LoadMobsBots(pWhereLocalWorld);
}

// add a new bot
void BotJob::ConAddCharacterBot(int ClientID, const char *pCharacter)
{
	CPlayer *pPlayer = GS()->GetPlayer(ClientID);
	if(!pPlayer) 
		return;

	// collect data from a player's skin
	char SkinPart[256], SkinColor[256];
	str_format(SkinPart, sizeof(SkinPart), "%s %s %s %s %s %s", pPlayer->Acc().m_aaSkinPartNames[0], pPlayer->Acc().m_aaSkinPartNames[1], 
		pPlayer->Acc().m_aaSkinPartNames[2], pPlayer->Acc().m_aaSkinPartNames[3], pPlayer->Acc().m_aaSkinPartNames[4], pPlayer->Acc().m_aaSkinPartNames[5]);
	str_format(SkinColor, sizeof(SkinColor), "%d %d %d %d %d %d", pPlayer->Acc().m_aSkinPartColors[0], pPlayer->Acc().m_aSkinPartColors[1], 
		pPlayer->Acc().m_aSkinPartColors[2], pPlayer->Acc().m_aSkinPartColors[3], pPlayer->Acc().m_aSkinPartColors[4], pPlayer->Acc().m_aSkinPartColors[5]);

	// check the nick
	CSqlString<16> cNick = CSqlString<16>(pCharacter);
	std::shared_ptr<ResultSet> RES(SJK.SD("*", "tw_bots_world", "WHERE BotName = '%s'", cNick.cstr()));
	if(RES->next())
	{
		// if the nickname is not in the database
		const int ID = RES->getInt("ID");
		SJK.UD("tw_bots_world", "SkinName = '%s', SkinColor = '%s' WHERE ID = '%d'", SkinPart, SkinColor, ID);
		GS()->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "parseskin", "Updated character bot!");
		return;	
	}

	// add a new bot
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
	const int ClientID = pPlayer->GetCID();
	if (!IsNpcBotValid(MobID) || Progress >= (int)ms_aNpcBot[MobID].m_aTalk.size())
	{
		GS()->ClearTalkText(ClientID);
		return false;
	}

	if (!GS()->CheckClient(ClientID))
		GS()->SBL(ClientID, BroadcastPriority::BROADCAST_GAME_PRIORITY, 100, "Press 'F4' to continue the dialog!");

	char reformTalkedText[512];
	const int BotID = ms_aNpcBot[MobID].m_BotID;
	const int sizeTalking = ms_aNpcBot[MobID].m_aTalk.size();
	if (str_comp_nocase(pText, "empty") != 0)
	{
		pPlayer->FormatTextQuest(BotID, pText);
		if(!GS()->CheckClient(ClientID))
		{
			str_format(reformTalkedText, sizeof(reformTalkedText), "( 1 of 1 ) %s:\n- %s", ms_aNpcBot[MobID].GetName(), pPlayer->FormatedTalkedText());
			GS()->SBL(ClientID, BroadcastPriority::BROADCAST_GAME_PRIORITY, 100, "Press 'F4' to continue the dialog!");
		}
		else
		{
			str_format(reformTalkedText, sizeof(reformTalkedText), "( 1 of 1 ) - %s", pPlayer->FormatedTalkedText());
		}
		pPlayer->ClearFormatQuestText();
		GS()->Mmo()->BotsData()->ProcessingTalkingNPC(ClientID, TalkedID, false, reformTalkedText, 0, EMOTE_BLINK);
		return true;
	}

	const bool PlayerTalked = ms_aNpcBot[MobID].m_aTalk[Progress].m_PlayerTalked;
	pPlayer->FormatTextQuest(BotID, ms_aNpcBot[MobID].m_aTalk[Progress].m_aTalkingText);
	if(!GS()->CheckClient(ClientID))
	{
		const char* TalkedNick = (PlayerTalked ? GS()->Server()->ClientName(ClientID) : ms_aNpcBot[MobID].GetName());
		str_format(reformTalkedText, sizeof(reformTalkedText), "( %d of %d ) %s:\n- %s", (1 + Progress), sizeTalking, TalkedNick, pPlayer->FormatedTalkedText());
		GS()->SBL(ClientID, BroadcastPriority::BROADCAST_GAME_PRIORITY, 100, "Press 'F4' to continue the dialog!");
	}
	else
	{
		str_format(reformTalkedText, sizeof(reformTalkedText), "( %d of %d ) - %s", (1 + Progress), sizeTalking, pPlayer->FormatedTalkedText());
	}
	pPlayer->ClearFormatQuestText();
	GS()->Mmo()->BotsData()->ProcessingTalkingNPC(ClientID, TalkedID,
		PlayerTalked, reformTalkedText, ms_aNpcBot[MobID].m_aTalk[Progress].m_Style, ms_aNpcBot[MobID].m_aTalk[Progress].m_Emote);
	return true;
}

bool BotJob::TalkingBotQuest(CPlayer* pPlayer, int MobID, int Progress, int TalkedID)
{
	const int ClientID = pPlayer->GetCID();
	if (!IsQuestBotValid(MobID) || Progress >= (int)ms_aQuestBot[MobID].m_aTalk.size())
	{
		GS()->ClearTalkText(ClientID);
		return false;
	}

	char reformTalkedText[512];
	const int BotID = ms_aQuestBot[MobID].m_BotID;
	const int sizeTalking = ms_aQuestBot[MobID].m_aTalk.size();
	const bool PlayerTalked = ms_aQuestBot[MobID].m_aTalk[Progress].m_PlayerTalked;
	pPlayer->FormatTextQuest(BotID, ms_aQuestBot[MobID].m_aTalk[Progress].m_aTalkingText);
	if(!GS()->CheckClient(ClientID))
	{
		const char* TalkedNick = (PlayerTalked ? GS()->Server()->ClientName(ClientID) : ms_aQuestBot[MobID].GetName());
		str_format(reformTalkedText, sizeof(reformTalkedText), "( %d of %d ) %s:\n- %s", (1 + Progress), sizeTalking, TalkedNick, pPlayer->FormatedTalkedText());
		GS()->SBL(ClientID, BroadcastPriority::BROADCAST_GAME_PRIORITY, 100, "Press 'F4' to continue the dialog!");
	}
	else
	{
		str_format(reformTalkedText, sizeof(reformTalkedText), "( %d of %d ) - %s", (1 + Progress), sizeTalking, pPlayer->FormatedTalkedText());
	}
	pPlayer->ClearFormatQuestText();
	GS()->Mmo()->BotsData()->ProcessingTalkingNPC(ClientID, TalkedID,
		PlayerTalked, reformTalkedText, ms_aQuestBot[MobID].m_aTalk[Progress].m_Style, ms_aQuestBot[MobID].m_aTalk[Progress].m_Emote);
	return true;
}

void BotJob::ShowBotQuestTaskInfo(CPlayer* pPlayer, int MobID, int Progress)
{
	const int ClientID = pPlayer->GetCID();
	if (!IsQuestBotValid(MobID) || Progress >= (int)ms_aQuestBot[MobID].m_aTalk.size())
	{
		GS()->ClearTalkText(ClientID);
		return;
	}

	// vanila clients
	const int BotID = BotJob::ms_aQuestBot[MobID].m_BotID;
	const int sizeTalking = BotJob::ms_aQuestBot[MobID].m_aTalk.size();
	if (!GS()->CheckClient(ClientID))
	{
		const bool PlayerTalked = ms_aQuestBot[MobID].m_aTalk[Progress].m_PlayerTalked;
		const char* TalkedNick = (PlayerTalked ? GS()->Server()->ClientName(ClientID) : ms_aQuestBot[MobID].GetName());

		char reformTalkedText[512];
		pPlayer->FormatTextQuest(BotID, BotJob::ms_aQuestBot[MobID].m_aTalk[Progress].m_aTalkingText);
		str_format(reformTalkedText, sizeof(reformTalkedText), "( %d of %d ) %s:\n- %s", (1 + Progress), sizeTalking, TalkedNick, pPlayer->FormatedTalkedText());
		pPlayer->ClearFormatQuestText();

		GS()->Mmo()->Quest()->QuestTableShowRequired(pPlayer, BotJob::ms_aQuestBot[MobID], reformTalkedText);
		return;
	}

	// mmo clients
	GS()->Mmo()->Quest()->QuestTableShowRequired(pPlayer, BotJob::ms_aQuestBot[MobID], "\0");
}

int BotJob::GetQuestNPC(int MobID) const
{
	if (!IsNpcBotValid(MobID))
		return -1;
		
	for (const auto& npc : ms_aNpcBot[MobID].m_aTalk)
	{
		if (npc.m_GivingQuest > 0)
			return npc.m_GivingQuest;
	}
	return -1;
}


// load basic information about bots
void BotJob::LoadMainInformationBots()
{
	if(!(ms_aDataBot.empty()))
		return;

	std::shared_ptr<ResultSet> RES(SJK.SD("*", "tw_bots_world"));
	while(RES->next())
	{
		const int BotID = (int)RES->getInt("ID");
		str_copy(ms_aDataBot[BotID].m_aNameBot, RES->getString("BotName").c_str(), sizeof(ms_aDataBot[BotID].m_aNameBot));

		if(!sscanf(RES->getString("SkinName").c_str(), "%s %s %s %s %s %s",
			ms_aDataBot[BotID].m_aaSkinNameBot[SKINPART_BODY], ms_aDataBot[BotID].m_aaSkinNameBot[SKINPART_MARKING],
			ms_aDataBot[BotID].m_aaSkinNameBot[SKINPART_DECORATION], ms_aDataBot[BotID].m_aaSkinNameBot[SKINPART_HANDS],
			ms_aDataBot[BotID].m_aaSkinNameBot[SKINPART_FEET], ms_aDataBot[BotID].m_aaSkinNameBot[SKINPART_EYES]))
			dbg_msg("Error", "Mised bots information");

		if(!sscanf(RES->getString("SkinColor").c_str(), "%d %d %d %d %d %d",
			&ms_aDataBot[BotID].m_aSkinColorBot[SKINPART_BODY], &ms_aDataBot[BotID].m_aSkinColorBot[SKINPART_MARKING],
			&ms_aDataBot[BotID].m_aSkinColorBot[SKINPART_DECORATION], &ms_aDataBot[BotID].m_aSkinColorBot[SKINPART_HANDS],
			&ms_aDataBot[BotID].m_aSkinColorBot[SKINPART_FEET], &ms_aDataBot[BotID].m_aSkinColorBot[SKINPART_EYES]))
			dbg_msg("Error", "Mised bots information");

		for(int j = SKINPART_BODY; j < NUM_SKINPARTS; j++) {
			if(ms_aDataBot[BotID].m_aSkinColorBot[j] != 0)
				ms_aDataBot[BotID].m_aUseCustomBot[j] = true;
		}

		for(int i = 0; i < MAX_PLAYERS; i++)
			ms_aDataBot[BotID].m_aAlreadySnapQuestBot[i] = false;

		ms_aDataBot[BotID].m_aEquipSlot[EQUIP_HAMMER] = RES->getInt("SlotHammer");
		ms_aDataBot[BotID].m_aEquipSlot[EQUIP_GUN] = RES->getInt("SlotGun");
		ms_aDataBot[BotID].m_aEquipSlot[EQUIP_SHOTGUN] = RES->getInt("SlotShotgun");
		ms_aDataBot[BotID].m_aEquipSlot[EQUIP_GRENADE] = RES->getInt("SlotGrenade");
		ms_aDataBot[BotID].m_aEquipSlot[EQUIP_RIFLE] = RES->getInt("SlotRifle");
		ms_aDataBot[BotID].m_aEquipSlot[EQUIP_MINER] = 0;
		ms_aDataBot[BotID].m_aEquipSlot[EQUIP_WINGS] = RES->getInt("SlotWings");
	}
}

// load quest bots
void BotJob::LoadQuestBots(const char* pWhereLocalWorld)
{
	std::shared_ptr<ResultSet> RES(SJK.SD("*", "tw_bots_quest", pWhereLocalWorld));
	while(RES->next())
	{
		// it for every world initilize quest progress size
		const int MobID = (int)RES->getInt("ID");
		ms_aQuestBot[MobID].m_SubBotID = MobID;
		ms_aQuestBot[MobID].m_BotID = (int)RES->getInt("BotID");
		ms_aQuestBot[MobID].m_QuestID = RES->getInt("QuestID");
		ms_aQuestBot[MobID].m_WorldID = (int)RES->getInt("WorldID");;
		ms_aQuestBot[MobID].m_PositionX = (int)RES->getInt("pos_x");
		ms_aQuestBot[MobID].m_PositionY = (int)RES->getInt("pos_y") + 1;
		ms_aQuestBot[MobID].m_aItemSearch[0] = (int)RES->getInt("it_need_0");
		ms_aQuestBot[MobID].m_aItemSearch[1] = (int)RES->getInt("it_need_1");
		ms_aQuestBot[MobID].m_aItemGives[0] = (int)RES->getInt("it_reward_0");
		ms_aQuestBot[MobID].m_aItemGives[1] = (int)RES->getInt("it_reward_1");
		ms_aQuestBot[MobID].m_aNeedMob[0] = (int)RES->getInt("mob_0");
		ms_aQuestBot[MobID].m_aNeedMob[1] = (int)RES->getInt("mob_1");
		ms_aQuestBot[MobID].m_InteractiveType = (int)RES->getInt("interactive_type");
		ms_aQuestBot[MobID].m_InteractiveTemp = (int)RES->getInt("interactive_temp");
		ms_aQuestBot[MobID].m_GenerateNick = (bool)RES->getBoolean("generate_nick");
		ms_aQuestBot[MobID].m_NextEqualProgress = (bool)RES->getBoolean("next_equal_progress");

		sscanf(RES->getString("it_count").c_str(), "|%d|%d|%d|%d|%d|%d|",
			&ms_aQuestBot[MobID].m_aItemSearchCount[0], &ms_aQuestBot[MobID].m_aItemSearchCount[1],
			&ms_aQuestBot[MobID].m_aItemGivesCount[0], &ms_aQuestBot[MobID].m_aItemGivesCount[1],
			&ms_aQuestBot[MobID].m_aNeedMobCount[0], &ms_aQuestBot[MobID].m_aNeedMobCount[1]);

		// load NPC
		std::shared_ptr<ResultSet> RES(SJK.SD("*", "tw_talk_quest_npc", "WHERE MobID = '%d'", MobID));
		while(RES->next())
		{
			TalkingData LoadTalk;
			LoadTalk.m_Emote = RES->getInt("TalkingEmote");
			LoadTalk.m_Style = RES->getInt("Style");
			LoadTalk.m_PlayerTalked = RES->getBoolean("PlayerTalked");
			LoadTalk.m_RequestComplete = RES->getBoolean("RequestComplete");
			str_copy(LoadTalk.m_aTalkingText, RES->getString("TalkText").c_str(), sizeof(LoadTalk.m_aTalkingText));
			ms_aQuestBot[MobID].m_aTalk.push_back(LoadTalk);
		}

		GS()->Server()->AddInformationBotsCount(1);
	}

	for(auto& qparseprogress : ms_aQuestBot)
	{
		qparseprogress.second.m_Progress = 1;
		for(const auto& qbots : ms_aQuestBot)
		{
			if(qbots.second.m_QuestID != qparseprogress.second.m_QuestID)
				continue;
			if(qbots.first == qparseprogress.first)
				break;
			if(qbots.second.m_NextEqualProgress)
				continue;

			qparseprogress.second.m_Progress++;
		}
	}
}

// load NPC
void BotJob::LoadNpcBots(const char* pWhereLocalWorld)
{
	std::shared_ptr<ResultSet> RES(SJK.SD("*", "tw_bots_npc", pWhereLocalWorld));
	while(RES->next())
	{
		const int MobID = (int)RES->getInt("ID");
		ms_aNpcBot[MobID].m_WorldID = RES->getInt("WorldID");
		ms_aNpcBot[MobID].m_Static = RES->getBoolean("Static");
		ms_aNpcBot[MobID].m_PositionX = RES->getInt("PositionX");
		ms_aNpcBot[MobID].m_PositionY = (ms_aNpcBot[MobID].m_Static ? RES->getInt("PositionY") + 1 : RES->getInt("PositionY"));
		ms_aNpcBot[MobID].m_Emote = RES->getInt("Emote");
		ms_aNpcBot[MobID].m_BotID = RES->getInt("BotID");
		ms_aNpcBot[MobID].Function = RES->getInt("Function");

		const int CountMobs = RES->getInt("Count");
		for(int c = 0; c < CountMobs; c++)
			GS()->CreateBot(BotsTypes::TYPE_BOT_NPC, ms_aNpcBot[MobID].m_BotID, MobID);

		std::shared_ptr<ResultSet> RES(SJK.SD("*", "tw_talk_other_npc", "WHERE MobID = '%d'", MobID));
		while(RES->next())
		{
			TalkingData LoadTalk;
			LoadTalk.m_Emote = RES->getInt("TalkingEmote");
			LoadTalk.m_Style = RES->getInt("Style");
			LoadTalk.m_PlayerTalked = RES->getBoolean("PlayerTalked");
			LoadTalk.m_GivingQuest = RES->getInt("GivingQuest");
			str_copy(LoadTalk.m_aTalkingText, RES->getString("TalkText").c_str(), sizeof(LoadTalk.m_aTalkingText));
			ms_aNpcBot[MobID].m_aTalk.push_back(LoadTalk);

			if(LoadTalk.m_GivingQuest > 0)
				ms_aNpcBot[MobID].Function = FunctionsNPC::FUNCTION_NPC_GIVE_QUEST;
		}

		GS()->Server()->AddInformationBotsCount(CountMobs);
	}
}

// load mobs
void BotJob::LoadMobsBots(const char* pWhereLocalWorld)
{
	std::shared_ptr<ResultSet> RES(SJK.SD("*", "tw_bots_mobs", pWhereLocalWorld));
	while(RES->next())
	{
		const int MobID = (int)RES->getInt("ID");
		const int BotID = RES->getInt("BotID");
		ms_aMobBot[MobID].m_WorldID = RES->getInt("WorldID");
		ms_aMobBot[MobID].m_PositionX = RES->getInt("PositionX");
		ms_aMobBot[MobID].m_PositionY = RES->getInt("PositionY");
		ms_aMobBot[MobID].m_Power = RES->getInt("Power");
		ms_aMobBot[MobID].m_Spread = RES->getInt("Spread");
		ms_aMobBot[MobID].m_Boss = RES->getBoolean("Boss");
		ms_aMobBot[MobID].m_Level = RES->getInt("Level");
		ms_aMobBot[MobID].m_RespawnTick = RES->getInt("Respawn");
		ms_aMobBot[MobID].m_BotID = BotID;
		str_copy(ms_aMobBot[MobID].m_aEffect, RES->getString("Effect").c_str(), sizeof(ms_aMobBot[MobID].m_aEffect));
		str_copy(ms_aMobBot[MobID].m_aBehavior, RES->getString("Behavior").c_str(), sizeof(ms_aMobBot[MobID].m_aBehavior));

		char aBuf[32];
		for(int i = 0; i < MAX_DROPPED_FROM_MOBS; i++)
		{
			str_format(aBuf, sizeof(aBuf), "it_drop_%d", i);
			ms_aMobBot[MobID].m_aDropItem[i] = RES->getInt(aBuf);
		}

		sscanf(RES->getString("it_drop_count").c_str(), "|%d|%d|%d|%d|%d|",
			&ms_aMobBot[MobID].m_aCountItem[0], &ms_aMobBot[MobID].m_aCountItem[1], &ms_aMobBot[MobID].m_aCountItem[2],
			&ms_aMobBot[MobID].m_aCountItem[3], &ms_aMobBot[MobID].m_aCountItem[4]);

		sscanf(RES->getString("it_drop_chance").c_str(), "|%f|%f|%f|%f|%f|",
			&ms_aMobBot[MobID].m_aRandomItem[0], &ms_aMobBot[MobID].m_aRandomItem[1], &ms_aMobBot[MobID].m_aRandomItem[2],
			&ms_aMobBot[MobID].m_aRandomItem[3], &ms_aMobBot[MobID].m_aRandomItem[4]);

		const int CountMobs = RES->getInt("Count");
		for(int c = 0; c < CountMobs; c++)
			GS()->CreateBot(BotsTypes::TYPE_BOT_MOB, BotID, MobID);
			
		GS()->Server()->AddInformationBotsCount(CountMobs);
	}
}

const char* BotJob::GetMeaninglessDialog()
{
	const char* pTalking[3] = { "[Player], do you have any questions? I'm sorry I can't help you.", 
								"What a beautiful [Time]. I don't have anything for you [Player].", 
								"[Player] are you interested something? I'm sorry, don't want to talk right now." };
	return pTalking[random_int()%3];
}

// threading CPathFinderThread botai ? TODO: protect BotPlayer?
std::mutex lockingPath;
void BotJob::FindThreadPath(class CPlayerBot* pBotPlayer, vec2 StartPos, vec2 SearchPos)
{
	if((int)StartPos.x <= 0 || (int)StartPos.y <= 0 || (int)SearchPos.x <= 0 || (int)SearchPos.y <= 0)
		return;

	std::thread([pBotPlayer, StartPos, SearchPos]()
	{
		lockingPath.lock();
		pBotPlayer->GS()->PathFinder()->Init();
		pBotPlayer->GS()->PathFinder()->SetStart(StartPos);
		pBotPlayer->GS()->PathFinder()->SetEnd(SearchPos);
		pBotPlayer->GS()->PathFinder()->FindPath();
		pBotPlayer->m_PathSize = pBotPlayer->GS()->PathFinder()->m_FinalSize;
		for(int i = pBotPlayer->m_PathSize - 1, j = 0; i >= 0; i--, j++)
			pBotPlayer->m_WayPoints[j] = vec2(pBotPlayer->GS()->PathFinder()->m_lFinalPath[i].m_Pos.x * 32 + 16, pBotPlayer->GS()->PathFinder()->m_lFinalPath[i].m_Pos.y * 32 + 16);

		lockingPath.unlock();
	}).detach();
}

void BotJob::GetThreadRandomWaypointTarget(class CPlayerBot* pBotPlayer)
{
	std::thread([this, pBotPlayer]()
	{
		lockingPath.lock();
		vec2 TargetPos = pBotPlayer->GS()->PathFinder()->GetRandomWaypoint();
		pBotPlayer->m_TargetPos = vec2(TargetPos.x * 32, TargetPos.y * 32);
		lockingPath.unlock();
	}).detach();
}