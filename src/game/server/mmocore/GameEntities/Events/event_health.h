#ifndef GAME_SERVER_ENTITIES_EVENT_HEALTH_H
#define GAME_SERVER_ENTITIES_EVENT_HEALTH_H

#include <game/server/entity.h>

class CEventHealth : public CEntity
{
	int m_LifeTime;

public:
	CEventHealth(CGameWorld* pGameWorld, vec2 Pos);

	virtual void Tick();
	virtual void Snap(int SnappingClient);
};

#endif