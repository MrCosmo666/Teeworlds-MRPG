/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */

#include <game/server/entities/character.h>
#include "TileHandle.h"

TileHandle::TileHandle(CCharacter *Character)
: m_pCharacter(Character)
{
	for(bool & i : m_Collide)
		i = false;
}

bool TileHandle::TileEnter(int IndexPlayer, int IndexNeed)
{
	if(IndexPlayer == IndexNeed && !m_Collide[IndexNeed])
	{
		m_Collide[IndexNeed] = true;
		return true;
	}
	return false;
}
bool TileHandle::TileExit(int IndexPlayer, int IndexNeed)
{
	if(IndexPlayer != IndexNeed && m_Collide[IndexNeed])
	{
		m_Collide[IndexNeed] = false;
		return true;
	}
	return false;
}

vec2 TileHandle::MousePos() const
{
	vec2 Direction = vec2(m_pCharacter->m_Core.m_Input.m_TargetX, m_pCharacter->m_Core.m_Input.m_TargetY);
	return m_pCharacter->m_Core.m_Pos+Direction;	
}