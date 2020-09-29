/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#include <teeother/components/localization.h>

#include <game/server/gamecontext.h>
#include "ItemJob.h"

using namespace sqlstr;
std::map < int , std::map < int , ItemJob::InventoryItem > > ItemJob::ms_aItems;
std::map < int , ItemJob::ItemInformation > ItemJob::ms_aItemsInfo;

void ItemJob::OnInit()
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
			str_copy(CGS::AttributInfo[AttID].Name, RES->getString("name").c_str(), sizeof(CGS::AttributInfo[AttID].Name));
			str_copy(CGS::AttributInfo[AttID].FieldName, RES->getString("field_name").c_str(), sizeof(CGS::AttributInfo[AttID].FieldName));
			CGS::AttributInfo[AttID].UpgradePrice = RES->getInt("price");
			CGS::AttributInfo[AttID].AtType = RES->getInt("at_type");
		}
	});
}

void ItemJob::OnInitAccount(CPlayer *pPlayer)
{
	const int ClientID = pPlayer->GetCID();
	std::shared_ptr<ResultSet> RES(SJK.SD("ItemID, Count, Settings, Enchant, Durability", "tw_accounts_items", "WHERE OwnerID = '%d'", pPlayer->Acc().m_AuthID));
	while(RES->next())
	{
		int ItemID = (int)RES->getInt("ItemID");
		ms_aItems[ClientID][ItemID] = InventoryItem(pPlayer, ItemID);
		ms_aItems[ClientID][ItemID].m_Count = (int)RES->getInt("Count");
		ms_aItems[ClientID][ItemID].m_Settings = (int)RES->getInt("Settings");
		ms_aItems[ClientID][ItemID].m_Enchant = (int)RES->getInt("Enchant");
		ms_aItems[ClientID][ItemID].m_Durability = (int)RES->getInt("Durability");
	}		
}

void ItemJob::OnResetClient(int ClientID)
{
	if (ms_aItems.find(ClientID) != ms_aItems.end())
		ms_aItems.erase(ClientID);
}

void ItemJob::RepairDurabilityFull(CPlayer *pPlayer)
{ 
	int ClientID = pPlayer->GetCID();
	SJK.UD("tw_accounts_items", "Durability = '100' WHERE OwnerID = '%d'", pPlayer->Acc().m_AuthID);
	for(auto& it : ms_aItems[ClientID])
		it.second.m_Durability = 100;
}

void ItemJob::FormatAttributes(InventoryItem& pItem, int size, char* pformat)
{
	dynamic_string Buffer;
	for (int i = 0; i < STATS_MAX_FOR_ITEM; i++)
	{
		const int BonusID = pItem.Info().m_aAttribute[i];
		const int BonusCount = pItem.GetEnchantStats(BonusID);
		if (BonusID <= 0)
			continue;

		char aBuf[64];
		str_format(aBuf, sizeof(aBuf), "%s+%d ", GS()->AtributeName(BonusID), BonusCount);
		Buffer.append_at(Buffer.length(), aBuf);
	}
	str_copy(pformat, Buffer.buffer(), size);
	Buffer.clear();
}

void ItemJob::FormatAttributes(ItemInformation& pInfoItem, int Enchant, int size, char* pformat)
{
	dynamic_string Buffer;
	for (int i = 0; i < STATS_MAX_FOR_ITEM; i++)
	{
		int BonusID = pInfoItem.m_aAttribute[i];
		int BonusCount = pInfoItem.GetInfoEnchantStats(BonusID, Enchant);
		if (BonusID <= 0 || BonusCount <= 0)
			continue;

		char aBuf[64];
		str_format(aBuf, sizeof(aBuf), "%s+%d ", GS()->AtributeName(BonusID), BonusCount);
		Buffer.append_at(Buffer.length(), aBuf);
	}
	str_copy(pformat, Buffer.buffer(), size);
	Buffer.clear();
}

void ItemJob::ListInventory(CPlayer *pPlayer, int TypeList, bool SortedFunction)
{
	const int ClientID = pPlayer->GetCID();
	GS()->AV(ClientID, "null", "");

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
	if(!Found) GS()->AVL(ClientID, "null", "There are no items in this tab");
}

int ItemJob::GiveItem(CPlayer *pPlayer, int ItemID, int Count, int Settings, int Enchant)
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

int ItemJob::SecureCheck(CPlayer *pPlayer, int ItemID, int Count, int Settings, int Enchant)
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

int ItemJob::RemoveItem(CPlayer *pPlayer, int ItemID, int Count, int Settings)
{
	const int SecureID = DeSecureCheck(pPlayer, ItemID, Count, Settings);
	if(SecureID == 1)
		SJK.UD("tw_accounts_items", "Count = Count - '%d', Settings = Settings - '%d' WHERE ItemID = '%d' AND OwnerID = '%d'", Count, Settings, ItemID, pPlayer->Acc().m_AuthID);
	return SecureID;
}

int ItemJob::DeSecureCheck(CPlayer *pPlayer, int ItemID, int Count, int Settings)
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

int ItemJob::ActionItemCountAllowed(CPlayer *pPlayer, int ItemID)
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

void ItemJob::ItemSelected(CPlayer* pPlayer, const InventoryItem& pPlayerItem, bool Dress)
{
	const int ClientID = pPlayer->GetCID();
	const int ItemID = pPlayerItem.GetID();
	const int HideID = NUM_TAB_MENU + ItemID;
	const char* NameItem = pPlayerItem.Info().GetName(pPlayer);

	// overwritten or not
	if (pPlayerItem.Info().IsEnchantable())
	{
		char aEnchantSize[16];
		str_format(aEnchantSize, sizeof(aEnchantSize), " [+%d]", pPlayerItem.m_Enchant);
		GS()->AVHI(ClientID, pPlayerItem.Info().GetIcon(), HideID, LIGHT_GRAY_COLOR, "{STR}{STR} {STR}",
			NameItem, (pPlayerItem.m_Enchant > 0 ? aEnchantSize : "\0"), (pPlayerItem.m_Settings ? " ✔" : "\0"));
		GS()->AVM(ClientID, "null", NOPE, HideID, "{STR}", pPlayerItem.Info().GetDesc(pPlayer));

		char aAttributes[64];
		FormatAttributes(pPlayerItem.Info(), pPlayerItem.m_Enchant, sizeof(aAttributes), aAttributes);
		GS()->AVM(ClientID, "null", NOPE, HideID, "{STR}", aAttributes);
	}
	else
	{
		GS()->AVHI(ClientID, pPlayerItem.Info().GetIcon(), HideID, LIGHT_GRAY_COLOR, "{STR}{STR} x{INT}",
			(pPlayerItem.m_Settings ? "Dressed - " : "\0"), NameItem, &pPlayerItem.m_Count);
		GS()->AVM(ClientID, "null", NOPE, HideID, "{STR}", pPlayerItem.Info().GetDesc(pPlayer));
	}

	if (pPlayerItem.Info().m_Function == FUNCTION_ONE_USED || pPlayerItem.Info().m_Function == FUNCTION_USED)
	{
		char aBuf[64];
		str_format(aBuf, sizeof(aBuf), "Bind command \"/useitem %d'\"", ItemID);
		GS()->AVM(ClientID, "null", NOPE, HideID, "{STR}", aBuf);
		GS()->AVM(ClientID, "IUSE", ItemID, HideID, "Use {STR}", NameItem);
	}

	if (pPlayerItem.Info().m_Type == ItemType::TYPE_POTION)
		GS()->AVM(ClientID, "ISETTINGS", ItemID, HideID, "Auto use {STR} - {STR}", NameItem, (pPlayerItem.m_Settings ? "Enable" : "Disable"));
	else if (pPlayerItem.Info().m_Type == ItemType::TYPE_DECORATION)
	{
		GS()->AVM(ClientID, "DECOSTART", ItemID, HideID, "Added {STR} to your house", NameItem);
		GS()->AVM(ClientID, "DECOGUILDSTART", ItemID, HideID, "Added {STR} to your guild house", NameItem);
	}
	else if(pPlayerItem.Info().m_Type == ItemType::TYPE_EQUIP || pPlayerItem.Info().m_Function == FUNCTION_SETTINGS)
	{
		if((pPlayerItem.Info().m_Function == EQUIP_HAMMER && pPlayerItem.IsEquipped()))
			GS()->AVM(ClientID, "null", NOPE, HideID, "You can not undress equiping hammer", NameItem);
		else
			GS()->AVM(ClientID, "ISETTINGS", ItemID, HideID, "{STR} {STR}", (pPlayerItem.m_Settings ? "Undress" : "Equip"), NameItem);
	}

	if (pPlayerItem.Info().m_Function == FUNCTION_PLANTS)
	{
		const int HouseID = Job()->House()->OwnerHouseID(pPlayer->Acc().m_AuthID);
		const int PlantItemID = Job()->House()->GetPlantsID(HouseID);
		if(HouseID > 0 && PlantItemID != ItemID)
		{
			const int random_change = random_int() % 900;
			GS()->AVD(ClientID, "HOMEPLANTSET", ItemID, random_change, HideID, "To plant {STR}, to house (0.06%)", NameItem);
		}
	}

	if (pPlayerItem.Info().IsEnchantable() && !pPlayerItem.IsEnchantMaxLevel())
	{
		const int Price = pPlayerItem.GetEnchantPrice();
		GS()->AVM(ClientID, "IENCHANT", ItemID, HideID, "Enchant {STR} ({INT} materials)", NameItem, &Price);
	}

	if (ItemID == pPlayer->GetEquippedItem(EQUIP_HAMMER))
		return;

	if (pPlayerItem.Info().m_Dysenthis > 0)
		GS()->AVM(ClientID, "IDESYNTHESIS", ItemID, HideID, "Disassemble {STR} (+{INT} materials)", NameItem, &pPlayerItem.Info().m_Dysenthis);

	GS()->AVM(ClientID, "IDROP", ItemID, HideID, "Drop {STR}", NameItem);

	if (pPlayerItem.Info().m_MinimalPrice > 0)
		GS()->AVM(ClientID, "AUCTIONSLOT", ItemID, HideID, "Create Slot Auction {STR}", NameItem);
}

bool ItemJob::OnVotingMenu(CPlayer *pPlayer, const char *CMD, const int VoteID, const int VoteID2, int Get, const char *GetText)
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

		if (Get > AvailableCount)
			Get = AvailableCount;

		InventoryItem& pPlayerDropItem = pPlayer->GetItem(VoteID);
		vec2 Force(pPlayer->GetCharacter()->m_Core.m_Input.m_TargetX, pPlayer->GetCharacter()->m_Core.m_Input.m_TargetY);
		if(length(Force) > 8.0f)
			Force = normalize(Force) * 8.0f;

		GS()->CreateDropItem(pPlayer->GetCharacter()->m_Core.m_Pos, -1, pPlayerDropItem, Get, Force);

		GS()->SBL(ClientID, BroadcastPriority::BROADCAST_GAME_WARNING, 100, "You drop {STR}x{INT}", pPlayerDropItem.Info().GetName(pPlayer), &Get);
		GS()->ResetVotes(ClientID, pPlayer->m_OpenVoteMenu);
		return true;
	}

	if(PPSTR(CMD, "IUSE") == 0)
	{
		int AvailableCount = ActionItemCountAllowed(pPlayer, VoteID);
		if (AvailableCount <= 0)
			return true;

		if (Get > AvailableCount)
			Get = AvailableCount;

		ItemInformation &pInformationItem = GS()->GetItemInfo(VoteID);
		if(pInformationItem.m_Function == FUNCTION_ONE_USED)
			Get = 1;

		UseItem(ClientID, VoteID, Get);
		return true;
	}

	if(PPSTR(CMD, "IDESYNTHESIS") == 0)
	{
		int AvailableCount = ActionItemCountAllowed(pPlayer, VoteID);
		if (AvailableCount <= 0)
			return true;

		if (Get > AvailableCount)
			Get = AvailableCount;

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

		InventoryItem& pPlayerSelectedItem = pPlayer->GetItem(VoteID);
		pPlayerSelectedItem.Equip();
		if(pPlayerSelectedItem.Info().m_Function == EQUIP_DISCORD)
			GS()->Mmo()->SaveAccount(pPlayer, SaveType::SAVE_STATS);

		GS()->CreatePlayerSound(ClientID, SOUND_ITEM_EQUIP);
		GS()->ResetVotes(ClientID, pPlayer->m_OpenVoteMenu);
		return true;
	}

	if(PPSTR(CMD, "IENCHANT") == 0)
	{
		InventoryItem &pPlayerSelectedItem = pPlayer->GetItem(VoteID);
		if(pPlayerSelectedItem.IsEnchantMaxLevel())
		{
			GS()->Chat(ClientID, "You enchant max level for this item!");
			return true;			
		}

		const int Price = pPlayerSelectedItem.GetEnchantPrice();
		InventoryItem &pPlayerMaterialItem = pPlayer->GetItem(itMaterial);
		if(Price > pPlayerMaterialItem.m_Count)
		{
			GS()->Chat(ClientID, "You need {INT} Your {INT} materials!", &Price, &pPlayerMaterialItem.m_Count);
			return true;
		}

		if(pPlayerMaterialItem.Remove(Price, 0))
		{
			const int EnchantLevel = pPlayerSelectedItem.m_Enchant+1;
			pPlayerSelectedItem.SetEnchant(EnchantLevel);
			if (pPlayerSelectedItem.IsEnchantMaxLevel())
				GS()->SendEquipItem(ClientID, -1);

			char aAttributes[128];
			FormatAttributes(pPlayerSelectedItem, sizeof(aAttributes), aAttributes);
			GS()->Chat(-1, "{STR} enchant {STR}+{INT} {STR}", GS()->Server()->ClientName(ClientID), pPlayerSelectedItem.Info().GetName(), &EnchantLevel, aAttributes);
			GS()->ResetVotes(ClientID, pPlayer->m_OpenVoteMenu);
		}
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

bool ItemJob::OnHandleMenulist(CPlayer* pPlayer, int Menulist, bool ReplaceMenu)
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
		GS()->AV(ClientID, "null", "");

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

		GS()->AddBack(ClientID);
		return true;
	}

	if (Menulist == MenuList::MENU_EQUIPMENT)
	{
		pPlayer->m_LastVoteMenu = MenuList::MAIN_MENU;
		GS()->AVH(ClientID, TAB_INFO_EQUIP, GREEN_COLOR, "Equip / Armor Information");
		GS()->AVM(ClientID, "null", NOPE, TAB_INFO_EQUIP, "Select tab and select armor.");
		GS()->AV(ClientID, "null", "");

		GS()->AVH(ClientID, TAB_EQUIP_SELECT, RED_COLOR, "Equip Select Slot");
		const char* pType[NUM_EQUIPS] = { "Hammer", "Gun", "Shotgun", "Grenade", "Rifle", "Pickaxe", "Wings", "Discord" };
		for (int i = 0; i < NUM_EQUIPS; i++)
		{
			const int ItemID = pPlayer->GetEquippedItem(i);
			ItemJob::InventoryItem& pPlayerItem = pPlayer->GetItem(ItemID);
			if (ItemID <= 0 || !pPlayerItem.IsEquipped())
			{
				GS()->AVM(ClientID, "SORTEDEQUIP", i, TAB_EQUIP_SELECT, "{STR} Not equipped", pType[i]);
				continue;
			}

			char aAttributes[128];
			FormatAttributes(pPlayerItem, sizeof(aAttributes), aAttributes);
			GS()->AVMI(ClientID, pPlayerItem.Info().GetIcon(), "SORTEDEQUIP", i, TAB_EQUIP_SELECT, "{STR} {STR} | {STR}", pType[i], pPlayerItem.Info().GetName(pPlayer), aAttributes);
		}

		GS()->AV(ClientID, "null", "");
		bool FindItem = false;
		for (const auto& it : ItemJob::ms_aItems[ClientID])
		{
			if (!it.second.m_Count || it.second.Info().m_Function != pPlayer->m_SortTabs[SORT_EQUIPING])
				continue;

			ItemSelected(pPlayer, it.second, true);
			FindItem = true;
		}

		if (!FindItem)
			GS()->AVL(ClientID, "null", "There are no items in this tab");

		GS()->AddBack(ClientID);
		return true;
	}

	return false;
}

int randomRangecount(int startrandom, int endrandom, int count)
{
	int result = 0;
	for(int i = 0; i < count; i++)
	{
		int random = startrandom + random_int() % (endrandom - startrandom);
		result += random;
	}
	return result;
}

void ItemJob::UseItem(int ClientID, int ItemID, int Count)
{
	CPlayer *pPlayer = GS()->GetPlayer(ClientID, true, true);
	if(!pPlayer || pPlayer->GetItem(ItemID).m_Count < Count) 
		return;

	InventoryItem &PlItem = pPlayer->GetItem(ItemID);
	if(ItemID == itPotionHealthRegen && PlItem.Remove(Count, 0))
	{
		pPlayer->GiveEffect("RegenHealth", 15);
		GS()->ChatFollow(ClientID, "You used {STR}x{INT}", PlItem.Info().GetName(pPlayer), &Count);
	}
	
	if(ItemID == itPotionManaRegen && PlItem.Remove(Count, 0))
	{
		pPlayer->GiveEffect("RegenMana", 15);
		GS()->ChatFollow(ClientID, "You used {STR}x{INT}", PlItem.Info().GetName(pPlayer), &Count);
	}

	if(ItemID == itPotionResurrection && PlItem.Remove(Count, 0))
	{
		pPlayer->GetTempData().m_TempSafeSpawn = false;
		pPlayer->GetTempData().m_TempHealth = pPlayer->GetStartHealth();
		GS()->ChatFollow(ClientID, "You used {STR}x{INT}", PlItem.Info().GetName(pPlayer), &Count);
	}

	if(ItemID == itTicketDiscountCraft)
	{
		GS()->Chat(ClientID, "This item can only be used (Auto Use, and then craft).");
		return;
	}

	if(ItemID == itCapsuleSurvivalExperience && PlItem.Remove(Count, 0)) 
	{
		int Getting = randomRangecount(10, 50, Count);
		GS()->Chat(-1, "{STR} used {STR}x{INT} and got {INT} Survival Experience.", GS()->Server()->ClientName(ClientID), PlItem.Info().GetName(), &Count, &Getting);
		pPlayer->AddExp(Getting);
	}

	if(ItemID == itLittleBagGold && PlItem.Remove(Count, 0))
	{
		int Getting = randomRangecount(10, 50, Count);
		GS()->Chat(-1, "{STR} used {STR}x{INT} and got {INT} gold.", GS()->Server()->ClientName(ClientID), PlItem.Info().GetName(), &Count, &Getting);
		pPlayer->AddMoney(Getting);
	}

	if(ItemID == itTicketResetClassStats && PlItem.Remove(Count, 0))
	{
		int BackUpgrades = 0;
		for(const auto& at : CGS::AttributInfo)
		{
			if(str_comp_nocase(at.second.FieldName, "unfield") == 0 || at.second.UpgradePrice <= 0 || pPlayer->Acc().m_aStats[at.first] <= 0)
				continue;

			// skip weapon spreading
			if(at.second.AtType == AtributType::AtWeapon)
				continue;

			BackUpgrades += (int)(pPlayer->Acc().m_aStats[at.first] * at.second.UpgradePrice);
			pPlayer->Acc().m_aStats[at.first] = 0;
		}

		GS()->Chat(-1, "{STR} used {STR} returned {INT} upgrades.", GS()->Server()->ClientName(ClientID), PlItem.Info().GetName(), &BackUpgrades);
		pPlayer->Acc().m_Upgrade += BackUpgrades;
		Job()->SaveAccount(pPlayer, SaveType::SAVE_UPGRADES);
	}

	if(ItemID == itTicketResetWeaponStats && PlItem.Remove(Count, 0))
	{
		int BackUpgrades = 0;
		for(const auto& at : CGS::AttributInfo)
		{
			if(str_comp_nocase(at.second.FieldName, "unfield") == 0 || at.second.UpgradePrice <= 0 || pPlayer->Acc().m_aStats[at.first] <= 0)
				continue;

			// skip all stats allow only weapons
			if(at.second.AtType != AtributType::AtWeapon)
				continue;

			int BackCount = pPlayer->Acc().m_aStats[at.first];
			if(at.first == Stats::StSpreadShotgun)
				BackCount = pPlayer->Acc().m_aStats[at.first] - 3;
			else if(at.first == Stats::StSpreadGrenade || at.first == Stats::StSpreadRifle)
				BackCount = pPlayer->Acc().m_aStats[at.first] - 1;

			if(BackCount <= 0)
				continue;

			BackUpgrades += (int)(BackCount * at.second.UpgradePrice);
			pPlayer->Acc().m_aStats[at.first] -= BackCount;
		}

		GS()->Chat(-1, "{STR} used {STR} returned {INT} upgrades.", GS()->Server()->ClientName(ClientID), PlItem.Info().GetName(), &BackUpgrades);
		pPlayer->Acc().m_Upgrade += BackUpgrades;
		Job()->SaveAccount(pPlayer, SaveType::SAVE_UPGRADES);
	}

	GS()->UpdateVotes(ClientID, MenuList::MENU_INVENTORY);
	return;
}

int ItemJob::GetCountItemsType(CPlayer *pPlayer, int Type) const
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

const char *ItemJob::ClassItemInformation::GetName(CPlayer *pPlayer) const
{
	if(!pPlayer) return m_aName;
	return pPlayer->GS()->Server()->Localization()->Localize(pPlayer->GetLanguage(), m_aName);
}

const char *ItemJob::ClassItemInformation::GetDesc(CPlayer *pPlayer) const
{
	if(!pPlayer) return m_aDesc;
	return pPlayer->GS()->Server()->Localization()->Localize(pPlayer->GetLanguage(), m_aDesc);	
}

bool ItemJob::ClassItemInformation::IsEnchantable() const
{
	for (int i = 0; i < STATS_MAX_FOR_ITEM; i++)
	{
		if (CGS::AttributInfo.find(m_aAttribute[i]) != CGS::AttributInfo.end() && m_aAttribute[i] > 0 && m_aAttributeCount[i] > 0)
			return true;
	}
	return false;
}

int ItemJob::ClassItems::GetEnchantPrice() const
{
	int FinishedPrice = 0;
	for(int i = 0; i < STATS_MAX_FOR_ITEM; i++)
	{
		if(CGS::AttributInfo.find(Info().m_aAttribute[i]) == CGS::AttributInfo.end())
			continue;

		int UpgradePrice;
		const int Attribute = Info().m_aAttribute[i];
		const int TypeAttribute = CGS::AttributInfo[Attribute].AtType;

		if(TypeAttribute == AtributType::AtHardtype || Attribute == Stats::StStrength)
		{
			UpgradePrice = max(8, CGS::AttributInfo[Attribute].UpgradePrice) * 20;
		}
		else if(TypeAttribute == AtributType::AtJob || TypeAttribute == AtributType::AtWeapon || Attribute == Stats::StLuckyDropItem)
		{
			UpgradePrice = max(20, CGS::AttributInfo[Attribute].UpgradePrice) * 30;
		}
		else
		{
			UpgradePrice = max(6, CGS::AttributInfo[Attribute].UpgradePrice) * 10;
		}

		const int PercentEnchant = max(1, (int)kurosio::translate_to_procent_rest(Info().m_aAttributeCount[i], PERCENT_OF_ENCHANT));
		const int FromCalculating = PercentEnchant * (1 + m_Enchant);
		FinishedPrice += (FromCalculating * UpgradePrice);
	}
	return FinishedPrice;
}

bool ItemJob::ClassItems::SetEnchant(int Enchant)
{
	if (m_Count < 1 || !m_pPlayer || !m_pPlayer->IsAuthed())
		return false;

	m_Enchant = Enchant;
	bool Successful = Save();
	return Successful;
}

bool ItemJob::ClassItems::Add(int Count, int Settings, int Enchant, bool Message)
{
	if(Count < 1 || !m_pPlayer || !m_pPlayer->IsAuthed())
		return false;

	CGS* GameServer = m_pPlayer->GS();
	const int ClientID = m_pPlayer->GetCID();
	if(Info().IsEnchantable())
	{
		if(m_Count > 0) 
		{
			m_pPlayer->GS()->Chat(ClientID, "This item cannot have more than 1 item");
			return false;
		}
		Count = 1;
	}

	// check the empty slot if yes then put the item on
	const bool AutoEquip = (Info().m_Type == ItemType::TYPE_EQUIP && m_pPlayer->GetEquippedItem(Info().m_Function) <= 0) || (Info().m_Function == FUNCTION_SETTINGS && Info().IsEnchantable());
	if(AutoEquip)
	{
		if(Info().m_Function == EQUIP_DISCORD)  
			GameServer->Mmo()->SaveAccount(m_pPlayer, SaveType::SAVE_STATS);

		char aAttributes[128];
		GameServer->Mmo()->Item()->FormatAttributes(*this, sizeof(aAttributes), aAttributes);
		GameServer->Chat(ClientID, "Auto equip {STR} - {STR}", Info().GetName(m_pPlayer), aAttributes);
		GameServer->CreatePlayerSound(ClientID, SOUND_ITEM_EQUIP);
	}

	const int Code = GameServer->Mmo()->Item()->GiveItem(m_pPlayer, m_ItemID, Count, (AutoEquip ? 1 : Settings), Enchant);
	if(Code <= 0)
		return false;
		
	if(AutoEquip)
	{
		if(m_pPlayer->GetCharacter())
			m_pPlayer->GetCharacter()->UpdateEquipingStats(m_ItemID);
		GameServer->ChangeEquipSkin(ClientID, m_ItemID);
	}

	if(!Message || Info().m_Type == ItemType::TYPE_SETTINGS) 
		return true;

	if(Info().m_Type == ItemType::TYPE_EQUIP || Info().m_Type == ItemType::TYPE_MODULE)
		GameServer->Chat(-1, "{STR} got of the {STR}x{INT}!", GameServer->Server()->ClientName(ClientID), Info().GetName(), &Count);
	else if(Info().m_Type != -1)
		GameServer->Chat(ClientID, "You got of the {STR}x{INT}!", Info().GetName(m_pPlayer), &Count);

	return true;
}

bool ItemJob::ClassItems::Remove(int Count, int Settings)
{
	if(m_Count <= 0 || Count < 1 || !m_pPlayer) 
		return false;

	if(m_Count < Count)
		Count = m_Count;

	if (IsEquipped())
	{
		m_Settings = 0;
		m_pPlayer->GS()->ChangeEquipSkin(m_pPlayer->GetCID(), m_ItemID);
	}

	const int Code = m_pPlayer->GS()->Mmo()->Item()->RemoveItem(m_pPlayer, m_ItemID, Count, Settings);
	return (bool)(Code > 0);
}

bool ItemJob::ClassItems::SetSettings(int Settings)
{
	if (m_Count < 1 || !m_pPlayer || !m_pPlayer->IsAuthed())
		return false;

	m_Settings = Settings;
	return Save();
}

bool ItemJob::ClassItems::SetDurability(int Durability)
{
	if(m_Count < 1 || !m_pPlayer || !m_pPlayer->IsAuthed())
		return false;

	m_Durability = Durability;
	return Save();
}

bool ItemJob::ClassItems::Equip()
{
	if (m_Count < 1 || !m_pPlayer || !m_pPlayer->IsAuthed())
		return false;

	if(Info().m_Type == ItemType::TYPE_EQUIP)
	{
		const int EquipID = Info().m_Function;
		int EquipItemID = m_pPlayer->GetEquippedItem(EquipID, m_ItemID);
		while (EquipItemID >= 1)
		{
			ItemJob::InventoryItem &EquipItem = m_pPlayer->GetItem(EquipItemID);
			EquipItem.m_Settings = 0;
			EquipItem.SetSettings(0);
			EquipItemID = m_pPlayer->GetEquippedItem(EquipID, m_ItemID);
		}
	}

	m_Settings ^= true;
	if(Info().m_Type == ItemType::TYPE_EQUIP)
		m_pPlayer->GS()->ChangeEquipSkin(m_pPlayer->GetCID(), m_ItemID);

	if(m_pPlayer->GetCharacter())
		m_pPlayer->GetCharacter()->UpdateEquipingStats(m_ItemID);

	m_pPlayer->ShowInformationStats();
	return Save();
}

bool ItemJob::ClassItems::Save()
{
	if (m_pPlayer && m_pPlayer->IsAuthed())
	{
		SJK.UD("tw_accounts_items", "Count = '%d', Settings = '%d', Enchant = '%d', Durability = '%d' WHERE OwnerID = '%d' AND ItemID = '%d'", 
			m_Count, m_Settings, m_Enchant, m_Durability, m_pPlayer->Acc().m_AuthID, m_ItemID);
		return true;
	}
	return false;
}

// TODO: FIX IT (lock .. unlock)
static std::map<int, std::mutex > lock_sleep;
void ItemJob::AddItemSleep(int AccountID, int ItemID, int GiveCount, int Milliseconds)
{
	std::thread([this, AccountID, ItemID, GiveCount, Milliseconds]()
		{
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