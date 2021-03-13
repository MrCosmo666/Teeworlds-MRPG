/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#ifndef GAME_SERVER_TILE_HANDLE_CHARACTER_H
#define GAME_SERVER_TILE_HANDLE_CHARACTER_H
#include <game/mapitems.h>

class CCharacter;
class TileHandle
{
	CCharacter *m_pCharacter;
	bool m_Collide[MAX_TILES];

public:
	TileHandle(CCharacter *Character);

	// tiles
	bool TileEnter(int IndexPlayer, int IndexNeed);
	bool TileExit(int IndexPlayer, int IndexNeed);
	bool BoolIndex(int Index) const { return m_Collide[Index]; }

	// vec2 and other
	vec2 MousePos() const;
};

#endif
