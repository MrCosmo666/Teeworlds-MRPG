/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#ifndef GAME_SERVER_ENTITIES_DROPINGBONUSES_H
#define GAME_SERVER_ENTITIES_DROPINGBONUSES_H

class CDropingBonuses : public CEntity
{
	vec2 m_ActualPos;
	vec2 m_ActualDir;
	vec2 m_Direction;
	int m_StartTick;
	int m_Type;
	bool m_Flashing;
	int m_LifeSpan;
	int m_FlashTimer;
	int m_Count;

	vec2 GetTimePos(float Time);

public:
	CDropingBonuses(CGameWorld *pGameWorld, vec2 Pos, vec2 Dir, int Type, int Count);

	virtual void Tick();
	virtual void TickPaused(); 
	virtual void Snap(int SnappingClient);

	vec2 GetPos() const { return m_ActualPos; }
};

#endif
