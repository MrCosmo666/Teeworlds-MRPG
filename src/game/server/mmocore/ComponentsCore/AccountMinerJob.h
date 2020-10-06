/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#ifndef GAME_SERVER_SQLMINERACC_H
#define GAME_SERVER_SQLMINERACC_H

#include "../MmoComponent.h"

class AccountMinerJob : public MmoComponent
{
	struct StructOres
	{
		int m_ItemID;
		int m_Level;
		int m_Health;
		int m_PositionX;
		int m_PositionY;
		int m_Distance;
	};
	static std::map < int, StructOres > ms_aOre;

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