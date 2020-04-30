/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#ifndef GAME_SERVER_SQLMINERACC_H
#define GAME_SERVER_SQLMINERACC_H

#include "../MmoComponent.h"

class AccountMinerJob : public MmoComponent
{
	struct StructOres
	{
		int ItemID;
		int Level;
		int Health;
		int PositionX;
		int PositionY;
		int Distance;
	};
	static std::map < int, StructOres > Ore;
	int ExpNeed(int Level) const;

public:
	int GetOreLevel(vec2 Pos) const;
	int GetOreItemID(vec2 Pos) const;
	int GetOreHealth(vec2 Pos) const;

	void ShowMenu(CPlayer *pPlayer);
	void Work(CPlayer *pPlayer, int Exp);

	virtual void OnInitAccount(CPlayer* pPlayer);
	virtual void OnInitWorld(const char* pWhereLocalWorld);
	virtual bool OnVotingMenu(CPlayer* pPlayer, const char* CMD, const int VoteID, const int VoteID2, int Get, const char* GetText);
};

#endif