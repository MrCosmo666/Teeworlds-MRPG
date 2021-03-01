/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#ifndef GAME_MMO_DATA_STRUCTURES_H
#define GAME_MMO_DATA_STRUCTURES_H

enum CDataList
{
	MMO_DATA_INVENTORY_INFORMATION = 0,
};

class CItemDataInformation
{
public:
	char m_aName[32];
	char m_aDesc[64];
	char m_aIcon[16];
	int m_Type;
	int m_Function;
	int m_Dysenthis;
	int m_MinimalPrice;
	int m_aAttribute[2];
	int m_aAttributeCount[2];
	int m_ProjID;
};

#endif
