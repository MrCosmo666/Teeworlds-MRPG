/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#ifndef GAME_SERVER_ENTITIES_DROPINGITEMS_H
#define GAME_SERVER_ENTITIES_DROPINGITEMS_H

class CDropItem : public CEntity
{
	enum
	{
		NUM_IDS = 4,
	};
	int m_IDs[NUM_IDS];

	vec2 m_Vel;
	float m_Angle;
	float m_AngleForce;
	bool m_Flashing;
	int m_LifeSpan;
	int m_FlashTimer;

	InventoryItem m_DropItem;
	int m_OwnerID;

	

public:
	CDropItem(CGameWorld *pGameWorld, vec2 Pos, vec2 Vel, float AngleForce, InventoryItem DropItem, int OwnerID);
	~CDropItem();

	virtual void Tick();
	virtual void Snap(int SnappingClient);

	bool TakeItem(int ClientID);
};

#endif
