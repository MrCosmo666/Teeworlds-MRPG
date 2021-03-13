/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#ifndef GAME_SERVER_COMPONENT_CRAFT_DATA_H
#define GAME_SERVER_COMPONENT_CRAFT_DATA_H

struct CCraftData
{
	int m_aItemNeedID[3];
	int	m_aItemNeedCount[3];
	int m_ReceivedItemID;
	int m_ReceivedItemCount;
	int m_Price;
	int m_WorldID;

	static std::map <int, CCraftData> ms_aCraft;
};

#endif