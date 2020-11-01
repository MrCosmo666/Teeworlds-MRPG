/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#ifndef GAME_SERVER_HOUSESJOB_H
#define GAME_SERVER_HOUSESJOB_H

#include <game/server/mmocore/GameEntities/decoration_houses.h>
#include "../MmoComponent.h"

class HouseDoor;
class CDecorationHouses;
class HouseJob : public MmoComponent
{
	/* #########################################################################
		VAR AND OBJECTS HOUSES 
	######################################################################### */
	struct HouseList
	{
		int m_PosX;
		int m_PosY;
		int m_DoorX;
		int m_DoorY;
		int m_Price;
		char m_aClass[32];
		int m_OwnerID;
		int m_Bank;
		int m_PlantID;
		int m_PlantPosX;
		int m_PlantPosY;
		int m_WorldID;

		HouseDoor *m_Door;
	};
	static std::map < int, HouseList > ms_aHouse;
	std::map < int , CDecorationHouses * > ms_aDecorationHouse;

	void OnInitWorld(const char* pWhereLocalWorld) override;
	bool OnHandleTile(CCharacter* pChr, int IndexCollision) override;
	bool OnHandleMenulist(CPlayer* pPlayer, int Menulist, bool ReplaceMenu) override;
	bool OnHandleVoteCommands(CPlayer* pPlayer, const char* CMD, const int VoteID, const int VoteID2, int Get, const char* GetText) override;

	/* #########################################################################
		FUNCTIONS HOUSES PLANTS
	######################################################################### */
	void ChangePlantsID(int HouseID, int PlantID);

public:
	/* #########################################################################
		FUNCTIONS HOUSES DECORATION
	######################################################################### */
	bool AddDecorationHouse(int DecoID, int OwnerID, vec2 Position);

private:
	bool DeleteDecorationHouse(int ID);
	void ShowDecorationList(CPlayer *pPlayer);

	/* #########################################################################
		MENUS HOUSES 
	######################################################################### */
	void ShowHouseMenu(CPlayer* pPlayer, int HouseID);
	void ShowPersonalHouse(CPlayer* pPlayer);

	/* #########################################################################
		GET CHECK HOUSES 
	######################################################################### */
public:
	bool IsHouseHasOwner(int HouseID) const;
	int GetHouse(vec2 Pos, bool Plants = false) const;
	int GetHousePrice(int HouseID) const;
	bool GetHouseDoor(int HouseID) const;
	vec2 GetPositionHouse(int HouseID) const;
	int PlayerHouseID(CPlayer *pPlayer) const;
	int OwnerHouseID(int AuthID) const;
	int GetPlantsID(int HouseID) const;

	const char *ClassName(int HouseID) const;
	const char *OwnerName(int HouseID);

	/* #########################################################################
		FUNCTIONS HOUSES 
	######################################################################### */
	void BuyHouse(int HouseID, CPlayer *pPlayer);
	void SellHouse(int HouseID);

	void TakeFromSafeDeposit(CPlayer *pPlayer, int TakeCount);
	void AddSafeDeposit(CPlayer *pPlayer, int Balance);
	
	bool ChangeStateDoor(int HouseID);
};

// - - - - - - - - - - - - - - DOOR HOUSES - - - - - - - - - - - - - - 
class HouseDoor : public CEntity
{
public:
	HouseDoor(CGameWorld *pGameWorld, vec2 Pos);

	virtual void Tick();
	virtual void Snap(int SnappingClient);
};

#endif