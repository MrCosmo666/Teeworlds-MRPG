/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#ifndef GAME_SERVER_ENTITIES_SLEEPYGRAVITY_H
#define GAME_SERVER_ENTITIES_SLEEPYGRAVITY_H

#include <game/server/entity.h>

class CSleepyGravity : public CEntity
{
public:
	enum
	{
		NUM_IDS = 12,
	};
	
public:
	CSleepyGravity(CGameWorld *pGameWorld, CPlayer *pPlayer, int SkillLevel, int ManaUseCost, vec2 Pos);
	virtual ~CSleepyGravity();

	virtual void Snap(int SnappingClient);
	virtual void Reset();
	virtual void Tick();

private:
	int m_IDs[NUM_IDS];
	int m_LifeSpan;	
	int m_SkillLevel;
	int m_ManaUseCost;

public:
	CPlayer *m_pPlayer;

};

#endif
