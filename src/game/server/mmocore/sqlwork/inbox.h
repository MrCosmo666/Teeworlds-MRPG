/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#ifndef GAME_SERVER_SQLINBOX_H
#define GAME_SERVER_SQLINBOX_H

#include "../component.h"

class InboxSql : public CMmoComponent
{
	void InteractiveInbox(CPlayer *pPlayer, int InboxID);

public:
	int GetActiveInbox(int ClientID);
	void GetInformationInbox(CPlayer *pPlayer);
	void SendInbox(int AuthID, const char* Name, const char* Desc, int ItemID = -1, int Count = -1, int Enchant = -1);

	virtual bool OnParseVotingMenu(CPlayer *pPlayer, const char *CMD, const int VoteID, const int VoteID2, int Get, const char *GetText);	
};

#endif