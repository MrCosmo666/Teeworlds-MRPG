/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#ifndef GAME_SERVER_SQLEATSHOP_H
#define GAME_SERVER_SQLEATSHOP_H

#include "../component.h"

class ShopEatSql : public CMmoComponent
{
/* #########################################################################
	VAR AND OBJECTS SHOP EAT 
######################################################################### */
	struct StructShopEat
	{
		char EatName[32];
		int Eat;
		int Price;
		int StorageID;
	};
	typedef std::map < int , StructShopEat > ShopEatType;
	static ShopEatType ShopEat;

public:
	virtual void OnInitGlobal();

private:
/* #########################################################################
	GET CHECK SHOP EAT
######################################################################### */
	bool CheckShopEatStorage(int StorageID);

/* #########################################################################
	FUNCTIONS SHOP EAT 
######################################################################### */
	void BuyEatItem(CPlayer *pPlayer, int EatID);

public:
	void ShowListShopEat(CPlayer *pPlayer, int StorageID);
	virtual bool OnParseVotingMenu(CPlayer *pPlayer, const char *CMD, const int VoteID, const int VoteID2, int Get, const char *GetText);
};

#endif