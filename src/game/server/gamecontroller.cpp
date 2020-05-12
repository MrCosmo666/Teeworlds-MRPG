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
}

void IGameController::OnCharacterDeath(CCharacter *pVictim, CPlayer *pKiller, int Weapon)
{
	return;
}

bool IGameController::OnCharacterSpawn(CCharacter* pChr)
{
	// если спавним бота
	if(pChr->GetPlayer()->IsBot())
	{
		pChr->IncreaseHealth(pChr->GetPlayer()->GetStartHealth());
		pChr->GiveWeapon(WEAPON_HAMMER, -1);
		for(int i = WEAPON_GUN; i < NUM_WEAPONS; i++)
			pChr->GiveWeapon(i, 10);
		return true;
	}

	// если спавним игрока
	int StartHealth = pChr->GetPlayer()->GetStartHealth();
	if(pChr->GetPlayer()->Acc().TempActiveSafeSpawn == true)
	{
		pChr->GetPlayer()->Acc().TempActiveSafeSpawn = false;
		StartHealth /= 2;
	}

	if(pChr->GetPlayer()->Acc().TempHealth > 0)
		StartHealth = pChr->GetPlayer()->Acc().TempHealth;
	pChr->IncreaseHealth(StartHealth);
	if(pChr->GetPlayer()->Acc().TempMana > 0)
	{
		const int StartMana = pChr->GetPlayer()->Acc().TempMana;
		pChr->IncreaseMana(StartMana);
	}

	const int StartAmmo = 10 + pChr->GetPlayer()->GetAttributeCount(Stats::StAmmo);
	pChr->GiveWeapon(WEAPON_HAMMER, -1);
	for(int i = 1; i < WEAPON_LASER+1; i++)
		pChr->GiveWeapon(i, StartAmmo);
	return true;
}

bool IGameController::OnEntity(int Index, vec2 Pos)
{
	int Type = -1;

	switch(Index)
	{
	case ENTITY_SPAWN:
		m_aaSpawnPoints[SpawnTypes::SPAWN_HUMAN][m_aNumSpawnPoints[0]++] = Pos;
		break;
	case ENTITY_SPAWN_MOBS:
		m_aaSpawnPoints[SpawnTypes::SPAWN_BOT][m_aNumSpawnPoints[1]++] = Pos;
		break;
	case ENTITY_SPAWN_SAFE:
		m_aaSpawnPoints[SpawnTypes::SPAWN_HUMAN_SAFE][m_aNumSpawnPoints[2]++] = Pos;
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
	const int ClientID = pPlayer->GetCID();
	if(Server()->ClientIngame(ClientID))
	{
		char aBuf[128];
		str_format(aBuf, sizeof(aBuf), "team_join player='%d:%s' team=%d", ClientID, Server()->ClientName(ClientID), pPlayer->GetTeam());
		GS()->Console()->Print(IConsole::OUTPUT_LEVEL_DEBUG, "game", aBuf);
		UpdateGameInfo(ClientID);
	}
}

void IGameController::OnPlayerDisconnect(CPlayer *pPlayer)
{
	GS()->Mmo()->SaveAccount(pPlayer, SAVE_POSITION);
	pPlayer->OnDisconnect();

	const int ClientID = pPlayer->GetCID();
	if(Server()->ClientIngame(ClientID))
	{
		char aBuf[128];
		str_format(aBuf, sizeof(aBuf), "leave player='%d:%s'", ClientID, Server()->ClientName(ClientID));
		GS()->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "game", aBuf);
	}
}

void IGameController::OnPlayerInfoChange(CPlayer *pPlayer, int WorldID) {}

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

bool IGameController::CanSpawn(int SpawnType, vec2 *pOutPos, vec2 BotPos) const
{
	if(SpawnType < SpawnTypes::SPAWN_HUMAN || SpawnType >= SpawnTypes::SPAWN_NUM || GS()->m_World.m_ResetRequested)
		return false;

	CSpawnEval Eval;
	EvaluateSpawnType(&Eval, SpawnType, BotPos);

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
		vec2 Positions[5] = { vec2(0.0f, 0.0f), vec2(-32.0f, 0.0f), vec2(0.0f, -32.0f), vec2(32.0f, 0.0f), vec2(0.0f, 32.0f) };
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
	const int ClientID = pPlayer->GetCID();
	const int Team = pPlayer->GetStartTeam();
	if(Team == pPlayer->GetTeam())
		return;

	pPlayer->Acc().Team = Team;
	GS()->SendTeam(ClientID, Team, DoChatMsg, -1);

	char aBuf[128];
	str_format(aBuf, sizeof(aBuf), "team_join player='%d:%s' m_Team=%d", ClientID, Server()->ClientName(ClientID), Team);
	GS()->Console()->Print(IConsole::OUTPUT_LEVEL_DEBUG, "game", aBuf);
	OnPlayerInfoChange(pPlayer, GS()->GetWorldID());
}

void IGameController::Com_Example(IConsole::IResult* pResult, void* pContext)
{
	//CCommandManager::SCommandContext* pComContext = (CCommandManager::SCommandContext*)pContext;
	//IGameController* pSelf = (IGameController*)pComContext->m_pContext;

}

void IGameController::RegisterChatCommands(CCommandManager* pManager)
{
	//pManager->AddCommand("test", "Test the command system", "r", Com_Example, this);
}
