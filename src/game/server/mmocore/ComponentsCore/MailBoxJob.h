/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#ifndef GAME_SERVER_MAILBOXJOB_H
#define GAME_SERVER_MAILBOXJOB_H

#include "../MmoComponent.h"

class MailBoxJob : public MmoComponent
{
	bool OnHandleVoteCommands(CPlayer* pPlayer, const char* CMD, const int VoteID, const int VoteID2, int Get, const char* GetText) override;

public:
	int GetActiveInbox(CPlayer* pPlayer);
	void GetInformationInbox(CPlayer *pPlayer);
	void SendInbox(int AccountID, const char* Name, const char* Desc, int ItemID = -1, int Count = -1, int Enchant = -1);

private:
	void ReceiveInbox(CPlayer* pPlayer, int InboxID);
};

#endif