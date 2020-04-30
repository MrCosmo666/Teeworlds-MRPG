/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#ifndef GAME_SERVER_SQLMINERACC_H
#define GAME_SERVER_SQLMINERACC_H

#include "../MmoComponent.h"

class AccountMinerJob : public MmoComponent
{
	/* #########################################################################
		VAR AND OBJECTS MINER 
	######################################################################### */
	struct StructOres
	{
		int ItemID;
		int Level;
		int Health;
		int PositionX;
		int PositionY;
		int Distance;
	};
	typedef std::map < int , StructOres > OresType;
	static OresType Ore;

public:
	virtual void OnInitLocal(const char *pLocal);
	virtual void OnInitAccount(CPlayer *pPlayer);

	int GetOreLevel(vec2 Pos) const;
	int GetOreItemID(vec2 Pos) const;
	int GetOreHealth(vec2 Pos) const;

	void ShowMenu(CPlayer *pPlayer);

	void Work(CPlayer *pPlayer, int Exp);
	virtual bool OnParseVotingMenu(CPlayer *pPlayer, const char *CMD, const int VoteID, const int VoteID2, int Get, const char *GetText);

private:
	int ExpNeed(int Level) const;
// ********************************************************************************************************
// ****************************** WARNING DON"T CHANGE NAME DOWN FUNCTION *********************************
// ********************************************************************************************************

};

#endif