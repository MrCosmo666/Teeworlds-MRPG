/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#include <limits.h>

#include <engine/shared/config.h>
#include <teeother/system/string.h>
#include <game/server/entity.h>
#include <game/server/gamecontext.h>

#include <game/server/mmocore/GameEntities/npcwall.h>
#include <game/server/mmocore/GameEntities/Logics/logicwall.h>

#include "dungeon.h"

CGameControllerDungeon::CGameControllerDungeon(class CGS *pGS) : IGameController(pGS)
{
	m_pGameType = "MmoTee";
	m_DungeonID = GS()->GetDungeonID();
	m_WorldID = GS()->GetWorldID();
	m_GameFlags = 0;
	m_StartedPlayers = 0;
	m_TankClientID = -1;

	// door creation to start
	vec2 PosDoor = vec2(DungeonJob::ms_aDungeon[m_DungeonID].m_DoorX, DungeonJob::ms_aDungeon[m_DungeonID].m_DoorY);
	m_DungeonDoor = new DungeonDoor(&GS()->m_World, PosDoor);
	ChangeState(DUNGEON_WAITING);

	// key door construction
	ResultPtr pRes = SJK.SD("*", "tw_dungeons_door", "WHERE DungeonID = '%d'", m_DungeonID);
	while (pRes->next())
	{
		const int DungeonBotID = pRes->getInt("BotID");
		vec2 Position = vec2(pRes->getInt("PosX"), pRes->getInt("PosY"));
		new CLogicDungeonDoorKey(&GS()->m_World, Position, DungeonBotID);
	}
}

void CGameControllerDungeon::KillAllPlayers()
{
	for (int i = 0; i < MAX_PLAYERS; i++)
	{
		if(GS()->m_apPlayers[i] && GS()->IsPlayerEqualWorldID(i, m_WorldID))
		{
			if(GS()->m_apPlayers[i]->GetCharacter())
				GS()->m_apPlayers[i]->GetCharacter()->Die(i, WEAPON_WORLD);
		}
	}
}

void CGameControllerDungeon::ChangeState(int State)
{
	DungeonJob::ms_aDungeon[m_DungeonID].m_State = State;
	m_StateDungeon = State;

	// - - - - - - - - - - - - - - - - - - - - - -
	// used when changing state to waiting
	if (State == DUNGEON_WAITING)
	{
		DungeonJob::ms_aDungeon[m_DungeonID].m_Progress = 0;
		m_MaximumTick = 0;
		m_FinishedTick = 0;
		m_StartingTick = 0;
		m_LastStartingTick = 0;
		m_SafeTick = 0;
		m_TankClientID = -1;
		SetMobsSpawn(false);
		ResetDoorKeyState();
	}

	// - - - - - - - - - - - - - - - - - - - - - -
	// used when changing state to waiting start
	else if (State == DUNGEON_WAITING_START)
	{
		m_SyncDungeon = GetSyncFactor();
		m_StartingTick = Server()->TickSpeed() * g_Config.m_SvTimeWaitingsDungeon;
		SetMobsSpawn(false);
	}

	// - - - - - - - - - - - - - - - - - - - - - -
	// used when changing state to start
	else if (State == DUNGEON_STARTED)
	{
		SelectTankPlayer();
		m_StartedPlayers = PlayersNum();
		m_MaximumTick = Server()->TickSpeed() * 600;
		m_SafeTick = Server()->TickSpeed() * 30;
		GS()->ChatWorldID(m_WorldID, "[Dungeon]", "The security timer is enabled for 30 seconds!");
		GS()->ChatWorldID(m_WorldID, "[Dungeon]", "You are given 10 minutes to complete of dungeon!");
		GS()->BroadcastWorldID(m_WorldID, 99999, 500, "Dungeon started!");
		SetMobsSpawn(true);
		KillAllPlayers();
		GS()->SendWorldMusic(-1);
	}

	// - - - - - - - - - - - - - - - - - - - - - -
	// used when changing state to waiting finish
	else if (State == DUNGEON_WAITING_FINISH)
	{
		m_SafeTick = 0;
		m_FinishedTick = Server()->TickSpeed() * 20;
		SetMobsSpawn(false);

		int Seconds = -1;
		dynamic_string Buffer;
		for (int i = 0; i < MAX_PLAYERS; i++)
		{
			if (!GS()->m_apPlayers[i] || !GS()->IsPlayerEqualWorldID(i, m_WorldID))
				continue;

			Buffer.append(", ");
			Buffer.append(Server()->ClientName(i));
			Seconds = GS()->m_apPlayers[i]->GetTempData().m_TempTimeDungeon / Server()->TickSpeed();
			GS()->Mmo()->Dungeon()->SaveDungeonRecord(GS()->m_apPlayers[i], m_DungeonID, Seconds);
			GS()->m_apPlayers[i]->GetTempData().m_TempTimeDungeon = 0;
		}
		char aTimeFormat[64];
		str_format(aTimeFormat, sizeof(aTimeFormat), "Time: %d minute(s) %d second(s)", Seconds / 60, Seconds - (Seconds / 60 * 60));
		GS()->Chat(-1, "Group{STR}!", Buffer.buffer());
		GS()->Chat(-1, "{STR} finished {STR}!", DungeonJob::ms_aDungeon[m_DungeonID].m_aName, aTimeFormat);
	}

	// - - - - - - - - - - - - - - - - - - - - - -
	// used when changing state to finished
	else if (State == DUNGEON_FINISHED)
	{
		SetMobsSpawn(false);
		KillAllPlayers();
	}

	// door state
	m_DungeonDoor->SetState(State);
}

void CGameControllerDungeon::StateTick()
{
	// - - - - - - - - - - - - - - - - - - - - - -
	// dungeon
	const int Players = PlayersNum();
	DungeonJob::ms_aDungeon[m_DungeonID].m_Players = Players;
	if (Players < 1 && m_StateDungeon != DUNGEON_WAITING)
		ChangeState(DUNGEON_WAITING);

	// - - - - - - - - - - - - - - - - - - - - - -
	// used in tick when waiting
	if (m_StateDungeon == DUNGEON_WAITING)
	{
		if (Players >= 1)
			ChangeState(DUNGEON_WAITING_START);
	}

	// - - - - - - - - - - - - - - - - - - - - - -
	// used in the tick when the waiting started
	else if (m_StateDungeon == DUNGEON_WAITING_START)
	{
		if (m_StartingTick)
		{
			// player availability
			const int PlayersReadyState = PlayersReady();
			if(PlayersReadyState >= Players && m_StartingTick > 10 * Server()->TickSpeed())
			{
				m_LastStartingTick = m_StartingTick;
				m_StartingTick = 10 * Server()->TickSpeed();
			}
			else if(PlayersReadyState < Players && m_LastStartingTick > 0)
			{
				const int SkippedTick = 10 * Server()->TickSpeed() - m_StartingTick;
				m_StartingTick = m_LastStartingTick - SkippedTick;
				m_LastStartingTick = 0;
			}

			// run up to speed // if(m_StartingTick % Server()->TickSpeed() == 0)
			const int Time = m_StartingTick / Server()->TickSpeed();
			GS()->BroadcastWorldID(m_WorldID, 99999, 500, "Dungeon waiting {INT} sec!\nPlayer's are ready to start right now {INT} of {INT}!\nYou can change state with 'Vote yes'", &Time, &PlayersReadyState, &Players);

			m_StartingTick--;
			if(!m_StartingTick)
			{
				m_StartedPlayers = Players;
				ChangeState(DUNGEON_STARTED);
			}
		}
	}

	// - - - - - - - - - - - - - - - - - - - - - -
	// used in tick when dunegon is started
	else if (m_StateDungeon == DUNGEON_STARTED)
	{
		for (int i = 0; i < MAX_PLAYERS; i++)
		{
			if (!GS()->m_apPlayers[i] || !GS()->IsPlayerEqualWorldID(i, m_WorldID))
				continue;

			GS()->m_apPlayers[i]->GetTempData().m_TempTimeDungeon++;
		}

		// security tick during which time the player will not return to the old world
		if (m_SafeTick)
		{
			m_SafeTick--;
			if (!m_SafeTick)
				GS()->ChatWorldID(m_WorldID, "[Dungeon]", "The security timer is over, be careful!");
		}

		// finish the dungeon when the dungeon is successfully completed
		if (LeftMobsToWin() <= 0)
			ChangeState(DUNGEON_WAITING_FINISH);
	}

	// - - - - - - - - - - - - - - - - - - - - - -
	// used in the tick when the waiting is finished
	else if (m_StateDungeon == DUNGEON_WAITING_FINISH)
	{
		if (m_FinishedTick)
		{
			const int Time = m_FinishedTick / Server()->TickSpeed();
			GS()->BroadcastWorldID(m_WorldID, 99999, 500, "Dungeon ended {INT} sec!", &Time);

			m_FinishedTick--;
		}
		if (!m_FinishedTick)
			ChangeState(DUNGEON_FINISHED);
	}
}

void CGameControllerDungeon::OnCharacterDeath(CCharacter* pVictim, CPlayer* pKiller, int Weapon)
{
	if (!pKiller || !pVictim || !pVictim->GetPlayer())
		return;

	const int KillerID = pKiller->GetCID();
	const int VictimID = pVictim->GetPlayer()->GetCID();
	if (KillerID != VictimID && pVictim->GetPlayer()->IsBot() && pVictim->GetPlayer()->GetBotType() == BotsTypes::TYPE_BOT_MOB)
	{
		const int Progress = 100 - (int)kurosio::translate_to_procent(CountMobs(), LeftMobsToWin());
		DungeonJob::ms_aDungeon[m_DungeonID].m_Progress = Progress;
		GS()->ChatWorldID(m_WorldID, "[Dungeon]", "The dungeon is completed on [{INT}%]", &Progress);
		UpdateDoorKeyState();
	}
}

bool CGameControllerDungeon::OnCharacterSpawn(CCharacter* pChr)
{
	if(!pChr->GetPlayer()->IsBot())
	{
		if(m_StateDungeon >= DUNGEON_STARTED)
		{
			const int ClientID = pChr->GetPlayer()->GetCID();
			if(ClientID == m_TankClientID)
				pChr->GetPlayer()->m_MoodState = MOOD_PLAYER_TANK;

			if(!m_SafeTick)
			{
				int LatestCorrectWorldID = GS()->Mmo()->Account()->GetHistoryLatestCorrectWorldID(pChr->GetPlayer());
				GS()->Chat(ClientID, "You were thrown out of dungeon!");
				pChr->GetPlayer()->ChangeWorld(LatestCorrectWorldID);
				return false;
			}
		}
		else
		{
			for(int i = 0; i < MAX_PLAYERS; i++)
			{
				if(!GS()->m_apPlayers[i] || !GS()->IsPlayerEqualWorldID(i, m_WorldID))
					continue;
				GS()->StrongUpdateVotes(i, MenuList::MENU_DUNGEONS);
			}
		}
	}

	IGameController::OnCharacterSpawn(pChr);
	return true;
}

void CGameControllerDungeon::UpdateDoorKeyState()
{
	for (CLogicDungeonDoorKey* pDoor = (CLogicDungeonDoorKey*)GS()->m_World.FindFirst(CGameWorld::ENTTYPE_DUNGEON_PROGRESS_DOOR);
		pDoor; pDoor = (CLogicDungeonDoorKey*)pDoor->TypeNext())
	{
		if (pDoor->SyncStateChanges())
			GS()->ChatWorldID(m_WorldID, "[Dungeon]", "Door creaking.. Opened door somewhere!");
	}
}

void CGameControllerDungeon::ResetDoorKeyState()
{
	for (CLogicDungeonDoorKey* pDoor = (CLogicDungeonDoorKey*)GS()->m_World.FindFirst(CGameWorld::ENTTYPE_DUNGEON_PROGRESS_DOOR);
		pDoor; pDoor = (CLogicDungeonDoorKey*)pDoor->TypeNext())
		pDoor->ResetDoor();
}

int CGameControllerDungeon::CountMobs() const
{
	int countMobs = 0;
	for (int i = MAX_PLAYERS; i < MAX_CLIENTS; i++)
	{
		CPlayerBot* BotPlayer = static_cast<CPlayerBot*>(GS()->m_apPlayers[i]);
		if (BotPlayer && BotPlayer->GetBotType() == BotsTypes::TYPE_BOT_MOB && m_WorldID == BotPlayer->GetPlayerWorldID())
			countMobs++;
	}
	return countMobs;
}

int CGameControllerDungeon::PlayersReady() const
{
	int readyPlayers = 0;
	for(int i = 0; i < MAX_PLAYERS; i++)
	{
		if(!GS()->m_apPlayers[i] || !GS()->IsPlayerEqualWorldID(i, m_WorldID) || !GS()->m_apPlayers[i]->GetTempData().m_TempDungeonReady)
			continue;
		readyPlayers++;
	}
	return readyPlayers;
}

int CGameControllerDungeon::PlayersNum() const
{
	int playIt = 0;
	for (int i = 0; i < MAX_PLAYERS; i++)
	{
		if(Server()->GetClientWorldID(i) == m_WorldID)
			playIt++;
	}
	return playIt;
}

int CGameControllerDungeon::LeftMobsToWin() const
{
	int leftMobs = 0;
	for (int i = MAX_PLAYERS; i < MAX_CLIENTS; i++)
	{
		CPlayerBot* BotPlayer = static_cast<CPlayerBot*>(GS()->m_apPlayers[i]);
		if (BotPlayer && BotPlayer->GetBotType() == BotsTypes::TYPE_BOT_MOB && BotPlayer->GetCharacter() && m_WorldID == BotPlayer->GetPlayerWorldID())
			leftMobs++;
	}
	return leftMobs;
}

void CGameControllerDungeon::SetMobsSpawn(bool AllowedSpawn)
{
	for (int i = MAX_PLAYERS; i < MAX_CLIENTS; i++)
	{
		CPlayerBot* BotPlayer = static_cast<CPlayerBot*>(GS()->m_apPlayers[i]);
		if (BotPlayer && BotPlayer->GetBotType() == BotsTypes::TYPE_BOT_MOB && m_WorldID == BotPlayer->GetPlayerWorldID())
		{
			BotPlayer->SetDungeonAllowedSpawn(AllowedSpawn);
			if(!AllowedSpawn && BotPlayer->GetCharacter())
				BotPlayer->GetCharacter()->Die(i, WEAPON_WORLD);
		}
	}
}

void CGameControllerDungeon::SelectTankPlayer()
{
	int MaximalVotes = 0;
	int MaximalHardness = 0;
	m_SelectedWithVotes = false;

	for(int i = 0; i < MAX_PLAYERS; i++)
	{
		CPlayer* pPlayer = GS()->m_apPlayers[i];
		if(!pPlayer || Server()->GetClientWorldID(i) != m_WorldID)
			continue;

		// small random set new tank where Votes equal
		if(MaximalVotes > 0 && pPlayer->GetTempData().m_TempTankVotingDungeon == MaximalVotes && random_int() % 2 == 0)
			m_TankClientID = i;

		// select tank what have more votes
		if(pPlayer->GetTempData().m_TempTankVotingDungeon > MaximalVotes)
		{
			m_TankClientID = i;
			MaximalVotes = pPlayer->GetTempData().m_TempTankVotingDungeon;
			m_SelectedWithVotes = true;
		}

		// skip here if it selected on votes
		if(MaximalVotes > 0)
			continue;

		// select hardness tank
		if(pPlayer->GetLevelTypeAttribute(AtributType::AtTank) > MaximalHardness)
		{
			m_TankClientID = i;
			MaximalHardness = pPlayer->GetLevelTypeAttribute(AtributType::AtTank);
		}
	}

	// show information about tank
	CPlayer* pTankPlayer = GS()->GetPlayer(m_TankClientID, true);
	if(pTankPlayer)
	{
		if(m_SelectedWithVotes)
			GS()->ChatWorldID(m_WorldID, "[Dungeon]", "Tank is assigned to {STR} with {INT} votes!",
				Server()->ClientName(m_TankClientID), &pTankPlayer->GetTempData().m_TempTankVotingDungeon);
		else
		{
			const int StrengthTank = pTankPlayer->GetLevelTypeAttribute(AtributType::AtTank);
			GS()->ChatWorldID(m_WorldID, "[Dungeon]", "Tank {STR} assigned with class strength {INT}p!",
				Server()->ClientName(m_TankClientID), &StrengthTank);
		}
	}
}

// TODO: something to do with the balance
int CGameControllerDungeon::GetSyncFactor() const
{
	int MaxFactor = 0;
	int MinFactor = INT_MAX;
	for(int i = MAX_PLAYERS; i < MAX_CLIENTS; i++)
	{
		CPlayerBot* pBotPlayer = static_cast<CPlayerBot*>(GS()->m_apPlayers[i]);
		if(!pBotPlayer || pBotPlayer->GetBotType() != BotsTypes::TYPE_BOT_MOB || pBotPlayer->GetPlayerWorldID() != m_WorldID)
			continue;

		int LevelDisciple = pBotPlayer->GetLevelAllAttributes();
		MinFactor = min(MinFactor, LevelDisciple);
		MaxFactor = max(MaxFactor, LevelDisciple);
	}
	return (MaxFactor + MinFactor) / 2;
}

int CGameControllerDungeon::GetAttributeDungeonSync(CPlayer* pPlayer, int BonusID) const
{
	float Percent = 0.0f;
	int AttributeType = CGS::ms_aAttributsInfo[BonusID].m_Type;

	// - - - - - - - - -- - - -
	// balance tanks
	if(pPlayer->m_MoodState == MOOD_PLAYER_TANK)
	{
		float ActiveAttribute = m_SyncDungeon / 2.0f;
		if(AttributeType == AtributType::AtTank)
			Percent = 25.0f;

		const int AttributeSyncProcent = kurosio::translate_to_procent_rest(ActiveAttribute, Percent);
		return max(AttributeSyncProcent, 1);
	}

	// - - - - - - - - -- - - -
	// balance healer damage divides the average attribute into the number of players
	float ActiveAttribute = m_SyncDungeon / m_StartedPlayers;
	if(AttributeType == AtributType::AtHealer)
		Percent = min(20.0f + (m_StartedPlayers * 2.0f), 35.0f);
	else if(AttributeType == AtributType::AtTank)
		Percent = 5.0f;
	else if(AttributeType == AtributType::AtHardtype || AttributeType == AtributType::AtDps)
		Percent = 0.1f;

	const int AttributeSyncProcent = kurosio::translate_to_procent_rest(ActiveAttribute, Percent);
	return max(AttributeSyncProcent, 1);
}

void CGameControllerDungeon::Tick()
{
	if (m_MaximumTick)
	{
		m_MaximumTick--;
		if (!m_MaximumTick)
			ChangeState(DUNGEON_FINISHED);
	}

	StateTick();
	IGameController::Tick();
}

void CGameControllerDungeon::CreateLogic(int Type, int Mode, vec2 Pos, int ParseInt)
{
	if(Type == 1)
		new CLogicWall(&GS()->m_World, Pos);

	if(Type == 2)
		new CLogicWallWall(&GS()->m_World, Pos, Mode, ParseInt);

	if(Type == 3)
		new CLogicDoorKey(&GS()->m_World, Pos, ParseInt, Mode);
}

bool CGameControllerDungeon::OnEntity(int Index, vec2 Pos)
{
	if(IGameController::OnEntity(Index, Pos))
		return true;

	if(Index == ENTITY_NPC_WALLUP)
	{
		new CNPCWall(&GS()->m_World, Pos, false);
		return true;
	}

	if(Index == ENTITY_NPC_WALLLEFT)
	{
		new CNPCWall(&GS()->m_World, Pos, true);
		return true;
	}
	
	return false;
}

DungeonDoor::DungeonDoor(CGameWorld *pGameWorld, vec2 Pos)
: CEntity(pGameWorld, CGameWorld::ENTTYPE_DUNGEON_DOOR, Pos)
{
	m_PosTo = GS()->Collision()->FindDirCollision(100, m_PosTo, 'y', '-');
	m_Pos.y += 30;
	m_State = DUNGEON_WAITING;
	GameWorld()->InsertEntity(this);
}

void DungeonDoor::Tick()
{
	if (m_State >= DUNGEON_STARTED)
		return;

	for(CCharacter *pChar = (CCharacter*) GameWorld()->FindFirst(CGameWorld::ENTTYPE_CHARACTER); pChar; pChar = (CCharacter *)pChar->TypeNext())
	{
		vec2 IntersectPos = closest_point_on_line(m_Pos, m_PosTo, pChar->m_Core.m_Pos);
		float Distance = distance(IntersectPos, pChar->m_Core.m_Pos);
		if(Distance <= g_Config.m_SvDoorRadiusHit)
		{
			pChar->m_DoorHit = true;
			pChar->Die(pChar->GetPlayer()->GetCID(), WEAPON_WORLD);
		}
	}
}

void DungeonDoor::Snap(int SnappingClient)
{
	if (m_State >= DUNGEON_STARTED || NetworkClipped(SnappingClient))
		return;
	
	CNetObj_Laser *pObj = static_cast<CNetObj_Laser *>(Server()->SnapNewItem(NETOBJTYPE_LASER, GetID(), sizeof(CNetObj_Laser)));
	if (!pObj)
		return;

	pObj->m_X = int(m_Pos.x);
	pObj->m_Y = int(m_Pos.y);
	pObj->m_FromX = int(m_PosTo.x);
	pObj->m_FromY = int(m_PosTo.y);
	pObj->m_StartTick = Server()->Tick()-2;
}