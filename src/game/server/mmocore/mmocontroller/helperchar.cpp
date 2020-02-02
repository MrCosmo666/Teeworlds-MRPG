/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */

#include <game/server/entities/character.h>
#include "helperchar.h"

// CHelperCharacter
CHelperCharacter::CHelperCharacter(CCharacter *Character)
: m_pCharacter(Character)
{
	for(int i = 0; i < MAX_TILES; i++)
		InFuction[i] = false;
}

CHelperCharacter::~CHelperCharacter()
{}

// teles helper
bool CHelperCharacter::TileEnter(int IndexPlayer, int IndexNeed)
{
	if(IndexPlayer == IndexNeed && !InFuction[IndexNeed])
	{
		InFuction[IndexNeed] = true;
		return true;
	}
	return false;
}
bool CHelperCharacter::TileExit(int IndexPlayer, int IndexNeed)
{
	if(IndexPlayer != IndexNeed && InFuction[IndexNeed])
	{
		InFuction[IndexNeed] = false;
		return true;
	}
	return false;
}

// vec2 helper and position
vec2 CHelperCharacter::MousePos() const
{
	vec2 Direction = vec2(m_pCharacter->m_Core.m_Input.m_TargetX, m_pCharacter->m_Core.m_Input.m_TargetY);
	return m_pCharacter->m_Core.m_Pos+Direction;	
}