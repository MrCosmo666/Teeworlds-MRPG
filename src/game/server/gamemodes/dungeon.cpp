/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */

#include <engine/shared/config.h>
#include <game/server/gamecontext.h>

#include <game/server/mmocore/GameEntities/jobitems.h>
#include <game/server/mmocore/GameEntities/npcwall.h>
#include <game/server/mmocore/GameEntities/Logics/logicwall.h>

#include "dungeon.h"

CGameControllerDungeon::CGameControllerDungeon(class CGS *pGS) : IGameController(pGS)
{
	m_pGameType = "MmoTee";
	m_DungeonID = GS()->DungeonID();
	m_WorldID = GS()->GetWorldID();
	m_GameFlags = 0;
	m_StartedPlayers = 0;
	m_ShowedTankingInfo = false;

	// создание двери для ожидания начала
	vec2 PosDoor = vec2(DungeonJob::Dungeon[m_DungeonID].DoorX, DungeonJob::Dungeon[m_DungeonID].DoorY);
	m_DungeonDoor = new DungeonDoor(&GS()->m_World, PosDoor);
	ChangeState(DUNGEON_WAITING);

	// создание ключевых дверей
	boost::scoped_ptr<ResultSet> RES(SJK.SD("*", "tw_dungeons_door", "WHERE DungeonID = '%d'", m_DungeonID));
	while (RES->next())
	{
		const int DungeonBotID = RES->getInt("BotID");
		vec2 Position = vec2(RES->getInt("PosX"), RES->getInt("PosY"));
		new CLogicDungeonDoorKey(&GS()->m_World, Position, DungeonBotID);
	}
}

void CGameControllerDungeon::KillAllPlayers()
{
	for (int i = 0; i < MAX_PLAYERS; i++)
	{
		if (GS()->m_apPlayers[i] && GS()->m_apPlayers[i]->GetCharacter() && Server()->GetWorldID(i) == m_WorldID)
			GS()->m_apPlayers[i]->GetCharacter()->Die(i, WEAPON_WORLD);
	}
}

void CGameControllerDungeon::ChangeState(int State)
{
	m_StateDungeon = State;

	// - - - - - - - - - - - - - - - - - - - - - -
	// Используется при смене статуса в Ожидание данжа
	if (State == DUNGEON_WAITING)
	{
		DungeonJob::Dungeon[m_DungeonID].Progress = 0;
		m_MaximumTick = 0;
		m_FinishedTick = 0;
		m_StartingTick = 0;
		m_SafeTick = 0;
		m_ShowedTankingInfo = false;
		SetMobsSpawn(false);
		ResetDoorKeyState();
	}

	// - - - - - - - - - - - - - - - - - - - - - -
	// Используется при смене статуса в Ожидание данжа
	else if (State == DUNGEON_WAITING_START)
	{
		m_StartingTick = Server()->TickSpeed() * g_Config.m_SvTimeWaitingsDungeon;
		SetMobsSpawn(false);
	}

	// - - - - - - - - - - - - - - - - - - - - - -
	// Используется при смене статуса в Начало данжа
	else if (State == DUNGEON_STARTED)
	{
		for (int i = 0; i < MAX_PLAYERS; i++)
		{
			if (!GS()->m_apPlayers[i] || Server()->GetWorldID(i) != m_WorldID)
				continue;

			GS()->m_apPlayers[i]->GetTempData().TempTimeDungeon = 0;
		}
		m_StartedPlayers = PlayersNum();
		m_MaximumTick = Server()->TickSpeed() * 900;
		m_SafeTick = Server()->TickSpeed() * 30;
		GS()->ChatWorldID(m_WorldID, "[Dungeon]", "The security timer is enabled for 30 seconds!");
		GS()->ChatWorldID(m_WorldID, "[Dungeon]", "You are given 15 minutes to complete of dungeon!");
		GS()->BroadcastWorldID(m_WorldID, 99999, 500, "Dungeon started!");
		SetMobsSpawn(true);
		KillAllPlayers();
	}

	// - - - - - - - - - - - - - - - - - - - - - -
	// Используется при смене статуса в Ожидание финиша данжа
	else if (State == DUNGEON_WAITING_FINISH)
	{
		m_SafeTick = 0;
		m_FinishedTick = Server()->TickSpeed() * 20;
		SetMobsSpawn(false);

		// элемент RACE
		for (int i = 0; i < MAX_PLAYERS; i++)
		{
			if (!GS()->m_apPlayers[i] || Server()->GetWorldID(i) != m_WorldID)
				continue;

			const int Seconds = GS()->m_apPlayers[i]->GetTempData().TempTimeDungeon / Server()->TickSpeed();
			GS()->Mmo()->Dungeon()->SaveDungeonRecord(GS()->m_apPlayers[i], m_DungeonID, Seconds);
			GS()->m_apPlayers[i]->GetTempData().TempTimeDungeon = 0;

			char aTimeFormat[64];
			str_format(aTimeFormat, sizeof(aTimeFormat), "Time: %d minute(s) %d second(s)", Seconds / 60, Seconds - (Seconds / 60 * 60));
			GS()->Chat(-1, "{STR} finished {STR} {STR}", Server()->ClientName(i), DungeonJob::Dungeon[m_DungeonID].Name, aTimeFormat);
		}
	}

	// - - - - - - - - - - - - - - - - - - - - - -
	// Используется при смене статуса в Завершение данжа
	else if (State == DUNGEON_FINISHED)
	{
		SetMobsSpawn(false);
		KillAllPlayers();
	}

	// установить статус дверям
	m_DungeonDoor->SetState(State);
}

void CGameControllerDungeon::StateTick()
{
	// сбросить данж
	const int Players = PlayersNum();
	if (Players < 1 && m_StateDungeon != DUNGEON_WAITING)
		ChangeState(DUNGEON_WAITING);

	// обновлять информацию каждую секунду
	if (Server()->Tick() % Server()->TickSpeed() == 0)
	{
		DungeonJob::Dungeon[m_DungeonID].Players = Players;
		DungeonJob::Dungeon[m_DungeonID].State = m_StateDungeon;
	}

	// - - - - - - - - - - - - - - - - - - - - - -
	// Используется в тике когда Ожидание данжа
	if (m_StateDungeon == DUNGEON_WAITING)
	{
		m_StartedPlayers = Players;
		if (Players >= 1)
			ChangeState(DUNGEON_WAITING_START);
	}

	// - - - - - - - - - - - - - - - - - - - - - -
	// Используется в тике когда Отчет начала данжа
	else if (m_StateDungeon == DUNGEON_WAITING_START)
	{
		if (m_StartingTick)
		{
			const int Time = m_StartingTick / Server()->TickSpeed();
			GS()->BroadcastWorldID(m_WorldID, 99999, 500, "Dungeon waiting {INT} sec!", &Time);
			m_StartingTick--;
			if (!m_StartingTick)
				ChangeState(DUNGEON_STARTED);
		}
	}

	// - - - - - - - - - - - - - - - - - - - - - -
	// Используется в тике когда Данж начат
	else if (m_StateDungeon == DUNGEON_STARTED)
	{
		// элемент RACE
		for (int i = 0; i < MAX_PLAYERS; i++)
		{
			if (!GS()->m_apPlayers[i] || Server()->GetWorldID(i) != m_WorldID)
				continue;

			GS()->m_apPlayers[i]->GetTempData().TempTimeDungeon++;
		}

		// тик безопасности в течении какого времени игрок не вернется в старый мир
		if (m_SafeTick)
		{
			m_SafeTick--;
			if (!m_SafeTick)
				GS()->ChatWorldID(m_WorldID, "[Dungeon]", "The security timer is over, be careful!");
		}

		// завершаем данж когда успешно данж завершен
		if (LeftMobsToWin() <= 0)
			ChangeState(DUNGEON_WAITING_FINISH);
	}

	// - - - - - - - - - - - - - - - - - - - - - -
	// Используется в тике когда Отчет начала данжа
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
		DungeonJob::Dungeon[m_DungeonID].Progress = Progress;
		GS()->ChatWorldID(m_WorldID, "[Dungeon]", "The dungeon is completed on [{INT}%]", &Progress);
		UpdateDoorKeyState();
	}
	return;
}

bool CGameControllerDungeon::OnCharacterSpawn(CCharacter* pChr)
{
	if(!pChr->GetPlayer()->IsBot())
	{
		pChr->GetPlayer()->m_SyncDungeon = GS()->Mmo()->Dungeon()->SyncFactor();
		pChr->GetPlayer()->m_SyncPlayers = m_StartedPlayers;
	}

	if(m_StateDungeon >= DUNGEON_STARTED)
	{
		const int ClientID = pChr->GetPlayer()->GetCID();
		if(pChr->GetPlayer()->m_MoodState == MOOD_PLAYER_TANK && !m_ShowedTankingInfo)
		{
			m_ShowedTankingInfo = true;
			const int StrengthTank = pChr->GetPlayer()->GetLevelDisciple(AtributType::AtTank, true);
			GS()->ChatWorldID(m_WorldID, "[Dungeon]", "Tank {STR} assigned with class strength {INT}p!", Server()->ClientName(ClientID), &StrengthTank);
		}
		if(!m_SafeTick)
		{
			GS()->Chat(ClientID, "You were thrown out of dungeon!");
			pChr->GetPlayer()->ChangeWorld(pChr->GetPlayer()->Acc().LastWorldID);
			return false;
		}
	}
	IGameController::OnCharacterSpawn(pChr);
	return true;
}

void CGameControllerDungeon::UpdateDoorKeyState()
{
	for (CLogicDungeonDoorKey* pDoor = (CLogicDungeonDoorKey*)GS()->m_World.FindFirst(CGameWorld::ENTTYPE_DUNGEONKEYDOOR);
		pDoor; pDoor = (CLogicDungeonDoorKey*)pDoor->TypeNext())
	{
		if (pDoor->SyncStateChanges())
			GS()->ChatWorldID(m_WorldID, "[Dungeon]", "Scr... Scrr... Opened door somewhere!");
	}
}

void CGameControllerDungeon::ResetDoorKeyState()
{
	for (CLogicDungeonDoorKey* pDoor = (CLogicDungeonDoorKey*)GS()->m_World.FindFirst(CGameWorld::ENTTYPE_DUNGEONKEYDOOR);
		pDoor; pDoor = (CLogicDungeonDoorKey*)pDoor->TypeNext())
		pDoor->ResetDoor();
}

int CGameControllerDungeon::CountMobs() const
{
	int countMobs = 0;
	for (int i = MAX_PLAYERS; i < MAX_CLIENTS; i++)
	{
		CPlayerBot* BotPlayer = static_cast<CPlayerBot*>(GS()->m_apPlayers[i]);
		if (BotPlayer && BotPlayer->GetBotType() == BotsTypes::TYPE_BOT_MOB && m_WorldID == GS()->GetClientWorldID(i))
			countMobs++;
	}
	return countMobs;
}

int CGameControllerDungeon::PlayersNum() const
{
	int playIt = 0;
	for (int i = 0; i < MAX_PLAYERS; i++)
	{
		if(Server()->GetWorldID(i) == m_WorldID)
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
		if (BotPlayer && BotPlayer->GetBotType() == BotsTypes::TYPE_BOT_MOB && BotPlayer->GetCharacter() && m_WorldID == GS()->GetClientWorldID(i))
			leftMobs++;
	}
	return leftMobs;
}

void CGameControllerDungeon::SetMobsSpawn(bool AllowedSpawn)
{
	for (int i = MAX_PLAYERS; i < MAX_CLIENTS; i++)
	{
		CPlayerBot* BotPlayer = static_cast<CPlayerBot*>(GS()->m_apPlayers[i]);
		if (BotPlayer && BotPlayer->GetBotType() == BotsTypes::TYPE_BOT_MOB && m_WorldID == GS()->GetClientWorldID(i))
		{
			BotPlayer->SetDungeonAllowedSpawn(AllowedSpawn);
			if (!AllowedSpawn && BotPlayer->GetCharacter())
				BotPlayer->GetCharacter()->Die(i, WEAPON_WORLD);
		}
	}
}

void CGameControllerDungeon::Tick()
{
	// тик максимально местонахождения там
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
	{
		new CLogicWall(&GS()->m_World, Pos);
	}
	if(Type == 2)
	{
		new CLogicWallWall(&GS()->m_World, Pos, Mode, ParseInt);
	}
	if(Type == 3)
	{
		new CLogicDoorKey(&GS()->m_World, Pos, ParseInt, Mode);
	}
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

// TODO: something to do with the balance
int CGameControllerDungeon::GetDungeonSync(CPlayer* pPlayer, int BonusID) const
{
	int Procent = 1;
	int Delay = 0;

	const int ParsePlayerStatsClass = CGS::AttributInfo[BonusID].AtType;
	if(pPlayer->m_MoodState == MOOD_PLAYER_TANK)
	{
		if(ParsePlayerStatsClass == AtributType::AtTank)
			Procent = 12;
		if(ParsePlayerStatsClass == AtributType::AtHealer || ParsePlayerStatsClass == AtributType::AtDps)
			Delay = 60;
	}
	else
	{
		if(ParsePlayerStatsClass == AtributType::AtTank)
			Procent = 3;
		else if(ParsePlayerStatsClass == AtributType::AtHealer)
			Procent = 6;
		else if(ParsePlayerStatsClass == AtributType::AtDps)
			Procent = 4;

	}

	if(ParsePlayerStatsClass == AtributType::AtHardtype || BonusID == Stats::StStrength)
		Delay = 50;

	const int AttributeSyncProcent = kurosio::translate_to_procent_rest(pPlayer->m_SyncDungeon, Procent);
	int AttributeCount = max(AttributeSyncProcent, 1);

	if(pPlayer->m_MoodState == MOOD_PLAYER_TANK && ParsePlayerStatsClass == AtributType::AtTank)
		return AttributeCount;

	AttributeCount /= max((pPlayer->m_SyncPlayers + Delay), 1);
	return AttributeCount;
}

DungeonDoor::DungeonDoor(CGameWorld *pGameWorld, vec2 Pos)
: CEntity(pGameWorld, CGameWorld::ENTTYPE_DUNGEONDOOR, Pos), m_To(Pos)
{
	m_To = GS()->Collision()->FindDirCollision(100, m_To, 'y', '-');
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
		vec2 IntersectPos = closest_point_on_line(m_Pos, m_To, pChar->m_Core.m_Pos);
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
	pObj->m_FromX = int(m_To.x);
	pObj->m_FromY = int(m_To.y);
	pObj->m_StartTick = Server()->Tick()-2;
}