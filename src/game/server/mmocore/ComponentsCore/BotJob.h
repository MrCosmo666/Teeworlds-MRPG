/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#ifndef GAME_SERVER_BOTS_INFO_SQL_H
#define GAME_SERVER_BOTS_INFO_SQL_H

#include "../MmoComponent.h"
 
class BotJob : public MmoComponent
{
	~BotJob()
	{
		ms_aDataBot.clear();
		ms_aQuestBot.clear();
		ms_aNpcBot.clear();
		ms_aMobBot.clear();
	};

	// bots talking data
	struct DialogData
	{
		char m_aTalkingText[512];
		int m_Style;
		int m_Emote;
		int m_GivingQuest;
		bool m_RequestComplete;
		bool m_PlayerTalked;
	};

	// main bots information
	struct StructDataBot
	{
		char m_aNameBot[MAX_NAME_ARRAY_SIZE];
		char m_aaSkinNameBot[NUM_SKINPARTS][MAX_SKIN_ARRAY_SIZE];
		int m_aUseCustomBot[NUM_SKINPARTS];
		int m_aSkinColorBot[NUM_SKINPARTS];
		int m_aEquipSlot[MAX_EQUIPPED_SLOTS_BOTS];
		bool m_aAlreadyActiveQuestBot[MAX_PLAYERS];
	};

	// npc type data information
	struct StructNpcBot
	{
		const char* GetName() const
		{
			dbg_assert(ms_aDataBot.find(m_BotID) != ms_aDataBot.end(), "Name bot it invalid");
			return ms_aDataBot[m_BotID].m_aNameBot;
		}

		bool m_Static;
		int m_PositionX;
		int m_PositionY;
		int m_Emote;
		int m_WorldID;
		int m_BotID;
		int m_Function;
		std::vector < DialogData > m_aDialog;
	};

	// quest type data information
	struct StructQuestBot
	{
		const char* GetName() const
		{
			dbg_assert(ms_aDataBot.find(m_BotID) != ms_aDataBot.end(), "Name bot it invalid");
			return ms_aDataBot[m_BotID].m_aNameBot;
		}

		int m_PositionX;
		int m_PositionY;
		int m_QuestID;
		int m_Step;
		int m_WorldID;
		int m_BotID;
		int m_SubBotID;

		int m_aItemSearch[2];
		int m_aItemSearchCount[2];

		int m_aItemGives[2];
		int m_aItemGivesCount[2];

		int m_aNeedMob[2];
		int m_aNeedMobCount[2];

		int m_InteractiveType;
		int m_InteractiveTemp;
		bool m_GenerateNick;
		std::vector < DialogData > m_aDialog;
	};

	// mob type data information
	struct StructMobsBot
	{
		const char* GetName() const
		{
			dbg_assert(ms_aDataBot.find(m_BotID) != ms_aDataBot.end(), "Name bot it invalid");
			return ms_aDataBot[m_BotID].m_aNameBot;
		}
		bool m_Boss;
		int m_Power;
		int m_Spread;
		int m_PositionX;
		int m_PositionY;
		int m_Level;
		int m_RespawnTick;
		int m_WorldID;
		char m_aEffect[16];
		char m_aBehavior[32];

		int m_aDropItem[MAX_DROPPED_FROM_MOBS];
		int m_aCountItem[MAX_DROPPED_FROM_MOBS];
		float m_aRandomItem[MAX_DROPPED_FROM_MOBS];
		int m_BotID;
	};

	void OnInitWorld(const char* pWhereLocalWorld) override;

	void LoadMainInformationBots();
	void LoadQuestBots(const char* pWhereLocalWorld);
	void LoadNpcBots(const char* pWhereLocalWorld);
	void LoadMobsBots(const char* pWhereLocalWorld);

public:
	typedef StructDataBot DataBotInfo;
	static std::map < int , DataBotInfo > ms_aDataBot;

	typedef StructQuestBot QuestBotInfo;
	static std::map < int , QuestBotInfo > ms_aQuestBot;

	typedef StructNpcBot NpcBotInfo;
	static std::map < int , NpcBotInfo > ms_aNpcBot;

	typedef StructMobsBot MobBotInfo;
	static std::map < int , MobBotInfo > ms_aMobBot;

	void ConAddCharacterBot(int ClientID, const char *pCharacter);
	void ProcessingTalkingNPC(int OwnID, int TalkingID, bool PlayerTalked, const char *Message, int Style, int TalkingEmote);
	void TalkingBotNPC(CPlayer* pPlayer, int MobID, int Progress, int TalkedID, const char *pText = "empty");
	void TalkingBotQuest(CPlayer* pPlayer, int MobID, int Progress, int TalkedID);
	void ShowBotQuestTaskInfo(CPlayer* pPlayer, int MobID, int Progress);
	int GetQuestNPC(int MobID) const;
	const char *GetMeaninglessDialog();

	// ------------------ CHECK VALID DATA --------------------
	bool IsDataBotValid(int BotID) const { return (ms_aDataBot.find(BotID) != ms_aDataBot.end()); }
	bool IsNpcBotValid(int MobID) const 
	{ 
		if(ms_aNpcBot.find(MobID) != ms_aNpcBot.end() && IsDataBotValid(ms_aNpcBot[MobID].m_BotID))
			return true;
		return false; 
	}
	bool IsMobBotValid(int MobID) const 
	{ 
		if(ms_aMobBot.find(MobID) != ms_aMobBot.end() && IsDataBotValid(ms_aMobBot[MobID].m_BotID))
			return true;
		return false;
	}
	bool IsQuestBotValid(int MobID) const 
	{ 
		if(ms_aQuestBot.find(MobID) != ms_aQuestBot.end() && IsDataBotValid(ms_aQuestBot[MobID].m_BotID))
			return true;
		return false;
	}
};

#endif