/* (c) Alexandre Díaz. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#ifndef GAME_SERVER_BOTAI_HELPER_H
#define GAME_SERVER_BOTAI_HELPER_H

#include <map>
#include <game/server/entity.h>

#include <game/gamecore.h>
#include "../character.h"

class CEntityFunctionNurse;
class CCharacterBotAI : public CCharacter
{
	MACRO_ALLOC_POOL_ID()

public:
	CCharacterBotAI(CGameWorld *pWorld);
	~CCharacterBotAI();

	int GetBotTarget() const { return m_BotTargetID; };

private: 
	class CPlayerBot* m_pBotPlayer;
	
	// bot ai
	bool m_BotActive;
	int m_MoveTick;
	int m_PrevDirection;
	vec2 m_PrevPos;
	vec2 m_WallPos;
	int m_EmotionsStyle;

	// target system
	int m_BotTargetID;
	int m_BotTargetLife;
	bool m_BotTargetCollised;
	std::map < int, int > m_ListDmgPlayers;

	virtual bool Spawn(class CPlayer *pPlayer, vec2 Pos);
	virtual void Tick();
	virtual void TickDefered();
	virtual bool TakeDamage(vec2 Force, int Dmg, int From, int Weapon);
	virtual void Die(int Killer, int Weapon);
	virtual bool GiveWeapon(int Weapon, int GiveAmmo);
	virtual int GetSnapFullID() const;

	void CreateRandomDropItem(int DropCID, int Random, int ItemID, int Count, vec2 Force);
	void DieRewardPlayer(CPlayer *pPlayer, vec2 ForceDies);
	
	void ClearTarget();
	void SetTarget(int ClientID);
	void ChangeWeapons();
	void ShowProgress();
	void EmoteActions(int EmotionStyle);
	void SetAim(vec2 Dir);

	void EngineBots();
	void EngineNPC();
	void EngineMobs();
	void EngineQuestMob();

	CPlayer *SearchPlayer(int Distance);
    CPlayer *SearchTenacityPlayer(float Distance);

	bool FunctionNurseNPC();
	bool BaseFunctionNPC();
	void Move();
	void Action();
};

class CEntityFunctionNurse : public CEntity
{
	int m_OwnerID;
public:
	CEntityFunctionNurse(CGameWorld* pGameWorld, int ClientID, vec2 Pos);
	virtual void Tick();
	virtual void Snap(int SnappingClient);
};

#endif
