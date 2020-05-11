/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#include <engine/shared/config.h>
#include <game/server/gamecontext.h>
#include "ShopJob.h"

using namespace sqlstr;
std::map < int , ShopJob::ShopPersonal > ShopJob::Shop;

void ShopJob::OnInit()
{
	boost::scoped_ptr<ResultSet> RES(SJK.SD("*", "tw_mailshop"));
	while(RES->next())
	{
		int ID = RES->getInt("ID");
		Shop[ ID ].StorageID = RES->getInt("StorageID");
	}
}

void ShopJob::OnTick()
{
	if(GS()->GetWorldID() == LOCALWORLD)
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
		GS()->Chat(ClientID, "You can see list of auctions items in the votes!");
		pChr->m_Core.m_ProtectHooked = pChr->m_NoAllowDamage = true;
		GS()->ResetVotes(ClientID, MenuList::MAIN_MENU);
		return true;
	}
	else if (pChr->GetHelper()->TileExit(IndexCollision, TILE_AUCTION))
	{
		GS()->Chat(ClientID, "You have left the active zone, menu is restored!");
		pChr->m_Core.m_ProtectHooked = pChr->m_NoAllowDamage = false;
		GS()->ResetVotes(ClientID, MenuList::MAIN_MENU);
		return true;
	}

	return false;
}

void ShopJob::ShowMailShop(CPlayer *pPlayer, int StorageID)
{
	const int ClientID = pPlayer->GetCID();
	GS()->AV(ClientID, "null", "");

	int HideID = NUM_TAB_MENU + ItemJob::ItemsInfo.size() + 300;
	boost::scoped_ptr<ResultSet> RES(SJK.SD("*", "tw_mailshop", "WHERE StorageID = '%d' ORDER BY Price", StorageID));
	while(RES->next())
	{
		int ID = RES->getInt("ID");
		int ItemID = RES->getInt("ItemID");
		int Price = RES->getInt("Price");
		int Enchant = RES->getInt("Enchant");
		int Count = RES->getInt("Count");
		int NeedItemID = RES->getInt("NeedItem");
		ItemJob::ItemInformation &BuyightItem = GS()->GetItemInfo(ItemID);
		ItemJob::ItemInformation &NeededItem = GS()->GetItemInfo(NeedItemID);
		
		// зачеровыванный или нет
		if (BuyightItem.IsEnchantable())
		{
			char aEnchantSize[16];
			str_format(aEnchantSize, sizeof(aEnchantSize), " [+%d]", Enchant);
			GS()->AVHI(ClientID, BuyightItem.GetIcon(), HideID, LIGHT_GRAY_COLOR, "{STR}{STR}{STR} :: {INT} {STR}",
				(pPlayer->GetItem(ItemID).Count > 0 ? "✔ " : "\0"), BuyightItem.GetName(pPlayer), (Enchant > 0 ? aEnchantSize : "\0"), &Price, NeededItem.GetName(pPlayer));

			char aAttributes[128];
			Job()->Item()->FormatAttributes(BuyightItem, Enchant, sizeof(aAttributes), aAttributes);
			GS()->AVM(ClientID, "null", NOPE, HideID, "{STR}", aAttributes);
		}
		else
		{
			GS()->AVHI(ClientID, BuyightItem.GetIcon(), HideID, LIGHT_GRAY_COLOR, "{STR}x{INT} ({INT}) :: {INT} {STR}",
				BuyightItem.GetName(pPlayer), &Count, &pPlayer->GetItem(ItemID).Count, &Price, NeededItem.GetName(pPlayer));
		}

		GS()->AVM(ClientID, "null", NOPE, HideID, "{STR}", BuyightItem.GetDesc(pPlayer));
		GS()->AVM(ClientID, "SHOP", ID, HideID, "Exchange {STR}x{INT} to {STR}x{INT}", NeededItem.GetName(pPlayer), &Price, BuyightItem.GetName(pPlayer), &Count);
		HideID++;
	}
}

// Показываем аукцион
void ShopJob::ShowAuction(CPlayer *pPlayer)
{
	const int ClientID = pPlayer->GetCID();
	GS()->AVH(ClientID, TAB_INFO_AUCTION, GREEN_COLOR, "Auction Information");
	GS()->AVM(ClientID, "null", NOPE, TAB_INFO_AUCTION, "To create a slot, see inventory item interact.");
	GS()->AV(ClientID, "null", "");

	bool FoundItems = false;
	int HideID = (int)(NUM_TAB_MENU + ItemJob::ItemsInfo.size() + 400);
	boost::scoped_ptr<ResultSet> RES(SJK.SD("*", "tw_mailshop", "WHERE OwnerID > 0 ORDER BY Price"));
	while(RES->next())
	{
		int ID = RES->getInt("ID");
		int ItemID = RES->getInt("ItemID");
		int Price = RES->getInt("Price");
		int Enchant = RES->getInt("Enchant");
		int Count = RES->getInt("Count");
		int OwnerID = RES->getInt("OwnerID");

		// зачеровыванный или нет
		ItemJob::ItemInformation &BuyightItem = GS()->GetItemInfo(ItemID);
		if (BuyightItem.IsEnchantable())
		{
			char aEnchantSize[16];
			str_format(aEnchantSize, sizeof(aEnchantSize), " [+%d]", Enchant);
			GS()->AVHI(ClientID, BuyightItem.GetIcon(), HideID, LIGHT_GRAY_COLOR, "{STR}{STR}{STR} :: {INT} gold",
				(pPlayer->GetItem(ItemID).Count > 0 ? "✔ " : "\0"), BuyightItem.GetName(pPlayer), (Enchant > 0 ? aEnchantSize : "\0"), &Price);

			char aAttributes[128];
			Job()->Item()->FormatAttributes(BuyightItem, Enchant, sizeof(aAttributes), aAttributes);
			GS()->AVM(ClientID, "null", NOPE, HideID, "{STR}", aAttributes);
		}
		else
		{
			GS()->AVHI(ClientID, BuyightItem.GetIcon(), HideID, LIGHT_GRAY_COLOR, "{STR}x{INT} ({INT}) :: {INT} gold",
				BuyightItem.GetName(pPlayer), &Count, &pPlayer->GetItem(ItemID).Count, &Price);
		}

		GS()->AVM(ClientID, "null", NOPE, HideID, "{STR}", BuyightItem.GetDesc(pPlayer));
		GS()->AVM(ClientID, "null", NOPE, HideID, "Seller {STR}", Job()->PlayerName(OwnerID));
		GS()->AVM(ClientID, "SHOP", ID, HideID, "Buy Price {INT} gold", &Price);
		FoundItems = true;
		++HideID;
	}
	if(!FoundItems)
		GS()->AVL(ClientID, "null", "Currently there are no products.");
}

void ShopJob::CreateAuctionSlot(CPlayer *pPlayer, AuctionItem &AuSellItem)
{
	int ItemID = AuSellItem.a_itemid;
	int ClientID = pPlayer->GetCID();
	ItemJob::InventoryItem &pPlayerAuctionItem = pPlayer->GetItem(ItemID);

	// проверяем кол-во слотов занято ли все или нет
	boost::scoped_ptr<ResultSet> RES(SJK.SD("ID", "tw_mailshop", "WHERE OwnerID > '0' LIMIT %d", g_Config.m_SvMaxMasiveAuctionSlots));
	if((int)RES->rowsCount() >= g_Config.m_SvMaxMasiveAuctionSlots)
		return GS()->Chat(ClientID, "Auction has run out of slots, wait for the release of slots!");

	// проверяем кол-во своих слотов
	boost::scoped_ptr<ResultSet> RES2(SJK.SD("ID", "tw_mailshop", "WHERE OwnerID = '%d' LIMIT %d", pPlayer->Acc().AuthID, g_Config.m_SvMaxAuctionSlots));
	int CountSlot = RES2->rowsCount();
	if(CountSlot >= g_Config.m_SvMaxAuctionSlots)
		return GS()->Chat(ClientID, "You use all open the slots in your auction!");

	// проверяем есть ли такой предмет в аукционе
	boost::scoped_ptr<ResultSet> RES3(SJK.SD("ID", "tw_mailshop", "WHERE ItemID = '%d' AND OwnerID = '%d'", ItemID, pPlayer->Acc().AuthID));
	if(RES3->next()) return GS()->Chat(ClientID, "Your same item found in the database, need reopen the slot!");

	// если снялись деньги за оплату аукцион слота
	if(pPlayer->CheckFailMoney(g_Config.m_SvAuctionPriceSlot))	
		return;

	// забираем предмет и добавляем слот
	if(pPlayerAuctionItem.Count >= AuSellItem.a_count && pPlayerAuctionItem.Remove(AuSellItem.a_count))
	{
		SJK.ID("tw_mailshop", "(ItemID, Price, Count, OwnerID, Enchant) VALUES ('%d', '%d', '%d', '%d', '%d')", 
			ItemID, AuSellItem.a_price, AuSellItem.a_count, pPlayer->Acc().AuthID, AuSellItem.a_enchant);

		int AvailableSlot = (g_Config.m_SvMaxAuctionSlots - CountSlot) - 1;
		GS()->Chat(-1, "{STR} created a slot [{STR}x{INT}] auction.", 
			GS()->Server()->ClientName(ClientID), pPlayerAuctionItem.Info().GetName(pPlayer), &AuSellItem.a_count);
		GS()->ChatFollow(ClientID, "Still available {INT} slots!", &AvailableSlot);
	}

	GS()->ClearInteractiveSub(ClientID);
	return;
}

bool ShopJob::BuyShopItem(CPlayer* pPlayer, int ID)
{
	const int ClientID = pPlayer->GetCID();
	boost::scoped_ptr<ResultSet> SHOPITEM(SJK.SD("*", "tw_mailshop", "WHERE ID = '%d'", ID));
	if (!SHOPITEM->next()) return false;

	const int ItemID = SHOPITEM->getInt("ItemID");
	ItemJob::InventoryItem &pPlayerBuyightItem = pPlayer->GetItem(ItemID);
	if (pPlayerBuyightItem.Count > 0 && pPlayerBuyightItem.Info().IsEnchantable())
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
		// забираем свой слот
		if (OwnerID == pPlayer->Acc().AuthID)
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
		Job()->Inbox()->SendInbox(OwnerID, "Auction Sell", aBuf, itMoney, Price, 0);
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
	boost::scoped_ptr<ResultSet> RES(SJK.SD("*", "tw_mailshop", "WHERE OwnerID > 0 AND DATE_SUB(NOW(),INTERVAL %d MINUTE) > Time", g_Config.m_SvTimeAuctionSlot));
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
	return;
}

bool ShopJob::OnHandleMenulist(CPlayer* pPlayer, int Menulist, bool ReplaceMenu)
{
	const int ClientID = pPlayer->GetCID();
	if (ReplaceMenu)
	{
		CCharacter* pChr = pPlayer->GetCharacter();
		if (!pChr) return false;

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
		ItemJob::ItemInformation& pInformationSellItem = GS()->GetItemInfo(CGS::InteractiveSub[ClientID].AuctionItem.a_itemid);

		const int ItemID = CGS::InteractiveSub[ClientID].AuctionItem.a_itemid;
		const int SlotCount = CGS::InteractiveSub[ClientID].AuctionItem.a_count;
		const int SlotPrice = CGS::InteractiveSub[ClientID].AuctionItem.a_price;
		const int SlotEnchant = CGS::InteractiveSub[ClientID].AuctionItem.a_enchant;
		const int MinimalPrice = SlotCount * pInformationSellItem.MinimalPrice;

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

	// купить предмет
	if(PPSTR(CMD, "SHOP") == 0)
	{
		if(BuyShopItem(pPlayer, VoteID))	
			GS()->ResetVotes(ClientID, MenuList::MAIN_MENU);
		return true;
	} 

	// аукцион установить кол-во
	if(PPSTR(CMD, "AUCTIONCOUNT") == 0)
	{
		// если предметов меньше установленно ставим кол-во что есть
		ItemJob::InventoryItem &pPlayerSellItem = pPlayer->GetItem(VoteID);
		if(Get > pPlayerSellItem.Count)
			Get = pPlayerSellItem.Count;

		// если предмет можно кол-во
		if(pPlayerSellItem.Info().IsEnchantable())
			Get = 1;

		// если сбрасываем цену если не хватает
		const int c_minimalprice = Get* pPlayerSellItem.Info().MinimalPrice;
		if(CGS::InteractiveSub[ClientID].AuctionItem.a_price < c_minimalprice)
			CGS::InteractiveSub[ClientID].AuctionItem.a_price = c_minimalprice;
			
		// устанавливаем кол-во предметов
		CGS::InteractiveSub[ClientID].AuctionItem.a_count = Get;
		GS()->VResetVotes(ClientID, MenuList::MENU_AUCTION_CREATE_SLOT);
		return true;
	}

	// аукцион установить цену
	if(PPSTR(CMD, "AUCTIONPRICE") == 0)
	{
		ItemJob::ItemInformation &pInformationSellItem = GS()->GetItemInfo(VoteID);
		const int c_minimalprice = pInformationSellItem.MinimalPrice * CGS::InteractiveSub[ClientID].AuctionItem.a_count;
		if(Get < c_minimalprice) 
			Get = c_minimalprice;

		CGS::InteractiveSub[ClientID].AuctionItem.a_price = Get;
		GS()->VResetVotes(ClientID, MenuList::MENU_AUCTION_CREATE_SLOT);		
		return true;
	}

	// аукцион установить предмет слот
	if(PPSTR(CMD, "AUCTIONSLOT") == 0)
	{
		int AvailableCount = Job()->Item()->ActionItemCountAllowed(pPlayer, VoteID);
		if (AvailableCount <= 0)
			return true;

		CGS::InteractiveSub[ClientID].AuctionItem.a_itemid = VoteID;
		CGS::InteractiveSub[ClientID].AuctionItem.a_enchant = pPlayer->GetItem(VoteID).Enchant;
		GS()->ResetVotes(ClientID, MenuList::MENU_AUCTION_CREATE_SLOT);
		return true;
	}

	// принять аукцион слот
	if(PPSTR(CMD, "AUCTIONACCEPT") == 0)
	{
		ItemJob::InventoryItem &pPlayerSellItem = pPlayer->GetItem(VoteID);
		if(pPlayerSellItem.Count >= CGS::InteractiveSub[ClientID].AuctionItem.a_count && CGS::InteractiveSub[ClientID].AuctionItem.a_price >= 10)
		{
			CreateAuctionSlot(pPlayer, CGS::InteractiveSub[ClientID].AuctionItem);
			GS()->ResetVotes(ClientID, MenuList::MENU_INVENTORY);
			return true;
		}
		GS()->VResetVotes(ClientID, MenuList::MENU_AUCTION_CREATE_SLOT);
		return true;
	}

	return false;
}