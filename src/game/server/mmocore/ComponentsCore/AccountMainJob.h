/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#ifndef GAME_SERVER_SQL_MAINACCOUNT_H
#define GAME_SERVER_SQL_MAINACCOUNT_H

#include "../MmoComponent.h"

class AccountMainJob : public MmoComponent
{
public:
	// - - - - - - - - - - - -
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
		int Relax[RELAX::NUM_RELAX];
		int Plant[PLANT::NUM_PLANT];
		int Miner[MINER::NUM_MINER];

		// save pos teleport
		int TeleportX;
		int TeleportY;
		short WorldID;
		short LastWorldID;
		short LatencyPing;
		int TimeDungeon;

		// skins
		char m_aaSkinPartNames[6][24];
		int m_aUseCustomColors[6];
		int m_aSkinPartColors[6];

		int Team;
		int PlayerHealth;
		int PlayerMana;

		std::map < int , bool > AetherLocation;

		bool IsGuild() { return (GuildID > 0); }
	};
	static std::map < int, StructData > Data;

private:
	// auth state
	int SendAuthCode(int ClientID, int Code);

public:
	virtual void OnResetClientData(int ClientID);
	virtual bool OnParseVotingMenu(CPlayer* pPlayer, const char* CMD, const int VoteID, const int VoteID2, int Get, const char* GetText);

	int RegisterAccount(int ClientID, const char *Login, const char *Password);
	int LoginAccount(int ClientID, const char *Login, const char *Password);
	void LoadAccount(CPlayer *pPlayer, bool FirstInitilize = false);
	int CheckOnlineAccount(int AuthID) const;

	// discord
	void ShowDiscordCard(int ClientID);
	void DiscordConnect(int ClientID, const char *pDID);
	
	//
	int GetRank(int AuthID);
	bool IsActive(int ClientID) const
	{
		if(Data.find(ClientID) != Data.end())
			return true;
		return false;
	}
};

#endif