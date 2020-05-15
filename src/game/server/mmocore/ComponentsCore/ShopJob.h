/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#ifndef GAME_SERVER_SQLSHOPMAIL_H
#define GAME_SERVER_SQLSHOPMAIL_H

#include "../MmoComponent.h"

class ShopJob : public MmoComponent
{
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

	bool BuyShopItem(CPlayer* pPlayer, int ID);
	void ShowAuction(CPlayer* pPlayer);
	void ShowMailShop(CPlayer* pPlayer, int StorageID);

public:
	typedef StructAuctionItem AuctionSlot;

	virtual void OnInit();
	virtual void OnTick();
	virtual bool OnHandleTile(CCharacter* pChr, int IndexCollision);

	void CreateAuctionSlot(CPlayer *pPlayer, AuctionSlot &AuSellItem);
	void CheckAuctionTime();
	virtual bool OnVotingMenu(CPlayer *pPlayer, const char *CMD, const int VoteID, const int VoteID2, int Get, const char *GetText);
	virtual bool OnHandleMenulist(CPlayer* pPlayer, int Menulist, bool ReplaceMenu);
};

#endif

