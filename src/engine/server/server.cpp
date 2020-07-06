/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */

#include <base/math.h>
#include <base/system.h>

#include <engine/config.h>
#include <engine/console.h>
#include <engine/engine.h>
#include <engine/map.h>
#include <engine/masterserver.h>
#include <engine/server.h>
#include <engine/storage.h>

#include <engine/server/sql_connect_pool.h>
#include <engine/server/sql_string_helpers.h>

#include <engine/shared/compression.h>
#include <engine/shared/config.h>
#include <engine/shared/datafile.h>
#include <engine/shared/econ.h>
#include <engine/shared/filecollection.h>
#include <engine/shared/mapchecker.h>
#include <engine/shared/netban.h>
#include <engine/shared/network.h>
#include <engine/shared/packer.h>
#include <engine/shared/protocol.h>
#include <engine/shared/snapshot.h>
#include <mastersrv/mastersrv.h>

#include "register.h"
#include "server.h"

#if defined(CONF_FAMILY_WINDOWS)
	#define WIN32_LEAN_AND_MEAN
	#include <windows.h>
#endif

#include <teeother/components/localization.h>

CSnapIDPool::CSnapIDPool()
{
	Reset();
}

void CSnapIDPool::Reset()
{
	for(int i = 0; i < MAX_IDS; i++)
	{
		m_aIDs[i].m_Next = i+1;
		m_aIDs[i].m_State = 0;
	}

	m_aIDs[MAX_IDS-1].m_Next = -1;
	m_FirstFree = 0;
	m_FirstTimed = -1;
	m_LastTimed = -1;
	m_Usage = 0;
	m_InUsage = 0;
}


void CSnapIDPool::RemoveFirstTimeout()
{
	int NextTimed = m_aIDs[m_FirstTimed].m_Next;

	// add it to the free list
	m_aIDs[m_FirstTimed].m_Next = m_FirstFree;
	m_aIDs[m_FirstTimed].m_State = 0;
	m_FirstFree = m_FirstTimed;

	// remove it from the timed list
	m_FirstTimed = NextTimed;
	if(m_FirstTimed == -1)
		m_LastTimed = -1;

	m_Usage--;
}

int CSnapIDPool::NewID()
{
	int64 Now = time_get();

	// process timed ids
	while(m_FirstTimed != -1 && m_aIDs[m_FirstTimed].m_Timeout < Now)
		RemoveFirstTimeout();

	int ID = m_FirstFree;
	dbg_assert(ID != -1, "id error");
	if(ID == -1)
		return ID;
	m_FirstFree = m_aIDs[m_FirstFree].m_Next;
	m_aIDs[ID].m_State = 1;
	m_Usage++;
	m_InUsage++;
	return ID;
}

void CSnapIDPool::TimeoutIDs()
{
	// process timed ids
	while(m_FirstTimed != -1)
		RemoveFirstTimeout();
}

void CSnapIDPool::FreeID(int ID)
{
	if(ID < 0)
		return;
	dbg_assert(m_aIDs[ID].m_State == 1, "id is not alloced");

	m_InUsage--;
	m_aIDs[ID].m_State = 2;
	m_aIDs[ID].m_Timeout = time_get()+time_freq()*5;
	m_aIDs[ID].m_Next = -1;

	if(m_LastTimed != -1)
	{
		m_aIDs[m_LastTimed].m_Next = ID;
		m_LastTimed = ID;
	}
	else
	{
		m_FirstTimed = ID;
		m_LastTimed = ID;
	}
}


void CServerBan::InitServerBan(IConsole *pConsole, IStorage *pStorage, CServer* pServer)
{
	CNetBan::Init(pConsole, pStorage);

	m_pServer = pServer;

	// overwrites base command, todo: improve this
	Console()->Register("ban", "s[id|ip|range] ?i[minutes] r[reason]", CFGFLAG_SERVER | CFGFLAG_STORE, ConBanExt, this, "Ban player with IP/IP range/client id for x minutes for any reason");
}

template<class T>
int CServerBan::BanExt(T *pBanPool, const typename T::CDataType *pData, int Seconds, const char *pReason)
{
	// validate address
	if(Server()->m_RconClientID >= 0 && Server()->m_RconClientID < MAX_PLAYERS &&
		Server()->m_aClients[Server()->m_RconClientID].m_State != CServer::CClient::STATE_EMPTY)
	{
		if(NetMatch(pData, Server()->m_NetServer.ClientAddr(Server()->m_RconClientID)))
		{
			Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "net_ban", "ban error (you can't ban yourself)");
			return -1;
		}

		for(int i = 0; i < MAX_PLAYERS; ++i)
		{
			if(i == Server()->m_RconClientID || Server()->m_aClients[i].m_State == CServer::CClient::STATE_EMPTY)
				continue;

			if(Server()->m_aClients[i].m_Authed >= Server()->m_RconAuthLevel && NetMatch(pData, Server()->m_NetServer.ClientAddr(i)))
			{
				Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "net_ban", "ban error (command denied)");
				return -1;
			}
		}
	}
	else if(Server()->m_RconClientID == IServer::RCON_CID_VOTE)
	{
		for(int i = 0; i < MAX_PLAYERS; ++i)
		{
			if(Server()->m_aClients[i].m_State == CServer::CClient::STATE_EMPTY)
				continue;

			if(Server()->m_aClients[i].m_Authed != CServer::AUTHED_NO && NetMatch(pData, Server()->m_NetServer.ClientAddr(i)))
			{
				Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "net_ban", "ban error (command denied)");
				return -1;
			}
		}
	}

	int Result = Ban(pBanPool, pData, Seconds, pReason);
	if(Result != 0)
		return Result;

	// drop banned clients
	typename T::CDataType Data = *pData;
	for(int i = 0; i < MAX_PLAYERS; ++i)
	{
		if(Server()->m_aClients[i].m_State == CServer::CClient::STATE_EMPTY)
			continue;

		if(NetMatch(&Data, Server()->m_NetServer.ClientAddr(i)))
		{
			CNetHash NetHash(&Data);
			char aBuf[256];
			MakeBanInfo(pBanPool->Find(&Data, &NetHash), aBuf, sizeof(aBuf), MSGTYPE_PLAYER);
			Server()->m_NetServer.Drop(i, aBuf);
		}
	}

	return Result;
}

int CServerBan::BanAddr(const NETADDR *pAddr, int Seconds, const char *pReason)
{
	return BanExt(&m_BanAddrPool, pAddr, Seconds, pReason);
}

int CServerBan::BanRange(const CNetRange *pRange, int Seconds, const char *pReason)
{
	if(pRange->IsValid())
		return BanExt(&m_BanRangePool, pRange, Seconds, pReason);

	Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "net_ban", "ban failed (invalid range)");
	return -1;
}

void CServerBan::ConBanExt(IConsole::IResult *pResult, void *pUser)
{
	CServerBan *pThis = static_cast<CServerBan *>(pUser);

	const char *pStr = pResult->GetString(0);
	int Minutes = pResult->NumArguments()>1 ? clamp(pResult->GetInteger(1), 0, 44640) : 30;
	const char *pReason = pResult->NumArguments()>2 ? pResult->GetString(2) : "No reason given";

	if(!str_is_number(pStr))
	{
		int ClientID = str_toint(pStr);
		if(ClientID < 0 || ClientID >= MAX_PLAYERS || pThis->Server()->m_aClients[ClientID].m_State == CServer::CClient::STATE_EMPTY)
			pThis->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "net_ban", "ban error (invalid client id)");
		else
			pThis->BanAddr(pThis->Server()->m_NetServer.ClientAddr(ClientID), Minutes*60, pReason);
	}
	else
		ConBan(pResult, pUser);
}


void CServer::CClient::Reset()
{
	// reset input
	for(int i = 0; i < 200; i++)
		m_aInputs[i].m_GameTick = -1;
	m_CurrentInput = 0;
	mem_zero(&m_LatestInput, sizeof(m_LatestInput));

	m_LastAckedSnapshot = -1;
	m_LastInputTick = -1;
	m_Snapshots.PurgeAll();
	m_SnapRate = CClient::SNAPRATE_INIT;
	m_Score = 0;
	m_MapChunk = 0;
}

CServer::CServer()
{
	m_TickSpeed = SERVER_TICK_SPEED;
	
	for(int i = 0 ; i < COUNT_WORLD ; i ++)
		m_pGameServer[i] = 0;

	m_CurrentGameTick = 0;
	m_RunServer = 1;

	m_RconClientID = IServer::RCON_CID_SERV;
	m_RconAuthLevel = AUTHED_ADMIN;

	m_RconPasswordSet = 0;
	m_GeneratedRconPassword = 0;

	Init();
}

// Get Secound World
int CServer::GetSecWorld() const
{
	return WorldSec;
}

// Get Static Hour Time
int CServer::GetHourWorld() const
{
	return WorldHour;
}

// check world time interactive effect KARLSON
bool CServer::CheckWorldTime(int Hour, int Sec)
{
	if(GetHourWorld() == Hour && GetSecWorld() == Sec)
	{
		WorldSec++;
		WorldCheckTime = true;
		return true;
	}
	return false;
}

// Format Day
const char* CServer::GetStringTypeDay() const
{
	if(GetHourWorld() >= 0 && GetHourWorld() < 6) return "Night";
	else if(GetHourWorld() >= 6 && GetHourWorld() < 13) return "Morning";
	else if(GetHourWorld() >= 13 && GetHourWorld() < 19) return "Day";
	else return "Evening";
}

// Format Day to Int
int CServer::GetEnumTypeDay() const
{
	if(GetHourWorld() >= 0 && GetHourWorld() < 6) return DayType::NIGHTTYPE;
	else if(GetHourWorld() >= 6 && GetHourWorld() < 13) return DayType::MORNINGTYPE;
	else if(GetHourWorld() >= 13 && GetHourWorld() < 19) return DayType::DAYTYPE;
	else return DayType::EVENINGTYPE;
}

void CServer::SetClientName(int ClientID, const char *pName)
{
	if(ClientID < 0 || ClientID >= MAX_CLIENTS || m_aClients[ClientID].m_State < CClient::STATE_READY || !pName)
		return;

	str_copy(m_aClients[ClientID].m_aName, pName, MAX_NAME_LENGTH);
}

void CServer::SetClientClan(int ClientID, const char *pClan)
{
	if(ClientID < 0 || ClientID >= MAX_CLIENTS || m_aClients[ClientID].m_State < CClient::STATE_READY || !pClan)
		return;

	str_copy(m_aClients[ClientID].m_aClan, pClan, MAX_CLAN_LENGTH);
}

void CServer::SetClientCountry(int ClientID, int Country)
{
	if(ClientID < 0 || ClientID >= MAX_CLIENTS || m_aClients[ClientID].m_State < CClient::STATE_READY)
		return;

	m_aClients[ClientID].m_Country = Country;
}

void CServer::SetClientScore(int ClientID, int Score)
{
	if(ClientID < 0 || ClientID >= MAX_CLIENTS || m_aClients[ClientID].m_State < CClient::STATE_READY)
		return;
	m_aClients[ClientID].m_Score = Score;
}

void CServer::SetClientVersion(int ClientID, int Version)
{
	if(ClientID < 0 || ClientID >= MAX_CLIENTS || m_aClients[ClientID].m_State < CClient::STATE_READY)
		return;
	m_aClients[ClientID].m_ClientVersion = Version;
}

int CServer::GetClientVersion(int ClientID)
{
	if(ClientID < 0 || ClientID >= MAX_CLIENTS || m_aClients[ClientID].m_State < CClient::STATE_READY)
		return 0;
	return m_aClients[ClientID].m_ClientVersion;
}

void CServer::SetClientLanguage(int ClientID, const char* pLanguage)
{
	if (ClientID < 0 || ClientID >= MAX_CLIENTS || m_aClients[ClientID].m_State < CClient::STATE_READY)
		return;
	str_copy(m_aClients[ClientID].m_aLanguage, pLanguage, sizeof(m_aClients[ClientID].m_aLanguage));
}

const char *CServer::GetWorldName(int WorldID)
{
	if(WorldID < 0 || WorldID >= COUNT_WORLD)
		return "UnknowName";
	
	switch(WorldID)
	{
		case 0: return "Pier Elfinia";
		case 1: return "Way to the Elfinia";
		case 2: return "Elfinia";
		case 3: return "Elfinia Deep cave";
		case 4: return "Elfia home room";
		case 5: return "Elfinia occupation of goblins";
		case 6: return "Elfinia Abandoned mine";
		case 7: return "Diana home room";
		case 8: return "Noctis Resonance";
		case 9: return "Departure";
		case 10: return "Underwater of Neptune";
		case 11: return "Kugan";
	}
	return "unknow";
}

const char* CServer::GetClientLanguage(int ClientID) const
{
	if (ClientID < 0 || ClientID >= MAX_CLIENTS || m_aClients[ClientID].m_State < CClient::STATE_READY)
		return "en";
	return m_aClients[ClientID].m_aLanguage;
}

void CServer::ChangeWorld(int ClientID, int MapID)
{
	if(MapID < 0 || MapID >= COUNT_WORLD || MapID == m_aClients[ClientID].m_MapID || ClientID < 0 || ClientID >= MAX_PLAYERS || m_aClients[ClientID].m_State < CClient::STATE_READY)
		return;

	GameServer(m_aClients[ClientID].m_MapID)->OnClientDrop(ClientID, "", true);
	m_aClients[ClientID].m_OldMapID = m_aClients[ClientID].m_MapID;
	m_aClients[ClientID].m_MapID = MapID;
	m_aClients[ClientID].m_ChangeMap = true;
	m_aClients[ClientID].m_MapChunk = 0;
	m_aClients[ClientID].m_State = CClient::STATE_CONNECTING;

	m_aClients[ClientID].m_Snapshots.PurgeAll();
	SendMap(ClientID);
}

void CServer::QuestBotUpdateOnWorld(int WorldID, int QuestID, int Step)
{
	if (WorldID < 0 || WorldID >= COUNT_WORLD)
		return;

	GameServer(WorldID)->UpdateQuestsBot(QuestID, Step);
}

void CServer::BackInformationFakeClient(int FakeClientID)
{
	for(int i = 0; i < COUNT_WORLD; i++)
		GameServer(i)->UpdateClientInformation(FakeClientID);
}

int CServer::GetWorldID(int ClientID)
{
	if(ClientID < 0 || ClientID >= MAX_CLIENTS || m_aClients[ClientID].m_State < CClient::STATE_READY)
		return -1;

	return m_aClients[ClientID].m_MapID;
}

void CServer::SendDiscordGenerateMessage(const char *pColor, const char *pTitle, const char *pMsg)
{
	#ifdef CONF_DISCORD
		char Color[16], Title[256], Msg[512];
		str_copy(Color, pColor, sizeof(Color));
		str_copy(Title, pTitle, sizeof(Title));
		str_copy(Msg, pMsg, sizeof(Msg));

		std::thread t([=]() { m_pDiscord->SendGenerateMessage(g_Config.m_SvDiscordChanal, Color, Title, Msg); });
		t.detach();
	#endif
}

void CServer::SendDiscordMessage(const char *pChanel, const char* pColor, const char* pTitle, const char* pText)
{
	#ifdef CONF_DISCORD
		char aText[512], aTitle[256], aColor[16];
		str_copy(aText, pText, sizeof(aText));
		str_copy(aTitle, pTitle, sizeof(aTitle));
		str_copy(aColor, pColor, sizeof(aColor));

		std::thread t([=]() { m_pDiscord->SendMessage(pChanel, aColor, aTitle, aText); });
		t.detach();
	#endif
}

void CServer::SendDiscordStatus(const char *pStatus, int Type)
{
	#ifdef CONF_DISCORD
		char aStatus[128];		
		int StatusType = Type;
		str_copy(aStatus, pStatus, sizeof(aStatus));
		std::thread t([=]() { m_pDiscord->SendStatus(aStatus, StatusType); });
		t.detach();
	#endif
}

void CServer::AddInformationBotsCount(int Count)
{
	m_BotsCount += Count;
}

void CServer::Kick(int ClientID, const char *pReason)
{
	if(ClientID < 0 || ClientID >= MAX_PLAYERS || m_aClients[ClientID].m_State == CClient::STATE_EMPTY)
	{
		Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "server", "invalid client id to kick");
		return;
	}
	else if(m_RconClientID == ClientID)
	{
		Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "server", "you can't kick yourself");
 		return;
	}
	else if(m_aClients[ClientID].m_Authed > m_RconAuthLevel)
	{
		Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "server", "kick command denied");
 		return;
	}

	m_NetServer.Drop(ClientID, pReason);
}

/*int CServer::Tick()
{
	return m_CurrentGameTick;
}*/

int64 CServer::TickStartTime(int Tick)
{
	return m_GameStartTime + (time_freq()*Tick)/SERVER_TICK_SPEED;
}

/*int CServer::TickSpeed()
{
	return SERVER_TICK_SPEED;
}*/

int CServer::Init()
{
	m_BotsCount = 0;
	WorldSec = 0;
	WorldHour = 0;
	m_CurrentGameTick = 0;
	WorldCheckTime = false;
	for(int i = 0; i < MAX_PLAYERS; i++)
	{
		str_copy(m_aClients[i].m_aLanguage, "en", sizeof(m_aClients[i].m_aLanguage));
		m_aClients[i].m_State = CClient::STATE_EMPTY;
		m_aClients[i].m_aName[0] = 0;
		m_aClients[i].m_aClan[0] = 0;
		m_aClients[i].m_Country = -1;
		m_aClients[i].m_Snapshots.Init();
	}	
	return 0;
}

void CServer::SetRconCID(int ClientID)
{
	m_RconClientID = ClientID;
}

bool CServer::IsAuthed(int ClientID) const
{
	return m_aClients[ClientID].m_Authed;
}

bool CServer::IsBanned(int ClientID)
{
	return m_ServerBan.IsBanned(m_NetServer.ClientAddr(ClientID), 0, 0, 0);
}

int CServer::GetClientInfo(int ClientID, CClientInfo *pInfo) const
{
	dbg_assert(ClientID >= 0 && ClientID < MAX_CLIENTS, "client_id is not valid");
	dbg_assert(pInfo != 0, "info can not be null");

	if(m_aClients[ClientID].m_State == CClient::STATE_INGAME)
	{
		pInfo->m_pName = m_aClients[ClientID].m_aName;
		pInfo->m_Latency = m_aClients[ClientID].m_Latency;
		return 1;
	}
	return 0;
}

void CServer::GetClientAddr(int ClientID, char *pAddrStr, int Size) const
{
	if(ClientID >= 0 && ClientID < MAX_CLIENTS && m_aClients[ClientID].m_State == CClient::STATE_INGAME)
		net_addr_str(m_NetServer.ClientAddr(ClientID), pAddrStr, Size, false);
}

const char *CServer::ClientName(int ClientID) const
{
	if(ClientID < 0 || ClientID >= MAX_CLIENTS || m_aClients[ClientID].m_State == CServer::CClient::STATE_EMPTY)
		return "(invalid)";
	if(m_aClients[ClientID].m_State == CServer::CClient::STATE_INGAME)
		return m_aClients[ClientID].m_aName;
	else
		return "(connecting)";
}

const char *CServer::ClientClan(int ClientID) const
{
	if(ClientID < 0 || ClientID >= MAX_CLIENTS || m_aClients[ClientID].m_State == CServer::CClient::STATE_EMPTY)
		return "";
	if(m_aClients[ClientID].m_State == CServer::CClient::STATE_INGAME)
		return m_aClients[ClientID].m_aClan;
	else
		return "";
}

int CServer::ClientCountry(int ClientID) const
{
	if(ClientID < 0 || ClientID >= MAX_CLIENTS || m_aClients[ClientID].m_State == CServer::CClient::STATE_EMPTY)
		return -1;
	if(m_aClients[ClientID].m_State == CServer::CClient::STATE_INGAME)
		return m_aClients[ClientID].m_Country;
	else
		return -1;
}

bool CServer::ClientIngame(int ClientID) const
{
	return ClientID >= 0 && ClientID < MAX_CLIENTS && m_aClients[ClientID].m_State == CServer::CClient::STATE_INGAME;
}

void CServer::InitRconPasswordIfUnset()
{
	if(m_RconPasswordSet)
	{
		return;
	}

	static const char VALUES[] = "ABCDEFGHKLMNPRSTUVWXYZabcdefghjkmnopqt23456789";
	static const size_t NUM_VALUES = sizeof(VALUES) - 1; // Disregard the '\0'.
	static const size_t PASSWORD_LENGTH = 6;
	dbg_assert(NUM_VALUES * NUM_VALUES >= 2048, "need at least 2048 possibilities for 2-character sequences");
	// With 6 characters, we get a password entropy of log(2048) * 6/2 = 33bit.

	dbg_assert(PASSWORD_LENGTH % 2 == 0, "need an even password length");
	unsigned short aRandom[PASSWORD_LENGTH / 2];
	char aRandomPassword[PASSWORD_LENGTH+1];
	aRandomPassword[PASSWORD_LENGTH] = 0;

	secure_random_fill(aRandom, sizeof(aRandom));
	for(size_t i = 0; i < PASSWORD_LENGTH / 2; i++)
	{
		unsigned short RandomNumber = aRandom[i] % 2048;
		aRandomPassword[2 * i + 0] = VALUES[RandomNumber / NUM_VALUES];
		aRandomPassword[2 * i + 1] = VALUES[RandomNumber % NUM_VALUES];
	}

	str_copy(g_Config.m_SvRconPassword, aRandomPassword, sizeof(g_Config.m_SvRconPassword));
	m_GeneratedRconPassword = 1;
}

int CServer::SendMsg(CMsgPacker *pMsg, int Flags, int ClientID, int WorldID)
{
	if (!pMsg)
		return -1;

	if(ClientID != -1 && (ClientID < 0 || ClientID >= MAX_PLAYERS || m_aClients[ClientID].m_State == CClient::STATE_EMPTY || m_aClients[ClientID].m_Quitting))
		return 0;

	CNetChunk Packet;
	mem_zero(&Packet, sizeof(CNetChunk));
	Packet.m_ClientID = ClientID;
	Packet.m_pData = pMsg->Data();
	Packet.m_DataSize = pMsg->Size();

	if(Flags&MSGFLAG_VITAL)
		Packet.m_Flags |= NETSENDFLAG_VITAL;
	if(Flags&MSGFLAG_FLUSH)
		Packet.m_Flags |= NETSENDFLAG_FLUSH;

	if(!(Flags&MSGFLAG_NOSEND))
	{
		if(ClientID == -1)
		{
			for (int i = 0; i < MAX_PLAYERS; i++)
			{
				if (m_aClients[i].m_State == CClient::STATE_INGAME && !m_aClients[i].m_Quitting)
				{
					if (WorldID != -1)
					{
						if (m_aClients[i].m_MapID == WorldID)
						{
							Packet.m_ClientID = i;
							m_NetServer.Send(&Packet);
						}
						continue;
					}
					Packet.m_ClientID = i;
					m_NetServer.Send(&Packet);
				}
			}
		}
		else
			m_NetServer.Send(&Packet);
	}
	return 0;
}

void CServer::DoSnapshot(int WorldID)
{
	GameServer(WorldID)->OnPreSnap();
	for(int i = 0; i < MAX_PLAYERS; i++)
	{
		// client must be ingame to recive snapshots
		if(m_aClients[i].m_MapID != WorldID || m_aClients[i].m_State != CClient::STATE_INGAME)
			continue;

		// this client is trying to recover, don't spam snapshots
		if(m_aClients[i].m_SnapRate == CClient::SNAPRATE_RECOVER && (Tick()%50) != 0)
			continue;

		// this client is trying to recover, don't spam snapshots
		if(m_aClients[i].m_SnapRate == CClient::SNAPRATE_INIT && (Tick()%10) != 0)
			continue;

		{
			char aData[CSnapshot::MAX_SIZE];
			CSnapshot *pData = (CSnapshot*)aData;	// Fix compiler warning for strict-aliasing
			char aDeltaData[CSnapshot::MAX_SIZE];
			char aCompData[CSnapshot::MAX_SIZE];
			int SnapshotSize;
			int Crc;
			static CSnapshot EmptySnap;
			CSnapshot *pDeltashot = &EmptySnap;
			int DeltashotSize;
			int DeltaTick = -1;
			int DeltaSize;

			m_SnapshotBuilder.Init();
			GameServer(WorldID)->OnSnap(i);

			// finish snapshot
			SnapshotSize = m_SnapshotBuilder.Finish(pData);
			Crc = pData->Crc();

			// remove old snapshos
			// keep 3 seconds worth of snapshots
			m_aClients[i].m_Snapshots.PurgeUntil(m_CurrentGameTick-SERVER_TICK_SPEED*3);

			// save it the snapshot
			m_aClients[i].m_Snapshots.Add(m_CurrentGameTick, time_get(), SnapshotSize, pData, 0);

			// find snapshot that we can preform delta against
			EmptySnap.Clear();

			{
				DeltashotSize = m_aClients[i].m_Snapshots.Get(m_aClients[i].m_LastAckedSnapshot, 0, &pDeltashot, 0);
				if(DeltashotSize >= 0)
					DeltaTick = m_aClients[i].m_LastAckedSnapshot;
				else
				{
					// no acked package found, force client to recover rate
					if(m_aClients[i].m_SnapRate == CClient::SNAPRATE_FULL)
						m_aClients[i].m_SnapRate = CClient::SNAPRATE_RECOVER;
				}
			}

			// create delta
			DeltaSize = m_SnapshotDelta.CreateDelta(pDeltashot, pData, aDeltaData);

			if(DeltaSize)
			{
				// compress it
				int SnapshotSize;
				const int MaxSize = MAX_SNAPSHOT_PACKSIZE;
				int NumPackets;

				SnapshotSize = CVariableInt::Compress(aDeltaData, DeltaSize, aCompData, sizeof(aCompData));
				NumPackets = (SnapshotSize+MaxSize-1)/MaxSize;

				for(int n = 0, Left = SnapshotSize; Left > 0; n++)
				{
					int Chunk = Left < MaxSize ? Left : MaxSize;
					Left -= Chunk;

					if(NumPackets == 1)
					{
						CMsgPacker Msg(NETMSG_SNAPSINGLE, true);
						Msg.AddInt(m_CurrentGameTick);
						Msg.AddInt(m_CurrentGameTick-DeltaTick);
						Msg.AddInt(Crc);
						Msg.AddInt(Chunk);
						Msg.AddRaw(&aCompData[n*MaxSize], Chunk);
						SendMsg(&Msg, MSGFLAG_FLUSH, i);
					}
					else
					{
						CMsgPacker Msg(NETMSG_SNAP, true);
						Msg.AddInt(m_CurrentGameTick);
						Msg.AddInt(m_CurrentGameTick-DeltaTick);
						Msg.AddInt(NumPackets);
						Msg.AddInt(n);
						Msg.AddInt(Crc);
						Msg.AddInt(Chunk);
						Msg.AddRaw(&aCompData[n*MaxSize], Chunk);
						SendMsg(&Msg, MSGFLAG_FLUSH, i);
					}
				}
			}
			else
			{
				CMsgPacker Msg(NETMSG_SNAPEMPTY, true);
				Msg.AddInt(m_CurrentGameTick);
				Msg.AddInt(m_CurrentGameTick-DeltaTick);
				SendMsg(&Msg, MSGFLAG_FLUSH, i);
			}
		}
	}
	GameServer(WorldID)->OnPostSnap();
}


int CServer::NewClientCallback(int ClientID, void *pUser)
{
	CServer *pThis = (CServer *)pUser;
	pThis->GameServer(LOCAL_WORLD)->ClearClientData(ClientID);
	str_copy(pThis->m_aClients[ClientID].m_aLanguage, "en", sizeof(pThis->m_aClients[ClientID].m_aLanguage));
	pThis->m_aClients[ClientID].m_State = CClient::STATE_AUTH;
	pThis->m_aClients[ClientID].m_aName[0] = 0;
	pThis->m_aClients[ClientID].m_aClan[0] = 0;
	pThis->m_aClients[ClientID].m_Country = -1;
	pThis->m_aClients[ClientID].m_Authed = AUTHED_NO;
	pThis->m_aClients[ClientID].m_AuthTries = 0;
	pThis->m_aClients[ClientID].m_pRconCmdToSend = 0;
	pThis->m_aClients[ClientID].m_OldMapID = 0;
	pThis->m_aClients[ClientID].m_MapID = 0;
	pThis->m_aClients[ClientID].m_ChangeMap = false;
	pThis->m_aClients[ClientID].m_NoRconNote = false;
	pThis->m_aClients[ClientID].m_ClientVersion = 0;
	pThis->m_aClients[ClientID].m_Quitting = false;
	pThis->m_aClients[ClientID].Reset();
	return 0;
}

int CServer::DelClientCallback(int ClientID, const char *pReason, void *pUser)
{
	CServer *pThis = (CServer *)pUser;

	char aAddrStr[NETADDR_MAXSTRSIZE];
	net_addr_str(pThis->m_NetServer.ClientAddr(ClientID), aAddrStr, sizeof(aAddrStr), true);
	char aBuf[256];
	str_format(aBuf, sizeof(aBuf), "client dropped. cid=%d addr=%s reason='%s'", ClientID, aAddrStr, pReason);
	pThis->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "server", aBuf);

	// notify the mod about the drop
	if(pThis->m_aClients[ClientID].m_State >= CClient::STATE_READY)
	{
		pThis->GameServer(LOCAL_WORLD)->ClearClientData(ClientID);
		for (int i = 0; i < COUNT_WORLD; i++)
		{
			pThis->m_aClients[ClientID].m_Quitting = true;
			pThis->GameServer(i)->OnClientDrop(ClientID, pReason);
		}
	}
	
	pThis->m_aClients[ClientID].m_State = CClient::STATE_EMPTY;
	pThis->m_aClients[ClientID].m_aName[0] = 0;
	pThis->m_aClients[ClientID].m_aClan[0] = 0;
	pThis->m_aClients[ClientID].m_Country = -1;
	pThis->m_aClients[ClientID].m_Authed = AUTHED_NO;
	pThis->m_aClients[ClientID].m_AuthTries = 0;
	pThis->m_aClients[ClientID].m_pRconCmdToSend = 0;
	pThis->m_aClients[ClientID].m_OldMapID = 0;
	pThis->m_aClients[ClientID].m_MapID = 0;
	pThis->m_aClients[ClientID].m_ChangeMap = false;
	pThis->m_aClients[ClientID].m_NoRconNote = false;
	pThis->m_aClients[ClientID].m_ClientVersion = 0;
	pThis->m_aClients[ClientID].m_Quitting = false;
	pThis->m_aClients[ClientID].m_Snapshots.PurgeAll();
	return 0;
}

void CServer::SendMap(int ClientID)
{
	int MapID = m_aClients[ClientID].m_MapID;
	unsigned Crc = m_pLoadedMap[MapID]->Crc();
	SHA256_DIGEST Sha256 = m_pLoadedMap[MapID]->Sha256();

	CMsgPacker Msg(NETMSG_MAP_CHANGE, true);
	Msg.AddString(GetWorldName(MapID), 0);
	Msg.AddInt(Crc);
	Msg.AddInt(m_pLoadedMap[MapID]->GetCurrentMapSize());
	Msg.AddInt(m_MapChunksPerRequest);
	Msg.AddInt(MAP_CHUNK_SIZE);
	Msg.AddRaw(&Sha256, sizeof(Sha256));
	SendMsg(&Msg, MSGFLAG_VITAL|MSGFLAG_FLUSH, ClientID);
}

void CServer::SendConnectionReady(int ClientID)
{
	CMsgPacker Msg(NETMSG_CON_READY, true);
	SendMsg(&Msg, MSGFLAG_VITAL|MSGFLAG_FLUSH, ClientID);
}

void CServer::SendRconLine(int ClientID, const char *pLine)
{
	CMsgPacker Msg(NETMSG_RCON_LINE, true);
	Msg.AddString(pLine, 512);
	SendMsg(&Msg, MSGFLAG_VITAL, ClientID);
}

void CServer::SendRconLineAuthed(const char *pLine, void *pUser, bool Highlighted)
{
	CServer *pThis = (CServer *)pUser;
	static volatile int ReentryGuard = 0;
	int i;

	if(ReentryGuard) return;
	ReentryGuard++;

	for(i = 0; i < MAX_PLAYERS; i++)
	{
		if(pThis->m_aClients[i].m_State != CClient::STATE_EMPTY && pThis->m_aClients[i].m_Authed >= pThis->m_RconAuthLevel)
			pThis->SendRconLine(i, pLine);
	}

	ReentryGuard--;
}

void CServer::SendRconCmdAdd(const IConsole::CCommandInfo *pCommandInfo, int ClientID)
{
	if (ClientID > MAX_PLAYERS)
		return;

	CMsgPacker Msg(NETMSG_RCON_CMD_ADD, true);
	Msg.AddString(pCommandInfo->m_pName, IConsole::TEMPCMD_NAME_LENGTH);
	Msg.AddString(pCommandInfo->m_pHelp, IConsole::TEMPCMD_HELP_LENGTH);
	Msg.AddString(pCommandInfo->m_pParams, IConsole::TEMPCMD_PARAMS_LENGTH);
	SendMsg(&Msg, MSGFLAG_VITAL, ClientID);
}

void CServer::SendRconCmdRem(const IConsole::CCommandInfo *pCommandInfo, int ClientID)
{
	if (ClientID > MAX_PLAYERS)
		return;

	CMsgPacker Msg(NETMSG_RCON_CMD_REM, true);
	Msg.AddString(pCommandInfo->m_pName, 256);
	SendMsg(&Msg, MSGFLAG_VITAL, ClientID);
}

void CServer::UpdateClientRconCommands()
{
	for(int ClientID = Tick() % MAX_RCONCMD_RATIO; ClientID < MAX_PLAYERS; ClientID += MAX_RCONCMD_RATIO)
	{
		if(m_aClients[ClientID].m_State != CClient::STATE_EMPTY && m_aClients[ClientID].m_Authed)
		{
			int ConsoleAccessLevel = m_aClients[ClientID].m_Authed == AUTHED_ADMIN ? IConsole::ACCESS_LEVEL_ADMIN : IConsole::ACCESS_LEVEL_MOD;
			for(int i = 0; i < MAX_RCONCMD_SEND && m_aClients[ClientID].m_pRconCmdToSend; ++i)
			{
				SendRconCmdAdd(m_aClients[ClientID].m_pRconCmdToSend, ClientID);
				m_aClients[ClientID].m_pRconCmdToSend = m_aClients[ClientID].m_pRconCmdToSend->NextCommandInfo(ConsoleAccessLevel, CFGFLAG_SERVER);
			}
		}
	}
}

void CServer::ProcessClientPacket(CNetChunk *pPacket)
{
	CUnpacker Unpacker;
	Unpacker.Reset(pPacket->m_pData, pPacket->m_DataSize);

	int ClientID = pPacket->m_ClientID;
	if (ClientID > MAX_PLAYERS)
		return;

	// unpack msgid and system flag
	int Msg = Unpacker.GetInt();
	int Sys = Msg&1;
	Msg >>= 1;

	if(Unpacker.Error())
		return;

	if(Sys)
	{
		// system message
		if(Msg == NETMSG_INFO)
		{
			if((pPacket->m_Flags&NET_CHUNKFLAG_VITAL) != 0 && m_aClients[ClientID].m_State == CClient::STATE_AUTH)
			{
				const char *pVersion = Unpacker.GetString(CUnpacker::SANITIZE_CC);
				if(str_comp(pVersion, "0.7 802f1be60a05665f") != 0 && str_comp(pVersion, GameServer()->NetVersion()) != 0)
				{
					// wrong version
					char aReason[256];
					str_format(aReason, sizeof(aReason), "Wrong version. Server is running '%s' and client '%s'", GameServer()->NetVersion(), pVersion);
					m_NetServer.Drop(ClientID, aReason);
					return;
				}

				const char *pPassword = Unpacker.GetString(CUnpacker::SANITIZE_CC);
				if(g_Config.m_Password[0] != 0 && str_comp(g_Config.m_Password, pPassword) != 0)
				{
					// wrong password
					m_NetServer.Drop(ClientID, "Wrong password");
					return;
				}

				m_aClients[ClientID].m_Version = Unpacker.GetInt();
				m_aClients[ClientID].m_State = CClient::STATE_CONNECTING;
				GameServer(LOCAL_WORLD)->ClearClientData(ClientID);
				SendMap(ClientID);
			}
		}
		else if(Msg == NETMSG_REQUEST_MAP_DATA)
		{
			if((pPacket->m_Flags&NET_CHUNKFLAG_VITAL) != 0 && m_aClients[ClientID].m_State == CClient::STATE_CONNECTING)
			{
				int ChunkSize = MAP_CHUNK_SIZE;
				int MapID = m_aClients[ClientID].m_MapID;
				unsigned char *CurrentMapData = m_pLoadedMap[MapID]->GetCurrentMapData();
				int CurrentMapSize = m_pLoadedMap[MapID]->GetCurrentMapSize();

				// send map chunks
				for(int i = 0; i < m_MapChunksPerRequest && m_aClients[ClientID].m_MapChunk >= 0; ++i)
				{
					int Chunk = m_aClients[ClientID].m_MapChunk;
					int Offset = Chunk * ChunkSize;
					if(Offset+ChunkSize >= CurrentMapSize)
					{
						ChunkSize = CurrentMapSize-Offset;
						m_aClients[ClientID].m_MapChunk = -1;
					}
					else
						m_aClients[ClientID].m_MapChunk++;

					CMsgPacker Msg(NETMSG_MAP_DATA, true);
					Msg.AddRaw(&CurrentMapData[Offset], ChunkSize);
					SendMsg(&Msg, MSGFLAG_VITAL|MSGFLAG_FLUSH, ClientID);
	
					if(g_Config.m_Debug)
					{
						char aBuf[64];
						str_format(aBuf, sizeof(aBuf), "sending chunk %d with size %d", Chunk, ChunkSize);
						Console()->Print(IConsole::OUTPUT_LEVEL_DEBUG, "server", aBuf);
					}
				}
			}
		}
		else if(Msg == NETMSG_READY)
		{
			if((pPacket->m_Flags&NET_CHUNKFLAG_VITAL) != 0 && m_aClients[ClientID].m_State == CClient::STATE_CONNECTING)
			{
				int MapID = m_aClients[ClientID].m_MapID;
				m_aClients[ClientID].m_State = CClient::STATE_READY;	
				if(!m_aClients[ClientID].m_ChangeMap)
				{
					char aAddrStr[NETADDR_MAXSTRSIZE];
					net_addr_str(m_NetServer.ClientAddr(ClientID), aAddrStr, sizeof(aAddrStr), true);

					for(int i = 0 ; i < COUNT_WORLD ; i ++)
						GameServer(i)->OnClientConnected(ClientID);
				}
				else
				{
					GameServer(MapID)->ChangeWorld(ClientID);
				}
				SendConnectionReady(ClientID);
			}
		}
		else if(Msg == NETMSG_ENTERGAME)
		{
			const int MapID = m_aClients[ClientID].m_MapID;
			if((pPacket->m_Flags&NET_CHUNKFLAG_VITAL) != 0 && m_aClients[ClientID].m_State == CClient::STATE_READY && GameServer(MapID)->IsClientReady(ClientID))
			{
				SendServerInfo(ClientID);
				if(!m_aClients[ClientID].m_ChangeMap)
				{
					char aAddrStr[NETADDR_MAXSTRSIZE];
					net_addr_str(m_NetServer.ClientAddr(ClientID), aAddrStr, sizeof(aAddrStr), true);
				
					char aBuf[256];
					str_format(aBuf, sizeof(aBuf), "player has entered the game. ClientID=%d addr=%s", ClientID, aAddrStr);
					Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "server", aBuf);
				}
				m_aClients[ClientID].m_State = CClient::STATE_INGAME;
				GameServer(MapID)->OnClientEnter(ClientID);
			}
		}
		else if(Msg == NETMSG_INPUT)
		{
			CClient::CInput *pInput;
			int64 TagTime;
			int64 Now = time_get();

			m_aClients[ClientID].m_LastAckedSnapshot = Unpacker.GetInt();
			int IntendedTick = Unpacker.GetInt();
			int Size = Unpacker.GetInt();

			// check for errors
			if(Unpacker.Error() || Size/4 > MAX_INPUT_SIZE)
				return;

			if(m_aClients[ClientID].m_LastAckedSnapshot > 0)
				m_aClients[ClientID].m_SnapRate = CClient::SNAPRATE_FULL;

			// add message to report the input timing
			// skip packets that are old
			if(IntendedTick > m_aClients[ClientID].m_LastInputTick)
			{
				int TimeLeft = ((TickStartTime(IntendedTick)-Now)*1000) / time_freq();

				CMsgPacker Msg(NETMSG_INPUTTIMING, true);
				Msg.AddInt(IntendedTick);
				Msg.AddInt(TimeLeft);
				SendMsg(&Msg, 0, ClientID);
			}

			m_aClients[ClientID].m_LastInputTick = IntendedTick;

			pInput = &m_aClients[ClientID].m_aInputs[m_aClients[ClientID].m_CurrentInput];

			if(IntendedTick <= Tick())
				IntendedTick = Tick()+1;

			pInput->m_GameTick = IntendedTick;

			for(int i = 0; i < Size/4; i++)
				pInput->m_aData[i] = Unpacker.GetInt();

			int PingCorrection = clamp(Unpacker.GetInt(), 0, 50);
			if(m_aClients[ClientID].m_Snapshots.Get(m_aClients[ClientID].m_LastAckedSnapshot, &TagTime, 0, 0) >= 0)
			{
				m_aClients[ClientID].m_Latency = (int)(((Now-TagTime)*1000)/time_freq());
				m_aClients[ClientID].m_Latency = max(0, m_aClients[ClientID].m_Latency - PingCorrection);
			}
			mem_copy(m_aClients[ClientID].m_LatestInput.m_aData, pInput->m_aData, MAX_INPUT_SIZE*sizeof(int));

			m_aClients[ClientID].m_CurrentInput++;
			m_aClients[ClientID].m_CurrentInput %= 200;

			// call the mod with the fresh input data
			if(m_aClients[ClientID].m_State == CClient::STATE_INGAME)
			{
				int MapID = m_aClients[ClientID].m_MapID;
				GameServer(MapID)->OnClientDirectInput(ClientID, m_aClients[ClientID].m_LatestInput.m_aData);
			}
		}
		else if(Msg == NETMSG_RCON_CMD)
		{
			const char *pCmd = Unpacker.GetString();

			if((pPacket->m_Flags&NET_CHUNKFLAG_VITAL) != 0 && Unpacker.Error() == 0 && m_aClients[ClientID].m_Authed)
			{
				char aBuf[256];
				str_format(aBuf, sizeof(aBuf), "ClientID=%d rcon='%s'", ClientID, pCmd);
				Console()->Print(IConsole::OUTPUT_LEVEL_ADDINFO, "server", aBuf);
				m_RconClientID = ClientID;
				m_RconAuthLevel = m_aClients[ClientID].m_Authed;
				Console()->SetAccessLevel(m_aClients[ClientID].m_Authed == AUTHED_ADMIN ? IConsole::ACCESS_LEVEL_ADMIN : IConsole::ACCESS_LEVEL_MOD);
				Console()->ExecuteLineFlag(pCmd, CFGFLAG_SERVER);
				Console()->SetAccessLevel(IConsole::ACCESS_LEVEL_ADMIN);
				m_RconClientID = IServer::RCON_CID_SERV;
				m_RconAuthLevel = AUTHED_ADMIN;
			}
		}
		else if(Msg == NETMSG_RCON_AUTH)
		{
			const char *pPw = Unpacker.GetString(CUnpacker::SANITIZE_CC);

			if((pPacket->m_Flags&NET_CHUNKFLAG_VITAL) != 0 && Unpacker.Error() == 0)
			{
				if(g_Config.m_SvRconPassword[0] == 0 && g_Config.m_SvRconModPassword[0] == 0)
				{
					if(!m_aClients[ClientID].m_NoRconNote)
					{
						SendRconLine(ClientID, "No rcon password set on server. Set sv_rcon_password and/or sv_rcon_mod_password to enable the remote console.");
						m_aClients[ClientID].m_NoRconNote = true;
					}
				}
				else if(g_Config.m_SvRconPassword[0] && str_comp(pPw, g_Config.m_SvRconPassword) == 0)
				{
					CMsgPacker Msg(NETMSG_RCON_AUTH_ON, true);
					SendMsg(&Msg, MSGFLAG_VITAL, ClientID);

					m_aClients[ClientID].m_Authed = AUTHED_ADMIN;
					m_aClients[ClientID].m_pRconCmdToSend = Console()->FirstCommandInfo(IConsole::ACCESS_LEVEL_ADMIN, CFGFLAG_SERVER);
					SendRconLine(ClientID, "Admin authentication successful. Full remote console access granted.");
					char aBuf[256];
					str_format(aBuf, sizeof(aBuf), "ClientID=%d authed (admin)", ClientID);
					Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "server", aBuf);
				}
				else if(g_Config.m_SvRconModPassword[0] && str_comp(pPw, g_Config.m_SvRconModPassword) == 0)
				{
					CMsgPacker Msg(NETMSG_RCON_AUTH_ON, true);
					SendMsg(&Msg, MSGFLAG_VITAL, ClientID);

					m_aClients[ClientID].m_Authed = AUTHED_MOD;
					m_aClients[ClientID].m_pRconCmdToSend = Console()->FirstCommandInfo(IConsole::ACCESS_LEVEL_MOD, CFGFLAG_SERVER);
					SendRconLine(ClientID, "Moderator authentication successful. Limited remote console access granted.");
					char aBuf[256];
					str_format(aBuf, sizeof(aBuf), "ClientID=%d authed (moderator)", ClientID);
					Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "server", aBuf);
				}
				else if (g_Config.m_SvRconMaxTries && m_ServerBan.IsBannable(m_NetServer.ClientAddr(ClientID)))
				{
					m_aClients[ClientID].m_AuthTries++;
					char aBuf[128];
					str_format(aBuf, sizeof(aBuf), "Wrong password %d/%d.", m_aClients[ClientID].m_AuthTries, g_Config.m_SvRconMaxTries);
					SendRconLine(ClientID, aBuf);
					if(m_aClients[ClientID].m_AuthTries >= g_Config.m_SvRconMaxTries)
					{
						if(!g_Config.m_SvRconBantime)
							m_NetServer.Drop(ClientID, "Too many remote console authentication tries");
						else
							m_ServerBan.BanAddr(m_NetServer.ClientAddr(ClientID), g_Config.m_SvRconBantime*60, "Too many remote console authentication tries");
					}
				}
				else
				{
					SendRconLine(ClientID, "Wrong password.");
				}
			}
		}
		else if(Msg == NETMSG_PING)
		{
			CMsgPacker Msg(NETMSG_PING_REPLY, true);
			SendMsg(&Msg, 0, ClientID);
		}
		else
		{
			if(g_Config.m_Debug)
			{
				char aHex[] = "0123456789ABCDEF";
				char aBuf[512];

				for(int b = 0; b < pPacket->m_DataSize && b < 32; b++)
				{
					aBuf[b*3] = aHex[((const unsigned char *)pPacket->m_pData)[b]>>4];
					aBuf[b*3+1] = aHex[((const unsigned char *)pPacket->m_pData)[b]&0xf];
					aBuf[b*3+2] = ' ';
					aBuf[b*3+3] = 0;
				}

				char aBufMsg[256];
				str_format(aBufMsg, sizeof(aBufMsg), "strange message ClientID=%d msg=%d data_size=%d", ClientID, Msg, pPacket->m_DataSize);
				Console()->Print(IConsole::OUTPUT_LEVEL_DEBUG, "server", aBufMsg);
				Console()->Print(IConsole::OUTPUT_LEVEL_DEBUG, "server", aBuf);
			}
		}
	}
	else
	{
		// game message
		if((pPacket->m_Flags&NET_CHUNKFLAG_VITAL) != 0 && m_aClients[ClientID].m_State >= CClient::STATE_READY)
		{
			int MapID = m_aClients[ClientID].m_MapID;
			GameServer(MapID)->OnMessage(Msg, &Unpacker, ClientID);
		}
	}
}

void CServer::GenerateServerInfo(CPacker *pPacker, int Token)
{
	// count the players
	int PlayerCount = 0, ClientCount = 0;
	for(int i = 0; i < MAX_PLAYERS; i++)
	{
		if(m_aClients[i].m_State != CClient::STATE_EMPTY)
		{
			if(GameServer()->IsClientPlayer(i))
				PlayerCount++;

			ClientCount++;
		}
	}

	if(Token != -1)
	{
		pPacker->Reset();
		pPacker->AddRaw(SERVERBROWSE_INFO, sizeof(SERVERBROWSE_INFO));
		pPacker->AddInt(Token);
	}

	pPacker->AddString(GameServer()->Version(), 32);

	if(g_Config.m_SvShowWorldInformation)
	{
		char aBuf[128];
		str_format(aBuf, sizeof(aBuf), "%s : Active mobs %d on %d zones", g_Config.m_SvName, m_BotsCount, COUNT_WORLD);
		pPacker->AddString(aBuf, 64);
	}
	else
	{
		pPacker->AddString(g_Config.m_SvName, 64);
	}
	pPacker->AddString(g_Config.m_SvHostname, 128);
	pPacker->AddString(GetMapName(), 32);

	// gametype
	pPacker->AddString(GameServer()->GameType(), 16);

	// flags
	int Flags = 0;
	if(g_Config.m_Password[0])  // password set
		Flags |= SERVERINFO_FLAG_PASSWORD;
	if(GameServer()->TimeScore())
		Flags |= SERVERINFO_FLAG_TIMESCORE;
	pPacker->AddInt(Flags);

	pPacker->AddInt(g_Config.m_SvSkillLevel);	// server skill level
	pPacker->AddInt(PlayerCount); // num players
	pPacker->AddInt(MAX_PLAYERS); // max players
	pPacker->AddInt(ClientCount); // num clients
	pPacker->AddInt(MAX_PLAYERS); // max clients

	if(Token != -1)
	{
		for(int i = 0; i < MAX_PLAYERS; i++)
		{
			if(m_aClients[i].m_State != CClient::STATE_EMPTY)
			{
				pPacker->AddString(ClientName(i), MAX_NAME_LENGTH); // client name
				pPacker->AddString(ClientClan(i), MAX_CLAN_LENGTH); // client clan
				pPacker->AddInt(m_aClients[i].m_Country); // client country
				pPacker->AddInt(m_aClients[i].m_Score); // client score
				pPacker->AddInt(GameServer()->IsClientPlayer(i)?0:1); // flag spectator=1, bot=2 (player=0)
			}
		}
	}
}

void CServer::SendServerInfo(int ClientID)
{
	CMsgPacker Msg(NETMSG_SERVERINFO, true);
	GenerateServerInfo(&Msg, -1);
	if(ClientID == -1)
	{
		for(int i = 0; i < MAX_PLAYERS; i++)
		{
			if(m_aClients[i].m_State != CClient::STATE_EMPTY)
				SendMsg(&Msg, MSGFLAG_VITAL|MSGFLAG_FLUSH, i);
		}
	}
	else if(ClientID >= 0 && ClientID < MAX_PLAYERS && m_aClients[ClientID].m_State != CClient::STATE_EMPTY)
		SendMsg(&Msg, MSGFLAG_VITAL|MSGFLAG_FLUSH, ClientID);
}

void CServer::PumpNetwork()
{
	CNetChunk Packet;
	TOKEN ResponseToken;

	m_NetServer.Update();

	// process packets
	while (m_NetServer.Recv(&Packet, &ResponseToken))
	{
		if (Packet.m_Flags & NETSENDFLAG_CONNLESS)
		{
			if (m_Register.RegisterProcessPacket(&Packet, ResponseToken))
				continue;
			if (Packet.m_DataSize >= int(sizeof(SERVERBROWSE_GETINFO)) &&
				mem_comp(Packet.m_pData, SERVERBROWSE_GETINFO, sizeof(SERVERBROWSE_GETINFO)) == 0)
			{
				CUnpacker Unpacker;
				Unpacker.Reset((unsigned char*)Packet.m_pData + sizeof(SERVERBROWSE_GETINFO), Packet.m_DataSize - sizeof(SERVERBROWSE_GETINFO));
				int SrvBrwsToken = Unpacker.GetInt();
				if (Unpacker.Error())
					continue;

				CPacker Packer;
				CNetChunk Response;

				GenerateServerInfo(&Packer, SrvBrwsToken);

				Response.m_ClientID = -1;
				Response.m_Address = Packet.m_Address;
				Response.m_Flags = NETSENDFLAG_CONNLESS;
				Response.m_pData = Packer.Data();
				Response.m_DataSize = Packer.Size();
				m_NetServer.Send(&Response, ResponseToken);
			}
		}
		else
			ProcessClientPacket(&Packet);
	}

	m_ServerBan.Update();
	m_Econ.Update();
}

const char *CServer::GetMapName() const
{
	// get the name of the map without his path
	char *pMapShortName = &g_Config.m_SvMap[0];
	for(int i = 0; i < str_length(g_Config.m_SvMap)-1; i++)
	{
		if(g_Config.m_SvMap[i] == '/' || g_Config.m_SvMap[i] == '\\')
			pMapShortName = &g_Config.m_SvMap[i+1];
	}
	return pMapShortName;
}

bool CServer::LoadMap(const char *pMapName, int ID)
{
	char aBuf[512];
	str_format(aBuf, sizeof(aBuf), "maps/%s.map", pMapName);

	// check for valid standard map
	if(!m_MapChecker.ReadAndValidateMap(Storage(), aBuf, IStorage::TYPE_ALL))
	{
		Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "mapchecker", "invalid standard map");
		return 0;
	}

	IEngineMap *pMap = m_pLoadedMap[ID];
	if(!pMap->Load(aBuf))
		return 0;

	// reinit snapshot ids
	m_IDPool.TimeoutIDs();

	// get the sha256 and crc of the map
	char aSha256[SHA256_MAXSTRSIZE];
	sha256_str(pMap->Sha256(), aSha256, sizeof(aSha256));
	char aBufMsg[256];
	str_format(aBufMsg, sizeof(aBufMsg), "%s sha256 is %s", aBuf, aSha256);
	Console()->Print(IConsole::OUTPUT_LEVEL_ADDINFO, "server", aBufMsg);
	str_format(aBufMsg, sizeof(aBufMsg), "%s crc is %08x", aBuf, pMap->Crc());
	Console()->Print(IConsole::OUTPUT_LEVEL_ADDINFO, "server", aBufMsg);
	str_copy(m_aCurrentMap, pMapName, sizeof(m_aCurrentMap));

	// load complete map into memory for download
	{
		IOHANDLE File = Storage()->OpenFile(aBuf, IOFLAG_READ, IStorage::TYPE_ALL);
		pMap->SetCurrentMapSize((int)io_length(File));	
		pMap->SetCurrentMapData((unsigned char *)mem_alloc(pMap->GetCurrentMapSize(), 1));
		io_read(File, pMap->GetCurrentMapData(), pMap->GetCurrentMapSize());
		io_close(File);
	}
	return true;
}

void CServer::InitRegister(CNetServer *pNetServer, IEngineMasterServer *pMasterServer, IConsole *pConsole)
{
	m_Register.Init(pNetServer, pMasterServer, pConsole);
}

int CServer::Run()
{
	m_PrintCBIndex = Console()->RegisterPrintCallback(g_Config.m_ConsoleOutputLevel, SendRconLineAuthed, this);
	m_MapChunksPerRequest = g_Config.m_SvMapDownloadSpeed;

	{
		int MapDontLoad = -1;
		for(int i = 0; i < COUNT_WORLD; i ++)
		{
			char aBuf[256];
			str_format(aBuf, sizeof(aBuf), "%s%d", g_Config.m_SvMap, i);
			if(!LoadMap(aBuf, i))
				MapDontLoad = i;
			if(MapDontLoad != -1 && i == COUNT_WORLD-1)
			{
				str_format(aBuf, sizeof(aBuf), "%s%d MAP NOT FOUND OR LOADING...", g_Config.m_SvMap, MapDontLoad);
				Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "server", aBuf);
				return -1;
			}
		}
	}

	// start server
	NETADDR BindAddr;
	if(g_Config.m_Bindaddr[0] && net_host_lookup(g_Config.m_Bindaddr, &BindAddr, NETTYPE_ALL) == 0)
	{
		BindAddr.type = NETTYPE_ALL;
		BindAddr.port = g_Config.m_SvPort;
	}
	else
	{
		mem_zero(&BindAddr, sizeof(BindAddr));
		BindAddr.type = NETTYPE_ALL;
		BindAddr.port = g_Config.m_SvPort;
	}

	if(!m_NetServer.Open(BindAddr, &m_ServerBan, g_Config.m_SvMaxClients, g_Config.m_SvMaxClientsPerIP, NewClientCallback, DelClientCallback, this))
	{
		dbg_msg("server", "couldn't open socket. port %d might already be in use", g_Config.m_SvPort);
		return -1;
	}

	m_Econ.Init(Console(), &m_ServerBan);

	char aBuf[256];
	str_format(aBuf, sizeof(aBuf), "server name is '%s'", g_Config.m_SvName);
	Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "server", aBuf);

	for(int i = 0; i < COUNT_WORLD; i++)
		GameServer(i)->OnInit(i);

	str_format(aBuf, sizeof(aBuf), "version %s", GameServer()->NetVersion());
	Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "server", aBuf);

	// process pending commands
	m_pConsole->StoreCommands(false);

	if(m_GeneratedRconPassword)
	{
		dbg_msg("server", "+-------------------------+");
		dbg_msg("server", "| rcon password: '%s' |", g_Config.m_SvRconPassword);
		dbg_msg("server", "+-------------------------+");
	}

	// start game
	{
		m_GameStartTime = time_get();
		while(m_RunServer)
		{
			int64 t = time_get();
			bool NewTicks = false;
			bool ShouldSnap = false;

			while(t > TickStartTime(m_CurrentGameTick+1))
			{
				m_CurrentGameTick++;
				NewTicks = true;
				if((m_CurrentGameTick % 2) == 0)
					ShouldSnap = true;

				// apply new input
				for(int c = 0; c < MAX_PLAYERS; c++)
				{
					if(m_aClients[c].m_State == CClient::STATE_EMPTY)
						continue;
					for(int i = 0; i < 200; i++)
					{
						if(m_aClients[c].m_aInputs[i].m_GameTick == Tick())
						{
							if(m_aClients[c].m_State == CClient::STATE_INGAME)
							{
								int MapID = m_aClients[c].m_MapID;
								GameServer(MapID)->OnClientPredictedInput(c, m_aClients[c].m_aInputs[i].m_aData);
							}
							break;
						}
					}
				}
				// World Tick
				if(Tick() % TickSpeed() == 0)
				{
					// effect KARLSON
					if(!WorldCheckTime) 
						WorldSec++;
					else
						WorldCheckTime = false;

					if(WorldSec >= 60)
					{
						WorldHour++;
						if(WorldHour >= 24)
							WorldHour = 0;
							
						WorldSec = 0;
					}
				}

				for(int o = 0; o < COUNT_WORLD; o++)
					GameServer(o)->OnTick();
			}

			// snap game
			if(NewTicks)
			{
				if(g_Config.m_SvHighBandwidth || ShouldSnap)
				{
					for (int o = 0; o < COUNT_WORLD; o++)
						DoSnapshot(o);
				}
				UpdateClientRconCommands();
			}

			// master server stuff
			m_Register.RegisterUpdate(m_NetServer.NetType());
			PumpNetwork();

			// wait for incomming data
			net_socket_read_wait(m_NetServer.Socket(), clamp(int((TickStartTime(m_CurrentGameTick + 1) - time_get()) * 1000 / time_freq()), 1, 1000 / SERVER_TICK_SPEED / 2));
		}
	}

	// disconnect all clients on shutdown
	m_NetServer.Close();
	m_Econ.Shutdown();

	for(int i = 0 ; i < COUNT_WORLD ; i ++)
	{
		GameServer(i)->OnShutdown();
		m_pLoadedMap[i]->Unload();
	}
	return 0;
}

void CServer::ConKick(IConsole::IResult *pResult, void *pUser)
{
	if(pResult->NumArguments() > 1)
	{
		char aBuf[128];
		str_format(aBuf, sizeof(aBuf), "Kicked (%s)", pResult->GetString(1));
		((CServer *)pUser)->Kick(pResult->GetInteger(0), aBuf);
	}
	else
		((CServer *)pUser)->Kick(pResult->GetInteger(0), "Kicked by console");
}

void CServer::ConStatus(IConsole::IResult *pResult, void *pUser)
{
	char aBuf[1024];
	char aAddrStr[NETADDR_MAXSTRSIZE];
	CServer* pThis = static_cast<CServer *>(pUser);

	for(int i = 0; i < MAX_PLAYERS; i++)
	{
		if(pThis->m_aClients[i].m_State != CClient::STATE_EMPTY)
		{
			net_addr_str(pThis->m_NetServer.ClientAddr(i), aAddrStr, sizeof(aAddrStr), true);
			if(pThis->m_aClients[i].m_State == CClient::STATE_INGAME)
			{
				const char *pAuthStr = pThis->m_aClients[i].m_Authed == CServer::AUTHED_ADMIN ? "(Admin)" :
										pThis->m_aClients[i].m_Authed == CServer::AUTHED_MOD ? "(Mod)" : "";
				str_format(aBuf, sizeof(aBuf), "id=%d addr=%s client=%x name='%s' score=%d %s", i, aAddrStr,
					pThis->m_aClients[i].m_Version, pThis->m_aClients[i].m_aName, pThis->m_aClients[i].m_Score, pAuthStr);
			}
			else
				str_format(aBuf, sizeof(aBuf), "id=%d addr=%s connecting", i, aAddrStr);
			pThis->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "Server", aBuf);
		}
	}
}

void CServer::ConShutdown(IConsole::IResult *pResult, void *pUser)
{
	((CServer *)pUser)->m_RunServer = 0;
}

void CServer::ConLogout(IConsole::IResult *pResult, void *pUser)
{
	CServer *pServer = (CServer *)pUser;

	if(pServer->m_RconClientID >= 0 && pServer->m_RconClientID < MAX_PLAYERS &&
		pServer->m_aClients[pServer->m_RconClientID].m_State != CServer::CClient::STATE_EMPTY)
	{
		CMsgPacker Msg(NETMSG_RCON_AUTH_OFF, true);
		pServer->SendMsg(&Msg, MSGFLAG_VITAL, pServer->m_RconClientID);

		pServer->m_aClients[pServer->m_RconClientID].m_Authed = AUTHED_NO;
		pServer->m_aClients[pServer->m_RconClientID].m_AuthTries = 0;
		pServer->m_aClients[pServer->m_RconClientID].m_pRconCmdToSend = 0;
		pServer->SendRconLine(pServer->m_RconClientID, "Logout successful.");
		char aBuf[32];
		str_format(aBuf, sizeof(aBuf), "ClientID=%d logged out", pServer->m_RconClientID);
		pServer->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "server", aBuf);
	}
}

void CServer::ConchainSpecialInfoupdate(IConsole::IResult *pResult, void *pUserData, IConsole::FCommandCallback pfnCallback, void *pCallbackUserData)
{
	pfnCallback(pResult, pCallbackUserData);
	if(pResult->NumArguments())
	{
		str_clean_whitespaces(g_Config.m_SvName);
		((CServer *)pUserData)->SendServerInfo(-1);
	}
}

void CServer::ConchainMaxclientsperipUpdate(IConsole::IResult *pResult, void *pUserData, IConsole::FCommandCallback pfnCallback, void *pCallbackUserData)
{
	pfnCallback(pResult, pCallbackUserData);
	if(pResult->NumArguments())
		((CServer *)pUserData)->m_NetServer.SetMaxClientsPerIP(pResult->GetInteger(0));
}

void CServer::ConchainModCommandUpdate(IConsole::IResult *pResult, void *pUserData, IConsole::FCommandCallback pfnCallback, void *pCallbackUserData)
{
	if(pResult->NumArguments() == 2)
	{
		CServer *pThis = static_cast<CServer *>(pUserData);
		const IConsole::CCommandInfo *pInfo = pThis->Console()->GetCommandInfo(pResult->GetString(0), CFGFLAG_SERVER, false);
		int OldAccessLevel = 0;
		if(pInfo)
			OldAccessLevel = pInfo->GetAccessLevel();
		pfnCallback(pResult, pCallbackUserData);
		if(pInfo && OldAccessLevel != pInfo->GetAccessLevel())
		{
			for(int i = 0; i < MAX_PLAYERS; ++i)
			{
				if(pThis->m_aClients[i].m_State == CServer::CClient::STATE_EMPTY || pThis->m_aClients[i].m_Authed != CServer::AUTHED_MOD ||
					(pThis->m_aClients[i].m_pRconCmdToSend && str_comp(pResult->GetString(0), pThis->m_aClients[i].m_pRconCmdToSend->m_pName) >= 0))
					continue;

				if(OldAccessLevel == IConsole::ACCESS_LEVEL_ADMIN)
					pThis->SendRconCmdAdd(pInfo, i);
				else
					pThis->SendRconCmdRem(pInfo, i);
			}
		}
	}
	else
		pfnCallback(pResult, pCallbackUserData);
}

void CServer::ConchainConsoleOutputLevelUpdate(IConsole::IResult *pResult, void *pUserData, IConsole::FCommandCallback pfnCallback, void *pCallbackUserData)
{
	pfnCallback(pResult, pCallbackUserData);
	if(pResult->NumArguments() == 1)
	{
		CServer *pThis = static_cast<CServer *>(pUserData);
		pThis->Console()->SetPrintOutputLevel(pThis->m_PrintCBIndex, pResult->GetInteger(0));
	}
}

void CServer::ConchainRconPasswordSet(IConsole::IResult *pResult, void *pUserData, IConsole::FCommandCallback pfnCallback, void *pCallbackUserData)
{
	pfnCallback(pResult, pCallbackUserData);
	if(pResult->NumArguments() >= 1)
	{
		static_cast<CServer *>(pUserData)->m_RconPasswordSet = 1;
	}
}

void CServer::RegisterCommands()
{
	m_pConsole = Kernel()->RequestInterface<IConsole>();
	for(int i = 0; i < COUNT_WORLD; i ++)
	{
		m_pGameServer[i] = Kernel()->RequestInterface<IGameServer>(i);
		m_pLoadedMap[i] = Kernel()->RequestInterface<IEngineMap>(i);
	}
	m_pStorage = Kernel()->RequestInterface<IStorage>();

	// register console commands
	Console()->Register("kick", "i[id] ?r[reason]", CFGFLAG_SERVER, ConKick, this, "Kick player with specified id for any reason");
	Console()->Register("status", "", CFGFLAG_SERVER, ConStatus, this, "List players");
	Console()->Register("shutdown", "", CFGFLAG_SERVER, ConShutdown, this, "Shut down");
	Console()->Register("logout", "", CFGFLAG_SERVER, ConLogout, this, "Logout of rcon");

	Console()->Chain("sv_name", ConchainSpecialInfoupdate, this);
	Console()->Chain("password", ConchainSpecialInfoupdate, this);

	Console()->Chain("sv_max_clients_per_ip", ConchainMaxclientsperipUpdate, this);
	Console()->Chain("mod_command", ConchainModCommandUpdate, this);
	Console()->Chain("console_output_level", ConchainConsoleOutputLevelUpdate, this);
	Console()->Chain("sv_rcon_password", ConchainRconPasswordSet, this);

	// register console commands in sub parts
	m_ServerBan.InitServerBan(Console(), Storage(), this);
	
	for(int i = 0; i < COUNT_WORLD; i ++)
		m_pGameServer[i]->OnConsoleInit();
}


void CServer::InitClientBot(int ClientID)
{
	if (ClientID < MAX_PLAYERS || ClientID >= MAX_CLIENTS)
		return;
		
	m_aClients[ClientID].m_State = CClient::STATE_INGAME;
	m_aClients[ClientID].m_MapID = -1;
	SetClientClan(ClientID, "::Bots:");
	SendConnectionReady(ClientID);
}

int CServer::SnapNewID()
{
	return m_IDPool.NewID();
}

void CServer::SnapFreeID(int ID)
{
	m_IDPool.FreeID(ID);
}


void *CServer::SnapNewItem(int Type, int ID, int Size)
{
	dbg_assert(Type >= 0 && Type <=0xffff, "incorrect type");
	dbg_assert(ID >= 0 && ID <=0xffff, "incorrect id");
	return ID < 0 ? 0 : m_SnapshotBuilder.NewItem(Type, ID, Size);
}

void CServer::SnapSetStaticsize(int ItemType, int Size)
{
	m_SnapshotDelta.SetStaticsize(ItemType, Size);
}

static CServer *CreateServer() { return new CServer(); }


#ifdef CONF_DISCORD
DiscordJob::DiscordJob(const char *token, int threads) : SleepyDiscord::DiscordClient(token, SleepyDiscord::USER_CONTROLED_THREADS)
{
	std::thread t1(&DiscordJob::run, this);
	t1.detach();
}

void DiscordJob::SetServer(CServer *pServer) 
{
	m_pServer = pServer;
}

void DiscordJob::onMessage(SleepyDiscord::Message message) 
{
	if(message.length() <= 0 || !g_Config.m_SvCreateDiscordBot || message.author == getCurrentUser().cast())
	 	return;

	// статистика
	if (message.startsWith("!mstats"))
	{
		// если количество символов меньше нужного
		std::string messagecont = message.content;
		if(messagecont.size() <= 8)
		{
			SendMessage(std::string(message.channelID).c_str(), DC_SERVER_WARNING, "Not right!",
			std::string("Use **!mstats <nick full or not>. Minimal symbols 1.**!!!"));
			return;
		}

		// информация для поиска
		bool founds = false; int Limit = 0;
		std::string input = "%" + messagecont.substr(8, messagecont.length() - 8) + "%";
		sqlstr::CSqlString<64> cDiscordIDorNick = sqlstr::CSqlString<64>(input.c_str());

		// ищим пользователя
		boost::scoped_ptr<ResultSet> RES(SJK.SD("*", "tw_accounts_data", "WHERE Nick LIKE '%s'LIMIT 5", cDiscordIDorNick.cstr()));
		while(RES->next())
		{
			const int AuthID = RES->getInt("ID");
			const int RandomColor = 1000+random_int()%10000000;
			const int Rank = Server()->GameServer()->GetRank(AuthID);

			char aBuf[256];
			str_format(aBuf, sizeof(aBuf), "?player=%s&rank=%d&dicid=%d", RES->getString("Nick").c_str(), Rank, RES->getInt("DiscordEquip"));
			SendGenerateMessage(std::string(message.channelID).c_str(), std::to_string(RandomColor).c_str(), "Discord MRPG Card", aBuf);
			founds = true;
		}

		if(!founds)
		{
			SendMessage(std::string(message.channelID).c_str(), DC_SERVER_WARNING, "Sorry!",
				"**This account not found in database!**");
		}
		return;
	}

	else if (message.startsWith("!mconnect"))
	{	
		// получаем Айди пользователя
		SleepyDiscord::Snowflake<SleepyDiscord::User> userAuth = getUser(message.author).cast();
		SendMessage(std::string(message.channelID).c_str(), DC_DISCORD_INFO, "Connector Information",  
			"```ini\n[Warning] Do not connect other people's discords to your account in the game\nThis is similar to hacking your account in the game\n# Use in-game for connect your personal discord:\n# Did '" + userAuth + "'\n# Command in-game: /discord_connect <did>```");

		// проверяем на поиск пользователя
		std::string Nick = "Refresh please.";
		std::string UserID = userAuth;

		// информация для поиска
		bool founds = false;
		sqlstr::CSqlString<64> cDiscordID = sqlstr::CSqlString<64>(UserID.c_str());

		// получаем подключение
		boost::scoped_ptr<ResultSet> RES(SJK.SD("Nick", "tw_accounts_data", "WHERE DiscordID = '%s'", cDiscordID.cstr()));
		while(RES->next())
		{
			// пишем хорошее подключение
			Nick = RES->getString("Nick").c_str();
			SendMessage(std::string(message.channelID).c_str(), DC_DISCORD_BOT, "Good work :)", "**Your account is enabled: Nickname in-game: " + Nick + "**");
			founds = true;
		}

		// завершаем подключение
		if(!founds)
		{
			SendMessage(std::string(message.channelID).c_str(), DC_SERVER_WARNING, "Fail in work :(", "**Fail connect. See !mconnect.\nUse in-game /discord_connect <DID>..**");
		}
		return;
	}

	// ПОКАЗАТЬ ВСЕХ ИГРОКОВ
	else if (message.startsWith("!monline"))
	{
		dynamic_string Buffer;
		for(int i = 0; i < MAX_PLAYERS; i++) 
		{
			if(!Server()->ClientIngame(i)) 
				continue;

			Buffer.append_at(Buffer.length(), Server()->ClientName(i));
			Buffer.append_at(Buffer.length(), "\n");
		}
		if(Buffer.length() <= 0)
		{
			SendMessage(std::string(message.channelID).c_str(), DC_DISCORD_BOT, "Online Server", "Server is empty");
			return;
		}
		SendMessage(std::string(message.channelID).c_str(), DC_DISCORD_BOT, "Online Server", Buffer.buffer());
		Buffer.clear();
		return;
	}
	// ПОМОЩЬ
	else if (message.startsWith("!mhelp"))
	{
		SendMessage(std::string(message.channelID).c_str(), DC_DISCORD_INFO, "Commands / Information", 
		"`!mconnect` - Info for connect your discord and account in game."
		"\n`!mstats <symbol>` - See stats players. Minimal 1 symbols."
		"\n`!monline` - Show players ingame.");
	}
	// отправить из дискорд чата на сервер
	else if(str_comp(std::string(message.channelID).c_str(), g_Config.m_SvDiscordChanal) == 0)
	{
		std::string Nickname("D|" + message.author.username);
		m_pServer->GameServer(FREE_SLOTS_WORLD)->FakeChat(Nickname.c_str(), message.content.c_str());
	}
	// контрорирование ideas-voting
	else if(str_comp(std::string(message.channelID).c_str(), g_Config.m_SvDiscordIdeasChanal) == 0)
	{
		deleteMessage(message.channelID, message);

		SleepyDiscord::Embed embed;
		embed.title = std::string("Suggestion");
		embed.color = 431050;

		SleepyDiscord::EmbedThumbnail embedthumb;
		embedthumb.url = message.author.avatarUrl();
		embedthumb.proxyUrl = message.author.avatarUrl();
		embed.thumbnail = embedthumb;

		SleepyDiscord::EmbedFooter embedfooter;
		embedfooter.text = "Use reactions for voting!";
		embedfooter.iconUrl = message.author.avatarUrl();
		embedfooter.proxyIconUrl = message.author.avatarUrl();
		embed.footer = embedfooter;
		embed.description = "From:" + message.author.showUser() + "!\n" + message.content;

		SleepyDiscord::Message pMessage = sendMessage(message.channelID, "\0", embed);
		addReaction(message.channelID, pMessage, "%E2%9C%85");
		addReaction(message.channelID, pMessage, "%E2%9D%8C");
	}
}

void DiscordJob::onReaction(SleepyDiscord::Snowflake<SleepyDiscord::User> userID, SleepyDiscord::Snowflake<SleepyDiscord::Channel> channelID, 
SleepyDiscord::Snowflake<SleepyDiscord::Message> messageID, SleepyDiscord::Emoji emoji)
{ 
}

void DiscordJob::onDeleteReaction(SleepyDiscord::Snowflake<SleepyDiscord::User> userID, SleepyDiscord::Snowflake<SleepyDiscord::Channel> channelID, 
SleepyDiscord::Snowflake<SleepyDiscord::Message> messageID, SleepyDiscord::Emoji emoji)
{
}

void DiscordJob::UpdateMessageIdeas(SleepyDiscord::Snowflake<SleepyDiscord::User> userID, SleepyDiscord::Snowflake<SleepyDiscord::Channel> channelID, 
SleepyDiscord::Snowflake<SleepyDiscord::Message> messageID)
{
	if(userID == getCurrentUser().cast())
		return;
}

void DiscordJob::SendStatus(const char *Status, int Type)
{
	if(!g_Config.m_SvCreateDiscordBot) return;
	this->updateStatus(Status, Type);
}

void DiscordJob::SendGenerateMessage(const char *pChanal, const char *Color, const char *Title, const char *pPhpArg)
{
	if(!g_Config.m_SvCreateDiscordBot) return;

	SleepyDiscord::Embed embed;
	embed.title = std::string(Title);
	embed.color = string_to_number(Color, 0, 1410065407);

	char aBuf[128];
	str_format(aBuf, sizeof(aBuf), "%s/%s/gentee.php%s", g_Config.m_SvSiteUrl, g_Config.m_SvGenerateURL, pPhpArg);

	SleepyDiscord::EmbedImage embedimage;
	embedimage.height = 800;
	embedimage.width = 600;
	embedimage.url = std::string(aBuf);
	embedimage.proxyUrl = std::string(aBuf);
	embed.image = embedimage;
	this->sendMessage(pChanal, "\0", embed);
}

void DiscordJob::SendMessage(const char *pChanal, const char *Color, const char *Title, std::string pMsg)
{
	if(!g_Config.m_SvCreateDiscordBot) return;

	SleepyDiscord::Embed embed;
	embed.title = std::string(Title);
	embed.color = string_to_number(Color, 0, 1410065407);
	embed.description = pMsg;
	this->sendMessage(pChanal, "\0", embed);
}
#endif

int main(int argc, const char **argv) // ignore_convention
{

#if defined(CONF_FAMILY_WINDOWS)
	for(int i = 1; i < argc; i++) // ignore_convention
	{
		if(str_comp("-s", argv[i]) == 0 || str_comp("--silent", argv[i]) == 0) // ignore_convention
		{
			ShowWindow(GetConsoleWindow(), SW_HIDE);
			break;
		}
	}
#endif

	bool UseDefaultConfig = false;
	for(int i = 1; i < argc; i++) // ignore_convention
	{
		if(str_comp("-d", argv[i]) == 0 || str_comp("--default", argv[i]) == 0) // ignore_convention
		{
			UseDefaultConfig = true;
			break;
		}
	}

	bool SkipPWGen = false;
	if(secure_random_init() != 0)
	{
		dbg_msg("secure", "could not initialize secure RNG");
		SkipPWGen = true;	// skip automatic password generation
	}

	CServer *pServer = CreateServer();
	IKernel *pKernel = IKernel::Create();

	// create the components
	int FlagMask = CFGFLAG_SERVER|CFGFLAG_ECON;
	IEngine *pEngine = CreateEngine("Teeworlds_Server", false, 1);
	IConsole *pConsole = CreateConsole(CFGFLAG_SERVER|CFGFLAG_ECON);
	IEngineMasterServer *pEngineMasterServer = CreateEngineMasterServer();
	IStorage *pStorage = CreateStorage("Teeworlds", IStorage::STORAGETYPE_SERVER, argc, argv); // ignore_convention
	IConfig *pConfig = CreateConfig();
	IEngineMap *pEngineMap[COUNT_WORLD];
	IGameServer *pGameServer[COUNT_WORLD];
	pServer->InitRegister(&pServer->m_NetServer, pEngineMasterServer, pConsole);

	{
		for(int i = 0 ; i < COUNT_WORLD ; i ++)
		{
			pEngineMap[i] = CreateEngineMap();
			pGameServer[i] = CreateGameServer();

			bool RegisterFail = false;
			if(i == 0)
			{
				RegisterFail = RegisterFail || !pKernel->RegisterInterface(pServer); // register as both
				RegisterFail = RegisterFail || !pKernel->RegisterInterface(pEngine);
			}
			RegisterFail = RegisterFail || !pKernel->RegisterInterface(static_cast<IEngineMap*>(pEngineMap[i]), i); // register as both
			RegisterFail = RegisterFail || !pKernel->RegisterInterface(static_cast<IMap*>(pEngineMap[i]), i);
			RegisterFail = RegisterFail || !pKernel->RegisterInterface(pGameServer[i], i);
			if(i == COUNT_WORLD-1)
			{
				RegisterFail = RegisterFail || !pKernel->RegisterInterface(pConsole);
				RegisterFail = RegisterFail || !pKernel->RegisterInterface(pStorage);
				RegisterFail = RegisterFail || !pKernel->RegisterInterface(pConfig);
				RegisterFail = RegisterFail || !pKernel->RegisterInterface(static_cast<IEngineMasterServer*>(pEngineMasterServer)); // register as both
				RegisterFail = RegisterFail || !pKernel->RegisterInterface(static_cast<IMasterServer*>(pEngineMasterServer));
			}

			if(RegisterFail)
				return -1;
		}
	}

	pServer->m_pLocalization = new CLocalization(pStorage);
	if(!pServer->m_pLocalization->Init())
	{
		dbg_msg("localization", "could not initialize localization");
		return -1;
	}

	pEngine->Init();
	pConfig->Init(FlagMask);
	pEngineMasterServer->Init();
	pEngineMasterServer->Load();

	if(!UseDefaultConfig)
	{
		// register all console commands
		pServer->RegisterCommands();

		// execute autoexec file
		pConsole->ExecuteFile("autoexec.cfg");

		// parse the command line arguments
		if(argc > 1) // ignore_convention
			pConsole->ParseArguments(argc-1, &argv[1]); // ignore_convention
	}

	// restore empty config strings to their defaults
	pConfig->RestoreStrings();

	pEngine->InitLogfile();

	if(!SkipPWGen)
		pServer->InitRconPasswordIfUnset();

	// run the server
	#ifdef CONF_DISCORD
		pServer->m_pDiscord = new DiscordJob(g_Config.m_SvDiscordToken, 3);
		pServer->m_pDiscord->SetServer(pServer);
	#endif

	dbg_msg("server", "starting...");
	pServer->Run();

	delete pServer->m_pLocalization;

	// free
	#ifdef CONF_DISCORD
		delete pServer->m_pDiscord;
	#endif

	delete pServer;
	delete pKernel;
	delete pEngine;
	for(int i = 0 ; i < COUNT_WORLD; i++)
	{
		delete pEngineMap[i];
		delete pGameServer[i];
	}
	delete pConsole;
	delete pEngineMasterServer;
	delete pStorage;
	delete pConfig;

	return 0;
}