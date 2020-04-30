/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#ifndef GAME_SERVER_ENTITIES_JOBITEMS_H
#define GAME_SERVER_ENTITIES_JOBITEMS_H

#include <game/server/entity.h>

const int PickupPhysSize = 14;

class CJobItems : public CEntity
{
public:
	CJobItems(CGameWorld *pGameWorld, int ItemID, int Level, vec2 Pos, int Type, int Health, int HouseID = -1);

	virtual void Reset();
	virtual void Tick();
	virtual void TickPaused();
	virtual void Snap(int SnappingClient);

	void SetSpawn(int Sec);
	void Work(int ClientID);
	void SpawnPositions();

	int m_ItemID;
	int m_HouseID;
private:
	int m_Level;
	int m_Progress;
	int m_Health;
	int m_SpawnTick;
	int m_Type;

	int SwitchToObject(bool MmoItem);
};

#endif
