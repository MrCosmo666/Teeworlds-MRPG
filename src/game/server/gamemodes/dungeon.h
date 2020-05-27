/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#ifndef GAME_SERVER_GAMEMODES_DUNGEON_H
#define GAME_SERVER_GAMEMODES_DUNGEON_H

#include <game/server/entity.h>
#include <game/server/gamecontroller.h>

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
	int m_WorldID;
	int m_StartingTick;
	int m_FinishedTick;
	int m_SafeTick;
	int m_MaximumTick;
	int m_Time[MAX_PLAYERS];
	bool m_ShowedTankingInfo;
	int m_StartedPlayers;
	int m_TankClientID;
	bool m_SelectedWithVotes;

public:
	CGameControllerDungeon(class CGS* pGameServer);

	virtual void Tick();
	virtual bool OnEntity(int Index, vec2 Pos);
	virtual void OnCharacterDeath(class CCharacter* pVictim, class CPlayer* pKiller, int Weapon);
	virtual bool OnCharacterSpawn(class CCharacter* pChr);
	virtual void CreateLogic(int Type, int Mode, vec2 Pos, int ParseID);
	int GetDungeonSync(class CPlayer* pPlayer, int BonusID) const;

private:
	int PlayersNum() const;
	int LeftMobsToWin() const;
	int CountMobs() const;

	void ChangeState(int State);
	void StateTick();
	void SetMobsSpawn(bool AllowedSpawn);
	void KillAllPlayers();

	void UpdateDoorKeyState();
	void ResetDoorKeyState();
	void SelectTankPlayer();
};

class DungeonDoor : public CEntity
{
	int m_State;
public:
	DungeonDoor(CGameWorld *pGameWorld, vec2 Pos);

	void SetState(int State) { m_State = State; };
	virtual void Tick();
	virtual void Snap(int SnappingClient);
};

#endif
