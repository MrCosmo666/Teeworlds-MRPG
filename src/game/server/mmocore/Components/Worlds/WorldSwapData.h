/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#ifndef GAME_SERVER_COMPONENT_WORLDSWAP_DATA_H
#define GAME_SERVER_COMPONENT_WORLDSWAP_DATA_H

#include <list>
#include <map>

struct CWorldSwapData
{
	int m_RequiredQuestID;
	int m_PositionX;
	int m_PositionY;
	int m_WorldID;
	int m_TwoPositionX;
	int m_TwoPositionY;
	int m_TwoWorldID;

	static std::map< int, CWorldSwapData > ms_aWorldSwap;
};
struct CWorldSwapPosition
{
	int m_BaseWorldID;
	int m_FindWorldID;
	vec2 m_Position;

	static std::list< CWorldSwapPosition > ms_aWorldPositionLogic;
};

#endif