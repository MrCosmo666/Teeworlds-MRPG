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
	struct TalkingData
	{
		char m_aTalkingText[512];
		int m_Style;
		int m_Emote;
		int m_GivingQuest;
		bool m_RequestComplete;
		bool m_PlayerTalked;
	};

	// main bots information
	struct DescDataBot
	{
		char m_aNameBot[16];
		char m_aaSkinNameBot[6][24];
		int m_aUseCustomBot[6];
		int m_aSkinColorBot[6];
		int m_aEquipSlot[MAX_EQUIPPED_SLOTS_BOTS];
		bool m_aAlreadySnapQuestBot[MAX_PLAYERS];
	};

	// types bots information
	struct ClassNpcBot
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
		std::vector < TalkingData > m_aTalk;
	};

	struct ClassQuestBot
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
		std::vector < TalkingData > m_aTalk;
	};

	struct ClassMobsBot
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

	void LoadMainInformationBots();
	void LoadQuestBots(const char* pWhereLocalWorld);
	void LoadNpcBots(const char* pWhereLocalWorld);
	void LoadMobsBots(const char* pWhereLocalWorld);
public:
	virtual void OnInitWorld(const char* pWhereLocalWorld);

	typedef DescDataBot DataBotInfo;
	static std::map < int , DataBotInfo > ms_aDataBot;

	typedef ClassQuestBot QuestBotInfo;
	static std::map < int , QuestBotInfo > ms_aQuestBot;

	typedef ClassNpcBot NpcBotInfo;
	static std::map < int , NpcBotInfo > ms_aNpcBot;

	typedef ClassMobsBot MobBotInfo;
	static std::map < int , MobBotInfo > ms_aMobBot;

	void ConAddCharacterBot(int ClientID, const char *pCharacter);
	void ProcessingTalkingNPC(int OwnID, int TalkingID, bool PlayerTalked, const char *Message, int Style, int TalkingEmote);
	bool TalkingBotNPC(CPlayer* pPlayer, int MobID, int Progress, int TalkedID, const char *pText = "empty");
	bool TalkingBotQuest(CPlayer* pPlayer, int MobID, int Progress, int TalkedID);
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

	// threading path finder
	void FindThreadPath(class CPlayerBot* pBotPlayer, vec2 StartPos, vec2 SearchPos);
	void GetThreadRandomWaypointTarget(class CPlayerBot* pBotPlayer);
};

#endif