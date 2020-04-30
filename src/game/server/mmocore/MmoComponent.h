/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#ifndef GAME_SERVER_MMO_COMPONENT_H
#define GAME_SERVER_MMO_COMPONENT_H

#include <game/server/enum_context.h>

class MmoComponent
{
protected:
	class MmoController *m_Job;
	friend MmoController;
	MmoController* Job() const { return m_Job; }

	class CGS *m_GameServer;
	CGS* GS() const { return m_GameServer; }

public:
	virtual ~MmoComponent() {}

	virtual void OnInitWorld(const char* pWhereLocalWorld) {};
	virtual void OnInit() {};
	virtual void OnInitAccount(CPlayer *pPlayer) {};
	virtual void OnTick() {};
	virtual void OnTickWorld() {};
	virtual void OnPaymentTime() {};
	virtual void OnResetClient(int ClientID) {};
	virtual bool OnHandleTile(CCharacter *pChr, int IndexCollision) { return false; };
	virtual bool OnHandleMenulist(CPlayer *pPlayer, int Menulist, bool ReplaceMenu) { return false; };
	virtual bool OnVotingMenu(CPlayer *pPlayer, const char *CMD, const int VoteID, const int VoteID2, int Get, const char *GetText) { return false; }
	virtual bool OnMessage(int MsgID, void *pRawMsg, int ClientID) { return false; };
};

#endif