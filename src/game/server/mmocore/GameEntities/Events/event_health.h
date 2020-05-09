#ifndef GAME_SERVER_ENTITIES_EVENT_HEALTH_H
#define GAME_SERVER_ENTITIES_EVENT_HEALTH_H

#include <game/server/entity.h>

class CNurseHealthNPC : public CEntity
{
	int m_LifeTime;
	int m_OwnerID;

public:
	CNurseHealthNPC(CGameWorld* pGameWorld, int ClientID, vec2 Pos);

	virtual void Tick();
	virtual void Snap(int SnappingClient);
};

#endif