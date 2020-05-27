/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#ifndef GAME_SERVER_ENTITIES_SLEEPYGRAVITY_H
#define GAME_SERVER_ENTITIES_SLEEPYGRAVITY_H

#include <game/server/entity.h>

class CNoctisTeleport : public CEntity
{
public:
	CNoctisTeleport(CGameWorld *pGameWorld, CCharacter* pPlayerChar, int SkillBonus, int PowerLevel);
	virtual ~CNoctisTeleport();

	virtual void Snap(int SnappingClient);
	virtual void Reset();
	virtual void Tick();

private:
	int m_LifeSpan;
	int m_Radius;
	int m_PowerLevel;
	vec2 m_Direction;

public:
	CCharacter *m_pPlayerChar;

};

#endif
