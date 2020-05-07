/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#ifndef GAME_SERVER_SQLTELEPORTS_H
#define GAME_SERVER_SQLTELEPORTS_H

#include "../MmoComponent.h"

class AetherJob : public MmoComponent
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

public:
	virtual void OnInit();
	virtual void OnInitAccount(CPlayer *pPlayer);
	virtual bool OnHandleTile(CCharacter *pChr, int IndexCollision);
	virtual bool OnHandleMenulist(CPlayer* pPlayer, int Menulist, bool ReplaceMenu);
	virtual bool OnVotingMenu(CPlayer* pPlayer, const char* CMD, const int VoteID, const int VoteID2, int Get, const char* GetText);

private:
	void UnlockLocation(CPlayer* pPlayer, vec2 Pos);
	void ShowTeleportList(CCharacter* pChar);
};

#endif