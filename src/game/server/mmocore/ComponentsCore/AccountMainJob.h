/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#ifndef GAME_SERVER_SQL_MAINACCOUNT_H
#define GAME_SERVER_SQL_MAINACCOUNT_H

#include "../MmoComponent.h"

class AccountMainJob : public MmoComponent
{
public:
	struct StructData
	{
		char Login[64];
		char Password[64];
		char LastLogin[64];
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

		// save pos teleport
		bool TempActiveSafeSpawn;
		int TempTeleportX;
		int TempTeleportY;
		short TempLatencyPing;
		int TempTimeDungeon;

		// world
		short WorldID;
		short LastWorldID;

		// skins
		char m_aaSkinPartNames[6][24];
		int m_aUseCustomColors[6];
		int m_aSkinPartColors[6];

		int Team;
		int TempHealth;
		int TempMana;

		std::map < int , bool > AetherLocation;
		bool IsGuild() { return (GuildID > 0); }
	};
	static std::map < int, StructData > Data;
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

	virtual void OnResetClient(int ClientID);
};

#endif