#ifndef GAME_SERVER_ENTITIES_DECORATIONS_HOUSES_H
#define GAME_SERVER_ENTITIES_DECORATIONS_HOUSES_H

#include <game/server/entity.h>

class DecoHouse : public CEntity
{
	enum
	{
		PERSPECT = 1,
		BODY,
		NUM_IDS,
	};
	int m_IDs[NUM_IDS];

	int SwitchToObject(bool Data);
public:
	int m_DecoID;
	int m_HouseID;

	DecoHouse(CGameWorld* pGameWorld, vec2 Pos, int OwnerID, int DecoID);
	~DecoHouse();

	virtual void Tick();
	virtual void Snap(int SnappingClient);
};

#endif