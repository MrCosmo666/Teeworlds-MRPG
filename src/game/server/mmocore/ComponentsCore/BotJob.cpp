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
	ResultPtr pRes = SJK.SD("*", "tw_bots_world", "WHERE BotName = '%s'", cNick.cstr());
	if(pRes->next())
	{
		// if the nickname is not in the database
		const int ID = pRes->getInt("ID");
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
	if(GS()->IsMmoClient(OwnID))
	{
		GS()->SendTalkText(OwnID, TalkingID, PlayerTalked, Message, Style, TalkingEmote);
		return;
	}

	GS()->Motd(OwnID, Message);
}

void BotJob::TalkingBotNPC(CPlayer* pPlayer, int MobID, int Progress, int TalkedID, const char *pText)
{
	const int SizeTalking = BotJob::ms_aNpcBot[MobID].m_aTalk.size();
	if(!IsNpcBotValid(MobID) || Progress >= SizeTalking)
	{
		pPlayer->ClearTalking();
		return;
	}

	const int ClientID = pPlayer->GetCID();
	if (!GS()->IsMmoClient(ClientID))
		GS()->Broadcast(ClientID, BroadcastPriority::BROADCAST_GAME_PRIORITY, 100, "Press 'F4' to continue the dialog!");

	char reformTalkedText[512];
	const int BotID = ms_aNpcBot[MobID].m_BotID;
	if (str_comp_nocase(pText, "empty") != 0)
	{
		pPlayer->FormatTextQuest(BotID, pText);
		if(!GS()->IsMmoClient(ClientID))
		{
			str_format(reformTalkedText, sizeof(reformTalkedText), "( 1 of 1 ) %s:\n- %s", ms_aNpcBot[MobID].GetName(), pPlayer->FormatedTalkedText());
			GS()->Broadcast(ClientID, BroadcastPriority::BROADCAST_GAME_PRIORITY, 100, "Press 'F4' to continue the dialog!");
		}
		else
		{
			str_format(reformTalkedText, sizeof(reformTalkedText), "( 1 of 1 ) - %s", pPlayer->FormatedTalkedText());
		}
		pPlayer->ClearFormatQuestText();
		GS()->Mmo()->BotsData()->ProcessingTalkingNPC(ClientID, TalkedID, false, reformTalkedText, 0, EMOTE_BLINK);
		return;
	}

	const bool PlayerTalked = ms_aNpcBot[MobID].m_aTalk[Progress].m_PlayerTalked;
	pPlayer->FormatTextQuest(BotID, ms_aNpcBot[MobID].m_aTalk[Progress].m_aTalkingText);
	if(!GS()->IsMmoClient(ClientID))
	{
		const char* TalkedNick = (PlayerTalked ? GS()->Server()->ClientName(ClientID) : ms_aNpcBot[MobID].GetName());
		str_format(reformTalkedText, sizeof(reformTalkedText), "( %d of %d ) %s:\n- %s", (1 + Progress), SizeTalking, TalkedNick, pPlayer->FormatedTalkedText());
		GS()->Broadcast(ClientID, BroadcastPriority::BROADCAST_GAME_PRIORITY, 100, "Press 'F4' to continue the dialog!");
	}
	else
	{
		str_format(reformTalkedText, sizeof(reformTalkedText), "( %d of %d ) - %s", (1 + Progress), SizeTalking, pPlayer->FormatedTalkedText());
	}
	pPlayer->ClearFormatQuestText();
	GS()->Mmo()->BotsData()->ProcessingTalkingNPC(ClientID, TalkedID,
		PlayerTalked, reformTalkedText, ms_aNpcBot[MobID].m_aTalk[Progress].m_Style, ms_aNpcBot[MobID].m_aTalk[Progress].m_Emote);
}

void BotJob::TalkingBotQuest(CPlayer* pPlayer, int MobID, int Progress, int TalkedID)
{
	const int SizeTalking = BotJob::ms_aQuestBot[MobID].m_aTalk.size();
	if(!IsQuestBotValid(MobID) || Progress >= SizeTalking)
	{
		pPlayer->ClearTalking();
		return;
	}

	const int ClientID = pPlayer->GetCID();
	char reformTalkedText[512];
	const int BotID = ms_aQuestBot[MobID].m_BotID;
	const bool PlayerTalked = ms_aQuestBot[MobID].m_aTalk[Progress].m_PlayerTalked;
	pPlayer->FormatTextQuest(BotID, ms_aQuestBot[MobID].m_aTalk[Progress].m_aTalkingText);
	if(!GS()->IsMmoClient(ClientID))
	{
		const int QuestID = ms_aQuestBot[MobID].m_QuestID;
		const char* TalkedNick = (PlayerTalked ? GS()->Server()->ClientName(ClientID) : ms_aQuestBot[MobID].GetName());
		str_format(reformTalkedText, sizeof(reformTalkedText), "%s\n=========\n\n( %d of %d ) %s:\n- %s", 
			GS()->GetQuestInfo(QuestID).GetName(), (1 + Progress), SizeTalking, TalkedNick, pPlayer->FormatedTalkedText());
		GS()->Broadcast(ClientID, BroadcastPriority::BROADCAST_GAME_PRIORITY, 100, "Press 'F4' to continue the dialog!");
	}
	else
	{
		str_format(reformTalkedText, sizeof(reformTalkedText), "( %d of %d ) - %s", (1 + Progress), SizeTalking, pPlayer->FormatedTalkedText());
	}
	pPlayer->ClearFormatQuestText();
	GS()->Mmo()->BotsData()->ProcessingTalkingNPC(ClientID, TalkedID,
		PlayerTalked, reformTalkedText, ms_aQuestBot[MobID].m_aTalk[Progress].m_Style, ms_aQuestBot[MobID].m_aTalk[Progress].m_Emote);
}

void BotJob::ShowBotQuestTaskInfo(CPlayer* pPlayer, int MobID, int Progress)
{
	const int ClientID = pPlayer->GetCID();
	const int SizeTalking = BotJob::ms_aQuestBot[MobID].m_aTalk.size();
	if (!IsQuestBotValid(MobID) || Progress >= SizeTalking)
	{
		pPlayer->ClearTalking();
		return;
	}

	// vanila clients
	const int BotID = BotJob::ms_aQuestBot[MobID].m_BotID;
	if (!GS()->IsMmoClient(ClientID))
	{
		const int QuestID = ms_aQuestBot[MobID].m_QuestID;
		const bool PlayerTalked = ms_aQuestBot[MobID].m_aTalk[Progress].m_PlayerTalked;
		const char* TalkedNick = (PlayerTalked ? GS()->Server()->ClientName(ClientID) : ms_aQuestBot[MobID].GetName());

		char reformTalkedText[512];
		pPlayer->FormatTextQuest(BotID, BotJob::ms_aQuestBot[MobID].m_aTalk[Progress].m_aTalkingText);
		str_format(reformTalkedText, sizeof(reformTalkedText), "%s\n=========\n\n( %d of %d ) %s:\n- %s", 
			GS()->GetQuestInfo(QuestID).GetName(), (1 + Progress), SizeTalking, TalkedNick, pPlayer->FormatedTalkedText());
		pPlayer->ClearFormatQuestText();

		GS()->Mmo()->Quest()->QuestShowRequired(pPlayer, BotJob::ms_aQuestBot[MobID], reformTalkedText);
		return;
	}

	// mmo clients
	GS()->Mmo()->Quest()->QuestShowRequired(pPlayer, BotJob::ms_aQuestBot[MobID], "\0");
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

	ResultPtr pRes = SJK.SD("*", "tw_bots_world");
	while(pRes->next())
	{
		const int BotID = (int)pRes->getInt("ID");
		str_copy(ms_aDataBot[BotID].m_aNameBot, pRes->getString("BotName").c_str(), sizeof(ms_aDataBot[BotID].m_aNameBot));

		if(!sscanf(pRes->getString("SkinName").c_str(), "%s %s %s %s %s %s",
			ms_aDataBot[BotID].m_aaSkinNameBot[SKINPART_BODY], ms_aDataBot[BotID].m_aaSkinNameBot[SKINPART_MARKING],
			ms_aDataBot[BotID].m_aaSkinNameBot[SKINPART_DECORATION], ms_aDataBot[BotID].m_aaSkinNameBot[SKINPART_HANDS],
			ms_aDataBot[BotID].m_aaSkinNameBot[SKINPART_FEET], ms_aDataBot[BotID].m_aaSkinNameBot[SKINPART_EYES]))
			dbg_msg("Error", "Mised bots information");

		if(!sscanf(pRes->getString("SkinColor").c_str(), "%d %d %d %d %d %d",
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

		ms_aDataBot[BotID].m_aEquipSlot[EQUIP_HAMMER] = pRes->getInt("SlotHammer");
		ms_aDataBot[BotID].m_aEquipSlot[EQUIP_GUN] = pRes->getInt("SlotGun");
		ms_aDataBot[BotID].m_aEquipSlot[EQUIP_SHOTGUN] = pRes->getInt("SlotShotgun");
		ms_aDataBot[BotID].m_aEquipSlot[EQUIP_GRENADE] = pRes->getInt("SlotGrenade");
		ms_aDataBot[BotID].m_aEquipSlot[EQUIP_RIFLE] = pRes->getInt("SlotRifle");
		ms_aDataBot[BotID].m_aEquipSlot[EQUIP_MINER] = 0;
		ms_aDataBot[BotID].m_aEquipSlot[EQUIP_WINGS] = pRes->getInt("SlotWings");
	}
}

// load quest bots
void BotJob::LoadQuestBots(const char* pWhereLocalWorld)
{
	ResultPtr pRes = SJK.SD("*", "tw_bots_quest", pWhereLocalWorld);
	while(pRes->next())
	{
		// it for every world initilize quest progress size
		const int MobID = (int)pRes->getInt("ID");
		ms_aQuestBot[MobID].m_SubBotID = MobID;
		ms_aQuestBot[MobID].m_BotID = (int)pRes->getInt("BotID");
		ms_aQuestBot[MobID].m_QuestID = (int)pRes->getInt("QuestID");
		ms_aQuestBot[MobID].m_Step = (int)pRes->getInt("Step");
		ms_aQuestBot[MobID].m_WorldID = (int)pRes->getInt("WorldID");
		ms_aQuestBot[MobID].m_PositionX = (int)pRes->getInt("pos_x");
		ms_aQuestBot[MobID].m_PositionY = (int)pRes->getInt("pos_y") + 1;
		ms_aQuestBot[MobID].m_aItemSearch[0] = (int)pRes->getInt("it_need_0");
		ms_aQuestBot[MobID].m_aItemSearch[1] = (int)pRes->getInt("it_need_1");
		ms_aQuestBot[MobID].m_aItemGives[0] = (int)pRes->getInt("it_reward_0");
		ms_aQuestBot[MobID].m_aItemGives[1] = (int)pRes->getInt("it_reward_1");
		ms_aQuestBot[MobID].m_aNeedMob[0] = (int)pRes->getInt("mob_0");
		ms_aQuestBot[MobID].m_aNeedMob[1] = (int)pRes->getInt("mob_1");
		ms_aQuestBot[MobID].m_InteractiveType = (int)pRes->getInt("interactive_type");
		ms_aQuestBot[MobID].m_InteractiveTemp = (int)pRes->getInt("interactive_temp");
		ms_aQuestBot[MobID].m_GenerateNick = (bool)pRes->getBoolean("generate_nick");

		sscanf(pRes->getString("it_count").c_str(), "|%d|%d|%d|%d|%d|%d|",
			&ms_aQuestBot[MobID].m_aItemSearchCount[0], &ms_aQuestBot[MobID].m_aItemSearchCount[1],
			&ms_aQuestBot[MobID].m_aItemGivesCount[0], &ms_aQuestBot[MobID].m_aItemGivesCount[1],
			&ms_aQuestBot[MobID].m_aNeedMobCount[0], &ms_aQuestBot[MobID].m_aNeedMobCount[1]);

		// load talk
		ResultPtr pResTalk = SJK.SD("*", "tw_talk_quest_npc", "WHERE MobID = '%d'", MobID);
		while(pResTalk->next())
		{
			TalkingData LoadTalk;
			LoadTalk.m_Emote = pResTalk->getInt("TalkingEmote");
			LoadTalk.m_Style = pResTalk->getInt("Style");
			LoadTalk.m_PlayerTalked = pResTalk->getBoolean("PlayerTalked");
			LoadTalk.m_RequestComplete = pResTalk->getBoolean("RequestComplete");
			str_copy(LoadTalk.m_aTalkingText, pResTalk->getString("TalkText").c_str(), sizeof(LoadTalk.m_aTalkingText));
			ms_aQuestBot[MobID].m_aTalk.push_back(LoadTalk);
		}
	}

	// init quests bots step
	for(auto& pQuestBot : ms_aQuestBot)
	{
		int QuestID = pQuestBot.second.m_QuestID;
		QuestJob::ms_aDataQuests[QuestID].m_StepsQuestBot[pQuestBot.first].m_Bot = &pQuestBot.second;
	}
}

// load NPC
void BotJob::LoadNpcBots(const char* pWhereLocalWorld)
{
	ResultPtr pRes = SJK.SD("*", "tw_bots_npc", pWhereLocalWorld);
	while(pRes->next())
	{
		const int MobID = (int)pRes->getInt("ID");
		ms_aNpcBot[MobID].m_WorldID = pRes->getInt("WorldID");
		ms_aNpcBot[MobID].m_Static = pRes->getBoolean("Static");
		ms_aNpcBot[MobID].m_PositionX = pRes->getInt("PositionX");
		ms_aNpcBot[MobID].m_PositionY = (ms_aNpcBot[MobID].m_Static ? pRes->getInt("PositionY") + 1 : pRes->getInt("PositionY"));
		ms_aNpcBot[MobID].m_Emote = pRes->getInt("Emote");
		ms_aNpcBot[MobID].m_BotID = pRes->getInt("BotID");
		ms_aNpcBot[MobID].m_Function = pRes->getInt("Function");

		const int CountMobs = pRes->getInt("Count");
		for(int c = 0; c < CountMobs; c++)
			GS()->CreateBot(BotsTypes::TYPE_BOT_NPC, ms_aNpcBot[MobID].m_BotID, MobID);

		ResultPtr pResTalk = SJK.SD("*", "tw_talk_other_npc", "WHERE MobID = '%d'", MobID);
		while(pResTalk->next())
		{
			TalkingData LoadTalk;
			LoadTalk.m_Emote = pResTalk->getInt("TalkingEmote");
			LoadTalk.m_Style = pResTalk->getInt("Style");
			LoadTalk.m_PlayerTalked = pResTalk->getBoolean("PlayerTalked");
			LoadTalk.m_GivingQuest = pResTalk->getInt("GivingQuest");
			str_copy(LoadTalk.m_aTalkingText, pResTalk->getString("TalkText").c_str(), sizeof(LoadTalk.m_aTalkingText));
			ms_aNpcBot[MobID].m_aTalk.push_back(LoadTalk);

			if(LoadTalk.m_GivingQuest > 0)
				ms_aNpcBot[MobID].m_Function = FunctionsNPC::FUNCTION_NPC_GIVE_QUEST;
		}
	}
}

// load mobs
void BotJob::LoadMobsBots(const char* pWhereLocalWorld)
{
	ResultPtr pRes = SJK.SD("*", "tw_bots_mobs", pWhereLocalWorld);
	while(pRes->next())
	{
		const int MobID = (int)pRes->getInt("ID");
		const int BotID = pRes->getInt("BotID");
		ms_aMobBot[MobID].m_WorldID = pRes->getInt("WorldID");
		ms_aMobBot[MobID].m_PositionX = pRes->getInt("PositionX");
		ms_aMobBot[MobID].m_PositionY = pRes->getInt("PositionY");
		ms_aMobBot[MobID].m_Power = pRes->getInt("Power");
		ms_aMobBot[MobID].m_Spread = pRes->getInt("Spread");
		ms_aMobBot[MobID].m_Boss = pRes->getBoolean("Boss");
		ms_aMobBot[MobID].m_Level = pRes->getInt("Level");
		ms_aMobBot[MobID].m_RespawnTick = pRes->getInt("Respawn");
		ms_aMobBot[MobID].m_BotID = BotID;
		str_copy(ms_aMobBot[MobID].m_aEffect, pRes->getString("Effect").c_str(), sizeof(ms_aMobBot[MobID].m_aEffect));
		str_copy(ms_aMobBot[MobID].m_aBehavior, pRes->getString("Behavior").c_str(), sizeof(ms_aMobBot[MobID].m_aBehavior));

		char aBuf[32];
		for(int i = 0; i < MAX_DROPPED_FROM_MOBS; i++)
		{
			str_format(aBuf, sizeof(aBuf), "it_drop_%d", i);
			ms_aMobBot[MobID].m_aDropItem[i] = pRes->getInt(aBuf);
		}

		sscanf(pRes->getString("it_drop_count").c_str(), "|%d|%d|%d|%d|%d|",
			&ms_aMobBot[MobID].m_aCountItem[0], &ms_aMobBot[MobID].m_aCountItem[1], &ms_aMobBot[MobID].m_aCountItem[2],
			&ms_aMobBot[MobID].m_aCountItem[3], &ms_aMobBot[MobID].m_aCountItem[4]);

		sscanf(pRes->getString("it_drop_chance").c_str(), "|%f|%f|%f|%f|%f|",
			&ms_aMobBot[MobID].m_aRandomItem[0], &ms_aMobBot[MobID].m_aRandomItem[1], &ms_aMobBot[MobID].m_aRandomItem[2],
			&ms_aMobBot[MobID].m_aRandomItem[3], &ms_aMobBot[MobID].m_aRandomItem[4]);

		const int CountMobs = pRes->getInt("Count");
		for(int c = 0; c < CountMobs; c++)
			GS()->CreateBot(BotsTypes::TYPE_BOT_MOB, BotID, MobID);
	}
}

const char* BotJob::GetMeaninglessDialog()
{
	const char* pTalking[3] = { 
		"[Player], do you have any questions? I'm sorry I can't help you.", 
		"What a beautiful [Time]. I don't have anything for you [Player].", 
		"[Player] are you interested something? I'm sorry, don't want to talk right now." };
	return pTalking[random_int()%3];
}

// threading CPathFinderThread botai ? TODO: protect BotPlayer?
std::mutex lockingPath;
void BotJob::FindThreadPath(class CPlayerBot* pBotPlayer, vec2 StartPos, vec2 SearchPos)
{
	if(length(StartPos) <= 0 || length(SearchPos) <= 0 || GS()->Collision()->CheckPoint(StartPos) || GS()->Collision()->CheckPoint(SearchPos))
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
		{
			pBotPlayer->m_WayPoints[j] = vec2(pBotPlayer->GS()->PathFinder()->m_lFinalPath[i].m_Pos.x * 32 + 16, pBotPlayer->GS()->PathFinder()->m_lFinalPath[i].m_Pos.y * 32 + 16);
		}
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