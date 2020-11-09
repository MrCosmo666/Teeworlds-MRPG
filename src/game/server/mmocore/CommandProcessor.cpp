#include <engine/server.h>
#include <engine/shared/config.h>

#include <teeother/components/localization.h>
#include "game/server/gamecontext.h"
#include "CommandProcessor.h"

/*
	Later lead to quality
*/

CCommandProcessor::CCommandProcessor(CGS *pGS)
{
	m_pGS = pGS;

	IServer* pServer = m_pGS->Server();
	AddCommand("login", "?s[username] ?s[password]", ConChatLogin, pServer, "");
	AddCommand("register", "?s[username] ?s[password]", ConChatRegister, pServer, "");
	AddCommand("gexit", "", ConChatGuildExit, pServer, "");
	AddCommand("gcreate", "?s[guildname]", ConChatGuildCreate, pServer, "");
	AddCommand("doorhouse", "", ConChatDoorHouse, pServer, "");
	AddCommand("sellhouse", "", ConChatSellHouse, pServer, "");
	AddCommand("pos", "", ConChatPosition, pServer, "");
	AddCommand("sd", "?i[sound]", ConChatSound, pServer, "");
	AddCommand("useitem", "?i[item]", ConChatUseItem, pServer, "");
	AddCommand("useskill", "?i[skill]", ConChatUseSkill, pServer, "");
	AddCommand("cmdlist", "", ConChatCmdList, pServer, "");
	AddCommand("help", "", ConChatCmdList, pServer, "");
	AddCommand("rules", "", ConChatRules, pServer, "");

#ifdef CONF_DISCORD
	AddCommand("discord_connect", "?s[DID]", ConChatDiscordConnect, pServer, "");
#endif
}

void CCommandProcessor::AddCommand(const char* pName, const char* pParams, IConsole::FCommandCallback pfnFunc, void* pUser, const char* pHelp)
{
	GS()->Console()->Register(pName, pParams, CFGFLAG_CHAT, pfnFunc, pUser, pHelp);

	if (!GS()->CommandManager()->AddCommand(pName, pHelp, pParams, pfnFunc, pUser))
	{
		if (pParams[0])
			dbg_msg("CCommandProcessor", "registered chat command: '/%s %s'", pName, pParams);
		else
			dbg_msg("CCommandProcessor", "registered chat command: '/%s'", pName);
	}
	else
		dbg_msg("CCommandProcessor", "failed to add command: '/%s'", pName);
}

void CCommandProcessor::ChatCmd(CNetMsg_Cl_Say* pMsg, CPlayer* pPlayer)
{
	LastChat(GS(), pPlayer);
	const int ClientID = pPlayer->GetCID();

	char aCommand[256][256] = { {0} };
	int Command = 0;
	int Char = 0;
	for (int i = 1; i < str_length(pMsg->m_pMessage); i++)
	{
		if (pMsg->m_pMessage[i] == ' ')
		{
			Command++;
			Char = 0;
			continue;
		}
		aCommand[Command][Char] = pMsg->m_pMessage[i];
		Char++;
	}

	if (GS()->Console()->IsCommand(aCommand[0], CFGFLAG_CHAT))
	{
		GS()->Console()->ExecuteLineFlag(pMsg->m_pMessage + 1, CFGFLAG_CHAT, ClientID, false);
		return;
	}

	GS()->ChatFollow(ClientID, "Command {STR} not found!", pMsg->m_pMessage);
}

void CCommandProcessor::ConChatLogin(IConsole::IResult* pResult, void* pUser)
{
	int ClientID = pResult->GetClientID();
	IServer* pServer = (IServer*)pUser;
	CGS* pGS = (CGS*)pServer->GameServer(pServer->GetClientWorldID(ClientID));

	CPlayer* pPlayer = pGS->m_apPlayers[ClientID];
	if (!pPlayer)
		return;

	if (pPlayer->IsAuthed())
	{
		pGS->Chat(ClientID, "You're already signed in.");
		return;
	}

	if (pResult->NumArguments() != 2)
	{
		pGS->Chat(ClientID, "Use: /login <username> <password>");
		return;
	}

	char aUsername[16];
	char aPassword[16];
	str_copy(aUsername, pResult->GetString(0), sizeof(aUsername));
	str_copy(aPassword, pResult->GetString(1), sizeof(aPassword));

	if (pGS->Mmo()->Account()->LoginAccount(ClientID, aUsername, aPassword) == AUTH_LOGIN_GOOD)
		pGS->Mmo()->Account()->LoadAccount(pPlayer, true);
}

void CCommandProcessor::ConChatRegister(IConsole::IResult* pResult, void* pUser)
{
	int ClientID = pResult->GetClientID();
	IServer* pServer = (IServer*)pUser;
	CGS* pGS = (CGS*)pServer->GameServer(pServer->GetClientWorldID(ClientID));

	CPlayer* pPlayer = pGS->m_apPlayers[ClientID];
	if (!pPlayer)
		return;

	if (pPlayer->IsAuthed())
	{
		pGS->Chat(ClientID, "Sign out first before you create a new account.");
		return;
	}

	if (pResult->NumArguments() != 2)
	{
		pGS->Chat(ClientID, "Use: /register <username> <password>");
		return;
	}

	char aUsername[16];
	char aPassword[16];
	str_copy(aUsername, pResult->GetString(0), sizeof(aUsername));
	str_copy(aPassword, pResult->GetString(1), sizeof(aPassword));

	pGS->Mmo()->Account()->RegisterAccount(ClientID, aUsername, aPassword);
}

#ifdef CONF_DISCORD
void CCommandProcessor::ConChatDiscordConnect(IConsole::IResult* pResult, void* pUser)
{
	int ClientID = pResult->GetClientID();
	IServer* pServer = (IServer*)pUser;
	CGS* pGS = (CGS*)pServer->GameServer(pServer->GetClientWorldID(ClientID));

	CPlayer* pPlayer = pGS->m_apPlayers[ClientID];
	if (!pPlayer || !pPlayer->IsAuthed())
		return;

	if (pResult->NumArguments() != 1)
	{
		pGS->Chat(ClientID, "Use: /discord_connect <DID>");
		return;
	}

	char aDiscordDID[32];
	str_copy(aDiscordDID, pResult->GetString(0), sizeof(aDiscordDID));

	if (str_length(aDiscordDID) > 30 || str_length(aDiscordDID) < 10)
	{
		pGS->ChatFollow(ClientID, "Discord ID must contain 10-30 characters.");
		return;
	}

	pGS->Mmo()->Account()->DiscordConnect(ClientID, aDiscordDID);
}
#endif

void CCommandProcessor::ConChatGuildExit(IConsole::IResult* pResult, void* pUser)
{
	int ClientID = pResult->GetClientID();
	IServer* pServer = (IServer*)pUser;
	CGS* pGS = (CGS*)pServer->GameServer(pServer->GetClientWorldID(ClientID));

	CPlayer* pPlayer = pGS->m_apPlayers[ClientID];
	if (!pPlayer || !pPlayer->IsAuthed() || !pPlayer->Acc().IsGuild())
		return;

	int AuthID = pPlayer->Acc().m_AuthID;
	pGS->Mmo()->Member()->ExitGuild(AuthID);
}

void CCommandProcessor::ConChatGuildCreate(IConsole::IResult* pResult, void* pUser)
{
	int ClientID = pResult->GetClientID();
	IServer* pServer = (IServer*)pUser;
	CGS* pGS = (CGS*)pServer->GameServer(pServer->GetClientWorldID(ClientID));

	CPlayer* pPlayer = pGS->m_apPlayers[ClientID];
	if (!pPlayer || !pPlayer->IsAuthed() || pPlayer->Acc().IsGuild())
		return;

	if (pResult->NumArguments() != 1)
	{
		pGS->Chat(ClientID, "Use: /gcreate <guildname>");
		return;
	}

	char aGuildName[16];
	str_copy(aGuildName, pResult->GetString(0), sizeof(aGuildName));
	if (str_length(aGuildName) > 8 || str_length(aGuildName) < 3)
	{
		pGS->ChatFollow(ClientID, "Guild name must contain 3-8 characters");
		return;
	}

	pGS->Mmo()->Member()->CreateGuild(pPlayer, aGuildName);
}

void CCommandProcessor::ConChatDoorHouse(IConsole::IResult* pResult, void* pUser)
{
	int ClientID = pResult->GetClientID();
	IServer* pServer = (IServer*)pUser;
	CGS* pGS = (CGS*)pServer->GameServer(pServer->GetClientWorldID(ClientID));

	CPlayer* pPlayer = pGS->m_apPlayers[ClientID];
	if (!pPlayer || !pPlayer->IsAuthed())
		return;

	int HouseID = pGS->Mmo()->House()->PlayerHouseID(pPlayer);
	pGS->Mmo()->House()->ChangeStateDoor(HouseID);
}

void CCommandProcessor::ConChatSellHouse(IConsole::IResult* pResult, void* pUser)
{
	int ClientID = pResult->GetClientID();
	IServer* pServer = (IServer*)pUser;
	CGS* pGS = (CGS*)pServer->GameServer(pServer->GetClientWorldID(ClientID));

	CPlayer* pPlayer = pGS->m_apPlayers[ClientID];
	if (!pPlayer || !pPlayer->IsAuthed())
		return;

	// check owner house id
	int HouseID = pGS->Mmo()->House()->PlayerHouseID(pPlayer);
	if(HouseID < 0)
	{
		pGS->Chat(ClientID, "You have no home.");
		return;
	}
	
	// sell house
	pGS->Mmo()->House()->SellHouse(HouseID);
}

void CCommandProcessor::ConChatPosition(IConsole::IResult* pResult, void* pUser)
{
	int ClientID = pResult->GetClientID();
	IServer* pServer = (IServer*)pUser;
	CGS* pGS = (CGS*)pServer->GameServer(pServer->GetClientWorldID(ClientID));

	CPlayer* pPlayer = pGS->m_apPlayers[ClientID];
	if (!pPlayer || !pPlayer->GetCharacter() || !pGS->Server()->IsAuthed(ClientID))
		return;

	const int PosX = pPlayer->GetCharacter()->m_Core.m_Pos.x / 32;
	const int PosY = pPlayer->GetCharacter()->m_Core.m_Pos.y/32;
	pGS->Chat(ClientID, "[{STR}] Position X: {INT} Y: {INT}.", pGS->Server()->GetWorldName(pGS->GetWorldID()), &PosX, &PosY);
	dbg_msg("test", "%0.f %0.f WorldID: %d", pPlayer->GetCharacter()->m_Core.m_Pos.x, pPlayer->GetCharacter()->m_Core.m_Pos.y, pGS->GetWorldID());
}

void CCommandProcessor::ConChatSound(IConsole::IResult* pResult, void* pUser)
{
	int ClientID = pResult->GetClientID();
	IServer* pServer = (IServer*)pUser;
	CGS* pGS = (CGS*)pServer->GameServer(pServer->GetClientWorldID(ClientID));

	CPlayer* pPlayer = pGS->m_apPlayers[ClientID];
	if (!pPlayer || !pPlayer->GetCharacter() || !pGS->Server()->IsAuthed(ClientID))
		return;

	if (pResult->NumArguments() != 1)
	{
		pGS->ChatFollow(ClientID, "Please use: /sd <idsound>");
		return;
	}

	const int SoundID = clamp(pResult->GetInteger(0), 0, 40);
	pGS->CreateSound(pPlayer->GetCharacter()->m_Core.m_Pos, SoundID);
}

void CCommandProcessor::ConChatUseItem(IConsole::IResult* pResult, void* pUser)
{
	int ClientID = pResult->GetClientID();
	IServer* pServer = (IServer*)pUser;
	CGS* pGS = (CGS*)pServer->GameServer(pServer->GetClientWorldID(ClientID));

	CPlayer* pPlayer = pGS->m_apPlayers[ClientID];
	if (!pPlayer || !pPlayer->IsAuthed())
		return;

	if (pResult->NumArguments() != 1)
	{
		pGS->ChatFollow(ClientID, "Please use: /useitem <itemid>");
		return;
	}

	int ItemID = pResult->GetInteger(0);
	pPlayer->GetItem(ItemID).Use(1);
}

void CCommandProcessor::ConChatUseSkill(IConsole::IResult* pResult, void* pUser)
{
	int ClientID = pResult->GetClientID();
	IServer* pServer = (IServer*)pUser;
	CGS* pGS = (CGS*)pServer->GameServer(pServer->GetClientWorldID(ClientID));

	CPlayer* pPlayer = pGS->m_apPlayers[ClientID];
	if (!pPlayer || !pPlayer->IsAuthed())
		return;

	if (pResult->NumArguments() != 1)
	{
		pGS->ChatFollow(ClientID, "Please use: /useskill <skillid>");
		return;
	}

	int SkillID = pResult->GetInteger(0);
	pPlayer->GetSkill(SkillID).Use();
}

void CCommandProcessor::ConChatCmdList(IConsole::IResult* pResult, void* pUser)
{
	int ClientID = pResult->GetClientID();
	IServer* pServer = (IServer*)pUser;
	CGS* pGS = (CGS*)pServer->GameServer(pServer->GetClientWorldID(ClientID));

	CPlayer* pPlayer = pGS->m_apPlayers[ClientID];
	if (!pPlayer)
		return;

	pGS->ChatFollow(ClientID, "Command List / Help");
	pGS->ChatFollow(ClientID, "/register <name> <pass> - new account.");
	pGS->ChatFollow(ClientID, "/login <name> <pass> - log in account.");
	pGS->ChatFollow(ClientID, "/rules - server rules.");
	pGS->ChatFollow(ClientID, "Another information see Wiki Page.");
}

void CCommandProcessor::ConChatRules(IConsole::IResult* pResult, void* pUser)
{
	int ClientID = pResult->GetClientID();
	IServer* pServer = (IServer*)pUser;
	CGS* pGS = (CGS*)pServer->GameServer(pServer->GetClientWorldID(ClientID));

	CPlayer* pPlayer = pGS->m_apPlayers[ClientID];
	if (!pPlayer)
		return;

	pGS->ChatFollow(ClientID, "Server rules");
	pGS->ChatFollow(ClientID, "- Don't abuse bugs");
	pGS->ChatFollow(ClientID, "- Don't use bots or other external software which give you unfair advantages");
	pGS->ChatFollow(ClientID, "- Don't use multiple accounts");
	pGS->ChatFollow(ClientID, "- Don't share your account credentials (username, password)");
	pGS->ChatFollow(ClientID, "- Do not use ads, that is not part of the game");
}

void CCommandProcessor::LastChat(CGS *pGS, CPlayer *pPlayer)
{
	if(pPlayer->m_aPlayerTick[TickState::LastChat] + pGS->Server()->TickSpeed() <= pGS->Server()->Tick())
		pPlayer->m_aPlayerTick[TickState::LastChat] = pGS->Server()->Tick();
}