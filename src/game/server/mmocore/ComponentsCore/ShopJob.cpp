/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#include <engine/shared/config.h>
#include <game/server/gamecontext.h>
#include "ShopJob.h"

using namespace sqlstr;
std::map < int , ShopJob::ShopPersonal > ShopJob::ms_aShopList;

void ShopJob::OnInit()
{
	std::shared_ptr<ResultSet> RES(SJK.SD("ID, StorageID", "tw_mailshop"));
	while(RES->next())
	{
		int ID = RES->getInt("ID");
		ms_aShopList[ID].m_StorageID = RES->getInt("StorageID");
	}
}

void ShopJob::OnTick()
{
	if(GS()->GetWorldID() == LOCAL_WORLD)
	{
		if(GS()->Server()->Tick() % (1 * GS()->Server()->TickSpeed() * (g_Config.m_SvTimeCheckAuction * 60)) == 0)
			CheckAuctionTime();
	}
}

bool ShopJob::OnHandleTile(CCharacter* pChr, int IndexCollision)
{
	CPlayer* pPlayer = pChr->GetPlayer();
	const int ClientID = pPlayer->GetCID();
	if (pChr->GetHelper()->TileEnter(IndexCollision, TILE_AUCTION))
	{
		GS()->Chat(ClientID, "You can see menu in the votes!");
		pChr->m_Core.m_ProtectHooked = pChr->m_NoAllowDamage = true;
		GS()->ResetVotes(ClientID, pPlayer->m_OpenVoteMenu);
		return true;
	}
	else if (pChr->GetHelper()->TileExit(IndexCollision, TILE_AUCTION))
	{
		GS()->Chat(ClientID, "You left the active zone, menu is restored!");
		pChr->m_Core.m_ProtectHooked = pChr->m_NoAllowDamage = false;
		GS()->ResetVotes(ClientID, pPlayer->m_OpenVoteMenu);
		return true;
	}

	return false;
}

void ShopJob::ShowMailShop(CPlayer *pPlayer, int StorageID)
{
	const int ClientID = pPlayer->GetCID();
	int HideID = NUM_TAB_MENU + InventoryJob::ms_aItemsInfo.size() + 300;
	std::shared_ptr<ResultSet> RES(SJK.SD("*", "tw_mailshop", "WHERE StorageID = '%d' ORDER BY Price", StorageID));
	while(RES->next())
	{
		const int ID = RES->getInt("ID");
		const int ItemID = RES->getInt("ItemID");
		const int Price = RES->getInt("Price");
		const int Enchant = RES->getInt("Enchant");
		const int Count = RES->getInt("Count");
		const int NeedItemID = RES->getInt("NeedItem");
		ItemInformation &BuyightItem = GS()->GetItemInfo(ItemID);
		ItemInformation &NeededItem = GS()->GetItemInfo(NeedItemID);
		
		if (BuyightItem.IsEnchantable())
		{
			char aEnchantBuf[16];
			BuyightItem.FormatEnchantLevel(aEnchantBuf, sizeof(aEnchantBuf), Enchant);
			GS()->AVHI(ClientID, BuyightItem.GetIcon(), HideID, LIGHT_GRAY_COLOR, "{STR}{STR} {STR} - {INT} {STR}",
				(pPlayer->GetItem(ItemID).m_Count > 0 ? "✔ " : "\0"), BuyightItem.GetName(pPlayer), (Enchant > 0 ? aEnchantBuf : "\0"), &Price, NeededItem.GetName(pPlayer));

			char aAttributes[128];
			Job()->Item()->FormatAttributes(BuyightItem, Enchant, sizeof(aAttributes), aAttributes);
			GS()->AVM(ClientID, "null", NOPE, HideID, "{STR}", aAttributes);
		}
		else
		{
			GS()->AVHI(ClientID, BuyightItem.GetIcon(), HideID, LIGHT_GRAY_COLOR, "{STR}x{INT} ({INT}) - {INT} {STR}",
				BuyightItem.GetName(pPlayer), &Count, &pPlayer->GetItem(ItemID).m_Count, &Price, NeededItem.GetName(pPlayer));
		}

		GS()->AVM(ClientID, "null", NOPE, HideID, "{STR}", BuyightItem.GetDesc(pPlayer));
		GS()->AVM(ClientID, "SHOP", ID, HideID, "Exchange {STR}x{INT} to {STR}x{INT}", NeededItem.GetName(pPlayer), &Price, BuyightItem.GetName(pPlayer), &Count);
		HideID++;
	}
	GS()->AV(ClientID, "null", "");
}

// show auction
void ShopJob::ShowAuction(CPlayer *pPlayer)
{
	const int ClientID = pPlayer->GetCID();
	GS()->AVH(ClientID, TAB_INFO_AUCTION, GREEN_COLOR, "Auction Information");
	GS()->AVM(ClientID, "null", NOPE, TAB_INFO_AUCTION, "To create a slot, see inventory item interact.");
	GS()->AV(ClientID, "null", "");
	GS()->ShowItemValueInformation(pPlayer);
	GS()->AV(ClientID, "null", "");

	bool FoundItems = false;
	int HideID = (int)(NUM_TAB_MENU + InventoryJob::ms_aItemsInfo.size() + 400);
	std::shared_ptr<ResultSet> RES(SJK.SD("*", "tw_mailshop", "WHERE OwnerID > 0 ORDER BY Price"));
	while(RES->next())
	{
		const int ID = RES->getInt("ID");
		const int ItemID = RES->getInt("ItemID");
		const int Price = RES->getInt("Price");
		const int Enchant = RES->getInt("Enchant");
		const int Count = RES->getInt("Count");
		const int OwnerID = RES->getInt("OwnerID");
		ItemInformation &BuyightItem = GS()->GetItemInfo(ItemID);

		if (BuyightItem.IsEnchantable())
		{
			char aEnchantBuf[16];
			BuyightItem.FormatEnchantLevel(aEnchantBuf, sizeof(aEnchantBuf), Enchant);
			GS()->AVHI(ClientID, BuyightItem.GetIcon(), HideID, LIGHT_GRAY_COLOR, "{STR}{STR} {STR} - {INT} gold",
				(pPlayer->GetItem(ItemID).m_Count > 0 ? "✔ " : "\0"), BuyightItem.GetName(pPlayer), (Enchant > 0 ? aEnchantBuf : "\0"), &Price);

			char aAttributes[128];
			Job()->Item()->FormatAttributes(BuyightItem, Enchant, sizeof(aAttributes), aAttributes);
			GS()->AVM(ClientID, "null", NOPE, HideID, "{STR}", aAttributes);
		}
		else
		{
			GS()->AVHI(ClientID, BuyightItem.GetIcon(), HideID, LIGHT_GRAY_COLOR, "{STR}x{INT} ({INT}) - {INT} gold",
				BuyightItem.GetName(pPlayer), &Count, &pPlayer->GetItem(ItemID).m_Count, &Price);
		}

		GS()->AVM(ClientID, "null", NOPE, HideID, "{STR}", BuyightItem.GetDesc(pPlayer));
		GS()->AVM(ClientID, "null", NOPE, HideID, "Seller {STR}", Job()->PlayerName(OwnerID));
		GS()->AVM(ClientID, "SHOP", ID, HideID, "Buy Price {INT} gold", &Price);
		FoundItems = true;
		++HideID;
	}
	if(!FoundItems)
		GS()->AVL(ClientID, "null", "Currently there are no products.");
		
	GS()->AV(ClientID, "null", "");
}

void ShopJob::CreateAuctionSlot(CPlayer *pPlayer, AuctionSlot& AuSellItem)
{
	const int ItemID = AuSellItem.m_ItemID;
	const int ClientID = pPlayer->GetCID();
	InventoryItem &pPlayerAuctionItem = pPlayer->GetItem(ItemID);

	// check the number of slots whether everything is occupied or not
	std::shared_ptr<ResultSet> RES(SJK.SD("ID", "tw_mailshop", "WHERE OwnerID > '0' LIMIT %d", g_Config.m_SvMaxMasiveAuctionSlots));
	if((int)RES->rowsCount() >= g_Config.m_SvMaxMasiveAuctionSlots)
		return GS()->Chat(ClientID, "Auction has run out of slots, wait for the release of slots!");

	// check your slots
	std::shared_ptr<ResultSet> RES2(SJK.SD("ID", "tw_mailshop", "WHERE OwnerID = '%d' LIMIT %d", pPlayer->Acc().m_AuthID, g_Config.m_SvMaxAuctionSlots));
	const int CountSlot = RES2->rowsCount();
	if(CountSlot >= g_Config.m_SvMaxAuctionSlots)
		return GS()->Chat(ClientID, "You use all open the slots in your auction!");

	// we check if the item is in the auction
	std::shared_ptr<ResultSet> RES3(SJK.SD("ID", "tw_mailshop", "WHERE ItemID = '%d' AND OwnerID = '%d'", ItemID, pPlayer->Acc().m_AuthID));
	if(RES3->next()) 
		return GS()->Chat(ClientID, "Your same item found in the database, need reopen the slot!");

	// if the money for the slot auction is withdrawn
	if(pPlayer->CheckFailMoney(g_Config.m_SvAuctionPriceSlot))	
		return;

	// pick up the item and add a slot
	if(pPlayerAuctionItem.m_Count >= AuSellItem.m_Count && pPlayerAuctionItem.Remove(AuSellItem.m_Count))
	{
		SJK.ID("tw_mailshop", "(ItemID, Price, Count, OwnerID, Enchant) VALUES ('%d', '%d', '%d', '%d', '%d')", 
			ItemID, AuSellItem.m_Price, AuSellItem.m_Count, pPlayer->Acc().m_AuthID, AuSellItem.m_Enchant);

		const int AvailableSlot = (g_Config.m_SvMaxAuctionSlots - CountSlot) - 1;
		GS()->Chat(-1, "{STR} created a slot [{STR}x{INT}] auction.", 
			GS()->Server()->ClientName(ClientID), pPlayerAuctionItem.Info().GetName(pPlayer), &AuSellItem.m_Count);
		GS()->ChatFollow(ClientID, "Still available {INT} slots!", &AvailableSlot);
	}
}

bool ShopJob::BuyShopItem(CPlayer* pPlayer, int ID)
{
	const int ClientID = pPlayer->GetCID();
	std::shared_ptr<ResultSet> SHOPITEM(SJK.SD("*", "tw_mailshop", "WHERE ID = '%d'", ID));
	if (!SHOPITEM->next())
		return false;

	const int ItemID = SHOPITEM->getInt("ItemID");
	InventoryItem &pPlayerBuyightItem = pPlayer->GetItem(ItemID);
	if (pPlayerBuyightItem.m_Count > 0 && pPlayerBuyightItem.Info().IsEnchantable())
	{
		GS()->Chat(ClientID, "Enchant item maximal count x1 in a backpack!");
		return false;
	}

	// - - - - - - - - - - AUCTION - - - - - - - - - - - - -
	int Price = SHOPITEM->getInt("Price");
	const int OwnerID = SHOPITEM->getInt("OwnerID");
	const int Count = SHOPITEM->getInt("Count");
	const int Enchant = SHOPITEM->getInt("Enchant");
	if (OwnerID > 0)
	{
		// take out your slot
		if (OwnerID == pPlayer->Acc().m_AuthID)
		{
			GS()->Chat(ClientID, "You closed auction slot!");
			GS()->SendInbox(ClientID, "Auction Alert", "You have bought a item, or canceled your slot", ItemID, Count, Enchant);
			SJK.DD("tw_mailshop", "WHERE ItemID = '%d' AND OwnerID = '%d'", ItemID, OwnerID);
			return true;
		}

		const int NeedItem = SHOPITEM->getInt("NeedItem");
		if (pPlayer->CheckFailMoney(Price, NeedItem))
			return false;

		char aBuf[128];
		str_format(aBuf, sizeof(aBuf), "Your [Slot %sx%d] was sold!", pPlayerBuyightItem.Info().GetName(pPlayer), Count);
		Job()->Inbox()->SendInbox(OwnerID, "Auction Sell", aBuf, itGold, Price, 0);
		SJK.DD("tw_mailshop", "WHERE ItemID = '%d' AND OwnerID = '%d'", ItemID, OwnerID);
		pPlayerBuyightItem.Add(Count, 0, Enchant);
		GS()->Chat(ClientID, "You buy {STR}x{INT}.", pPlayerBuyightItem.Info().GetName(pPlayer), &Count);
		return true;
	}

	// - - - - - - - - - - - -SHOP - - - - - - - - - - - - -
	const int NeedItem = SHOPITEM->getInt("NeedItem");
	if (pPlayer->CheckFailMoney(Price, NeedItem))
		return false;

	pPlayerBuyightItem.Add(Count, 0, Enchant);
	GS()->Chat(ClientID, "You exchange {STR}x{INT} to {STR}x{INT}.", pPlayerBuyightItem.Info().GetName(pPlayer), &Count, GS()->GetItemInfo(NeedItem).GetName(pPlayer), &Price);
	return true;
}

void ShopJob::CheckAuctionTime()
{
	std::shared_ptr<ResultSet> RES(SJK.SD("*", "tw_mailshop", "WHERE OwnerID > 0 AND DATE_SUB(NOW(),INTERVAL %d MINUTE) > Time", g_Config.m_SvTimeAuctionSlot));
	int ReleaseSlots = (int)RES->rowsCount();
	while(RES->next())
	{
		const int ID = RES->getInt("ID");
		const int ItemID = RES->getInt("ItemID");
		const int Count = RES->getInt("Count");
		const int Enchant = RES->getInt("Enchant");
		const int OwnerID = RES->getInt("OwnerID");
		Job()->Inbox()->SendInbox(OwnerID, "Auction expired", "Your slot has expired", ItemID, Count, Enchant);
		SJK.DD("tw_mailshop", "WHERE ID = '%d'", ID);
	}
	if(ReleaseSlots) 
		GS()->ChatFollow(-1, "Auction {INT} slots has been released!", &ReleaseSlots);
}

bool ShopJob::OnHandleMenulist(CPlayer* pPlayer, int Menulist, bool ReplaceMenu)
{
	const int ClientID = pPlayer->GetCID();
	if (ReplaceMenu)
	{
		CCharacter* pChr = pPlayer->GetCharacter();
		if (!pChr || !pChr->IsAlive())
			return false;

		if (pChr->GetHelper()->BoolIndex(TILE_AUCTION))
		{
			ShowAuction(pPlayer);
			return true;
		}

		if (pChr->GetHelper()->BoolIndex(TILE_SHOP_ZONE))
		{
			const int StorageID = Job()->Storage()->GetStorageID(pChr->m_Core.m_Pos);
			Job()->Storage()->ShowStorageMenu(pChr->GetPlayer(), StorageID);
			ShowMailShop(pPlayer, StorageID);
			return true;
		}
		return false;
	}


	if (Menulist == MenuList::MENU_AUCTION_CREATE_SLOT)
	{
		pPlayer->m_LastVoteMenu = MenuList::MENU_INVENTORY;
		const int ItemID = pPlayer->GetTempData().m_SellItem.m_ItemID;
		ItemInformation& pInformationSellItem = GS()->GetItemInfo(ItemID);

		const int SlotCount = pPlayer->GetTempData().m_SellItem.m_Count;
		const int MinimalPrice = SlotCount * pInformationSellItem.m_MinimalPrice;
		const int SlotPrice = pPlayer->GetTempData().m_SellItem.m_Price;
		const int SlotEnchant = pPlayer->GetTempData().m_SellItem.m_Enchant;

		GS()->AVH(ClientID, TAB_INFO_AUCTION_BIND, GREEN_COLOR, "Information Auction Slot");
		GS()->AVM(ClientID, "null", NOPE, TAB_INFO_AUCTION_BIND, "The reason for write the number for each row");
		pPlayer->m_Colored = { 15,15,15 };
		GS()->AVM(ClientID, "null", NOPE, NOPE, "Item x{INT} Minimal Price: {INT}gold", &SlotCount, &MinimalPrice);
		GS()->AVM(ClientID, "null", NOPE, NOPE, "Auction Slot Price: {INT}gold", &g_Config.m_SvAuctionPriceSlot);
		if (SlotEnchant > 0)
			GS()->AVM(ClientID, "null", NOPE, NOPE, "Warning selling enchanted: +{INT}", &SlotEnchant);

		GS()->AVM(ClientID, "AUCTIONCOUNT", ItemID, NOPE, "Item Count: {INT}", &SlotCount);
		GS()->AVM(ClientID, "AUCTIONPRICE", ItemID, NOPE, "Item Price: {INT}", &SlotPrice);
		GS()->AVM(ClientID, "AUCTIONACCEPT", ItemID, NOPE, "Add {STR}x{INT} {INT}gold", pInformationSellItem.GetName(pPlayer), &SlotCount, &SlotPrice);
		GS()->AddBack(ClientID);
		return true;
	}
	return false;
}

bool ShopJob::OnVotingMenu(CPlayer *pPlayer, const char *CMD, const int VoteID, const int VoteID2, int Get, const char *GetText)
{
	const int ClientID = pPlayer->GetCID();

	if(PPSTR(CMD, "SHOP") == 0)
	{
		if(BuyShopItem(pPlayer, VoteID))
		{
			GS()->CreatePlayerSound(ClientID, SOUND_ITEM_SELL_BUY);
			GS()->ResetVotes(ClientID, MenuList::MAIN_MENU);
		}
		return true;
	} 

	if(PPSTR(CMD, "AUCTIONCOUNT") == 0)
	{
		// if there are fewer items installed, we set the number of items.
		InventoryItem &pPlayerSellItem = pPlayer->GetItem(VoteID);
		if(Get > pPlayerSellItem.m_Count)
			Get = pPlayerSellItem.m_Count;

		// if it is possible to number
		if(pPlayerSellItem.Info().IsEnchantable())
			Get = 1;

		const int c_minimalprice = (Get * pPlayerSellItem.Info().m_MinimalPrice);
		if(pPlayer->GetTempData().m_SellItem.m_Price < c_minimalprice)
			pPlayer->GetTempData().m_SellItem.m_Price = c_minimalprice;

		pPlayer->GetTempData().m_SellItem.m_Count = Get;
		GS()->UpdateVotes(ClientID, MenuList::MENU_AUCTION_CREATE_SLOT);
		return true;
	}

	if(PPSTR(CMD, "AUCTIONPRICE") == 0)
	{
		const int c_minimalprice = (pPlayer->GetTempData().m_SellItem.m_Count * GS()->GetItemInfo(VoteID).m_MinimalPrice);
		if(Get < c_minimalprice) 
			Get = c_minimalprice;

		pPlayer->GetTempData().m_SellItem.m_Price = Get;
		GS()->UpdateVotes(ClientID, MenuList::MENU_AUCTION_CREATE_SLOT);		
		return true;
	}

	if(PPSTR(CMD, "AUCTIONSLOT") == 0)
	{
		int AvailableCount = Job()->Item()->ActionItemCountAllowed(pPlayer, VoteID);
		if (AvailableCount <= 0)
			return true;

		pPlayer->GetTempData().m_SellItem.m_ItemID = VoteID;
		pPlayer->GetTempData().m_SellItem.m_Enchant = pPlayer->GetItem(VoteID).m_Enchant;
		GS()->ResetVotes(ClientID, MenuList::MENU_AUCTION_CREATE_SLOT);
		return true;
	}

	if(PPSTR(CMD, "AUCTIONACCEPT") == 0)
	{
		InventoryItem &pPlayerSellItem = pPlayer->GetItem(VoteID);
		if(pPlayerSellItem.m_Count >= pPlayer->GetTempData().m_SellItem.m_Count && pPlayer->GetTempData().m_SellItem.m_Price >= 10)
		{
			CreateAuctionSlot(pPlayer, pPlayer->GetTempData().m_SellItem);
			GS()->ResetVotes(ClientID, MenuList::MENU_INVENTORY);
			return true;
		}
		GS()->UpdateVotes(ClientID, MenuList::MENU_AUCTION_CREATE_SLOT);
		return true;
	}

	return false;
}