/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#ifndef GAME_SERVER_MMO_COMPONENT_H
#define GAME_SERVER_MMO_COMPONENT_H

#include <game/server/enum_context.h>

class SqlController;
class CMmoComponent
{
protected:
	friend SqlController;
	class SqlController *m_Job;
	SqlController* Job() const { return m_Job; }

	class CGS *m_GameServer;
	CGS* GS() const { return m_GameServer; }

public:
	virtual ~CMmoComponent() {}

	virtual void OnInitLocal(const char *pLocal) {};
	virtual void OnInitGlobal() {};
	virtual void OnInitAccount(CPlayer *pPlayer) {};
	virtual void OnTick() {};
	virtual void OnTickLocalWorld() {};
	virtual void OnPaymentTime() {};
	virtual bool OnPlayerHandleTile(CCharacter *pChr, int IndexCollision) { return false; };
	virtual bool OnPlayerHandleMainMenu(CPlayer *pPlayer, int Menulist, bool ReplaceMenu) { return false; };
	virtual bool OnParseVotingMenu(CPlayer *pPlayer, const char *CMD, const int VoteID, const int VoteID2, int Get, const char *GetText) { return false; }
	virtual bool OnMessage(int MsgID, void *pRawMsg, int ClientID) { return false; };
};

#endif