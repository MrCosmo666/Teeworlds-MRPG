/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#include <engine/shared/config.h>
#include <game/server/gamecontext.h>
#include "shopmail.h"

using namespace sqlstr;

/* Information NON STATIC DATA LOAD ONLY DATABASE (MultiWorld Work) Later need changes object on static class*/
std::map < int , ShopMailSql::ShopPersonal > ShopMailSql::Shop;

// Инициализация класса
void ShopMailSql::OnInitGlobal() 
{
	// загрузить магазины
	boost::scoped_ptr<ResultSet> RES(SJK.SD("*", "tw_mailshop"));
	while(RES->next())
	{
		int ID = RES->getInt("ID");
		Shop[ ID ].StorageID = RES->getInt("StorageID");
	}
}

// Локальный таймер в лок мире
void ShopMailSql::OnTickLocalWorld()
{
	// проверять оконченые слоты аукцион
	if(GS()->Server()->Tick() % (1 * GS()->Server()->TickSpeed() * (g_Config.m_SvTimeCheckAuction*60)) == 0)
		CheckAuctionTime();
}

// Показываем список предметов в магазине
void ShopMailSql::ShowMailShop(CPlayer *pPlayer, int StorageID)
{
	if(!CheckCurrentStorage(StorageID)) return;

	int ClientID = pPlayer->GetCID();
	GS()->AV(ClientID, "null", "");

	int HideID = NUMHIDEMENU + ItemSql::ItemsInfo.size() + 300;
	boost::scoped_ptr<ResultSet> RES(SJK.SD("*", "tw_mailshop", "WHERE OwnerID = 0 AND StorageID = '%d' ORDER BY Level", StorageID));
	while(RES->next())
	{
		int ID  = RES->getInt("ID"), ItemID = RES->getInt("ItemID"), Price = RES->getInt("Price"), Enchant = RES->getInt("Enchant");
		int Level = RES->getInt("Level"), Count = RES->getInt("Count"), NeedItemID = RES->getInt("NeedItem");
		
		ItemSql::ItemInformation &BuyightItem = GS()->GetItemInfo(ItemID);
		ItemSql::ItemInformation &NeededItem = GS()->GetItemInfo(NeedItemID);
		
		if(Enchant > 0)
		{
			GS()->AVHI(ClientID, BuyightItem.GetIcon(), HideID, vec3(15,20,30), _("L{i:level} - {s:name}x{i:count}(+{i:enchant}) [{i:price} {s:need}]"), 
				"level", &Level, "name", BuyightItem.GetName(pPlayer), "count", &Count, "enchant", &Enchant,
				"price", &Price, "need", NeededItem.GetName(pPlayer), NULL);
		}
		else
		{
			GS()->AVHI(ClientID, BuyightItem.GetIcon(), HideID, vec3(15,20,30), _("L{i:level} - {s:name}x{i:count} [{i:price} {s:need}]"), 
				"level", &Level, "name", BuyightItem.GetName(pPlayer), "count", &Count, "price", &Price, "need", NeededItem.GetName(pPlayer), NULL);
		}

		if(CGS::AttributInfo.find(BuyightItem.BonusID) != CGS::AttributInfo.end())
		{
			int BonusCount = BuyightItem.BonusCount*(Enchant+1);
			GS()->AVM(ClientID, "null", NOPE, HideID, _("Astro +{i:bonus} {s:name}"), "bonus", &BonusCount, "name", pPlayer->AtributeName(BuyightItem.BonusID), NULL);
		}

		GS()->AVM(ClientID, "null", NOPE, HideID, _("{s:desc}"), "desc", BuyightItem.GetDesc(pPlayer), NULL);
		GS()->AVM(ClientID, "SHOP", ID, HideID, _("Exchange {s:ineed}x{i:iprice} to {s:need}x{i:price}"), 
			"ineed", NeededItem.GetName(pPlayer), "iprice", &Price, "need", BuyightItem.GetName(pPlayer), "price", &Count, NULL);
		HideID++;
	}
}

// Показываем аукцион
void ShopMailSql::ShowAuction(CPlayer *pPlayer)
{
	int ClientID = pPlayer->GetCID();
	GS()->AVH(ClientID, HAUCTIONINFO, vec3(35,80,40), _("Auction Information"), NULL);
	GS()->AVM(ClientID, "null", NOPE, HAUCTIONINFO, _("To create a slot, see inventory item interact."), NULL);
	GS()->AV(ClientID, "null", "");

	int HideID = NUMHIDEMENU + ItemSql::ItemsInfo.size() + 400;
	boost::scoped_ptr<ResultSet> RES(SJK.SD("*", "tw_mailshop", "WHERE OwnerID > 0 ORDER BY Price"));
	while(RES->next())
	{
		int ID  = RES->getInt("ID"), ItemID = RES->getInt("ItemID"), Price = RES->getInt("Price"), Enchant = RES->getInt("Enchant");
		int Level = RES->getInt("Level"), Count = RES->getInt("Count"), OwnerID = RES->getInt("OwnerID");

		ItemSql::ItemInformation &BuyightItem = GS()->GetItemInfo(ItemID);
		GS()->AVH(ClientID, HideID, vec3(15,20,30), _("{s:name}x{i:count}(+{i:enchant}) {i:price} gold"), 
			"name", BuyightItem.GetName(pPlayer), "count", &Count, "enchant", &Enchant, "price", &Price, NULL);

		if(CGS::AttributInfo.find(BuyightItem.BonusID) != CGS::AttributInfo.end())
		{
			int BonusCount = BuyightItem.BonusCount*(Enchant+1);
			GS()->AVM(ClientID, "null", NOPE, HideID, _("Astro +{i:bonus} {s:name}"), "bonus", &BonusCount, "name", pPlayer->AtributeName(BuyightItem.BonusID), NULL);
		}

		GS()->AVM(ClientID, "null", NOPE, HideID, _("{s:desc}"), "desc", BuyightItem.GetDesc(pPlayer), NULL);
		GS()->AVM(ClientID, "null", NOPE, HideID, _("Seller {s:seller}"), "seller", Job()->PlayerName(OwnerID), NULL);
		GS()->AVM(ClientID, "SHOP", ID, HideID, _("Buy Price {i:price} gold"), "price", &Price, NULL);
		++HideID;
	}
	if(HideID == (NUMHIDEMENU + ItemSql::ItemsInfo.size() + 400))
		GS()->AVL(ClientID, "null", _("Currently there are no products."), NULL);
}

// Новый слот аукциона
void ShopMailSql::CreateAuctionSlot(CPlayer *pPlayer, AuctionItem &AuSellItem)
{
	int ItemID = AuSellItem.a_itemid;
	ItemSql::ItemPlayer &PlSellItem = pPlayer->GetItem(ItemID);

	// проверяем кол-во слотов занято ли все или нет
	int ClientID = pPlayer->GetCID();
	boost::scoped_ptr<ResultSet> RES(SJK.SD("ID", "tw_mailshop", "LIMIT %d", g_Config.m_SvMaxMasiveAuctionSlots));
	if(RES->rowsCount() >= g_Config.m_SvMaxMasiveAuctionSlots)
		return GS()->Chat(ClientID, "Auction has run out of slots, wait for the release of slots!");

	// проверяем кол-во своих слотов
	boost::scoped_ptr<ResultSet> RES2(SJK.SD("ID", "tw_mailshop", "WHERE OwnerID = '%d' LIMIT %d", pPlayer->Acc().AuthID, g_Config.m_SvMaxAuctionSlots));
	int CountSlot = RES2->rowsCount();
	if(CountSlot >= g_Config.m_SvMaxAuctionSlots)
		return GS()->Chat(ClientID, "You use all open the slots in your auction!");

	// проверяем есть ли такой предмет в аукционе
	boost::scoped_ptr<ResultSet> RES3(SJK.SD("OwnerID", "tw_mailshop", "WHERE ItemID = '%d' AND OwnerID = '%d'", ItemID, pPlayer->Acc().AuthID));
	if(RES3->next()) return GS()->Chat(ClientID, "Your same item found in the database, need reopen the slot!");

	// если снялись деньги за оплату аукцион слота
	if(pPlayer->CheckFailMoney(g_Config.m_SvAuctionPriceSlot))	
		return;

	// забираем предмет и добавляем слот
	if(PlSellItem.Count >= AuSellItem.a_count && PlSellItem.Remove(AuSellItem.a_count))
	{
		SJK.ID("tw_mailshop", "(ItemID, Price, Level, Count, OwnerID, Enchant) VALUES ('%d', '%d', '1', '%d', '%d', '%d')", 
			ItemID, AuSellItem.a_price, AuSellItem.a_count, pPlayer->Acc().AuthID, PlSellItem.Enchant);

		int AvailableSlot = (g_Config.m_SvMaxAuctionSlots - CountSlot)-1;
		GS()->Chat(-1, "{STR} created a slot [{STR}x{INT}] on auction.", 
			GS()->Server()->ClientName(ClientID), PlSellItem.Info().GetName(pPlayer), &AuSellItem.a_count);
		GS()->ChatFollow(ClientID, "Still available {INT} slots!", &AvailableSlot);
	}	
	return;
}

// покупка предмета в аукционе или магазине
bool ShopMailSql::BuyShopAuctionSlot(CPlayer *pPlayer, int ID)
{
	const int ClientID = pPlayer->GetCID();

	// ищем слот который нам нужен
	boost::scoped_ptr<ResultSet> RES(SJK.SD("*", "tw_mailshop", "WHERE ID = '%d'", ID));
	if(!RES->next()) return false;

	// проверяем уровень
	const int Level = RES->getInt("Level");
	if(pPlayer->Acc().Level < Level)
	{
		GS()->Chat(ClientID, "Required level {INT} for buy!", &Level);	
		return false;		
	}

	// проверяем имеется ли такой зачарованный предмет
	const int ItemID = RES->getInt("ItemID");
	ItemSql::ItemPlayer &BuyightItem = pPlayer->GetItem(ItemID);
	if(BuyightItem.Count >= 1 && BuyightItem.Info().BonusCount > 0)
	{
		GS()->Chat(ClientID, "Enchant item maximal count x1!");	
		return false;		
	}		

	// снятие слота если мы владелец
	const int OwnerID = RES->getInt("OwnerID");
	const int Count = RES->getInt("Count");
	const int Enchant = RES->getInt("Enchant");
	if(pPlayer->Acc().AuthID == OwnerID)
	{
		GS()->Chat(ClientID, "You closed auction slot!");	
		GS()->SendInbox(ClientID, "Auction Alert", "You have bought a item, or canceled your slot", ItemID, Count, Enchant);
		SJK.DD("tw_mailshop", "WHERE ItemID = '%d' AND OwnerID = '%d'", ItemID, OwnerID);
		return true;
	}

	// проверяем склад но если нет владельца магазинный предмет
	const int StorageID = RES->getInt("StorageID");
	int Price = RES->getInt("Price");
	if(OwnerID <= 0 && StorageID > 0) 
	{
		const int CountStorage = Job()->Storage()->GetCountStorage(StorageID);
		if(!Job()->Storage()->BuyStorageItem(true, ClientID, StorageID, Price)) 
			return false;
	}

	// проверяем есть ли золото
	const int NeedItem = RES->getInt("NeedItem");
	if(pPlayer->CheckFailMoney(Price, NeedItem)) 
		return false;

	// купить не со предмет со склада
	// if(OwnerID <= 0 && StorageID > 0) 
	// 	Job()->Storage()->BuyStorageItem(false, ClientID, StorageID, Price);

	// купить с аукциона
	if(OwnerID > 0) 
	{
		char aBuf[128];
		str_format(aBuf, sizeof(aBuf), "Your [Slot %sx%d] was sold!", BuyightItem.Info().GetName(pPlayer), Count);
		Job()->Inbox()->SendInbox(OwnerID, "Auction Sell", aBuf, itMoney, Price, 0);
		SJK.DD("tw_mailshop", "WHERE ItemID = '%d' AND OwnerID = '%d'", ItemID, OwnerID);
	}

	// купили предмет
	BuyightItem.Add(Count, 0, Enchant);
	GS()->Chat(ClientID, "You exchange {STR}x{INT} to {STR}x{INT}.", BuyightItem.Info().GetName(pPlayer), &Count, GS()->GetItemInfo(NeedItem).GetName(pPlayer), &Price);
	return true;
}

// Проверить склад существует ли он и в магазине указан он
bool ShopMailSql::CheckCurrentStorage(int StorageID)
{
	for(const auto& sh : Shop)
	{
		if(StorageID == sh.second.StorageID)
			return true;
	}
	return false;
}

// Проверить время истекших слотов
void ShopMailSql::CheckAuctionTime()
{
	boost::scoped_ptr<ResultSet> RES(SJK.SD("*", "tw_mailshop", "WHERE DATE_SUB(NOW(),INTERVAL %d MINUTE) > Time AND OwnerID > 0", g_Config.m_SvTimeAuctionSlot));
	int ReleaseSlots = RES->rowsCount();
	while(RES->next())
	{
		const int ID  = RES->getInt("ID"), ItemID = RES->getInt("ItemID"), Count = RES->getInt("Count");
		const int Enchant = RES->getInt("Enchant"), OwnerID = RES->getInt("OwnerID");

		Job()->Inbox()->SendInbox(OwnerID, "Auction expired", "Your slot has expired", ItemID, Count, Enchant);
		SJK.DD("tw_mailshop", "WHERE ID = '%d'", ID);
	}
	if(ReleaseSlots) 
		GS()->ChatFollow(-1, "Auction {INT} slots has been released!", &ReleaseSlots);
	return;
}

// Парсинг голосований магазина аукциона
bool ShopMailSql::OnParseVotingMenu(CPlayer *pPlayer, const char *CMD, const int VoteID, const int VoteID2, int Get, const char *GetText)
{
	const int ClientID = pPlayer->GetCID();

	// купить предмет
	if(PPSTR(CMD, "SHOP") == 0)
	{
		if(BuyShopAuctionSlot(pPlayer, VoteID))	
			GS()->ResetVotes(ClientID, MAINMENU);
		return true;
	} 

	// аукцион установить кол-во
	if(PPSTR(CMD, "AUCTIONCOUNT") == 0)
	{
		ItemSql::ItemPlayer &PlSellItem = pPlayer->GetItem(VoteID);
		AuctionItem &AuSellItem = CGS::InteractiveSub[ClientID].SelectedAuctionItem;

		// если предметов меньше установленно ставим кол-во что есть
		if(Get > PlSellItem.Count)
			Get = PlSellItem.Count;

		// если предмет можно кол-во
		if(PlSellItem.Info().BonusCount)
			Get = 1;

		// если сбрасываем цену если не хватает
		const int c_minimalprice = Get*PlSellItem.Info().MinimalPrice;
		if(AuSellItem.a_price < c_minimalprice)
			AuSellItem.a_price = c_minimalprice;
			
		// устанавливаем кол-во предметов
		AuSellItem.a_count = Get;
		GS()->VResetVotes(ClientID, AUCTIONSETSLOT);
		return true;
	}

	// аукцион установить цену
	if(PPSTR(CMD, "AUCTIONPRICE") == 0)
	{
		ItemSql::ItemPlayer &PlSellItem = pPlayer->GetItem(VoteID);
		AuctionItem &AuSellItem = CGS::InteractiveSub[ClientID].SelectedAuctionItem;

		const int c_minimalprice = PlSellItem.Info().MinimalPrice*AuSellItem.a_count;
		if(Get < c_minimalprice) Get = c_minimalprice;

		AuSellItem.a_price = Get;
		GS()->VResetVotes(ClientID, AUCTIONSETSLOT);		
		return true;
	}

	// аукцион установить предмет слот
	if(PPSTR(CMD, "AUCTIONSLOT") == 0)
	{
		CGS::InteractiveSub[ClientID].SelectedAuctionItem.a_itemid = VoteID;
		GS()->ResetVotes(ClientID, AUCTIONSETSLOT);
		return true;
	}

	// принять аукцион слот
	if(PPSTR(CMD, "AUCTIONACCEPT") == 0)
	{
		ItemSql::ItemPlayer &SellItem = pPlayer->GetItem(VoteID);
		AuctionItem &AuSellItem = CGS::InteractiveSub[ClientID].SelectedAuctionItem;
	
		if(SellItem.Count >= AuSellItem.a_count && AuSellItem.a_price >= 10)
		{
			// создаем слот
			CreateAuctionSlot(pPlayer, CGS::InteractiveSub[ClientID].SelectedAuctionItem);
			GS()->ResetVotes(ClientID, INVENTORY);

			// очищаем данные для создания
			GS()->ClearInteractiveSub(ClientID);
			return true;
		}

		// обновляем меню аукцион слота
		GS()->VResetVotes(ClientID, AUCTIONSETSLOT);
		return true;
	}
	return false;
}