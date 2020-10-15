/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#include <game/server/gamecontext.h>
#include "InventoryJob.h"

using namespace sqlstr;
std::map < int , std::map < int , InventoryItem > > InventoryJob::ms_aItems;
std::map < int , ItemInformation > InventoryJob::ms_aItemsInfo;

void InventoryJob::OnInit()
{
	SJK.SDT("*", "tw_items_list", [&](ResultSet* RES)
	{
		while(RES->next())
		{
			const int ItemID = (int)RES->getInt("ItemID");
			str_copy(ms_aItemsInfo[ItemID].m_aName, RES->getString("Name").c_str(), sizeof(ms_aItemsInfo[ItemID].m_aName));
			str_copy(ms_aItemsInfo[ItemID].m_aDesc, RES->getString("Description").c_str(), sizeof(ms_aItemsInfo[ItemID].m_aDesc));
			str_copy(ms_aItemsInfo[ItemID].m_aIcon, RES->getString("Icon").c_str(), sizeof(ms_aItemsInfo[ItemID].m_aIcon));
			ms_aItemsInfo[ItemID].m_Type = (int)RES->getInt("Type");
			ms_aItemsInfo[ItemID].m_Function = (int)RES->getInt("Function");
			ms_aItemsInfo[ItemID].m_Dysenthis = (int)RES->getInt("Desynthesis");
			ms_aItemsInfo[ItemID].m_MinimalPrice = (int)RES->getInt("Selling");
			for(int i = 0; i < STATS_MAX_FOR_ITEM; i++)
			{
				char aBuf[32];
				str_format(aBuf, sizeof(aBuf), "Stat_%d", i);
				ms_aItemsInfo[ItemID].m_aAttribute[i] = (int)RES->getInt(aBuf);
				str_format(aBuf, sizeof(aBuf), "StatCount_%d", i);
				ms_aItemsInfo[ItemID].m_aAttributeCount[i] = (int)RES->getInt(aBuf);
			}
			ms_aItemsInfo[ItemID].m_ProjID = (int)RES->getInt("ProjectileID");
		}
	});

	SJK.SDT("*", "tw_attributs", [&](ResultSet* RES)
	{
		while(RES->next())
		{
			const int AttID = RES->getInt("ID");
			str_copy(CGS::ms_aAttributsInfo[AttID].m_aName, RES->getString("name").c_str(), sizeof(CGS::ms_aAttributsInfo[AttID].m_aName));
			str_copy(CGS::ms_aAttributsInfo[AttID].m_aFieldName, RES->getString("field_name").c_str(), sizeof(CGS::ms_aAttributsInfo[AttID].m_aFieldName));
			CGS::ms_aAttributsInfo[AttID].m_UpgradePrice = RES->getInt("price");
			CGS::ms_aAttributsInfo[AttID].m_Type = RES->getInt("at_type");
			CGS::ms_aAttributsInfo[AttID].m_Devide = RES->getInt("divide");
		}
	});
}

void InventoryJob::OnInitAccount(CPlayer *pPlayer)
{
	const int ClientID = pPlayer->GetCID();
	std::shared_ptr<ResultSet> RES(SJK.SD("*", "tw_accounts_items", "WHERE OwnerID = '%d'", pPlayer->Acc().m_AuthID));
	while(RES->next())
	{
		int ItemID = (int)RES->getInt("ItemID");
		ms_aItems[ClientID][ItemID].SetItemOwner(pPlayer);
		ms_aItems[ClientID][ItemID].m_ItemID = ItemID;
		ms_aItems[ClientID][ItemID].m_Count = (int)RES->getInt("Count");
		ms_aItems[ClientID][ItemID].m_Settings = (int)RES->getInt("Settings");
		ms_aItems[ClientID][ItemID].m_Enchant = (int)RES->getInt("Enchant");
		ms_aItems[ClientID][ItemID].m_Durability = (int)RES->getInt("Durability");
	}		
}

void InventoryJob::OnResetClient(int ClientID)
{
	if (ms_aItems.find(ClientID) != ms_aItems.end())
		ms_aItems.erase(ClientID);
}

void InventoryJob::RepairDurabilityItems(CPlayer *pPlayer)
{ 
	const int ClientID = pPlayer->GetCID();
	SJK.UD("tw_accounts_items", "Durability = '100' WHERE OwnerID = '%d'", pPlayer->Acc().m_AuthID);
	for(auto& it : ms_aItems[ClientID])
		it.second.m_Durability = 100;
}

void InventoryJob::ListInventory(CPlayer *pPlayer, int TypeList, bool SortedFunction)
{
	const int ClientID = pPlayer->GetCID();
	GS()->AV(ClientID, "null");

	// show a list of items to the player
	bool Found = false;
	for(const auto& it : ms_aItems[ClientID])
	{
		if(!it.second.m_Count || ((SortedFunction && it.second.Info().m_Function != TypeList) 
			|| (!SortedFunction && it.second.Info().m_Type != TypeList)))
			continue;

		ItemSelected(pPlayer, it.second);
		Found = true;
	}
	if(!Found)
		GS()->AVL(ClientID, "null", "There are no items in this tab");
}

int InventoryJob::GiveItem(CPlayer *pPlayer, int ItemID, int Count, int Settings, int Enchant)
{
	const int ClientID = pPlayer->GetCID();
	const int SecureID = SecureCheck(pPlayer, ItemID, Count, Settings, Enchant);
	if(SecureID == 1)
	{
		SJK.UD("tw_accounts_items", "Count = '%d', Settings = '%d', Enchant = '%d' WHERE ItemID = '%d' AND OwnerID = '%d'",
			ms_aItems[ClientID][ItemID].m_Count, ms_aItems[ClientID][ItemID].m_Settings, ms_aItems[ClientID][ItemID].m_Enchant, ItemID, pPlayer->Acc().m_AuthID);
	}
	return SecureID;
}

int InventoryJob::SecureCheck(CPlayer *pPlayer, int ItemID, int Count, int Settings, int Enchant)
{
	// check initialize and add the item
	const int ClientID = pPlayer->GetCID();
	std::shared_ptr<ResultSet> RES(SJK.SD("Count, Settings", "tw_accounts_items", "WHERE ItemID = '%d' AND OwnerID = '%d'", ItemID, pPlayer->Acc().m_AuthID));
	if(RES->next())
	{
		ms_aItems[ClientID][ItemID].m_Count = RES->getInt("Count")+Count;
		ms_aItems[ClientID][ItemID].m_Settings = RES->getInt("Settings")+Settings;
		ms_aItems[ClientID][ItemID].m_Enchant = Enchant;
		return 1;	
	}

	// create an object if not found
	ms_aItems[ClientID][ItemID].m_Count = Count;
	ms_aItems[ClientID][ItemID].m_Settings = Settings;
	ms_aItems[ClientID][ItemID].m_Enchant = Enchant;
	ms_aItems[ClientID][ItemID].m_Durability = 100;
	SJK.ID("tw_accounts_items", "(ItemID, OwnerID, Count, Settings, Enchant) VALUES ('%d', '%d', '%d', '%d', '%d')",
		ItemID, pPlayer->Acc().m_AuthID, Count, Settings, Enchant);
	return 2;
}

int InventoryJob::RemoveItem(CPlayer *pPlayer, int ItemID, int Count, int Settings)
{
	const int SecureID = DeSecureCheck(pPlayer, ItemID, Count, Settings);
	if(SecureID == 1)
	{
		SJK.UD("tw_accounts_items", "Count = Count - '%d', Settings = Settings - '%d' WHERE ItemID = '%d' AND OwnerID = '%d'",
			Count, Settings, ItemID, pPlayer->Acc().m_AuthID);
	}
	return SecureID;
}

int InventoryJob::DeSecureCheck(CPlayer *pPlayer, int ItemID, int Count, int Settings)
{
	// we check the database
	const int ClientID = pPlayer->GetCID();
	std::shared_ptr<ResultSet> RES(SJK.SD("Count, Settings", "tw_accounts_items", "WHERE ItemID = '%d' AND OwnerID = '%d'", ItemID, pPlayer->Acc().m_AuthID));
	if(RES->next())
	{
		// update if there is more
		if(RES->getInt("Count") > Count)
		{
			ms_aItems[ClientID][ItemID].m_Count = RES->getInt("Count")-Count;
			ms_aItems[ClientID][ItemID].m_Settings = RES->getInt("Settings")-Settings;
			return 1;		
		}

		// remove the object if it is less than the required amount
		ms_aItems[ClientID][ItemID].m_Count = 0;
		ms_aItems[ClientID][ItemID].m_Settings = 0;
		ms_aItems[ClientID][ItemID].m_Enchant = 0;
		SJK.DD("tw_accounts_items", "WHERE ItemID = '%d' AND OwnerID = '%d'", ItemID, pPlayer->Acc().m_AuthID);
		return 2;		
	}

	ms_aItems[ClientID][ItemID].m_Count = 0;
	ms_aItems[ClientID][ItemID].m_Settings = 0;
	ms_aItems[ClientID][ItemID].m_Enchant = 0;	
	return 0;		
}

int InventoryJob::ActionItemCountAllowed(CPlayer *pPlayer, int ItemID)
{
	const int ClientID = pPlayer->GetCID();
	const int AvailableCount = Job()->Quest()->QuestingAllowedItemsCount(pPlayer, ItemID);
	if (AvailableCount <= 0)
	{
		GS()->Chat(ClientID, "This count of items that you have, iced for the quest!");
		GS()->Chat(ClientID, "Can see in which quest they are required in Adventure journal!");
		return -1;
	}

	return AvailableCount;
}

void InventoryJob::ItemSelected(CPlayer* pPlayer, const InventoryItem& pItemPlayer, bool Dress)
{
	const int ClientID = pPlayer->GetCID();
	const int ItemID = pItemPlayer.GetID();
	const int HideID = NUM_TAB_MENU + ItemID;
	const char* pNameItem = pItemPlayer.Info().GetName(pPlayer);

	// overwritten or not
	if (pItemPlayer.Info().IsEnchantable())
	{
		char aEnchantBuf[16];
		pItemPlayer.FormatEnchantLevel(aEnchantBuf, sizeof(aEnchantBuf));
		GS()->AVHI(ClientID, pItemPlayer.Info().GetIcon(), HideID, LIGHT_GRAY_COLOR, "{STR} {STR}{STR}",
			pNameItem, (pItemPlayer.m_Enchant > 0 ? aEnchantBuf : "\0"), (pItemPlayer.m_Settings ? " ✔" : "\0"));
		GS()->AVM(ClientID, "null", NOPE, HideID, "{STR}", pItemPlayer.Info().GetDesc(pPlayer));

		char aAttributes[64];
		pItemPlayer.FormatAttributes(aAttributes, sizeof(aAttributes));
		GS()->AVM(ClientID, "null", NOPE, HideID, "{STR}", aAttributes);
	}
	else
	{
		GS()->AVHI(ClientID, pItemPlayer.Info().GetIcon(), HideID, LIGHT_GRAY_COLOR, "{STR}{STR} x{INT}",
			(pItemPlayer.m_Settings ? "Dressed - " : "\0"), pNameItem, &pItemPlayer.m_Count);
		GS()->AVM(ClientID, "null", NOPE, HideID, "{STR}", pItemPlayer.Info().GetDesc(pPlayer));
	}

	if (pItemPlayer.Info().m_Function == FUNCTION_ONE_USED || pItemPlayer.Info().m_Function == FUNCTION_USED)
	{
		char aBuf[64];
		str_format(aBuf, sizeof(aBuf), "Bind command \"/useitem %d'\"", ItemID);
		GS()->AVM(ClientID, "null", NOPE, HideID, "{STR}", aBuf);
		GS()->AVM(ClientID, "IUSE", ItemID, HideID, "Use {STR}", pNameItem);
	}

	if (pItemPlayer.Info().m_Type == ItemType::TYPE_POTION)
		GS()->AVM(ClientID, "ISETTINGS", ItemID, HideID, "Auto use {STR} - {STR}", pNameItem, (pItemPlayer.m_Settings ? "Enable" : "Disable"));
	else if (pItemPlayer.Info().m_Type == ItemType::TYPE_DECORATION)
	{
		GS()->AVM(ClientID, "DECOSTART", ItemID, HideID, "Added {STR} to your house", pNameItem);
		GS()->AVM(ClientID, "DECOGUILDSTART", ItemID, HideID, "Added {STR} to your guild house", pNameItem);
	}
	else if(pItemPlayer.Info().m_Type == ItemType::TYPE_EQUIP || pItemPlayer.Info().m_Function == FUNCTION_SETTINGS)
	{
		if((pItemPlayer.Info().m_Function == EQUIP_HAMMER && pItemPlayer.IsEquipped()))
			GS()->AVM(ClientID, "null", NOPE, HideID, "You can not undress equipping hammer", pNameItem);
		else
			GS()->AVM(ClientID, "ISETTINGS", ItemID, HideID, "{STR} {STR}", (pItemPlayer.m_Settings ? "Undress" : "Equip"), pNameItem);
	}

	if(pItemPlayer.Info().m_Function == FUNCTION_PLANTS)
	{
		const int HouseID = Job()->House()->OwnerHouseID(pPlayer->Acc().m_AuthID);
		const int PlantItemID = Job()->House()->GetPlantsID(HouseID);
		if(HouseID > 0 && PlantItemID != ItemID)
		{
			const int random_change = random_int() % 900;
			GS()->AVD(ClientID, "HOMEPLANTSET", ItemID, random_change, HideID, "To plant {STR}, to house (0.06%)", pNameItem);
		}
	}

	if (pItemPlayer.Info().IsEnchantable() && !pItemPlayer.IsEnchantMaxLevel())
	{
		const int Price = pItemPlayer.GetEnchantPrice();
		GS()->AVM(ClientID, "IENCHANT", ItemID, HideID, "Enchant {STR} ({INT} materials)", pNameItem, &Price);
	}

	// not allowed drop equipped hammer
	if (ItemID == pPlayer->GetEquippedItem(EQUIP_HAMMER))
		return;

	if (pItemPlayer.Info().m_Dysenthis > 0)
		GS()->AVM(ClientID, "IDESYNTHESIS", ItemID, HideID, "Disassemble {STR} (+{INT} materials)", pNameItem, &pItemPlayer.Info().m_Dysenthis);

	GS()->AVM(ClientID, "IDROP", ItemID, HideID, "Drop {STR}", pNameItem);

	if (pItemPlayer.Info().m_MinimalPrice > 0)
		GS()->AVM(ClientID, "AUCTIONSLOT", ItemID, HideID, "Create Slot Auction {STR}", pNameItem);
}

bool InventoryJob::OnParsingVoteCommands(CPlayer *pPlayer, const char *CMD, const int VoteID, const int VoteID2, int Get, const char *GetText)
{
	const int ClientID = pPlayer->GetCID();

	if(PPSTR(CMD, "SORTEDINVENTORY") == 0)
	{
		pPlayer->m_SortTabs[SORT_INVENTORY] = VoteID;
		GS()->UpdateVotes(ClientID, MenuList::MENU_INVENTORY);
		return true;
	}

	if(PPSTR(CMD, "IDROP") == 0)
	{
		if (!pPlayer->GetCharacter())
			return true;

		int AvailableCount = ActionItemCountAllowed(pPlayer, VoteID);
		if (AvailableCount <= 0)
			return true;

		Get = min(AvailableCount, Get);
		InventoryItem& pItemPlayer = pPlayer->GetItem(VoteID);
		pItemPlayer.Drop(Get);

		GS()->Broadcast(ClientID, BroadcastPriority::BROADCAST_GAME_WARNING, 100, "You drop {STR}x{INT}", pItemPlayer.Info().GetName(pPlayer), &Get);
		GS()->ResetVotes(ClientID, pPlayer->m_OpenVoteMenu);
		return true;
	}

	if(PPSTR(CMD, "IUSE") == 0)
	{
		int AvailableCount = ActionItemCountAllowed(pPlayer, VoteID);
		if (AvailableCount <= 0)
			return true;

		Get = min(AvailableCount, Get);
		pPlayer->GetItem(VoteID).Use(Get);
		return true;
	}

	if(PPSTR(CMD, "IDESYNTHESIS") == 0)
	{
		int AvailableCount = ActionItemCountAllowed(pPlayer, VoteID);
		if (AvailableCount <= 0)
			return true;

		Get = min(AvailableCount, Get);
		InventoryItem &pPlayerSelectedItem = pPlayer->GetItem(VoteID);
		InventoryItem &pPlayerMaterialItem = pPlayer->GetItem(itMaterial);
		const int DesCount = pPlayerSelectedItem.Info().m_Dysenthis * Get;
		if(pPlayerSelectedItem.Remove(Get) && pPlayerMaterialItem.Add(DesCount))
		{
			GS()->Chat(ClientID, "Disassemble {STR}x{INT}, you receive {INT} {STR}(s)", 
				pPlayerSelectedItem.Info().GetName(pPlayer), &Get, &DesCount, pPlayerMaterialItem.Info().GetName(pPlayer));
			GS()->ResetVotes(ClientID, pPlayer->m_OpenVoteMenu);
		}
		return true;
	}

	if(PPSTR(CMD, "ISETTINGS") == 0)
	{
		if(!pPlayer->GetCharacter())
			return true;

		pPlayer->GetItem(VoteID).Equip();
		GS()->CreatePlayerSound(ClientID, SOUND_ITEM_EQUIP);
		GS()->ResetVotes(ClientID, pPlayer->m_OpenVoteMenu);
		return true;
	}

	if(PPSTR(CMD, "IENCHANT") == 0)
	{
		InventoryItem &pItemPlayer = pPlayer->GetItem(VoteID);
		if(pItemPlayer.IsEnchantMaxLevel())
		{
			GS()->Chat(ClientID, "You enchant max level for this item!");
			return true;			
		}

		const int Price = pItemPlayer.GetEnchantPrice();
		if(!pPlayer->SpendCurrency(Price, itMaterial))
			return true;

		const int EnchantLevel = pItemPlayer.m_Enchant + 1;
		pItemPlayer.SetEnchant(EnchantLevel);
		if(pItemPlayer.IsEnchantMaxLevel())
			GS()->SendEquipItem(ClientID, -1);

		char aEnchantBuf[16];
		pItemPlayer.FormatEnchantLevel(aEnchantBuf, sizeof(aEnchantBuf));

		char aAttributes[128];
		pItemPlayer.FormatAttributes(aAttributes, sizeof(aAttributes));
		GS()->Chat(-1, "{STR} enchant {STR} {STR} {STR}", GS()->Server()->ClientName(ClientID), pItemPlayer.Info().GetName(), aEnchantBuf, aAttributes);
		GS()->ResetVotes(ClientID, pPlayer->m_OpenVoteMenu);
		return true;
	}

	if(PPSTR(CMD, "SORTEDEQUIP") == 0)
	{
		pPlayer->m_SortTabs[SORT_EQUIPING] = VoteID;
		GS()->UpdateVotes(ClientID, MenuList::MENU_EQUIPMENT);
		return true;				
	}

	return false;
}

bool InventoryJob::OnHandleMenulist(CPlayer* pPlayer, int Menulist, bool ReplaceMenu)
{
	const int ClientID = pPlayer->GetCID();
	if (ReplaceMenu)
		return false;

	if (Menulist == MenuList::MENU_INVENTORY)
	{
		pPlayer->m_LastVoteMenu = MenuList::MAIN_MENU;
		GS()->AVH(ClientID, TAB_INFO_INVENTORY, GREEN_COLOR, "Inventory Information");
		GS()->AVM(ClientID, "null", NOPE, TAB_INFO_INVENTORY, "Choose the type of items you want to show");
		GS()->AVM(ClientID, "null", NOPE, TAB_INFO_INVENTORY, "After, need select item to interact");
		GS()->AV(ClientID, "null");

		GS()->AVH(ClientID, TAB_INVENTORY_SELECT, RED_COLOR, "Inventory Select List");
		int SizeItems = GetCountItemsType(pPlayer, ItemType::TYPE_USED); 
		GS()->AVM(ClientID, "SORTEDINVENTORY", ItemType::TYPE_USED, TAB_INVENTORY_SELECT, "Used ({INT})", &SizeItems);

		SizeItems = GetCountItemsType(pPlayer, ItemType::TYPE_CRAFT);
		GS()->AVM(ClientID, "SORTEDINVENTORY", ItemType::TYPE_CRAFT, TAB_INVENTORY_SELECT, "Craft ({INT})", &SizeItems);
		
		SizeItems = GetCountItemsType(pPlayer, ItemType::TYPE_MODULE);
		GS()->AVM(ClientID, "SORTEDINVENTORY", ItemType::TYPE_MODULE, TAB_INVENTORY_SELECT, "Modules ({INT})", &SizeItems);

		SizeItems = GetCountItemsType(pPlayer, ItemType::TYPE_POTION);
		GS()->AVM(ClientID, "SORTEDINVENTORY", ItemType::TYPE_POTION, TAB_INVENTORY_SELECT, "Potion ({INT})", &SizeItems);

		SizeItems = GetCountItemsType(pPlayer, ItemType::TYPE_OTHER);
		GS()->AVM(ClientID, "SORTEDINVENTORY", ItemType::TYPE_OTHER, TAB_INVENTORY_SELECT, "Other ({INT})", &SizeItems);
		if (pPlayer->m_SortTabs[SORT_INVENTORY])	
			ListInventory(pPlayer, pPlayer->m_SortTabs[SORT_INVENTORY]);

		GS()->AddBackpage(ClientID);
		return true;
	}

	if (Menulist == MenuList::MENU_EQUIPMENT)
	{
		pPlayer->m_LastVoteMenu = MenuList::MAIN_MENU;
		GS()->AVH(ClientID, TAB_INFO_EQUIP, GREEN_COLOR, "Equip / Armor Information");
		GS()->AVM(ClientID, "null", NOPE, TAB_INFO_EQUIP, "Select tab and select armor.");
		GS()->AV(ClientID, "null");

		GS()->AVH(ClientID, TAB_EQUIP_SELECT, RED_COLOR, "Equip Select Slot");
		const char* pType[NUM_EQUIPS] = { "Hammer", "Gun", "Shotgun", "Grenade", "Rifle", "Pickaxe", "Wings", "Discord" };
		for (int i = 0; i < NUM_EQUIPS; i++)
		{
			const int ItemID = pPlayer->GetEquippedItem(i);
			InventoryItem& pItemPlayer = pPlayer->GetItem(ItemID);
			if (ItemID <= 0 || !pItemPlayer.IsEquipped())
			{
				GS()->AVM(ClientID, "SORTEDEQUIP", i, TAB_EQUIP_SELECT, "{STR} Not equipped", pType[i]);
				continue;
			}

			char aAttributes[128];
			pItemPlayer.FormatAttributes(aAttributes, sizeof(aAttributes));
			GS()->AVMI(ClientID, pItemPlayer.Info().GetIcon(), "SORTEDEQUIP", i, TAB_EQUIP_SELECT, "{STR} {STR} | {STR}", pType[i], pItemPlayer.Info().GetName(pPlayer), aAttributes);
		}

		GS()->AV(ClientID, "null");
		bool FindItem = false;
		for (const auto& it : ms_aItems[ClientID])
		{
			if (!it.second.m_Count || it.second.Info().m_Function != pPlayer->m_SortTabs[SORT_EQUIPING])
				continue;

			ItemSelected(pPlayer, it.second, true);
			FindItem = true;
		}

		if (!FindItem)
			GS()->AVL(ClientID, "null", "There are no items in this tab");

		GS()->AddBackpage(ClientID);
		return true;
	}

	return false;
}

int InventoryJob::GetCountItemsType(CPlayer *pPlayer, int Type) const
{
	int SizeItems = 0;
	const int ClientID = pPlayer->GetCID();
	for(const auto& item : ms_aItems[ClientID])
	{
		if(item.second.m_Count > 0 && item.second.Info().m_Type == Type)
			SizeItems++;
	}
	return SizeItems;
}

// TODO: FIX IT (lock .. unlock)
static std::map<int, std::mutex > lock_sleep;
void InventoryJob::AddItemSleep(int AccountID, int ItemID, int GiveCount, int Milliseconds)
{
	std::thread([this, AccountID, ItemID, GiveCount, Milliseconds]()
	{
		if(Milliseconds > 0)
			std::this_thread::sleep_for(std::chrono::milliseconds(Milliseconds));

		lock_sleep[AccountID].lock();
		const int ClientID = Job()->Account()->CheckOnlineAccount(AccountID);
		CPlayer* pPlayer = GS()->GetPlayer(ClientID, true);
		if(pPlayer)
		{
			pPlayer->GetItem(ItemID).Add(GiveCount);
			lock_sleep[AccountID].unlock();
			return;
		}

		std::shared_ptr<ResultSet> RES(SJK.SD("Count", "tw_accounts_items", "WHERE ItemID = '%d' AND OwnerID = '%d'", ItemID, AccountID));
		if(RES->next())
		{
			const int ReallyCount = (int)RES->getInt("Count") + GiveCount;
			SJK.UD("tw_accounts_items", "Count = '%d' WHERE OwnerID = '%d' AND ItemID = '%d'", ReallyCount, AccountID, ItemID);
			lock_sleep[AccountID].unlock();
			return;
		}
		SJK.ID("tw_accounts_items", "(ItemID, OwnerID, Count, Settings, Enchant) VALUES ('%d', '%d', '%d', '0', '0')", ItemID, AccountID, GiveCount);
		lock_sleep[AccountID].unlock();
	}).detach();
}