/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#ifndef GAME_SERVER_SQL_MAINACCOUNT_H
#define GAME_SERVER_SQL_MAINACCOUNT_H
#include <game/server/mmocore/MmoComponent.h>

#include "AccountData.h"

class CAccountCore : public MmoComponent
{
	~CAccountCore() override
	{
		CAccountData::ms_aData.clear();
		CAccountTempData::ms_aPlayerTempData.clear();
	};

	bool OnHandleVoteCommands(CPlayer* pPlayer, const char* CMD, const int VoteID, const int VoteID2, int Get, const char* GetText) override;
	bool OnHandleMenulist(CPlayer* pPlayer, int Menulist, bool ReplaceMenu) override;
	void OnResetClient(int ClientID) override;
	void OnMessage(int MsgID, void* pRawMsg, int ClientID) override;

public:
	int SendAuthCode(int ClientID, int Code);
	int RegisterAccount(int ClientID, const char *Login, const char *Password);
	int LoginAccount(int ClientID, const char *Login, const char *Password);
	void LoadAccount(CPlayer *pPlayer, bool FirstInitilize = false);
	void DiscordConnect(int ClientID, const char *pDID);

	int GetHistoryLatestCorrectWorldID(CPlayer* pPlayer) const;
	int GetRank(int AccountID);

	static bool IsActive(int ClientID)
	{
		return CAccountData::ms_aData.find(ClientID) != CAccountData::ms_aData.end();
	}

	std::string HashPassword(const char* pPassword, const char* pSalt);
	void UseVoucher(int ClientID, const char* pVoucher);
};

#endif