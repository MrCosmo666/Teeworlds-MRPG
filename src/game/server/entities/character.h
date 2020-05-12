/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#ifndef GAME_SERVER_ENTITIES_CHARACTER_H
#define GAME_SERVER_ENTITIES_CHARACTER_H

#include <generated/protocol.h>

#include <game/gamecore.h>
#include <game/server/entity.h>
#include "../mmocore/TileHandle.h"

class CCharacter : public CEntity
{
	MACRO_ALLOC_POOL_ID()

	// player controlling this character
	class CPlayer *m_pPlayer;
	class TileHandle *m_pHelper;

	// Ядро неследование этого думаю не потребуется
	void FireWeapon();
	bool DecoInteractive();

	void HandleTilesets();
	void HandleEvents();

	void HandleWeapons();

	void HandleWeaponSwitch();
	void DoWeaponSwitch();

	bool InteractiveHammer(vec2 Direction, vec2 ProjStartPos);
	void InteractiveGun(vec2 Direction, vec2 ProjStartPos);
	void InteractiveShotgun(vec2 Direction, vec2 ProjStartPos);
	void InteractiveGrenade(vec2 Direction, vec2 ProjStartPos);
	void InteractiveRifle(vec2 Direction, vec2 ProjStartPos);
	void HandleTunning();
	void HandleAuthedPlayer();
	bool IsLockedWorld();

public:
	//character's size
	static const int ms_PhysSize = 28;
	CCharacter(CGameWorld *pWorld);
	~CCharacter();

	CPlayer *GetPlayer() const { return m_pPlayer; }
	TileHandle *GetHelper() const { return m_pHelper; }

	virtual int GetSnapFullID() const;
	virtual void Tick();
	virtual void TickDefered();
	virtual void TickPaused();
	virtual void Snap(int SnappingClient);
	virtual void PostSnap();
	virtual bool Spawn(class CPlayer *pPlayer, vec2 Pos);
	virtual bool TakeDamage(vec2 Force, vec2 Source, int Dmg, int From, int Weapon);
	virtual void Die(int Killer, int Weapon);

	void OnPredictedInput(CNetObj_PlayerInput *pNewInput);
	void OnDirectInput(CNetObj_PlayerInput *pNewInput);
	void ResetInput();
	bool IsGrounded();


	bool IsAlive() const { return m_Alive; }
	void SetEvent(int EventID) { m_Event = EventID; };
	void SetEmote(int Emote, int Sec);
	void SetWeapon(int W);
	bool IncreaseHealth(int Amount);
	bool IncreaseMana(int Amount);
	bool CheckFailMana(int Mana);
	int Mana() const { return m_Mana; }
	int Health() const { return m_Health; }

	void CreateQuestsStep(int QuestID);
	bool GiveWeapon(int Weapon, int GiveAmmo);

	void CreateSnapProj(int SnapID, int Count, int TypeID, bool Dynamic, bool Projectile);
	void RemoveSnapProj(int Count, int SnapID, bool Effect = false);
	
	void CreateRandomDropItem(int DropCID, int Random, int ItemID, int Count, vec2 Force);
	void GiveRandomMobEffect(int FromID);
	void ChangePosition(vec2 NewPos);
	void ResetDoorPos();
	int GetPowerWeapons(int WeaponID);

	// these are non-heldback inputs
	CNetObj_PlayerInput m_Input;
	CNetObj_PlayerInput m_LatestPrevInput;
	CNetObj_PlayerInput m_LatestInput;

	// input
	int m_ActiveWeapon;
	int m_Jumped;

	// the player core for the physics
	CCharacterCore m_Core;

	// allow perm
	bool m_NoAllowDamage;
	int m_AmmoRegen;
	int m_ReloadTimer;

	vec2 m_OldPos;
	vec2 m_OlderPos;
	int m_DoorHit;

private:
	bool m_Alive;

	int m_Event;
	int m_Mana;

	// weapon info
	struct WeaponStat
	{
		int m_AmmoRegenStart;
		int m_Ammo;
		bool m_Got;

	} m_aWeapons[NUM_WEAPONS];

	int m_LastWeapon;
	int m_QueuedWeapon;

	int m_AttackTick;

	int m_EmoteType;
	int m_EmoteStop;

	// last tick that the player took any action ie some input
	int m_LastAction;
	int m_LastNoAmmoSound;
	int m_NumInputs;
	int m_TriggeredEvents;

	// info for dead reckoning
	int m_ReckoningTick; // tick that we are performing dead reckoning From
	CCharacterCore m_SendCore; // core that we should send
	CCharacterCore m_ReckoningCore; // the dead reckoning core

	bool TalkInteractiveHammer(CPlayer* pTarget);

protected:
	int m_Health;
	int m_Armor;
};

#endif
