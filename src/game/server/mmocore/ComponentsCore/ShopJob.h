/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#ifndef GAME_SERVER_SQLSHOPMAIL_H
#define GAME_SERVER_SQLSHOPMAIL_H

#include "../MmoComponent.h"

class ShopJob : public MmoComponent
{
	struct StructAuctionItem
	{
		int m_ItemID;
		int m_Count;
		int m_Price;
		int m_Enchant;
	};

	struct ShopPersonal
	{
		int m_StorageID;
	};
	static std::map < int, ShopPersonal > ms_aShopList;

	void OnInit() override;
	void OnTick() override;
	bool OnHandleTile(CCharacter* pChr, int IndexCollision) override;
	bool OnHandleMenulist(CPlayer* pPlayer, int Menulist, bool ReplaceMenu) override;
	bool OnHandleVoteCommands(CPlayer* pPlayer, const char* CMD, const int VoteID, const int VoteID2, int Get, const char* GetText) override;

public:
	typedef StructAuctionItem AuctionSlot;
	void CreateAuctionSlot(CPlayer *pPlayer, AuctionSlot &pAuctionItem);
	void CheckAuctionTime();

private:
	bool BuyShopItem(CPlayer* pPlayer, int ID);
	void ShowAuction(CPlayer* pPlayer);
	void ShowMailShop(CPlayer* pPlayer, int StorageID);
};

#endif

