/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#ifndef GAME_SERVER_BOTS_INFO_SQL_H
#define GAME_SERVER_BOTS_INFO_SQL_H

#include "../component.h"
 
class ContextBots : public CMmoComponent
{
	~ContextBots()
	{
		DataBot.clear();
		QuestBot.clear();
		NpcBot.clear();
		MobBot.clear();
	};

	void LoadQuestBots();
	void LoadNpcBots();
	void LoadMobsBots();

	struct TalkingData
	{
		char m_TalkingText[512];
		int m_Style;
		int m_Emote;
		bool m_RequestComplete;
		bool m_PlayerTalked;
	};

	struct DescDataBot
	{
		char NameBot[16];
		char SkinNameBot[6][24];
		int UseCustomBot[6];
		int SkinColorBot[6];

		const char *Name(CPlayer *pPlayer) const;
	};

	struct ClassNpcBot
	{
		char Name[32];
		bool Static;
		int PositionX;
		int PositionY;
		int Emote;
		int WorldID;
		int BotID;

		std::vector < TalkingData > m_Talk;
	};

	struct ClassQuestBot
	{
		char Name[32];
		int PositionX;
		int PositionY;
		int QuestID;
		int Progress;
		int Emote;
		int WorldID;
		int BotID;
		int SubBotID;

		int Interactive[6];
		int InterCount[6];
		int InterRandom[6];

		std::vector < TalkingData > m_Talk;

		bool IsActive() const { return (QuestID > 0 && BotID > 0 && SubBotID > 0); };
	};

	struct ClassMobsBot
	{
		char Name[32];
		bool Boss;
		int Health;
		int Spread;
		int PositionX;
		int PositionY;
		int Emote;
		int Level;
		int RespawnTick;
		int WorldID;

		int DropItem[6];
		int CountItem[6];
		int RandomItem[6];
		int BotID;
	};


public:
	typedef DescDataBot DataBotInfo;
	static std::map < int , DataBotInfo > DataBot;

	typedef ClassQuestBot QuestBotInfo;
	static std::map < int , QuestBotInfo > QuestBot;

	typedef ClassNpcBot NpcBotInfo;
	static std::map < int , NpcBotInfo > NpcBot;

	typedef ClassMobsBot MobBotInfo;
	static std::map < int , MobBotInfo > MobBot;

	void LoadGlobalBots();
	void ConAddCharacterBot(int ClientID, const char *pCharacter);

	void ProcessingTalkingNPC(int OwnID, int TalkingID, bool PlayerTalked, const char *Message, int Style, int TalkingEmote);


	// ------------------ CHECK VALID DATA --------------------
	// --------------------------------------------------------
	// --------------------------------------------------------
	bool IsDataBotValid(int BotID) const { return (DataBot.find(BotID) != DataBot.end()); }
	bool IsNpcBotValid(int MobID) const 
	{ 
		if(NpcBot.find(MobID) != NpcBot.end() && IsDataBotValid(NpcBot[MobID].BotID))
			return true;
		return false; 
	}
	bool IsMobBotValid(int MobID) const 
	{ 
		if(MobBot.find(MobID) != MobBot.end() && IsDataBotValid(MobBot[MobID].BotID))
			return true;
		return false;
	}
	
	bool IsQuestBotValid(int MobID) const 
	{ 
		if(QuestBot.find(MobID) != QuestBot.end() && IsDataBotValid(QuestBot[MobID].BotID))
			return true;
		return false;
	}
	// --------------------------------------------------------
	// --------------------------------------------------------
};

#endif