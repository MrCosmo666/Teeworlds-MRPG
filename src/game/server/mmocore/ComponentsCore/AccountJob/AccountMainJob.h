/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#ifndef GAME_SERVER_SQL_MAINACCOUNT_H
#define GAME_SERVER_SQL_MAINACCOUNT_H
#include <game/server/mmocore/MmoComponent.h>

#include <string>
#include <game/server/mmocore/ComponentsCore/ShopJob.h>

class AccountMainJob : public MmoComponent
{
	~AccountMainJob()
	{
		ms_aData.clear();
		ms_aPlayerTempData.clear();
	};

	bool OnHandleVoteCommands(CPlayer* pPlayer, const char* CMD, const int VoteID, const int VoteID2, int Get, const char* GetText) override;
	bool OnHandleMenulist(CPlayer* pPlayer, int Menulist, bool ReplaceMenu) override;
	void OnResetClient(int ClientID) override;
	void OnMessage(int MsgID, void* pRawMsg, int ClientID) override;

public:
	struct StructData
	{
		// main
		char m_aLogin[64];
		char m_aLastLogin[64];
		char m_aLanguage[8];
		int m_AccountID;
		int m_Level;
		int m_Exp;
		int m_GuildID;
		int m_GuildRank;
		std::list < int > m_aHistoryWorld;

		// upgrades
		int m_Upgrade;
		std::map < int, int > m_aStats;

		// skins
		char m_aaSkinPartNames[NUM_SKINPARTS][MAX_SKIN_ARRAY_SIZE];
		int m_aUseCustomColors[NUM_SKINPARTS];
		int m_aSkinPartColors[NUM_SKINPARTS];

		int m_Team;
		std::map < int , bool > m_aAetherLocation;
		bool IsGuild() const { return m_GuildID > 0; }

		StructData()
		{
			m_AccountID = -1;
			m_Level = 0;
			m_Team = TEAM_SPECTATORS;

			// Name :: Field name database :: value
			m_aMiner[JOB_LEVEL] =      { "Miner level", "MnrLevel", 0 };
			m_aMiner[JOB_EXPERIENCE] = { "Miner experience", "MnrExp", 0 };
			m_aMiner[JOB_UPGRADES] =   { "Miner upgrades", "MnrUpgrade", 0 };
			m_aMiner[JOB_UPGR_COUNTS] ={ "Miner counts", "MnrCount", 0 };

			m_aPlant[JOB_LEVEL] =      { "Farmer level", "PlLevel", 0 };
			m_aPlant[JOB_EXPERIENCE] = { "Farmer experience", "PlExp", 0 };
			m_aPlant[JOB_UPGRADES] =   { "Farmer upgrades", "PlUpgrade", 0 };
			m_aPlant[JOB_UPGR_COUNTS] ={ "Farmer counts", "PlCounts", 0 };
		}
		// TODO: do not store data in the account
		struct
		{
			char m_aName[64];
			char m_aFieldName[64];
			int m_Value;
		} m_aMiner[NUM_JOB_ACCOUNTS_STATS], m_aPlant[NUM_JOB_ACCOUNTS_STATS];
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
		int m_TempPing;

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
	int RegisterAccount(int ClientID, const char *Login, const char *Password);
	int LoginAccount(int ClientID, const char *Login, const char *Password);
	void LoadAccount(CPlayer *pPlayer, bool FirstInitilize = false);
	void DiscordConnect(int ClientID, const char *pDID);

	int GetHistoryLatestCorrectWorldID(CPlayer* pPlayer) const;
	int GetRank(int AccountID);
	bool IsActive(int ClientID) const
	{
		return (bool)(ms_aData.find(ClientID) != ms_aData.end());
	}

	std::string HashPassword(const char* pPassword, const char* pSalt);
	void UseVoucher(int ClientID, const char* pVoucher);
};

#endif