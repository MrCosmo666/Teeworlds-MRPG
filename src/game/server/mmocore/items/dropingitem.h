/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#ifndef GAME_SERVER_ENTITIES_DROPINGITEMS_H
#define GAME_SERVER_ENTITIES_DROPINGITEMS_H

class CDropingItem : public CEntity
{
	enum
	{
		PERSPECT = 1,
		BODY,
		NUM_IDS,
	};

	vec2 m_Vel;
	float m_Angle;
	float m_AngleForce;

	int m_StartTick;
	bool m_Flashing;
	int m_LifeSpan;
	int m_FlashTimer;

	ItemSql::ItemPlayer m_DropItem;
	int m_ForID;
	int m_IDs[NUM_IDS];

public:
	CDropingItem(CGameWorld *pGameWorld, vec2 Pos, vec2 Vel, float AngleForce, ItemSql::ItemPlayer DropItem, int ForID);
	~CDropingItem();

	virtual void Tick();
	virtual void TickPaused(); 
	virtual void Snap(int SnappingClient);

	bool TakeItem(int ClientID);
};

#endif
