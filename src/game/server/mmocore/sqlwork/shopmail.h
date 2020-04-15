/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#ifndef GAME_SERVER_SQLSHOPMAIL_H
#define GAME_SERVER_SQLSHOPMAIL_H

#include "../component.h"

class ShopMailSql : public CMmoComponent
{
	bool BuyShopAuctionSlot(CPlayer *pPlayer, int ID);

	struct StructAuctionItem
	{
		int a_itemid;
		int a_count;
		int a_price;
		int a_enchant;
	};

	struct ShopPersonal
	{
		int StorageID;
	};
	typedef std::map < int , ShopPersonal > ShopType;
	static ShopType Shop;

	bool CheckCurrentStorage(int StorageID);


public:
	typedef StructAuctionItem AuctionItem;
	
	virtual void OnInitGlobal();
	virtual void OnTickLocalWorld();

	void ShowMailShop(CPlayer *pPlayer, int StorageID);

	void ShowAuction(CPlayer *pPlayer);
	void CreateAuctionSlot(CPlayer *pPlayer, AuctionItem &AuSellItem);
	void CheckAuctionTime();
	virtual bool OnParseVotingMenu(CPlayer *pPlayer, const char *CMD, const int VoteID, const int VoteID2, int Get, const char *GetText);
	virtual bool OnPlayerHandleMainMenu(CPlayer* pPlayer, int Menulist);
};

#endif

