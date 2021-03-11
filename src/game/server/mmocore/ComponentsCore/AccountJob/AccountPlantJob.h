/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#ifndef GAME_SERVER_SQLPLANTACC_H
#define GAME_SERVER_SQLPLANTACC_H

#include <game/server/mmocore/MmoComponent.h>

class AccountPlantJob : public MmoComponent
{
	~AccountPlantJob()
	{
		ms_aPlants.clear();
	};

	struct StructPlants
	{
		int m_ItemID;
		int m_Level;
		int m_PositionX;
		int m_PositionY;
		int m_Distance;
	};
	static std::map < int, StructPlants > ms_aPlants;

	void OnInitWorld(const char* pWhereLocalWorld) override;
	void OnInitAccount(CPlayer* pPlayer) override;
	bool OnHandleVoteCommands(CPlayer* pPlayer, const char* CMD, const int VoteID, const int VoteID2, int Get, const char* GetText) override;

public:
	int GetPlantLevel(vec2 Pos) const;
	int GetPlantItemID(vec2 Pos) const;

	void ShowMenu(CPlayer* pPlayer);
	void ShowPlantsItems(int ClientID);

	void Work(CPlayer* pPlayer, int Level);
};

#endif