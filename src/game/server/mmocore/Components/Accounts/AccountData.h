/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#ifndef GAME_SERVER_COMPONENT_ACCOUNT_DATA_H
#define GAME_SERVER_COMPONENT_ACCOUNT_DATA_H
#include <game/game_context.h>
#include <game/server/mmocore/Components/Shops/ShopData.h>

struct CAccountData
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
	std::map < int, bool > m_aAetherLocation;
	bool IsGuild() const { return m_GuildID > 0; }

	CAccountData()
	{
		m_AccountID = -1;
		m_Level = 0;
		m_Team = TEAM_SPECTATORS;

		// Name :: Field name database :: value
		m_aMiner[JOB_LEVEL] = { "Miner level", "MnrLevel", 0 };
		m_aMiner[JOB_EXPERIENCE] = { "Miner experience", "MnrExp", 0 };
		m_aMiner[JOB_UPGRADES] = { "Miner upgrades", "MnrUpgrade", 0 };
		m_aMiner[JOB_UPGR_COUNTS] = { "Miner counts", "MnrCount", 0 };

		m_aPlant[JOB_LEVEL] = { "Farmer level", "PlLevel", 0 };
		m_aPlant[JOB_EXPERIENCE] = { "Farmer experience", "PlExp", 0 };
		m_aPlant[JOB_UPGRADES] = { "Farmer upgrades", "PlUpgrade", 0 };
		m_aPlant[JOB_UPGR_COUNTS] = { "Farmer counts", "PlCounts", 0 };
	}
	// TODO: do not store data in the account
	struct CFieldStruct
	{
		CFieldStruct() = default;
		CFieldStruct(const CFieldStruct& pField) = default;

		char m_aName[64];
		char m_aFieldName[64];
		int m_Value;
	};
	CFieldStruct m_aMiner[NUM_JOB_ACCOUNTS_STATS];
	CFieldStruct m_aPlant[NUM_JOB_ACCOUNTS_STATS];

	static std::map < int, CAccountData > ms_aData;
};

struct CAccountTempData
{
	int m_TempDecoractionID;
	int m_TempDecorationType;
	int m_TempID3;

	CAuctionItem m_SellItem;

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

	static std::map < int, CAccountTempData > ms_aPlayerTempData;
};

#endif