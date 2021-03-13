/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#include <base/stdafx.h>

#include "BotCore.h"

#include <game/server/gamecontext.h>

#include <game/server/mmocore/Components/Quests/QuestCore.h>

using namespace sqlstr;

// loading of all skins and mobs to use for connection with other bots
void CBotCore::OnInitWorld(const char* pWhereLocalWorld)
{
	LoadMainInformationBots();
	LoadQuestBots(pWhereLocalWorld);
	LoadNpcBots(pWhereLocalWorld);
	LoadMobsBots(pWhereLocalWorld);
}

void CBotCore::ProcessingTalkingNPC(int OwnID, int TalkingID, bool PlayerTalked, const char *Message, int Style, int TalkingEmote)
{
	if(GS()->IsMmoClient(OwnID))
	{
		GS()->SendTalkText(OwnID, TalkingID, PlayerTalked, Message, Style, TalkingEmote);
		return;
	}

	GS()->Motd(OwnID, Message);
}

void CBotCore::TalkingBotNPC(CPlayer* pPlayer, int MobID, int Progress, int TalkedID, const char *pText)
{
	const int SizeTalking = NpcBotInfo::ms_aNpcBot[MobID].m_aDialog.size();
	if(!NpcBotInfo::IsNpcBotValid(MobID) || Progress >= SizeTalking)
	{
		pPlayer->ClearTalking();
		return;
	}

	const int ClientID = pPlayer->GetCID();
	if (!GS()->IsMmoClient(ClientID))
		GS()->Broadcast(ClientID, BROADCAST_GAME_PRIORITY, 100, "Press 'F4' to continue the dialog!");

	char reformTalkedText[512];
	const int BotID = NpcBotInfo::ms_aNpcBot[MobID].m_BotID;
	if (str_comp_nocase(pText, "empty") != 0)
	{
		pPlayer->FormatDialogText(BotID, pText);
		if(!GS()->IsMmoClient(ClientID))
		{
			str_format(reformTalkedText, sizeof(reformTalkedText), "( 1 of 1 ) %s:\n- %s", NpcBotInfo::ms_aNpcBot[MobID].GetName(), pPlayer->GetDialogText());
			GS()->Broadcast(ClientID, BROADCAST_GAME_PRIORITY, 100, "Press 'F4' to continue the dialog!");
		}
		else
		{
			str_format(reformTalkedText, sizeof(reformTalkedText), "( 1 of 1 ) - %s", pPlayer->GetDialogText());
		}
		pPlayer->ClearDialogText();
		GS()->Mmo()->BotsData()->ProcessingTalkingNPC(ClientID, TalkedID, false, reformTalkedText, 0, EMOTE_BLINK);
		return;
	}

	const bool PlayerTalked = NpcBotInfo::ms_aNpcBot[MobID].m_aDialog[Progress].m_PlayerTalked;
	pPlayer->FormatDialogText(BotID, NpcBotInfo::ms_aNpcBot[MobID].m_aDialog[Progress].m_aTalkingText);
	if(!GS()->IsMmoClient(ClientID))
	{
		const char* TalkedNick = (PlayerTalked ? Server()->ClientName(ClientID) : NpcBotInfo::ms_aNpcBot[MobID].GetName());
		str_format(reformTalkedText, sizeof(reformTalkedText), "( %d of %d ) %s:\n- %s", (1 + Progress), SizeTalking, TalkedNick, pPlayer->GetDialogText());
		GS()->Broadcast(ClientID, BROADCAST_GAME_PRIORITY, 100, "Press 'F4' to continue the dialog!");
	}
	else
	{
		str_format(reformTalkedText, sizeof(reformTalkedText), "( %d of %d ) - %s", (1 + Progress), SizeTalking, pPlayer->GetDialogText());
	}
	pPlayer->ClearDialogText();
	GS()->Mmo()->BotsData()->ProcessingTalkingNPC(ClientID, TalkedID,
		PlayerTalked, reformTalkedText, NpcBotInfo::ms_aNpcBot[MobID].m_aDialog[Progress].m_Style, NpcBotInfo::ms_aNpcBot[MobID].m_aDialog[Progress].m_Emote);
}

void CBotCore::TalkingBotQuest(CPlayer* pPlayer, int MobID, int Progress, int TalkedID)
{
	const int SizeTalking = QuestBotInfo::ms_aQuestBot[MobID].m_aDialog.size();
	if(!QuestBotInfo::IsQuestBotValid(MobID) || Progress >= SizeTalking)
	{
		pPlayer->ClearTalking();
		return;
	}

	const int ClientID = pPlayer->GetCID();
	char reformTalkedText[512];
	const int BotID = QuestBotInfo::ms_aQuestBot[MobID].m_BotID;
	const bool PlayerTalked = QuestBotInfo::ms_aQuestBot[MobID].m_aDialog[Progress].m_PlayerTalked;
	pPlayer->FormatDialogText(BotID, QuestBotInfo::ms_aQuestBot[MobID].m_aDialog[Progress].m_aTalkingText);
	if(!GS()->IsMmoClient(ClientID))
	{
		const int QuestID = QuestBotInfo::ms_aQuestBot[MobID].m_QuestID;
		const char* TalkedNick = (PlayerTalked ? Server()->ClientName(ClientID) : QuestBotInfo::ms_aQuestBot[MobID].GetName());
		str_format(reformTalkedText, sizeof(reformTalkedText), "%s\n=========\n\n( %d of %d ) %s:\n- %s",
			GS()->GetQuestInfo(QuestID).GetName(), (1 + Progress), SizeTalking, TalkedNick, pPlayer->GetDialogText());
		GS()->Broadcast(ClientID, BROADCAST_GAME_PRIORITY, 100, "Press 'F4' to continue the dialog!");
	}
	else
	{
		str_format(reformTalkedText, sizeof(reformTalkedText), "( %d of %d ) - %s", (1 + Progress), SizeTalking, pPlayer->GetDialogText());
	}
	pPlayer->ClearDialogText();
	GS()->Mmo()->BotsData()->ProcessingTalkingNPC(ClientID, TalkedID,
		PlayerTalked, reformTalkedText, QuestBotInfo::ms_aQuestBot[MobID].m_aDialog[Progress].m_Style, QuestBotInfo::ms_aQuestBot[MobID].m_aDialog[Progress].m_Emote);
}

void CBotCore::ShowBotQuestTaskInfo(CPlayer* pPlayer, int MobID, int Progress)
{
	const int ClientID = pPlayer->GetCID();
	const int SizeTalking = QuestBotInfo::ms_aQuestBot[MobID].m_aDialog.size();
	if (!QuestBotInfo::IsQuestBotValid(MobID) || Progress >= SizeTalking)
	{
		pPlayer->ClearTalking();
		return;
	}

	// vanila clients
	const int BotID = QuestBotInfo::ms_aQuestBot[MobID].m_BotID;
	if (!GS()->IsMmoClient(ClientID))
	{
		const int QuestID = QuestBotInfo::ms_aQuestBot[MobID].m_QuestID;
		const bool PlayerTalked = QuestBotInfo::ms_aQuestBot[MobID].m_aDialog[Progress].m_PlayerTalked;
		const char* TalkedNick = (PlayerTalked ? Server()->ClientName(ClientID) : QuestBotInfo::ms_aQuestBot[MobID].GetName());

		char reformTalkedText[512];
		pPlayer->FormatDialogText(BotID, QuestBotInfo::ms_aQuestBot[MobID].m_aDialog[Progress].m_aTalkingText);
		str_format(reformTalkedText, sizeof(reformTalkedText), "%s\n=========\n\n( %d of %d ) %s:\n- %s",
			GS()->GetQuestInfo(QuestID).GetName(), (1 + Progress), SizeTalking, TalkedNick, pPlayer->GetDialogText());
		pPlayer->ClearDialogText();

		GS()->Mmo()->Quest()->QuestShowRequired(pPlayer, QuestBotInfo::ms_aQuestBot[MobID], reformTalkedText);
		return;
	}

	// mmo clients
	GS()->Mmo()->Quest()->QuestShowRequired(pPlayer, QuestBotInfo::ms_aQuestBot[MobID], "\0");
}

int CBotCore::GetQuestNPC(int MobID) const
{
	if (!NpcBotInfo::IsNpcBotValid(MobID))
		return -1;

	for (const auto& npc : NpcBotInfo::ms_aNpcBot[MobID].m_aDialog)
	{
		if (npc.m_GivingQuest > 0)
			return npc.m_GivingQuest;
	}
	return -1;
}


// load basic information about bots
void CBotCore::LoadMainInformationBots()
{
	if(!(DataBotInfo::ms_aDataBot.empty()))
		return;

	ResultPtr pRes = SJK.SD("*", "tw_bots_world");
	while(pRes->next())
	{
		const int BotID = (int)pRes->getInt("ID");
		str_copy(DataBotInfo::ms_aDataBot[BotID].m_aNameBot, pRes->getString("BotName").c_str(), sizeof(DataBotInfo::ms_aDataBot[BotID].m_aNameBot));

		if(!sscanf(pRes->getString("SkinName").c_str(), "%s %s %s %s %s %s",
		           DataBotInfo::ms_aDataBot[BotID].m_aaSkinNameBot[SKINPART_BODY], DataBotInfo::ms_aDataBot[BotID].m_aaSkinNameBot[SKINPART_MARKING],
		           DataBotInfo::ms_aDataBot[BotID].m_aaSkinNameBot[SKINPART_DECORATION], DataBotInfo::ms_aDataBot[BotID].m_aaSkinNameBot[SKINPART_HANDS],
		           DataBotInfo::ms_aDataBot[BotID].m_aaSkinNameBot[SKINPART_FEET], DataBotInfo::ms_aDataBot[BotID].m_aaSkinNameBot[SKINPART_EYES]))
			dbg_msg("Error", "Mised bots information");

		if(!sscanf(pRes->getString("SkinColor").c_str(), "%d %d %d %d %d %d",
			&DataBotInfo::ms_aDataBot[BotID].m_aSkinColorBot[SKINPART_BODY], &DataBotInfo::ms_aDataBot[BotID].m_aSkinColorBot[SKINPART_MARKING],
			&DataBotInfo::ms_aDataBot[BotID].m_aSkinColorBot[SKINPART_DECORATION], &DataBotInfo::ms_aDataBot[BotID].m_aSkinColorBot[SKINPART_HANDS],
			&DataBotInfo::ms_aDataBot[BotID].m_aSkinColorBot[SKINPART_FEET], &DataBotInfo::ms_aDataBot[BotID].m_aSkinColorBot[SKINPART_EYES]))
			dbg_msg("Error", "Mised bots information");

		for(int j = SKINPART_BODY; j < NUM_SKINPARTS; j++) {
			if(DataBotInfo::ms_aDataBot[BotID].m_aSkinColorBot[j] != 0)
				DataBotInfo::ms_aDataBot[BotID].m_aUseCustomBot[j] = true;
		}

		for(int i = 0; i < MAX_PLAYERS; i++)
			DataBotInfo::ms_aDataBot[BotID].m_aAlreadyActiveQuestBot[i] = false;

		DataBotInfo::ms_aDataBot[BotID].m_aEquipSlot[EQUIP_HAMMER] = pRes->getInt("SlotHammer");
		DataBotInfo::ms_aDataBot[BotID].m_aEquipSlot[EQUIP_GUN] = pRes->getInt("SlotGun");
		DataBotInfo::ms_aDataBot[BotID].m_aEquipSlot[EQUIP_SHOTGUN] = pRes->getInt("SlotShotgun");
		DataBotInfo::ms_aDataBot[BotID].m_aEquipSlot[EQUIP_GRENADE] = pRes->getInt("SlotGrenade");
		DataBotInfo::ms_aDataBot[BotID].m_aEquipSlot[EQUIP_RIFLE] = pRes->getInt("SlotRifle");
		DataBotInfo::ms_aDataBot[BotID].m_aEquipSlot[EQUIP_MINER] = 0;
		DataBotInfo::ms_aDataBot[BotID].m_aEquipSlot[EQUIP_WINGS] = pRes->getInt("SlotWings");
	}
}

// load quest bots
void CBotCore::LoadQuestBots(const char* pWhereLocalWorld)
{
	ResultPtr pRes = SJK.SD("*", "tw_bots_quest", pWhereLocalWorld);
	while(pRes->next())
	{
		// it for every world initilize quest progress size
		const int MobID = (int)pRes->getInt("ID");
		QuestBotInfo::ms_aQuestBot[MobID].m_SubBotID = MobID;
		QuestBotInfo::ms_aQuestBot[MobID].m_BotID = (int)pRes->getInt("BotID");
		QuestBotInfo::ms_aQuestBot[MobID].m_QuestID = (int)pRes->getInt("QuestID");
		QuestBotInfo::ms_aQuestBot[MobID].m_Step = (int)pRes->getInt("Step");
		QuestBotInfo::ms_aQuestBot[MobID].m_WorldID = (int)pRes->getInt("WorldID");
		QuestBotInfo::ms_aQuestBot[MobID].m_PositionX = (int)pRes->getInt("pos_x");
		QuestBotInfo::ms_aQuestBot[MobID].m_PositionY = (int)pRes->getInt("pos_y") + 1;
		QuestBotInfo::ms_aQuestBot[MobID].m_aItemSearch[0] = (int)pRes->getInt("it_need_0");
		QuestBotInfo::ms_aQuestBot[MobID].m_aItemSearch[1] = (int)pRes->getInt("it_need_1");
		QuestBotInfo::ms_aQuestBot[MobID].m_aItemGives[0] = (int)pRes->getInt("it_reward_0");
		QuestBotInfo::ms_aQuestBot[MobID].m_aItemGives[1] = (int)pRes->getInt("it_reward_1");
		QuestBotInfo::ms_aQuestBot[MobID].m_aNeedMob[0] = (int)pRes->getInt("mob_0");
		QuestBotInfo::ms_aQuestBot[MobID].m_aNeedMob[1] = (int)pRes->getInt("mob_1");
		QuestBotInfo::ms_aQuestBot[MobID].m_InteractiveType = (int)pRes->getInt("interactive_type");
		QuestBotInfo::ms_aQuestBot[MobID].m_InteractiveTemp = (int)pRes->getInt("interactive_temp");
		QuestBotInfo::ms_aQuestBot[MobID].m_GenerateNick = (bool)pRes->getBoolean("generate_nick");

		sscanf(pRes->getString("it_count").c_str(), "|%d|%d|%d|%d|%d|%d|",
			&QuestBotInfo::ms_aQuestBot[MobID].m_aItemSearchCount[0], &QuestBotInfo::ms_aQuestBot[MobID].m_aItemSearchCount[1],
			&QuestBotInfo::ms_aQuestBot[MobID].m_aItemGivesCount[0], &QuestBotInfo::ms_aQuestBot[MobID].m_aItemGivesCount[1],
			&QuestBotInfo::ms_aQuestBot[MobID].m_aNeedMobCount[0], &QuestBotInfo::ms_aQuestBot[MobID].m_aNeedMobCount[1]);

		// load talk
		ResultPtr pResTalk = SJK.SD("*", "tw_talk_quest_npc", "WHERE MobID = '%d'", MobID);
		while(pResTalk->next())
		{
			DialogData LoadTalk;
			LoadTalk.m_Emote = pResTalk->getInt("TalkingEmote");
			LoadTalk.m_Style = pResTalk->getInt("Style");
			LoadTalk.m_PlayerTalked = pResTalk->getBoolean("PlayerTalked");
			LoadTalk.m_RequestComplete = pResTalk->getBoolean("RequestComplete");
			str_copy(LoadTalk.m_aTalkingText, pResTalk->getString("TalkText").c_str(), sizeof(LoadTalk.m_aTalkingText));
			QuestBotInfo::ms_aQuestBot[MobID].m_aDialog.push_back(LoadTalk);
		}
	}

	// init quests bots step
	for(auto& pQuestBot : QuestBotInfo::ms_aQuestBot)
	{
		int QuestID = pQuestBot.second.m_QuestID;
		CQuestDataInfo::ms_aDataQuests[QuestID].m_StepsQuestBot[pQuestBot.first].m_Bot = &pQuestBot.second;
	}
}

// load NPC
void CBotCore::LoadNpcBots(const char* pWhereLocalWorld)
{
	ResultPtr pRes = SJK.SD("*", "tw_bots_npc", pWhereLocalWorld);
	while(pRes->next())
	{
		const int MobID = (int)pRes->getInt("ID");
		NpcBotInfo::ms_aNpcBot[MobID].m_WorldID = pRes->getInt("WorldID");
		NpcBotInfo::ms_aNpcBot[MobID].m_Static = pRes->getBoolean("Static");
		NpcBotInfo::ms_aNpcBot[MobID].m_PositionX = pRes->getInt("PositionX");
		NpcBotInfo::ms_aNpcBot[MobID].m_PositionY = (NpcBotInfo::ms_aNpcBot[MobID].m_Static ? pRes->getInt("PositionY") + 1 : pRes->getInt("PositionY"));
		NpcBotInfo::ms_aNpcBot[MobID].m_Emote = pRes->getInt("Emote");
		NpcBotInfo::ms_aNpcBot[MobID].m_BotID = pRes->getInt("BotID");
		NpcBotInfo::ms_aNpcBot[MobID].m_Function = pRes->getInt("Function");

		const int CountMobs = pRes->getInt("Count");
		for(int c = 0; c < CountMobs; c++)
			GS()->CreateBot(TYPE_BOT_NPC, NpcBotInfo::ms_aNpcBot[MobID].m_BotID, MobID);

		ResultPtr pResTalk = SJK.SD("*", "tw_talk_other_npc", "WHERE MobID = '%d'", MobID);
		while(pResTalk->next())
		{
			DialogData LoadTalk;
			LoadTalk.m_Emote = pResTalk->getInt("TalkingEmote");
			LoadTalk.m_Style = pResTalk->getInt("Style");
			LoadTalk.m_PlayerTalked = pResTalk->getBoolean("PlayerTalked");
			LoadTalk.m_GivingQuest = pResTalk->getInt("GivingQuest");
			str_copy(LoadTalk.m_aTalkingText, pResTalk->getString("TalkText").c_str(), sizeof(LoadTalk.m_aTalkingText));
			NpcBotInfo::ms_aNpcBot[MobID].m_aDialog.push_back(LoadTalk);

			if(LoadTalk.m_GivingQuest > 0)
				NpcBotInfo::ms_aNpcBot[MobID].m_Function = FUNCTION_NPC_GIVE_QUEST;
		}
	}
}

// load mobs
void CBotCore::LoadMobsBots(const char* pWhereLocalWorld)
{
	ResultPtr pRes = SJK.SD("*", "tw_bots_mobs", pWhereLocalWorld);
	while(pRes->next())
	{
		const int MobID = (int)pRes->getInt("ID");
		const int BotID = pRes->getInt("BotID");
		MobBotInfo::ms_aMobBot[MobID].m_WorldID = pRes->getInt("WorldID");
		MobBotInfo::ms_aMobBot[MobID].m_PositionX = pRes->getInt("PositionX");
		MobBotInfo::ms_aMobBot[MobID].m_PositionY = pRes->getInt("PositionY");
		MobBotInfo::ms_aMobBot[MobID].m_Power = pRes->getInt("Power");
		MobBotInfo::ms_aMobBot[MobID].m_Spread = pRes->getInt("Spread");
		MobBotInfo::ms_aMobBot[MobID].m_Boss = pRes->getBoolean("Boss");
		MobBotInfo::ms_aMobBot[MobID].m_Level = pRes->getInt("Level");
		MobBotInfo::ms_aMobBot[MobID].m_RespawnTick = pRes->getInt("Respawn");
		MobBotInfo::ms_aMobBot[MobID].m_BotID = BotID;
		str_copy(MobBotInfo::ms_aMobBot[MobID].m_aEffect, pRes->getString("Effect").c_str(), sizeof(MobBotInfo::ms_aMobBot[MobID].m_aEffect));
		str_copy(MobBotInfo::ms_aMobBot[MobID].m_aBehavior, pRes->getString("Behavior").c_str(), sizeof(MobBotInfo::ms_aMobBot[MobID].m_aBehavior));

		char aBuf[32];
		for(int i = 0; i < MAX_DROPPED_FROM_MOBS; i++)
		{
			str_format(aBuf, sizeof(aBuf), "it_drop_%d", i);
			MobBotInfo::ms_aMobBot[MobID].m_aDropItem[i] = pRes->getInt(aBuf);
		}

		sscanf(pRes->getString("it_drop_count").c_str(), "|%d|%d|%d|%d|%d|",
			&MobBotInfo::ms_aMobBot[MobID].m_aCountItem[0], &MobBotInfo::ms_aMobBot[MobID].m_aCountItem[1], &MobBotInfo::ms_aMobBot[MobID].m_aCountItem[2],
			&MobBotInfo::ms_aMobBot[MobID].m_aCountItem[3], &MobBotInfo::ms_aMobBot[MobID].m_aCountItem[4]);

		sscanf(pRes->getString("it_drop_chance").c_str(), "|%f|%f|%f|%f|%f|",
			&MobBotInfo::ms_aMobBot[MobID].m_aRandomItem[0], &MobBotInfo::ms_aMobBot[MobID].m_aRandomItem[1], &MobBotInfo::ms_aMobBot[MobID].m_aRandomItem[2],
			&MobBotInfo::ms_aMobBot[MobID].m_aRandomItem[3], &MobBotInfo::ms_aMobBot[MobID].m_aRandomItem[4]);

		const int CountMobs = pRes->getInt("Count");
		for(int c = 0; c < CountMobs; c++)
			GS()->CreateBot(TYPE_BOT_MOB, BotID, MobID);
	}
}

const char* CBotCore::GetMeaninglessDialog()
{
	const char* pTalking[3] =
	{
		"[Player], do you have any questions? I'm sorry I can't help you.",
		"What a beautiful [Time]. I don't have anything for you [Player].",
		"[Player] are you interested something? I'm sorry, don't want to talk right now."
	};
	return pTalking[random_int()%3];
}

// add a new bot
void CBotCore::ConAddCharacterBot(int ClientID, const char* pCharacter)
{
	CPlayer* pPlayer = GS()->GetPlayer(ClientID);
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
