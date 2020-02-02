/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#ifndef GAME_SERVER_SQLPLANTACC_H
#define GAME_SERVER_SQLPLANTACC_H
 
#include "../component.h"

class PlantsAccSql : public CMmoComponent
{
	struct StructPlants
	{
		int ItemID;
		int Level;
		int PositionX;
		int PositionY;
		int Distance;
	};
	typedef std::map < int , StructPlants > PlantsType;
	static PlantsType Plants;

public:
	virtual void OnInitLocal(const char *pLocal);
	virtual void OnInitAccount(CPlayer *pPlayer);

	int GetPlantLevel(vec2 Pos);
	int GetPlantItemID(vec2 Pos);

	void ShowMenu(int ClientID);
	void ShowPlantsItems(int ClientID);

	void Work(int ClientID, int Exp);
	virtual bool OnParseVotingMenu(CPlayer *pPlayer, const char *CMD, const int VoteID, const int VoteID2, int Get, const char *GetText);

private:
	int ExpNeed(int Level) const;
};

#endif