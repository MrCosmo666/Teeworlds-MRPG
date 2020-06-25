/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#ifndef ENGINE_SERVER_H
#define ENGINE_SERVER_H

#include "kernel.h"
#include "message.h"

#define DC_SERVER_INFO g_Config.m_SvDiscordColorServerInfo
#define DC_PLAYER_INFO g_Config.m_SvDiscordColorPlayerInfo
#define DC_JOIN_LEAVE g_Config.m_SvDiscordColorJoinLeave
#define DC_SERVER_CHAT g_Config.m_SvDiscordColorServerChat
#define DC_SERVER_WARNING g_Config.m_SvDiscordColorWarning
#define DC_DISCORD_BOT g_Config.m_SvDiscordColorDiscordBot
#define DC_DISCORD_INFO g_Config.m_SvDiscordColorDiscordInfo

class IServer : public IInterface
{
	MACRO_INTERFACE("server", 0)
protected:
	int m_CurrentGameTick;
	int m_TickSpeed;
	float WorldTime;

public:
	class CLocalization* m_pLocalization;
	inline class CLocalization* Localization() { return m_pLocalization; }

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
	virtual void GetClientAddr(int ClientID, char *pAddrStr, int Size) const = 0;

	virtual int SendMsg(CMsgPacker *pMsg, int Flags, int ClientID, int WorldID = -1) = 0;

	template<class T>
	int SendPackMsg(T *pMsg, int Flags, int ClientID, int WorldID = -1)
	{
		CMsgPacker Packer(pMsg->MsgID(), false);
		if(pMsg->Pack(&Packer))
			return -1;
		return SendMsg(&Packer, Flags, ClientID, WorldID);
	}

	// World Time
	virtual int GetSecWorld() const = 0;
	virtual int GetHourWorld() const = 0;
	virtual bool CheckWorldTime(int Hour, int Sec) = 0;
	virtual const char* GetStringTypeDay() const = 0;
	virtual int GetEnumTypeDay() const = 0;

	virtual void SetClientName(int ClientID, char const *pName) = 0;
	virtual void SetClientClan(int ClientID, char const *pClan) = 0;
	virtual void SetClientCountry(int ClientID, int Country) = 0;
	virtual void SetClientScore(int ClientID, int Score) = 0;

	virtual void ChangeWorld(int ClientID, int WorldID) = 0;
	virtual void QuestBotUpdateOnWorld(int WorldID, int QuestID, int Step) = 0;
	virtual int GetWorldID(int ClientID) = 0;
	virtual const char* GetWorldName(int WorldID) = 0;

	virtual void SetClientVersion(int ClientID, int Version) = 0;
	virtual int GetClientVersion(int ClientID) = 0;

	virtual void SetClientLanguage(int ClientID, const char* pLanguage) = 0;
	virtual const char* GetClientLanguage(int ClientID) const = 0;

	virtual void SendDiscordMessage(const char *pChanel, const char* pColor, const char* pTitle, const char* pText) = 0;
	virtual void SendDiscordGenerateMessage(const char *pColor, const char *pTitle, const char *pMsg) = 0;
	virtual void SendDiscordStatus(const char *pStatus, int Type) = 0;
	virtual void AddInformationBotsCount(int Count) = 0;
	
	// Bots
	virtual void InitClientBot(int ClientID) = 0;

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
	virtual bool IsAuthed(int ClientID) const = 0;
	virtual bool IsBanned(int ClientID) = 0;
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
	virtual void OnPreSnap() = 0;
	virtual void OnSnap(int ClientID) = 0;
	virtual void OnPostSnap() = 0;

	virtual void OnMessage(int MsgID, CUnpacker *pUnpacker, int ClientID) = 0;
	virtual void ClearClientData(int ClientID) = 0;

	virtual void ChangeWorld(int ClientID) = 0;
	virtual void UpdateQuestsBot(int QuestID, int Step) = 0;

	virtual void OnClientConnected(int ClientID) = 0;
	virtual void OnClientEnter(int ClientID) = 0;
	virtual void OnClientDrop(int ClientID, const char *pReason, bool ChangeWorld = false) = 0;
	virtual void OnClientDirectInput(int ClientID, void *pInput) = 0;
	virtual void OnClientPredictedInput(int ClientID, void *pInput) = 0;

	virtual bool IsClientReady(int ClientID) const = 0;
	virtual bool IsClientPlayer(int ClientID) const = 0;
	virtual void Chat(int ClientID, const char* pText, ...) = 0;

	virtual const char *GameType() const = 0;
	virtual const char *Version() const = 0;
	virtual const char *NetVersion() const = 0;

	virtual bool TimeScore() const { return false; }
	virtual int GetRank(int AuthID) = 0;
};

extern IGameServer *CreateGameServer();
#endif