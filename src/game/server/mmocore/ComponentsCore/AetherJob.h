/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#ifndef GAME_SERVER_SQLTELEPORTS_H
#define GAME_SERVER_SQLTELEPORTS_H

#include "../MmoComponent.h"

class AetherJob : public MmoComponent
{
	struct StructTeleport
	{
		int m_TeleX;
		int m_TeleY;
		int m_WorldID;
		char m_aTeleName[64];
	};
	static std::map < int, StructTeleport > ms_aTeleport;

public:
	virtual void OnInit();
	virtual void OnInitAccount(CPlayer *pPlayer);
	virtual bool OnHandleTile(CCharacter *pChr, int IndexCollision);
	virtual bool OnHandleMenulist(CPlayer* pPlayer, int Menulist, bool ReplaceMenu);
	virtual bool OnParsingVoteCommands(CPlayer* pPlayer, const char* CMD, const int VoteID, const int VoteID2, int Get, const char* GetText);

private:
	void UnlockLocation(CPlayer* pPlayer, vec2 Pos);
	void ShowTeleportList(CCharacter* pChar);
};

#endif