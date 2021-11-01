/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#include <game/mapitems.h>
#include "gamecontroller.h"

#include "gamecontext.h"
#include "entities/pickup.h"

/*
	Here you need to put it in order make more events
	For modes that each map can have one of them
*/

IGameController::IGameController(CGS *pGS)
{
	m_pGS = pGS;
	m_GameFlags = 0;
	m_pServer = m_pGS->Server();

	for(int i = 0; i < SPAWN_NUM; i++)
		m_aNumSpawnPoints[i] = 0;
}



void IGameController::OnCharacterDamage(CPlayer* pFrom, CPlayer* pTo, int Damage)
{
}

void IGameController::OnCharacterDeath(CCharacter *pVictim, CPlayer *pKiller, int Weapon)
{
	return;
}

bool IGameController::OnCharacterSpawn(CCharacter* pChr)
{
	// if we spawn the bot
	if(pChr->GetPlayer()->IsBot())
	{
		pChr->IncreaseHealth(pChr->GetPlayer()->GetStartHealth());
		pChr->GiveWeapon(WEAPON_HAMMER, -1);
		for(int i = WEAPON_GUN; i < NUM_WEAPONS-1; i++)
			pChr->GiveWeapon(i, 10);
		return true;
	}

	// Health
	int StartHealth = pChr->GetPlayer()->GetStartHealth();
	if(!GS()->IsDungeon())
	{
		if(pChr->GetPlayer()->GetHealth() > 0)
			StartHealth = pChr->GetPlayer()->GetHealth();
		else if(pChr->GetPlayer()->GetTempData().m_TempSafeSpawn == true)
		{
			pChr->GetPlayer()->GetTempData().m_TempSafeSpawn = false;
			StartHealth /= 2;
		}
	}
	pChr->IncreaseHealth(StartHealth);

	// Mana
	if(pChr->GetPlayer()->GetMana() > 0)
	{
		const int StartMana = pChr->GetPlayer()->GetMana();
		pChr->IncreaseMana(StartMana);
	}

	// Weapons
	const int StartAmmo = 10 + pChr->GetPlayer()->GetAttributeCount(Stats::StAmmo);
	pChr->GiveWeapon(WEAPON_HAMMER, -1);
	for(int i = 1; i < NUM_WEAPONS-1; i++)
		pChr->GiveWeapon(i, StartAmmo);
	return true;
}

bool IGameController::OnEntity(int Index, vec2 Pos)
{
	int Type = -1;
	switch(Index)
	{
	case ENTITY_SPAWN:
		m_aaSpawnPoints[SPAWN_HUMAN][m_aNumSpawnPoints[SPAWN_HUMAN]++] = Pos;
		break;
	case ENTITY_SPAWN_MOBS:
		m_aaSpawnPoints[SPAWN_BOT][m_aNumSpawnPoints[SPAWN_BOT]++] = Pos;
		break;
	case ENTITY_SPAWN_SAFE:
		m_aaSpawnPoints[SPAWN_HUMAN_SAFE][m_aNumSpawnPoints[SPAWN_HUMAN_SAFE]++] = Pos;
		break;
	case ENTITY_ARMOR_1:
		Type = PICKUP_ARMOR;
		break;
	case ENTITY_HEALTH_1:
		Type = PICKUP_HEALTH;
		break;
	case ENTITY_PICKUP_SHOTGUN:
		Type = PICKUP_SHOTGUN;
		break;
	case ENTITY_PICKUP_GRENADE:
		Type = PICKUP_GRENADE;
		break;
	case ENTITY_PICKUP_LASER:
		Type = PICKUP_LASER;
		break;
	default: break;
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
	if(Server()->ClientIngame(ClientID) && pPlayer->GetPlayerWorldID() == GS()->GetWorldID())
	{
		char aBuf[128];
		str_format(aBuf, sizeof(aBuf), "team_join player='%d:%s' team=%d", ClientID, Server()->ClientName(ClientID), pPlayer->GetTeam());
		GS()->Console()->Print(IConsole::OUTPUT_LEVEL_DEBUG, "game", aBuf);
		UpdateGameInfo(ClientID);
	}
}

void IGameController::OnPlayerDisconnect(CPlayer *pPlayer)
{
	const int ClientID = pPlayer->GetCID();
	if(Server()->ClientIngame(ClientID) && pPlayer->GetPlayerWorldID() == GS()->GetWorldID())
	{
		char aBuf[128];
		str_format(aBuf, sizeof(aBuf), "leave player='%d:%s'", ClientID, Server()->ClientName(ClientID));
		GS()->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "game", aBuf);
		GS()->Mmo()->SaveAccount(pPlayer, SaveType::SAVE_POSITION);
	}

	pPlayer->OnDisconnect();
}

void IGameController::OnPlayerInfoChange(CPlayer *pPlayer, int WorldID) {}

void IGameController::OnReset()
{
	for(int i = 0; i < MAX_CLIENTS; i++)
	{
		if(GS()->m_apPlayers[i])
			GS()->m_apPlayers[i]->m_aPlayerTick[TickState::Respawn] = Server()->Tick()+Server()->TickSpeed()/2;
	}
}

// general
void IGameController::Snap()
{
	CNetObj_GameData *pGameData = static_cast<CNetObj_GameData *>(Server()->SnapNewItem(NETOBJTYPE_GAMEDATA, 0, sizeof(CNetObj_GameData)));
	if(!pGameData)
		return;

	pGameData->m_GameStartTick = Server()->GetOffsetWorldTime();
	pGameData->m_GameStateFlags = 0;
	pGameData->m_GameStateEndTick = 0;
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

			if((!GS()->IsMmoClient(i) && Server()->GetClientProtocolVersion(i) < MIN_RACE_CLIENTVERSION))
				GameInfoMsg.m_GameFlags &= ~GAMEFLAG_RACE;

			Server()->SendPackMsg(&GameInfoMsg, MSGFLAG_VITAL|MSGFLAG_NORECORD, i, Server()->GetClientWorldID(ClientID));
		}
	}
	else
	{
		if((!GS()->IsMmoClient(ClientID) && Server()->GetClientProtocolVersion(ClientID) < MIN_RACE_CLIENTVERSION))
			GameInfoMsg.m_GameFlags &= ~GAMEFLAG_RACE;

		Server()->SendPackMsg(&GameInfoMsg, MSGFLAG_VITAL|MSGFLAG_NORECORD, ClientID, Server()->GetClientWorldID(ClientID));
	}
}

bool IGameController::CanSpawn(int SpawnType, vec2 *pOutPos, vec2 BotPos) const
{
	if(SpawnType < SPAWN_HUMAN || SpawnType >= SPAWN_NUM || GS()->m_World.m_ResetRequested)
		return false;

	CSpawnEval Eval;
	EvaluateSpawnType(&Eval, SpawnType, BotPos);

	*pOutPos = Eval.m_Pos;
	return Eval.m_Got;
}

float IGameController::EvaluateSpawnPos(CSpawnEval *pEval, vec2 Pos) const
{
	float Score = 0.0f;
	CCharacter *pC = dynamic_cast<CCharacter *>(GS()->m_World.FindFirst(CGameWorld::ENTTYPE_CHARACTER));
	for(; pC; pC = (CCharacter *)pC->TypeNext())
	{
		// team mates are not as dangerous as enemies
		float Scoremod = 1.0f;
		if(pEval->m_FriendlyTeam != -1 && pC->GetPlayer()->GetTeam() == pEval->m_FriendlyTeam)
			Scoremod = 0.5f;

		float d = distance(Pos, pC->GetPos());
		Score += Scoremod * (d == 0.f ? 1000000000.0f : 1.0f/d);
	}

	return Score;
}

void IGameController::EvaluateSpawnType(CSpawnEval *pEval, int SpawnType, vec2 BotPos) const
{
	// get spawn point
	for(int i = 0; i < m_aNumSpawnPoints[SpawnType]; i++)
	{
		// check if the position is occupado
		CCharacter *aEnts[MAX_CLIENTS];
		int Num = GS()->m_World.FindEntities(m_aaSpawnPoints[SpawnType][i], 64, (CEntity**)aEnts, MAX_CLIENTS, CGameWorld::ENTTYPE_CHARACTER);
		vec2 Positions[5] = { vec2(0.0f, 0.0f), vec2(-32.0f, 0.0f), vec2(0.0f, -32.0f), vec2(32.0f, 0.0f), vec2(0.0f, 32.0f) };
		int Result = -1;

		if(BotPos != vec2(-1, -1) && distance(BotPos, m_aaSpawnPoints[SpawnType][i]) > 800.0f)
			continue;

		for(int Index = 0; Index < 5 && Result == -1; ++Index)
		{
			Result = Index;
			for(int c = 0; c < Num; ++c)
			{
				if(
					GS()->Collision()->CheckPoint(m_aaSpawnPoints[SpawnType][i]+Positions[Index]) ||
					distance(aEnts[c]->GetPos(), m_aaSpawnPoints[SpawnType][i]+Positions[Index]) <= aEnts[c]->GetProximityRadius())
				{
					Result = -1;
					break;
				}
			}
		}
		if(Result == -1)
			continue; // try next spawn point

		const vec2 P = m_aaSpawnPoints[SpawnType][i]+Positions[Result];
		const float S = EvaluateSpawnPos(pEval, P);
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

	pPlayer->Acc().m_Team = Team;
	GS()->SendTeam(ClientID, Team, DoChatMsg, -1);

	char aBuf[128];
	str_format(aBuf, sizeof(aBuf), "team_join player='%d:%s' m_Team=%d", ClientID, Server()->ClientName(ClientID), Team);
	GS()->Console()->Print(IConsole::OUTPUT_LEVEL_DEBUG, "game", aBuf);
	OnPlayerInfoChange(pPlayer, GS()->GetWorldID());
}
