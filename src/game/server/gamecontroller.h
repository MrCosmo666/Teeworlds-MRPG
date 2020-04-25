/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#ifndef GAME_SERVER_GAMECONTROLLER_H
#define GAME_SERVER_GAMECONTROLLER_H

#include <base/tl/array.h>
#include <base/vmath.h>
#include <game/commands.h>

#include <generated/protocol.h>

/*
	Class: Game Controller
		Controls the main game logic. Keeping track of team and player score,
		winning conditions and specific game logic.
*/
class IGameController
{
	class CGS *m_pGS;
	class IServer *m_pServer;

	// spawn
	struct CSpawnEval
	{
		CSpawnEval()
		{
			m_Got = false;
			m_FriendlyTeam = -1;
			m_Pos = vec2(100,100);
			m_Score = 0;
		}

		vec2 m_Pos;
		bool m_Got;
		int m_FriendlyTeam;
		float m_Score;
	};
	vec2 m_aaSpawnPoints[3][64];
	int m_aNumSpawnPoints[3];
	
	float EvaluateSpawnPos(CSpawnEval *pEval, vec2 Pos) const;
	void EvaluateSpawnType(CSpawnEval *pEval, int Type, vec2 BotPos) const;

protected:
	CGS *GS() const { return m_pGS; }
	IServer *Server() const { return m_pServer; }

	// info
	const char *m_pGameType;
	int m_GameFlags;
	
	void UpdateGameInfo(int ClientID);

public:
	IGameController(class CGS *pGS);
	virtual ~IGameController() {};

	// event
	/*
		Function: on_CCharacter_death
			Called when a CCharacter in the world dies.

		Arguments:
			victim - The CCharacter that died.
			killer - The player that killed it.
			weapon - What weapon that killed it. Can be -1 for undefined
				weapon when switching team or player suicides.
	*/
	virtual void OnCharacterDeath(class CCharacter *pVictim, class CPlayer *pKiller, int Weapon);
	/*
		Function: on_CCharacter_spawn
			Called when a CCharacter spawns into the game world.

		Arguments:
			chr - The CCharacter that was spawned.
	*/
	virtual void OnCharacterSpawn(class CCharacter *pChr);

	/*
		Function: on_entity
			Called when the map is loaded to process an entity
			in the map.

		Arguments:
			index - Entity index.
			pos - Where the entity is located in the world.

		Returns:
			bool?
	*/
	virtual bool OnEntity(int Index, vec2 Pos);

	static void Com_Example(IConsole::IResult* pResult, void* pContext);
	virtual void RegisterChatCommands(CCommandManager* pManager);

	void OnPlayerConnect(class CPlayer *pPlayer);
	void OnPlayerDisconnect(class CPlayer *pPlayer);
	void OnPlayerInfoChange(class CPlayer *pPlayer, int WorldID);
	void OnReset();

	// виртуал аномаль
	virtual void RespawnedClickEvent() = 0;
	virtual void CreateLogic(int Type, int Mode, vec2 Pos, int Health) = 0;
	
	// general
	virtual void Snap(int SnappingClient);
	virtual void Tick();

	const char *GetGameType() const { return m_pGameType; }
	
	//spawn
	bool CanSpawn(int Team, vec2 *pPos, vec2 BotPos) const;

	// team
	void DoTeamChange(class CPlayer *pPlayer, bool DoChatMsg=true);
	int GetRealPlayer();

};

#endif
