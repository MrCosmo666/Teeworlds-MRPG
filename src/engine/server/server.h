/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#ifndef ENGINE_SERVER_SERVER_H
#define ENGINE_SERVER_SERVER_H

#include <engine/server.h>
#include <game/server/enum_global.h>

#ifdef CONF_DISCORD
	#include <teeother/sleepy_discord/websocketpp_websocket.h>
#endif

STRINGABLE_ENUM_IMPL(MINER)
STRINGABLE_ENUM_IMPL(PLANT)
STRINGABLE_ENUM_IMPL(EMEMBERUPGRADE)

class CSnapIDPool
{
	enum
	{
		MAX_IDS = 16*1024,
	};

	class CID
	{
	public:
		short m_Next;
		short m_State; // 0 = free, 1 = alloced, 2 = timed
		int m_Timeout;
	};

	CID m_aIDs[MAX_IDS];

	int m_FirstFree;
	int m_FirstTimed;
	int m_LastTimed;
	int m_Usage;
	int m_InUsage;

public:

	CSnapIDPool();

	void Reset();
	void RemoveFirstTimeout();
	int NewID();
	void TimeoutIDs();
	void FreeID(int ID);
};


class CServerBan : public CNetBan
{
	class CServer *m_pServer;

	template<class T> int BanExt(T *pBanPool, const typename T::CDataType *pData, int Seconds, const char *pReason);

public:
	class CServer *Server() const { return m_pServer; }

	void InitServerBan(class IConsole *pConsole, class IStorage *pStorage, class CServer* pServer);

	virtual int BanAddr(const NETADDR *pAddr, int Seconds, const char *pReason);
	virtual int BanRange(const CNetRange *pRange, int Seconds, const char *pReason);

	static void ConBanExt(class IConsole::IResult *pResult, void *pUser);
};

#ifdef CONF_DISCORD
class DiscordJob : public SleepyDiscord::DiscordClient
{
	CServer *m_pServer;
	CServer *Server() const { return m_pServer; }

	class CGS *m_GameServer;
	CGS *GS() const { return m_GameServer; }

	// роли
	std::vector<SleepyDiscord::Role> RoleList;

	void onMessage(SleepyDiscord::Message message) override;
	void onReaction(SleepyDiscord::Snowflake<SleepyDiscord::User> userID, SleepyDiscord::Snowflake<SleepyDiscord::Channel> channelID, 
						SleepyDiscord::Snowflake<SleepyDiscord::Message> messageID, SleepyDiscord::Emoji emoji) override;
	void onDeleteReaction(SleepyDiscord::Snowflake<SleepyDiscord::User> userID, SleepyDiscord::Snowflake<SleepyDiscord::Channel> channelID, 
						SleepyDiscord::Snowflake<SleepyDiscord::Message> messageID, SleepyDiscord::Emoji emoji) override;
	void UpdateMessageIdeas(SleepyDiscord::Snowflake<SleepyDiscord::User> userID, SleepyDiscord::Snowflake<SleepyDiscord::Channel> channelID, 
						SleepyDiscord::Snowflake<SleepyDiscord::Message> messageID);


public:
	using SleepyDiscord::DiscordClient::DiscordClient;
	DiscordJob(const char *token, int threads);

	void SetServer(CServer *pServer);
	void SetGameServer(CGS *pGameServer);

	void SendMessage(const char *pChanal, const char *Color, const char *Title, std::string pMsg);
	void SendGenerateMessage(const char *pChanal, const char *Color, const char *Title, const char *pPhpArg);
	void SendStatus(const char* Status, int Type);
};
#endif

class CServer : public IServer
{
	class IGameServer *m_pGameServer[COUNT_WORLD];
	class IConsole *m_pConsole;
	class IStorage *m_pStorage;

public:
	class IGameServer *GameServer(int id = 0) { return m_pGameServer[id]; }
	class IConsole *Console() { return m_pConsole; }
	class IStorage *Storage() { return m_pStorage; }
	class DiscordJob *m_pDiscord;

	enum
	{
		AUTHED_NO=0,
		AUTHED_MOD,
		AUTHED_ADMIN,

		MAX_RCONCMD_RATIO = 8,
		MAX_RCONCMD_SEND=16,
	};

	class CClient
	{
	public:

		enum
		{
			STATE_EMPTY = 0,
			STATE_AUTH,
			STATE_CONNECTING,
			STATE_READY,
			STATE_INGAME,

			SNAPRATE_INIT=0,
			SNAPRATE_FULL,
			SNAPRATE_RECOVER
		};

		class CInput
		{
		public:
			int m_aData[MAX_INPUT_SIZE];
			int m_GameTick; // the tick that was chosen for the input
		};

		// connection state info
		int m_State;
		int m_Latency;
		int m_SnapRate;

		int m_LastAckedSnapshot;
		int m_LastInputTick;
		CSnapshotStorage m_Snapshots;

		CInput m_LatestInput;
		CInput m_aInputs[200]; // TODO: handle input better
		int m_CurrentInput;

		// names update
		char m_aName[MAX_NAME_LENGTH];
		char m_aClan[MAX_CLAN_LENGTH];
		char m_aLanguage[MAX_LANGUAGE_LENGTH];

		int m_Version;
		int m_Country;
		int m_Score;
		int m_Authed;
		int m_AuthTries;

		int m_MapID;
		int m_OldMapID;
		bool m_ChangeMap;

		int m_MapChunk;
		bool m_NoRconNote;
		bool m_Quitting;
		const IConsole::CCommandInfo *m_pRconCmdToSend;

		int m_ClientVersion;
		void Reset();
	};

	CClient m_aClients[MAX_CLIENTS];

	CSnapshotDelta m_SnapshotDelta;
	CSnapshotBuilder m_SnapshotBuilder;
	CSnapIDPool m_IDPool;
	CNetServer m_NetServer;
	CEcon m_Econ;
	CServerBan m_ServerBan;

	int64 m_GameStartTime;
	int m_RunServer;
	int m_RconClientID;
	int m_RconAuthLevel;
	int m_PrintCBIndex;
	int m_BotsCount;

	// map
	enum
	{
		MAP_CHUNK_SIZE=NET_MAX_PAYLOAD-NET_MAX_CHUNKHEADERSIZE-4, // msg type
	};
	char m_aCurrentMap[64];
	int m_MapChunksPerRequest;

	IEngineMap *m_pLoadedMap[COUNT_WORLD];

	int m_RconPasswordSet;
	int m_GeneratedRconPassword;

	CRegister m_Register;
	CMapChecker m_MapChecker;

	CServer();

	// world time
	int WorldSec;
	int WorldHour;
	bool WorldCheckTime;

	virtual int GetSecWorld() const;
	virtual int GetHourWorld() const;
	virtual bool CheckWorldTime(int Hour, int Sec);
	virtual const char* GetStringTypeDay() const;
	virtual int GetEnumTypeDay() const;

	virtual void SetClientName(int ClientID, const char *pName);
	virtual void SetClientClan(int ClientID, char const *pClan);
	virtual void SetClientCountry(int ClientID, int Country);
	virtual void SetClientScore(int ClientID, int Score);

	virtual void ChangeWorld(int ClientID, int WorldID);
	virtual void QuestBotUpdateOnWorld(int WorldID, int QuestID, int Step);
	virtual int GetWorldID(int ClientID);

	virtual void SetClientVersion(int ClientID, int Version);
	virtual int GetClientVersion(int ClientID);

	virtual void SetClientLanguage(int ClientID, const char* pLanguage);
	virtual const char* GetClientLanguage(int ClientID) const;
	virtual const char* GetWorldName(int WorldID);

	virtual void SendDiscordMessage(const char *pChanel, const char* pColor, const char* pTitle, const char* pText);
	virtual void SendDiscordGenerateMessage(const char *pColor, const char *pTitle, const char *pMsg);
	virtual void SendDiscordStatus(const char *pStatus, int Type);
	virtual void AddInformationBotsCount(int Count);

	void Kick(int ClientID, const char *pReason);

	int64 TickStartTime(int Tick);
	int Init();

	void InitRconPasswordIfUnset();

	void SetRconCID(int ClientID);
	bool IsAuthed(int ClientID) const;
	bool IsBanned(int ClientID);
	int GetClientInfo(int ClientID, CClientInfo *pInfo) const;
	void GetClientAddr(int ClientID, char *pAddrStr, int Size) const;
	const char *ClientName(int ClientID) const;
	const char *ClientClan(int ClientID) const;
	int ClientCountry(int ClientID) const;
	bool ClientIngame(int ClientID) const;

	virtual int SendMsg(CMsgPacker *pMsg, int Flags, int ClientID, int WorldID = -1);

	void DoSnapshot(int WorldID);

	static int NewClientCallback(int ClientID, void *pUser);
	static int DelClientCallback(int ClientID, const char *pReason, void *pUser);

	void SendMap(int ClientID);
	void SendConnectionReady(int ClientID);
	void SendRconLine(int ClientID, const char *pLine);
	static void SendRconLineAuthed(const char *pLine, void *pUser, bool Highlighted);

	void SendRconCmdAdd(const IConsole::CCommandInfo *pCommandInfo, int ClientID);
	void SendRconCmdRem(const IConsole::CCommandInfo *pCommandInfo, int ClientID);
	void UpdateClientRconCommands();

	void ProcessClientPacket(CNetChunk *pPacket);

	void SendServerInfo(int ClientID);
	void GenerateServerInfo(CPacker *pPacker, int Token);

	void PumpNetwork();

	const char *GetMapName() const;
	bool LoadMap(const char *pMapName, int ID);

	void InitRegister(CNetServer *pNetServer, IEngineMasterServer *pMasterServer, IConsole *pConsole);
	int Run();

	static void ConKick(IConsole::IResult *pResult, void *pUser);
	static void ConStatus(IConsole::IResult *pResult, void *pUser);
	static void ConShutdown(IConsole::IResult *pResult, void *pUser);
	static void ConLogout(IConsole::IResult *pResult, void *pUser);

	static void ConchainSpecialInfoupdate(IConsole::IResult *pResult, void *pUserData, IConsole::FCommandCallback pfnCallback, void *pCallbackUserData);
	static void ConchainMaxclientsperipUpdate(IConsole::IResult *pResult, void *pUserData, IConsole::FCommandCallback pfnCallback, void *pCallbackUserData);
	static void ConchainModCommandUpdate(IConsole::IResult *pResult, void *pUserData, IConsole::FCommandCallback pfnCallback, void *pCallbackUserData);
	static void ConchainConsoleOutputLevelUpdate(IConsole::IResult *pResult, void *pUserData, IConsole::FCommandCallback pfnCallback, void *pCallbackUserData);
	static void ConchainRconPasswordSet(IConsole::IResult *pResult, void *pUserData, IConsole::FCommandCallback pfnCallback, void *pCallbackUserData);

	void RegisterCommands();

	// Bots
	virtual void InitClientBot(int ClientID);

	virtual int SnapNewID();
	virtual void SnapFreeID(int ID);
	virtual void *SnapNewItem(int Type, int ID, int Size);
	void SnapSetStaticsize(int ItemType, int Size);
};

#endif