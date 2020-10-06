/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#ifndef GAME_SERVER_SQLPLANTACC_H
#define GAME_SERVER_SQLPLANTACC_H
 
#include "../MmoComponent.h"

class AccountPlantJob : public MmoComponent
{
	struct StructPlants
	{
		int m_ItemID;
		int m_Level;
		int m_PositionX;
		int m_PositionY;
		int m_Distance;
	};
	static std::map < int, StructPlants > ms_aPlants;

public:
	virtual void OnInitWorld(const char* pWhereLocalWorld);
	virtual void OnInitAccount(CPlayer *pPlayer);

	int GetPlantLevel(vec2 Pos);
	int GetPlantItemID(vec2 Pos);

	void ShowMenu(int ClientID);
	void ShowPlantsItems(int ClientID);

	void Work(CPlayer* pPlayer, int Level);
	virtual bool OnVotingMenu(CPlayer *pPlayer, const char *CMD, const int VoteID, const int VoteID2, int Get, const char *GetText);
};

#endif