/* (c) Alexandre Díaz. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#ifndef GAME_SERVER_BOTAI_HELPER_H
#define GAME_SERVER_BOTAI_HELPER_H

#include <map>

#include <game/server/entity.h>

#include <game/gamecore.h>
#include "../character.h"

class BotAI : public CCharacter
{
	MACRO_ALLOC_POOL_ID()

public:
	BotAI(CGameWorld *pWorld);
	~BotAI();

	int GetBotTarget() const { return m_BotTargetID; };

private: 
	virtual bool Spawn(class CPlayer *pPlayer, vec2 Pos);
	virtual void Tick();
	virtual bool TakeDamage(vec2 Force, vec2 Source, int Dmg, int From, int Weapon);
	virtual void Die(int Killer, int Weapon);
	virtual int GetSnapFullID() const;

	int m_BotTick;
	int m_BotTargetID;
	
	int m_StartHealth;
	int m_HookTick;
	bool m_MessagePlayers[MAX_CLIENTS];

	std::map < int , int > m_ListDmgPlayers;

	void FindHardHealth();
	void ClearTarget();
    void EngineBots();
	void ChangeWeapons();
	void ShowProgress();

	void EngineNPC();
	void EngineMobs();
	void EngineQuestMob();
	// void HandleHook();

	CPlayer *SearchPlayer(int Distance);
    CPlayer *SearchTenacityPlayer(float Distance);

	void TalkingBots(const char *Text);

	int GetEmoticon(int EmoteEyes) const;

	int GetMoodType() const;
};

#endif