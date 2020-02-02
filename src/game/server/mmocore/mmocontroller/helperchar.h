/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#ifndef GAME_SERVER_HELPER_CHARACTER_H
#define GAME_SERVER_HELPER_CHARACTER_H

#include <game/mapitems.h>

class CHelperCharacter
{
private:
	CCharacter *m_pCharacter;
	CCharacter *Character() const { return m_pCharacter; }
	bool InFuction[MAX_TILES];

public:
	CHelperCharacter(CCharacter *Character);
	~CHelperCharacter();

	// tiles
	bool TileEnter(int IndexPlayer, int IndexNeed);
	bool TileExit(int IndexPlayer, int IndexNeed);
	bool BoolIndex(int Index) const { return InFuction[Index]; }

	// vec2 and other
	vec2 MousePos() const;
};

#endif
