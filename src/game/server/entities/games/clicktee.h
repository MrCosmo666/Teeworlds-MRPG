/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#ifndef GAME_SERVER_ENTITIES_CLICKTEE_H
#define GAME_SERVER_ENTITIES_CLICKTEE_H

#include <game/server/entity.h>

class ClickTee : public CEntity
{
	enum
	{
		NUM_SIDE = 5,
		NUM_IDS,
	};

	int m_IDs[NUM_IDS];
	short m_ReloadTick;
	int m_Health;
	bool m_Spawn;

public:

	ClickTee(CGameWorld *pGameWorld, vec2 Pos);
	~ClickTee();

	virtual void Tick();
	virtual void TickPaused();
	virtual void Snap(int SnappingClient);
	
	bool TakeDamage(int Damage, int ClientID, vec2 Force);
	bool IsSpawned() const { return m_Spawn; }

	void SpawnedClick();
	vec2 GetPos() const { return m_Pos; }

};

#endif
