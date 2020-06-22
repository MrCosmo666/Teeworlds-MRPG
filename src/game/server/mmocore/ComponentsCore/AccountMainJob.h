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
		char Login[64];
		char Password[64];
		char LastLogin[64];
		char Language[8];
		int AuthID;
		int Level;
		int Exp; 
		int GuildID;
		int GuildRank;

		// upgrades
		int Upgrade;
		std::map < int, int > Stats;

		// jobs
		int Plant[PLANT::NUM_PLANT];
		int Miner[MINER::NUM_MINER];

		// world
		short WorldID;
		short LastWorldID;

		// skins
		char m_aaSkinPartNames[6][24];
		int m_aUseCustomColors[6];
		int m_aSkinPartColors[6];

		int Team;
		std::map < int , bool > AetherLocation;
		bool IsGuild() { return (GuildID > 0); }
	};
	static std::map < int, StructData > Data;

	struct StructTempPlayerData
	{
		// ������� ��� �� ����� ��������� �� ���
		int TempDecoractionID;
		int TempDecorationType;
		int TempID3;

		// ��������� ���
		ShopJob::AuctionSlot SellItem;

		// temp rankname for guild rank settings
		char m_aRankGuildBuf[32];

		// temp guild name for searching
		char m_aGuildSearchBuf[32];

		// player stats
		int TempHealth;
		int TempMana;
		short TempLatencyPing;

		// save pos teleport
		bool TempActiveSafeSpawn;
		int TempTeleportX;
		int TempTeleportY;
		int TempTimeDungeon;

		// dungeon
		int TempTankVotingDungeon;
		bool TempAlreadyVotedDungeon;
	};
	static std::map < int, StructTempPlayerData > PlayerTempData;

	int SendAuthCode(int ClientID, int Code);

public:
	int CheckOnlineAccount(int AuthID) const;

	int RegisterAccount(int ClientID, const char *Login, const char *Password);
	int LoginAccount(int ClientID, const char *Login, const char *Password);
	void LoadAccount(CPlayer *pPlayer, bool FirstInitilize = false);
	void ShowDiscordCard(int ClientID);
	void DiscordConnect(int ClientID, const char *pDID);
	
	int GetRank(int AuthID);
	bool IsActive(int ClientID) const
	{
		return (bool)(Data.find(ClientID) != Data.end());
	}

	virtual bool OnVotingMenu(CPlayer* pPlayer, const char* CMD, const int VoteID, const int VoteID2, int Get, const char* GetText);
	virtual bool OnHandleMenulist(CPlayer* pPlayer, int Menulist, bool ReplaceMenu);
	virtual void OnResetClient(int ClientID);
};

#endif