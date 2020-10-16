/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#ifndef GAME_SERVER_SQL_MAINACCOUNT_H
#define GAME_SERVER_SQL_MAINACCOUNT_H

#include "../MmoComponent.h"
#include "ShopJob.h"

class AccountMainJob : public MmoComponent
{
public:
	struct StructData
	{
		char m_aLogin[64];
		// ðåchar Password[64];
		char m_aLastLogin[64];
		char m_aLanguage[8];
		int m_AuthID;
		int m_Level;
		int m_Exp; 
		int m_GuildID;
		int m_GuildRank;

		// upgrades
		int m_Upgrade;
		std::map < int, int > m_aStats;

		// jobs
		int m_aPlant[PLANT::NUM_PLANT];
		int m_aMiner[MINER::NUM_MINER];

		// world
		std::list < int > m_aHistoryWorld;

		// skins
		char m_aaSkinPartNames[NUM_SKINPARTS][MAX_SKIN_LENGTH];
		int m_aUseCustomColors[NUM_SKINPARTS];
		int m_aSkinPartColors[NUM_SKINPARTS];

		int m_Team;
		std::map < int , bool > m_aAetherLocation;
		bool IsGuild() const { return m_GuildID > 0; }
	};
	static std::map < int, StructData > ms_aData;

	struct StructTempPlayerData
	{
		int m_TempDecoractionID;
		int m_TempDecorationType;
		int m_TempID3;

		ShopJob::AuctionSlot m_SellItem;

		// temp rankname for guild rank settings
		char m_aRankGuildBuf[32];

		// temp guild name for searching
		char m_aGuildSearchBuf[32];

		// player stats
		int m_TempHealth;
		int m_TempMana;
		short m_TempPing;

		// save pos teleport
		bool m_TempSafeSpawn;
		int m_TempTeleportX;
		int m_TempTeleportY;

		// dungeon
		int m_TempTimeDungeon;
		bool m_TempDungeonReady;
		int m_TempTankVotingDungeon;
		bool m_TempAlreadyVotedDungeon;
	};
	static std::map < int, StructTempPlayerData > ms_aPlayerTempData;

	int SendAuthCode(int ClientID, int Code);
	int CheckOnlineAccount(int AuthID) const;

	int RegisterAccount(int ClientID, const char *Login, const char *Password);
	int LoginAccount(int ClientID, const char *Login, const char *Password);
	void LoadAccount(CPlayer *pPlayer, bool FirstInitilize = false);
	void DiscordConnect(int ClientID, const char *pDID);

	int GetHistoryLatestCorrectWorldID(CPlayer* pPlayer) const;
	int GetRank(int AuthID);
	bool IsActive(int ClientID) const
	{
		return (bool)(ms_aData.find(ClientID) != ms_aData.end());
	}

	std::string HashPassword(const char* pPassword, const char* pSalt);

	virtual bool OnParsingVoteCommands(CPlayer* pPlayer, const char* CMD, const int VoteID, const int VoteID2, int Get, const char* GetText);
	virtual bool OnHandleMenulist(CPlayer* pPlayer, int Menulist, bool ReplaceMenu);
	virtual void OnResetClient(int ClientID);
};

#endif