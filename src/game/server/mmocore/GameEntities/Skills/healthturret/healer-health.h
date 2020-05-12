/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#ifndef GAME_SERVER_ENTITIES_HEALTH_HEALER_H
#define GAME_SERVER_ENTITIES_HEALTH_HEALER_H

#include <game/server/entity.h>

class CHealthHealer : public CEntity
{
public:
	enum
	{
		NUM_IDS = 5,
	};
	
public:
	CHealthHealer(CGameWorld *pGameWorld, CPlayer* pPlayer, int SkillLevel, int PowerLevel, vec2 Pos);
	virtual ~CHealthHealer();

	virtual void Snap(int SnappingClient);
	virtual void Reset();
	virtual void Tick();

private:
	int m_IDs[NUM_IDS];
	int m_LifeSpan;	
	int m_ReloadTick;
	int m_PowerLevel;

public:
	CPlayer *m_pPlayer;

};

#endif
