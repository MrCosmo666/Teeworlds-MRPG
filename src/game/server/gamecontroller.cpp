/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#include <engine/shared/config.h>
#include <game/mapitems.h>
#include <game/version.h>

#include "entities/pickup.h"
#include "gamecontext.h"

#include "gamecontroller.h"

IGameController::IGameController(CGS *pGS)
{
	m_pGS = pGS;
	m_pServer = m_pGS->Server();

	// info
	m_pGameType = "unknown";

	// spawn
	m_aNumSpawnPoints[0] = 0;
	m_aNumSpawnPoints[1] = 0;
	m_aNumSpawnPoints[2] = 0;

	OnInitCommands();
}

void IGameController::OnCharacterDeath(CCharacter *pVictim, CPlayer *pKiller, int Weapon)
{
	pVictim->GetPlayer()->ClearTalking();
	return;
}

void IGameController::OnCharacterSpawn(CCharacter *pChr)
{
	pChr->GetPlayer()->ClearTalking();

	// если спавним бота
	if(pChr->GetPlayer()->IsBot())
	{
		pChr->IncreaseHealth(pChr->GetPlayer()->GetStartHealth());
		pChr->GiveWeapon(WEAPON_HAMMER, -1);
		for(int i = 1; i < WEAPON_LASER+1; i++)
			pChr->GiveWeapon(i, 10);

		return;
	}

	// миркатсцены начало игры
	if(GS()->GetWorldID() == LOCALWORLD && pChr->GetPlayer()->GetItem(itTitleNewHero).Count <= 0)
		pChr->GetPlayer()->GetItem(itTitleNewHero).Add(1);


	// если спавним игрока
	int StartHealth = pChr->GetPlayer()->Acc().PlayerHealth > 0 ? pChr->GetPlayer()->Acc().PlayerHealth : pChr->GetPlayer()->GetStartHealth();
	pChr->IncreaseHealth(StartHealth);

	// оружие и здоровье
	int StartAmmo = 10 + pChr->GetPlayer()->GetAttributeCount(Stats::StAmmo);
	pChr->GiveWeapon(WEAPON_HAMMER, -1);
	for(int i = 1; i < WEAPON_LASER+1; i++)
		pChr->GiveWeapon(i, StartAmmo);
}

bool IGameController::OnEntity(int Index, vec2 Pos)
{
	int Type = -1;

	switch(Index)
	{
	case ENTITY_SPAWN:
		m_aaSpawnPoints[0][m_aNumSpawnPoints[0]++] = Pos;
		break;
	case ENTITY_SPAWN_MOBS:
		m_aaSpawnPoints[1][m_aNumSpawnPoints[1]++] = Pos;
		break;
	case ENTITY_SPAWN_BLUE:
		m_aaSpawnPoints[2][m_aNumSpawnPoints[2]++] = Pos;
		break;
	case ENTITY_ARMOR_1:
		Type = PICKUP_ARMOR;
		break;
	case ENTITY_HEALTH_1:
		Type = PICKUP_HEALTH;
		break;
	}
	if(Type != -1)
	{
		new CPickup(&GS()->m_World, Type, Pos);
		return true;
	}
	return false;
}

void IGameController::OnPlayerConnect(CPlayer *pPlayer)
{
	int ClientID = pPlayer->GetCID();

	char aBuf[128];
	str_format(aBuf, sizeof(aBuf), "team_join player='%d:%s' team=%d", ClientID, Server()->ClientName(ClientID), pPlayer->GetTeam());
	GS()->Console()->Print(IConsole::OUTPUT_LEVEL_DEBUG, "game", aBuf);

	// update game info
	UpdateGameInfo(ClientID);

	CommandsManager()->SendRemoveCommand(Server(), "all", pPlayer->GetCID());
	CommandsManager()->OnPlayerConnect(Server(), pPlayer);
}

void IGameController::OnPlayerDisconnect(CPlayer *pPlayer)
{
	// обновить позицию игрока
	GS()->Mmo()->SaveAccount(pPlayer, SAVEPOSITION);

	pPlayer->OnDisconnect();

	int ClientID = pPlayer->GetCID();
	if(Server()->ClientIngame(ClientID))
	{
		char aBuf[128];
		str_format(aBuf, sizeof(aBuf), "leave player='%d:%s'", ClientID, Server()->ClientName(ClientID));
		GS()->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "game", aBuf);
	}
}

void IGameController::OnPlayerInfoChange(CPlayer *pPlayer, int WorldID)
{

}

void IGameController::OnReset()
{
	for(int i = 0; i < MAX_CLIENTS; i++)
	{
		if(GS()->m_apPlayers[i])
			GS()->m_apPlayers[i]->m_PlayerTick[TickState::Respawn] = Server()->Tick()+Server()->TickSpeed()/2;
	}
}

// general
void IGameController::Snap(int SnappingClient)
{
	CNetObj_GameData *pGameData = static_cast<CNetObj_GameData *>(Server()->SnapNewItem(NETOBJTYPE_GAMEDATA, 0, sizeof(CNetObj_GameData)));
	if(!pGameData)
		return;

	pGameData->m_GameStartTick = 0;
	pGameData->m_GameStateFlags = 0;
	pGameData->m_GameStateEndTick = 0;

	// demo recording
	if(SnappingClient == -1)
	{
		CNetObj_De_GameInfo *pGameInfo = static_cast<CNetObj_De_GameInfo *>(Server()->SnapNewItem(NETOBJTYPE_DE_GAMEINFO, 0, sizeof(CNetObj_De_GameInfo)));
		if(!pGameInfo)
			return;

		pGameInfo->m_GameFlags = 0;
		pGameInfo->m_ScoreLimit = /*m_GameInfo.m_ScoreLimit*/ 0;
		pGameInfo->m_TimeLimit = /*m_GameInfo.m_TimeLimit*/ 0;
		pGameInfo->m_MatchNum = /*m_GameInfo.m_MatchNum*/ 0;
		pGameInfo->m_MatchCurrent = /*m_GameInfo.m_MatchCurrent*/ 0;
	}
}

void IGameController::Tick() { }

void IGameController::UpdateGameInfo(int ClientID)
{
	CNetMsg_Sv_GameInfo GameInfoMsg;
	GameInfoMsg.m_GameFlags = m_GameFlags;
	GameInfoMsg.m_ScoreLimit = 0;
	GameInfoMsg.m_TimeLimit = 0;
	GameInfoMsg.m_MatchNum = 0;
	GameInfoMsg.m_MatchCurrent = 0;

	if(ClientID == -1)
	{
		for(int i = 0; i < MAX_PLAYERS; ++i)
		{
			if(!GS()->m_apPlayers[i] || !Server()->ClientIngame(i))
				continue;

			if((GS()->CheckClient(i) && Server()->GetClientVersion(i) < CLIENT_VERSION_MMO) ||
					(!GS()->CheckClient(i) && Server()->GetClientVersion(i) < MIN_RACE_CLIENTVERSION))
				GameInfoMsg.m_GameFlags &= ~GAMEFLAG_RACE;

			Server()->SendPackMsg(&GameInfoMsg, MSGFLAG_VITAL|MSGFLAG_NORECORD, i, Server()->GetWorldID(ClientID));
		}
	}
	else
	{
		if((GS()->CheckClient(ClientID) && Server()->GetClientVersion(ClientID) < CLIENT_VERSION_MMO) ||
				(!GS()->CheckClient(ClientID) && Server()->GetClientVersion(ClientID) < MIN_RACE_CLIENTVERSION))
			GameInfoMsg.m_GameFlags &= ~GAMEFLAG_RACE;

		Server()->SendPackMsg(&GameInfoMsg, MSGFLAG_VITAL|MSGFLAG_NORECORD, ClientID, Server()->GetWorldID(ClientID));
	}
}

// spawn
bool IGameController::CanSpawn(int Team, vec2 *pOutPos, vec2 BotPos) const
{
	// spectators can't spawn
	if(Team == TEAM_SPECTATORS || GS()->m_World.m_ResetRequested)
		return false;

	CSpawnEval Eval;
	EvaluateSpawnType(&Eval, Team, BotPos);

	*pOutPos = Eval.m_Pos;
	return Eval.m_Got;
}

float IGameController::EvaluateSpawnPos(CSpawnEval *pEval, vec2 Pos) const
{
	float Score = 0.0f;
	CCharacter *pC = static_cast<CCharacter *>(GS()->m_World.FindFirst(CGameWorld::ENTTYPE_CHARACTER));
	for(; pC; pC = (CCharacter *)pC->TypeNext())
	{
		// team mates are not as dangerous as enemies
		float Scoremod = 1.0f;
		if(pEval->m_FriendlyTeam != -1 && pC->GetPlayer()->GetTeam() == pEval->m_FriendlyTeam)
			Scoremod = 0.5f;

		float d = distance(Pos, pC->GetPos());
		Score += Scoremod * (d == 0 ? 1000000000.0f : 1.0f/d);
	}

	return Score;
}

void IGameController::EvaluateSpawnType(CSpawnEval *pEval, int Type, vec2 BotPos) const
{
	// get spawn point
	for(int i = 0; i < m_aNumSpawnPoints[Type]; i++)
	{
		// check if the position is occupado
		CCharacter *aEnts[MAX_CLIENTS];
		int Num = GS()->m_World.FindEntities(m_aaSpawnPoints[Type][i], 64, (CEntity**)aEnts, MAX_CLIENTS, CGameWorld::ENTTYPE_CHARACTER);
		vec2 Positions[5] = { vec2(0.0f, 0.0f), vec2(-32.0f, 0.0f), vec2(0.0f, -32.0f), vec2(32.0f, 0.0f), vec2(0.0f, 32.0f) };	// start, left, up, right, down
		int Result = -1;
	
		if(BotPos != vec2(-1, -1) && distance(BotPos, m_aaSpawnPoints[Type][i]) > 800.0f)
			continue;

		for(int Index = 0; Index < 5 && Result == -1; ++Index)
		{
			Result = Index;
			for(int c = 0; c < Num; ++c)
			{
				if( 
					GS()->Collision()->CheckPoint(m_aaSpawnPoints[Type][i]+Positions[Index]) ||
					distance(aEnts[c]->GetPos(), m_aaSpawnPoints[Type][i]+Positions[Index]) <= aEnts[c]->GetProximityRadius())
				{
					Result = -1;
					break;
				}
			}
		}
		if(Result == -1)
			continue;	// try next spawn point

		vec2 P = m_aaSpawnPoints[Type][i]+Positions[Result];
		float S = EvaluateSpawnPos(pEval, P);
		if(!pEval->m_Got || pEval->m_Score > S)
		{
			pEval->m_Got = true;
			pEval->m_Score = S;
			pEval->m_Pos = P;
		}
	}
}

void IGameController::DoTeamChange(CPlayer *pPlayer, bool DoChatMsg)
{
	int ClientID = pPlayer->GetCID();
	{
		// проверяем равняется ли команда той на которуюю меняем
		int Team = pPlayer->GetStartTeam();
		if(Team == pPlayer->GetTeam())
			return;

		// получаем старую команду и меняем ее игроку
		pPlayer->SetTeam(Team);

		// информируем о смене команды
		GS()->SendTeam(ClientID, Team, DoChatMsg, -1);

		char aBuf[128];
		str_format(aBuf, sizeof(aBuf), "team_join player='%d:%s' m_Team=%d", ClientID, Server()->ClientName(ClientID), Team);
		GS()->Console()->Print(IConsole::OUTPUT_LEVEL_DEBUG, "game", aBuf);
	}
	OnPlayerInfoChange(pPlayer, GS()->GetWorldID());
}

int IGameController::GetRealPlayer()
{
	int NumPlayers = 0;
	for(int i = 0; i < MAX_PLAYERS; i++)
	{
		if(GS()->m_apPlayers[i] && GS()->m_apPlayers[i]->GetTeam() != TEAM_SPECTATORS)
			NumPlayers++;
	}
	return NumPlayers;
}

IGameController::CChatCommands::CChatCommands()
{
	mem_zero(m_aCommands, sizeof(m_aCommands));
}

void IGameController::CChatCommands::AddCommand(const char *pName, const char *pArgsFormat, const char *pHelpText, COMMAND_CALLBACK pfnCallback)
{
	if(GetCommand(pName))
		return;

	for(int i = 0; i < MAX_COMMANDS; i++)
	{
		if(!m_aCommands[i].m_Used)
		{
			mem_zero(&m_aCommands[i], sizeof(CChatCommand));

			str_copy(m_aCommands[i].m_aName, pName, sizeof(m_aCommands[i].m_aName));
			str_copy(m_aCommands[i].m_aHelpText, pHelpText, sizeof(m_aCommands[i].m_aHelpText));
			str_copy(m_aCommands[i].m_aArgsFormat, pArgsFormat, sizeof(m_aCommands[i].m_aArgsFormat));

			m_aCommands[i].m_pfnCallback = pfnCallback;
			m_aCommands[i].m_Used = true;
			break;
		}
	}
}

void IGameController::CChatCommands::SendRemoveCommand(IServer *pServer, const char *pName, int ClientID)
{
	CNetMsg_Sv_CommandInfoRemove Msg;
	Msg.m_pName = pName;

	pServer->SendPackMsg(&Msg, MSGFLAG_VITAL, ClientID, pServer->GetWorldID(ClientID));
}

void IGameController::CChatCommands::RemoveCommand(const char *pName)
{
	CChatCommand *pCommand = GetCommand(pName);

	if(pCommand)
	{
		mem_zero(pCommand, sizeof(CChatCommand));
	}
}

IGameController::CChatCommand *IGameController::CChatCommands::GetCommand(const char *pName)
{
	for(int i = 0; i < MAX_COMMANDS; i++)
	{
		if(m_aCommands[i].m_Used && str_comp(m_aCommands[i].m_aName, pName) == 0)
		{
			return &m_aCommands[i];
		}
	}
	return 0;
}

void IGameController::CChatCommands::OnPlayerConnect(IServer *pServer, CPlayer *pPlayer)
{
	for(int i = 0; i < MAX_COMMANDS; i++)
	{
		CChatCommand *pCommand = &m_aCommands[i];

		if(pCommand->m_Used)
		{
			CNetMsg_Sv_CommandInfo Msg;
			Msg.m_pName = pCommand->m_aName;
			Msg.m_HelpText = pCommand->m_aHelpText;
			Msg.m_ArgsFormat = pCommand->m_aArgsFormat;

			pServer->SendPackMsg(&Msg, MSGFLAG_VITAL, pPlayer->GetCID());
		}
	}
}

void IGameController::OnPlayerCommand(CPlayer *pPlayer, const char *pCommandName, const char *pCommandArgs)
{
	// TODO: Add a argument parser?
	CChatCommand *pCommand = CommandsManager()->GetCommand(pCommandName);

	if(pCommand)
		pCommand->m_pfnCallback(pPlayer, pCommandArgs);
}

void IGameController::OnInitCommands()
{
	// авторизация
	CommandsManager()->AddCommand("login", "ss", "Log in use example: / login name pass", [](CPlayer *pPlayer, const char *Args) 
	{
		CGS *GS = pPlayer->GS();
		int ClientID = pPlayer->GetCID();

		// проверяем авторизацию и проверку клиента
		if(pPlayer->IsAuthed()) 
			return GS->Chat(ClientID, "You already authed.");
		
		if(pPlayer->m_PlayerTick[TickState::CheckClient] && 
				pPlayer->m_PlayerTick[TickState::CheckClient]+GS->Server()->TickSpeed()*5 > GS->Server()->Tick())	
			return GS->Chat(ClientID, "Please wait your client check repeat after 1-4 sec!");
		
		// если аргументы не совпадают
		char Username[256], Password[256];
		if(sscanf(Args, "/login %s %s", Username, Password) != 2)  
			return GS->ChatFollow(ClientID, "Use: /login <username> <password>");
		
		// если размер пароля логина мал или слишком большой
		if(str_length(Username) > 15 || str_length(Username) < 4 || str_length(Password) > 15 || str_length(Password) < 4)
			return GS->Chat(ClientID, "Username / Password must contain 4-15 characters");

		// загружаем аккаунт и данные если он загрузился
		if(GS->Mmo()->Account()->LoginAccount(ClientID, Username, Password) == AUTH_ALL_GOOD)
			GS->Mmo()->Account()->LoadAccount(pPlayer, true);
	});

	// регистрация
	CommandsManager()->AddCommand("register", "ss", "Register new account", [](CPlayer *pPlayer, const char *Args)
	{
		CGS *GS = pPlayer->GS();
		int ClientID = pPlayer->GetCID();

		// проверяем авторизацию и проверку клиента
		if(pPlayer->IsAuthed()) 
			return GS->Chat(ClientID, "Logout account and create!");
		
		if(pPlayer->m_PlayerTick[TickState::CheckClient] && 
				pPlayer->m_PlayerTick[TickState::CheckClient]+GS->Server()->TickSpeed()*5 > GS->Server()->Tick())	
			return GS->Chat(ClientID, "Please wait your client check repeat after 1-4 sec!");

		// если аргументы не совпадают
		char Username[256], Password[256];
		if(sscanf(Args, "/register %s %s", Username, Password) != 2) 
			return GS->ChatFollow(ClientID, "Use: /register <username> <password>");
	
		// регестрируем аккаунт
		GS->Mmo()->Account()->RegisterAccount(ClientID, Username, Password);
	});

#ifdef CONF_DISCORD
	CommandsManager()->AddCommand("discord_connect", "s", "Connect discord to account", [](CPlayer *pPlayer, const char *Args) 
	{
		// check authed
		CGS *GS = pPlayer->GS();
		int ClientID = pPlayer->GetCID();

		if(!pPlayer->IsAuthed())
			return;
		
		char DiscordDID[256];
		if(sscanf(Args, "/discord_connect %s", DiscordDID) != 1) 
			return GS->ChatFollow(ClientID, "Use: /discord_connect <DID>");
		if(str_length(DiscordDID) > 30 || str_length(DiscordDID) < 10)
			return GS->ChatFollow(ClientID, "Discord ID must contain 10-30 characters.");

		GS->Mmo()->Account()->DiscordConnect(ClientID, DiscordDID);
	});
#endif

	// список команд
	CommandsManager()->AddCommand("cmdlist", "", "Command list", [](CPlayer *pPlayer, const char *Args) 
	{
		CGS *GS = pPlayer->GS();
		int ClientID = pPlayer->GetCID();
		GS->ChatFollow(ClientID, "Command List / Help");
		GS->ChatFollow(ClientID, "/register <name> <pass> - new account.");
		GS->ChatFollow(ClientID, "/login <name> <pass> - log in account.");
		GS->ChatFollow(ClientID, "Another information see Wiki Page.");
	});
}