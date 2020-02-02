/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#ifndef GAME_SERVER_SQLTELEPORTS_H
#define GAME_SERVER_SQLTELEPORTS_H

#include "../component.h"

class TeleportsSql : public CMmoComponent
{
	struct StructTeleport
	{
		int TeleX;
		int TeleY;
		int WorldID;
		char TeleName[64];
	};
	typedef std::map < int , StructTeleport > Telep;
	static Telep Teleport;

	void UnlockLocation(int ClientID, vec2 Pos);
	void ShowTeleportList(CPlayer *pPlayer);

public:
	virtual void OnInitGlobal();
	virtual void OnInitAccount(CPlayer *pPlayer);
	virtual bool OnPlayerHandleTile(CCharacter *pChr, int IndexCollision);
	virtual bool OnPlayerHandleMainMenu(CPlayer *pPlayer, int Menulist);
	virtual bool OnParseVotingMenu(CPlayer *pPlayer, const char *CMD, const int VoteID, const int VoteID2, int Get, const char *GetText);
};

#endif