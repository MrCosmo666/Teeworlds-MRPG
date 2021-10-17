/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#ifndef ENGINE_SERVER_H
#define ENGINE_SERVER_H

#include "kernel.h"
#include "message.h"

#include <engine/shared/protocol.h>
#include <generated/protocol.h>
#include <game/version.h>

#define DC_SERVER_INFO 13872503
#define DC_PLAYER_INFO 1346299
#define DC_JOIN_LEAVE 14494801
#define DC_SERVER_CHAT 7899095
#define DC_DISCORD_WARNING 13183530
#define DC_DISCORD_SUCCESS 1346299
#define DC_DISCORD_INFO 431050
#define DC_INVISIBLE_GRAY 3553599

class IServer : public IInterface
{
	MACRO_INTERFACE("server", 0)
protected:
	int m_CurrentGameTick;
	int m_TickSpeed;

public:
	// static std::mutex m_aMutexPlayerDataSafe[MAX_CLIENTS];
	virtual class IGameServer* GameServer(int WorldID = 0) = 0;

	class CLocalization* m_pLocalization;
	inline class CLocalization* Localization() const { return m_pLocalization; }

	struct CClientInfo
	{
		const char *m_pName;
		int m_Latency;
	};

	int Tick() const { return m_CurrentGameTick; }
	int TickSpeed() const { return m_TickSpeed; }

	virtual const char *ClientName(int ClientID) const = 0;
	virtual const char *ClientClan(int ClientID) const = 0;
	virtual int ClientCountry(int ClientID) const = 0;
	virtual bool ClientIngame(int ClientID) const = 0;
	virtual int GetClientInfo(int ClientID, CClientInfo *pInfo) const = 0;
	virtual void GetClientAddr(int ClientID, char* pAddrStr, int Size) const = 0;

	virtual int SendMsg(CMsgPacker *pMsg, int Flags, int ClientID, int64 Mask = -1, int WorldID = -1) = 0;

	template<class T>
	int SendPackMsgMask(T* pMsg, int Flags, int ClientID, int64 Mask, int WorldID = -1)
	{
		CMsgPacker Packer(pMsg->MsgID(), false);
		if(pMsg->Pack(&Packer))
			return -1;
		return SendMsg(&Packer, Flags, ClientID, Mask, WorldID);
	}

	template<class T>
	int SendPackMsg(T* pMsg, int Flags, int ClientID, int WorldID = -1)
	{
		CMsgPacker Packer(pMsg->MsgID(), false);
		if(pMsg->Pack(&Packer))
			return -1;

		auto IsNotVanilaMsg = [=](int CID) -> bool // first mmo msg
		{
			return  GetClientProtocolVersion(CID) == PROTOCOL_VERSION_MMO && pMsg->MsgID() >= NETMSGTYPE_CL_ISMMOSERVER;
		};

		int64 Mask = -1;
		if(ClientID == -1)
		{
			for(int i = 0; i < MAX_PLAYERS; i++)
			{
				if(ClientIngame(i) && (IsNotVanilaMsg(i) || pMsg->MsgID() < NETMSGTYPE_CL_ISMMOSERVER))
					Mask |= (int64)1 << i;
			}
		}
		else if(ClientIngame(ClientID) && (IsNotVanilaMsg(ClientID) || pMsg->MsgID() < NETMSGTYPE_CL_ISMMOSERVER))
			Mask |= (int64)1 << ClientID;

		return SendMsg(&Packer, Flags, ClientID, Mask, WorldID);
	}

	// World Time
	virtual int GetMinutesWorldTime() const = 0;
	virtual int GetHourWorldTime() const = 0;
	virtual int GetOffsetWorldTime() const = 0;
	virtual void SetOffsetWorldTime(int Hour) = 0;
	virtual bool CheckWorldTime(int Hour, int Minute) = 0;
	virtual const char* GetStringTypeDay() const = 0;
	virtual int GetEnumTypeDay() const = 0;

	// main client functions
	virtual void SetClientName(int ClientID, char const *pName) = 0;
	virtual void SetClientClan(int ClientID, char const *pClan) = 0;
	virtual void SetClientCountry(int ClientID, int Country) = 0;
	virtual void SetClientScore(int ClientID, int Score) = 0;

	virtual void ChangeWorld(int ClientID, int NewWorldID) = 0;
	virtual int GetClientWorldID(int ClientID) = 0;
	virtual const char* GetWorldName(int WorldID) = 0;

	virtual void SetClientProtocolVersion(int ClientID, int Version) = 0;
	virtual int GetClientProtocolVersion(int ClientID) = 0;

	virtual void SetClientLanguage(int ClientID, const char* pLanguage) = 0;
	virtual const char* GetClientLanguage(int ClientID) const = 0;

	// discord
	virtual void SendDiscordMessage(const char *pChannel, int Color, const char* pTitle, const char* pText) = 0;
	virtual void SendDiscordGenerateMessage(const char* pTitle, int AccountID, int Color = 0) = 0;
	virtual void UpdateDiscordStatus(const char *pStatus) = 0;

	// Bots
	virtual void InitClientBot(int ClientID) = 0;
	virtual void BackInformationFakeClient(int FakeClientID) = 0;

	// snapshots
	virtual int SnapNewID() = 0;
	virtual void SnapFreeID(int ID) = 0;
	virtual void *SnapNewItem(int Type, int ID, int Size) = 0;
	virtual void SnapSetStaticsize(int ItemType, int Size) = 0;

	enum
	{
		RCON_CID_SERV=-1,
		RCON_CID_VOTE=-2,
	};
	virtual void SetRconCID(int ClientID) = 0;
	virtual int GetRconCID() const = 0;
	virtual int GetRconAuthLevel() const = 0;
	virtual int GetAuthedState(int ClientID) const = 0;
	virtual bool IsAuthed(int ClientID) const = 0;
	virtual bool IsBanned(int ClientID) = 0;
	virtual bool IsEmpty(int ClientID) const = 0;
	virtual void Kick(int ClientID, const char *pReason) = 0;
};

class IGameServer : public IInterface
{
	MACRO_INTERFACE("gameserver", 0)
protected:
public:
	virtual void OnInit(int WorldID) = 0;
	virtual void OnConsoleInit() = 0;
	virtual void OnShutdown() = 0;

	virtual void OnTick() = 0;
	virtual void OnTickMainWorld() = 0;
	virtual void OnPreSnap() = 0;
	virtual void OnSnap(int ClientID) = 0;
	virtual void OnPostSnap() = 0;

	virtual void OnMessage(int MsgID, CUnpacker *pUnpacker, int ClientID) = 0;
	virtual void ClearClientData(int ClientID) = 0;

	virtual void PrepareClientChangeWorld(int ClientID) = 0;
	virtual void UpdateClientInformation(int FakeClientID) = 0;

	virtual void OnClientConnected(int ClientID) = 0;
	virtual void OnClientEnter(int ClientID) = 0;
	virtual void OnClientDrop(int ClientID, const char *pReason) = 0;
	virtual void OnClientDirectInput(int ClientID, void *pInput) = 0;
	virtual void OnClientPredictedInput(int ClientID, void *pInput) = 0;

	virtual bool IsClientReady(int ClientID) const = 0;
	virtual bool IsClientPlayer(int ClientID) const = 0;
	virtual void FakeChat(const char *pName, const char *pText) = 0;

	virtual const char *Version() const = 0;
	virtual const char *NetVersion() const = 0;
	virtual int GetRank(int AuthID) = 0;
};

extern IGameServer *CreateGameServer();
#endif