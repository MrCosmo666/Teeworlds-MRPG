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

	vec2 m_ActualPos;
	vec2 m_ActualDir;
	vec2 m_Direction;
	int m_StartTick;
	bool m_Flashing;
	int m_LifeSpan;
	int m_FlashTimer;

	ItemSql::ItemPlayer m_DropItem;
	int m_ForID;
	int m_IDs[NUM_IDS];

	vec2 GetTimePos(float Time);
public:
	CDropingItem(CGameWorld *pGameWorld, vec2 Pos, vec2 Dir, ItemSql::ItemPlayer DropItem, int ForID);
	virtual ~CDropingItem();

	virtual void Tick();
	virtual void TickPaused(); 
	virtual void Snap(int SnappingClient);

	bool TakeItem(int ClientID);
};

#endif
