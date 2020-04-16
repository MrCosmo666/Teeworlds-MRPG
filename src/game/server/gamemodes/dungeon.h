/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#ifndef GAME_SERVER_GAMEMODES_DUNGEON_H
#define GAME_SERVER_GAMEMODES_DUNGEON_H

#include <game/server/entity.h>
#include <game/server/gamecontroller.h>

// you can subclass GAMECONTROLLER_CTF, GAMECONTROLLER_TDM etc if you want
// todo a modification with their base as well.
enum DungeonState
{
	DUNGEON_WAITING,
	DUNGEON_WAITING_START,
	DUNGEON_STARTED,
	DUNGEON_WAITING_FINISH,
	DUNGEON_FINISHED,

};

class DungeonDoor;
class CGameControllerDungeon : public IGameController
{
	DungeonDoor *m_DungeonDoor;
	int m_StateDungeon;
	int m_DungeonID;
	int m_StartingTick;
	int m_FinishedTick;
	int m_SafeTick;
	int m_MaximumTick;

	int m_Time[MAX_PLAYERS];

public:
	CGameControllerDungeon(class CGS* pGameServer);

	virtual void Tick();
	virtual bool OnEntity(int Index, vec2 Pos);
	virtual int OnCharacterDeath(class CCharacter* pVictim, class CPlayer* pKiller, int Weapon);
	virtual void OnCharacterSpawn(class CCharacter* pChr);

	virtual void CreateLogic(int Type, int Mode, vec2 Pos, int ParseID);
	
	// пустые функции с mod
	virtual void RespawnedClickEvent() {};

private:
	int PlayersNum() const;
	int LeftMobsToWin() const;
	int CountMobs() const;

	void ChangeState(int State);
	void StateTick();
	void SetMobsSpawn(bool AllowedSpawn);
	void KillAllPlayers();

	void UpdateDoorKeyState(bool StartingGame = false);
};

// CHomeDoor class
class DungeonDoor : public CEntity
{
	vec2 m_To;
	int m_State;
public:
	DungeonDoor(CGameWorld *pGameWorld, vec2 Pos);

	void SetState(int State) { m_State = State; };
	virtual void Tick();
	virtual void Snap(int SnappingClient);
};

#endif
