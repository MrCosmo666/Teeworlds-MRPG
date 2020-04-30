/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#include <engine/shared/config.h>
#include <game/server/gamecontext.h>
#include "ItemJob.h"

using namespace sqlstr;
std::map < int , std::map < int , ItemJob::ItemPlayer > > ItemJob::Items;
std::map < int , ItemJob::ItemInformation > ItemJob::ItemsInfo;

void ItemJob::OnInitGlobal() 
{
	boost::scoped_ptr<ResultSet> RES(SJK.SD("*", "tw_items_list", "WHERE ItemID > '0'"));
	while(RES->next())
	{
		int ItemID = (int)RES->getInt("ItemID");
		str_copy(ItemsInfo[ItemID].iItemName, RES->getString("ItemName").c_str(), sizeof(ItemsInfo[ItemID].iItemName));
		str_copy(ItemsInfo[ItemID].iItemDesc, RES->getString("ItemDesc").c_str(), sizeof(ItemsInfo[ItemID].iItemDesc));
		str_copy(ItemsInfo[ItemID].iItemIcon, RES->getString("ItemIcon").c_str(), sizeof(ItemsInfo[ItemID].iItemIcon));
		ItemsInfo[ItemID].Type = (int)RES->getInt("ItemType");
		ItemsInfo[ItemID].Function = (int)RES->getInt("ItemFunction");
		ItemsInfo[ItemID].Notify = (bool)RES->getBoolean("ItemMessage");
		ItemsInfo[ItemID].Dysenthis = (int)RES->getInt("ItemDesynthesis");
		ItemsInfo[ItemID].MinimalPrice = (int)RES->getInt("ItemAuctionPrice");
		ItemsInfo[ItemID].BonusID = (int)RES->getInt("ItemBonus");
		ItemsInfo[ItemID].BonusCount = (int)RES->getInt("ItemBonusCount");
		ItemsInfo[ItemID].MaximalEnchant = (int)RES->getInt("ItemEnchantMax");
		ItemsInfo[ItemID].iItemEnchantPrice = (int)RES->getInt("ItemEnchantPrice");
		ItemsInfo[ItemID].Dropable = (bool)RES->getBoolean("ItemDropable");
		ItemsInfo[ItemID].ItemProjID = (int)RES->getInt("ItemProjID");
	}

	// загрузить аттрибуты
	boost::scoped_ptr<ResultSet> AttributsLoadRES(SJK.SD("*", "tw_attributs", "WHERE ID > '0'"));
	while(AttributsLoadRES->next())
	{
		int AttID = AttributsLoadRES->getInt("ID");
		str_copy(CGS::AttributInfo[AttID].Name, AttributsLoadRES->getString("name").c_str(), sizeof(CGS::AttributInfo[AttID].Name));
		str_copy(CGS::AttributInfo[AttID].FieldName, AttributsLoadRES->getString("field_name").c_str(), sizeof(CGS::AttributInfo[AttID].FieldName));
		CGS::AttributInfo[AttID].ProcentID = AttributsLoadRES->getInt("procent_id");
		CGS::AttributInfo[AttID].UpgradePrice = AttributsLoadRES->getInt("price");
		CGS::AttributInfo[AttID].AtType = AttributsLoadRES->getInt("at_type");
	}
}

// Загрузка данных игрока
void ItemJob::OnInitAccount(CPlayer *pPlayer)
{
	int ClientID = pPlayer->GetCID();
	boost::scoped_ptr<ResultSet> RES(SJK.SD("ItemID, ItemCount, ItemSettings, ItemEnchant, ItemDurability", "tw_items", "WHERE OwnerID = '%d'", pPlayer->Acc().AuthID));
	while(RES->next())
	{
		int ItemID = (int)RES->getInt("ItemID");
		Items[ClientID][ItemID].Count = (int)RES->getInt("ItemCount");
		Items[ClientID][ItemID].Settings = (int)RES->getInt("ItemSettings");
		Items[ClientID][ItemID].Enchant = (int)RES->getInt("ItemEnchant");
		Items[ClientID][ItemID].Durability = (int)RES->getInt("ItemDurability");
		Items[ClientID][ItemID].SetBasic(pPlayer, ItemID);
	}		
}

void ItemJob::OnResetClientData(int ClientID)
{
	if (Items.find(ClientID) != Items.end())
		Items.erase(ClientID);
}

// Восстановить прочность всем предметам
void ItemJob::RepairDurabilityFull(CPlayer *pPlayer)
{ 
	int ClientID = pPlayer->GetCID();
	SJK.UD("tw_items", "ItemDurability = '100' WHERE OwnerID = '%d'", pPlayer->Acc().AuthID);
	for(auto& it : Items[ClientID])
		it.second.Durability = 100;
}

// Установить прочность предмету
bool ItemJob::SetDurability(CPlayer *pPlayer, int ItemID, int Durability)
{
	ResultSet* RES = SJK.SD("ID, ItemSettings", "tw_items", "WHERE ItemID = '%d' AND OwnerID = '%d'", ItemID, pPlayer->Acc().AuthID);
	if(RES->next())
	{
		const int ID = RES->getInt("ID"); 
		SJK.UD("tw_items", "ItemDurability = '%d' WHERE ID = '%d'", Durability, ID);
		pPlayer->GetItem(ItemID).Durability = Durability;
		return true;		
	}	
	return false;	
}

// Устанавливаем настройку предмету
bool ItemJob::SetSettings(CPlayer *pPlayer, int ItemID, int Settings)
{
	// устанавливаем настройку и сохраняем
	boost::scoped_ptr<ResultSet> RES(SJK.SD("ID, ItemSettings", "tw_items", "WHERE ItemID = '%d' AND OwnerID = '%d'", ItemID, pPlayer->Acc().AuthID));
	if(RES->next())
	{
		const int ID = RES->getInt("ID"); 
		SJK.UD("tw_items", "ItemSettings = '%d' WHERE ID = '%d'", Settings, ID);
		pPlayer->GetItem(ItemID).Settings = Settings;
		return true;
	}	
	return false;		
}

// Устанавливаем настройку предмету
bool ItemJob::SetEnchant(CPlayer *pPlayer, int ItemID, int Enchant)
{
	// устанавливаем настройку и сохраняем
	boost::scoped_ptr<ResultSet> RES(SJK.SD("ID, ItemEnchant", "tw_items", "WHERE ItemID = '%d' AND OwnerID = '%d'", ItemID, pPlayer->Acc().AuthID));
	if(RES->next())
	{
		const int ID = RES->getInt("ID"); 
		SJK.UD("tw_items", "ItemEnchant = '%d' WHERE ID = '%d'", Enchant, ID);
		pPlayer->GetItem(ItemID).Enchant = Enchant;
		return true;		
	}	
	return false;		
}

// Лист предметов
void ItemJob::ListInventory(CPlayer *pPlayer, int TypeList, bool SortedFunction)
{
	const int ClientID = pPlayer->GetCID();
	GS()->AV(ClientID, "null", "");

	// показываем лист предметов игроку 
	bool Found = false;	
	for(const auto& it : Items[ClientID])
	{
		if(!it.second.Count || ((SortedFunction && it.second.Info().Function != TypeList) || (!SortedFunction && it.second.Info().Type != TypeList)))
			continue;

		ItemSelected(pPlayer, it.second);
		Found = true;
	}
	if(!Found) GS()->AVL(ClientID, "null", "There are no items in this tab");
}

// Выдаем предмет обработка повторная
void ItemJob::GiveItem(short *SecureCode, CPlayer *pPlayer, int ItemID, int Count, int Settings, int Enchant)
{
	// проверяем выдан ли и вправду предмет
	const int ClientID = pPlayer->GetCID();
	*SecureCode = SecureCheck(pPlayer, ItemID, Count, Settings, Enchant);
	if(*SecureCode != 1) return;

	// обновляем значения в базе
	SJK.UD("tw_items", "ItemCount = '%d', ItemSettings = '%d', ItemEnchant = '%d' WHERE ItemID = '%d' AND OwnerID = '%d'", 
		Items[ClientID][ItemID].Count, Items[ClientID][ItemID].Settings, Items[ClientID][ItemID].Enchant, ItemID, pPlayer->Acc().AuthID);
}

// Выдаем предмет обработка первичная
int ItemJob::SecureCheck(CPlayer *pPlayer, int ItemID, int Count, int Settings, int Enchant)
{
	// проверяем инициализируем и добавляем предмет
	const int ClientID = pPlayer->GetCID();
	std::unique_ptr<ResultSet> RES(SJK.SD("ItemCount, ItemSettings", "tw_items", "WHERE ItemID = '%d' AND OwnerID = '%d'", ItemID, pPlayer->Acc().AuthID));
	if(RES->next())
	{
		Items[ClientID][ItemID].Count = RES->getInt("ItemCount")+Count;
		Items[ClientID][ItemID].Settings = RES->getInt("ItemSettings")+Settings;
		Items[ClientID][ItemID].Enchant = Enchant;
		return 1;	
	}
	// создаем предмет если не найден
	Items[ClientID][ItemID].Count = Count;
	Items[ClientID][ItemID].Settings = Settings;
	Items[ClientID][ItemID].Enchant = Enchant;
	Items[ClientID][ItemID].Durability = 100;
	SJK.ID("tw_items", "(ItemID, OwnerID, ItemCount, ItemSettings, ItemEnchant) VALUES ('%d', '%d', '%d', '%d', '%d');",
		ItemID, pPlayer->Acc().AuthID, Count, Settings, Enchant);
	return 2;
}

// удаляем предмет второстепенная обработка
void ItemJob::RemoveItem(short *SecureCode, CPlayer *pPlayer, int ItemID, int Count, int Settings)
{
	*SecureCode = DeSecureCheck(pPlayer, ItemID, Count, Settings);
	if(*SecureCode != 1) return;

	// если прошла и предметов больше удаляемых то обновляем таблицу
	SJK.UD("tw_items", "ItemCount = ItemCount - '%d', ItemSettings = ItemSettings - '%d' "
		"WHERE ItemID = '%d' AND OwnerID = '%d'", Count, Settings, ItemID, pPlayer->Acc().AuthID);	
}

// удаление предмета первостепенная обработка
int ItemJob::DeSecureCheck(CPlayer *pPlayer, int ItemID, int Count, int Settings)
{
	// проверяем в базе данных и проверяем 
	const int ClientID = pPlayer->GetCID();
	boost::scoped_ptr<ResultSet> RES(SJK.SD("ItemCount, ItemSettings", "tw_items", "WHERE ItemID = '%d' AND OwnerID = '%d'", ItemID, pPlayer->Acc().AuthID));
	if(RES->next())
	{
		// обновляем если количество больше
		if(RES->getInt("ItemCount") > Count)
		{
			Items[ClientID][ItemID].Count = RES->getInt("ItemCount")-Count;
			Items[ClientID][ItemID].Settings = RES->getInt("ItemSettings")-Settings;
			return 1;		
		}
		// удаляем предмет если кол-во меньше положенного
		Items[ClientID][ItemID].Count = 0;
		Items[ClientID][ItemID].Settings = 0;
		Items[ClientID][ItemID].Enchant = 0;
		SJK.DD("tw_items", "WHERE ItemID = '%d' AND OwnerID = '%d'", ItemID, pPlayer->Acc().AuthID);
		return 2;		
	}
	// суда мы заходим если предметов нет и нечего удалять
	Items[ClientID][ItemID].Count = 0;
	Items[ClientID][ItemID].Settings = 0;
	Items[ClientID][ItemID].Enchant = 0;	
	return 0;		
}

int ItemJob::ActionItemCountAllowed(CPlayer *pPlayer, int ItemID)
{
	int ClientID = pPlayer->GetCID();
	int AvailableCount = Job()->Quest()->QuestingAllowedItemsCount(pPlayer, ItemID);
	if (AvailableCount <= 0)
	{
		GS()->Chat(ClientID, "This count of items that you have, iced for the quest!");
		GS()->Chat(ClientID, "Can see in which quest they are required in Adventure journal!");
		return -1;
	}
	return AvailableCount;
}

void ItemJob::ItemSelected(CPlayer* pPlayer, const ItemPlayer& PlItem, bool Dress)
{
	const int ClientID = pPlayer->GetCID();
	const int ItemID = PlItem.GetID();
	const int HideID = NUM_TAB_MENU + ItemID;
	const char* NameItem = PlItem.Info().GetName(pPlayer);

	// зачеровыванный или нет
	if (PlItem.Info().IsEnchantable())
	{
		char aEnchantSize[16];
		str_format(aEnchantSize, sizeof(aEnchantSize), " [+%d]", PlItem.Enchant);
		GS()->AVHI(ClientID, PlItem.Info().GetIcon(), HideID, LIGHT_RED_COLOR, "{STR}{STR} {STR}",
			NameItem, (PlItem.Enchant > 0 ? aEnchantSize : "\0"), (PlItem.Settings ? " ✔" : "\0"));
	}
	else
	{
		GS()->AVHI(ClientID, PlItem.Info().GetIcon(), HideID, LIGHT_RED_COLOR, "{STR}{STR} x{INT}",
			(PlItem.Settings ? "Dressed - " : "\0"), NameItem, &PlItem.Count);
	}

	const char* DescItem = PlItem.Info().GetDesc(pPlayer);
	GS()->AVM(ClientID, "null", NOPE, HideID, "{STR}", DescItem);

	// бонус предметов
	if (PlItem.Info().IsEnchantable())
	{
		int BonusCountAct = PlItem.Info().BonusCount * (PlItem.Enchant + 1);
		GS()->AVM(ClientID, "null", NOPE, HideID, "Astro stats +{INT} {STR}", &BonusCountAct, pPlayer->AtributeName(PlItem.Info().BonusID));
	}

	// используемое или нет
	if (PlItem.Info().Function == FUNCTION_ONE_USED || PlItem.Info().Function == FUNCTION_USED)
	{
		char aBuf[64];
		str_format(aBuf, sizeof(aBuf), "Bind command \"/useitem %d'\"", ItemID);
		GS()->AVM(ClientID, "null", NOPE, HideID, "{STR}", aBuf);
		GS()->AVM(ClientID, "IUSE", ItemID, HideID, "Use {STR}", NameItem);
	}

	// зелье или нет
	if (PlItem.Info().Type == ItemType::TYPE_POTION)
	{
		GS()->AVM(ClientID, "ISETTINGS", ItemID, HideID, "Auto use {STR} - {STR}", NameItem, (PlItem.Settings ? "Enable" : "Disable"));
	}

	// поставить дома предмет
	if (PlItem.Info().Type == ItemType::TYPE_DECORATION)
	{
		GS()->AVM(ClientID, "DECOSTART", ItemID, HideID, "Added {STR} to your house", NameItem);
		GS()->AVM(ClientID, "DECOGUILDSTART", ItemID, HideID, "Added {STR} to your guild house", NameItem);
	}

	// установить предмет как расстение
	if (PlItem.Info().Function == FUNCTION_PLANTS)
	{
		const int HouseID = Job()->House()->OwnerHouseID(pPlayer->Acc().AuthID);
		const int PlantItemID = Job()->House()->GetPlantsID(HouseID);
		if (PlantItemID != ItemID)
			GS()->AVD(ClientID, "HOMEPLANTSET", ItemID, 20000, HideID, "Change plants {STR} to house (20000 items)", NameItem);
		else
			GS()->AVL(ClientID, "null", "▲ This plant is active in the house ▲");
	}

	// снаряжение или настройка
	if (PlItem.Info().Type == ItemType::TYPE_EQUIP || PlItem.Info().Function == FUNCTION_SETTINGS)
	{
		GS()->AVM(ClientID, "ISETTINGS", ItemID, HideID, "{STR} {STR}", (PlItem.Settings ? "Undress" : "Equip"), NameItem);
	}

	// десинтез или уничтожение
	if (PlItem.Info().Dysenthis > 0)
	{
		GS()->AVM(ClientID, "IDESYNTHESIS", ItemID, HideID, "Disassemble {STR} (+{INT}{STR} - 1 item)",
			NameItem, &PlItem.Info().Dysenthis, (PlItem.Info().Function == FUNCTION_PLANTS ? "goods" : "mat"));
	}

	// можно ли дропнуть 
	if (PlItem.Info().Dropable)
	{
		GS()->AVM(ClientID, "IDROP", ItemID, HideID, "Drop {STR}", NameItem);
	}

	if (PlItem.Info().IsEnchantable())
	{
		int Price = PlItem.EnchantMaterCount();
		GS()->AVM(ClientID, "IENCHANT", ItemID, HideID, "Enchant {STR}+{INT} ({INT} material)", NameItem, &PlItem.Enchant, &Price);
	}

	// аукцион
	if (PlItem.Info().MinimalPrice)
		GS()->AVM(ClientID, "AUCTIONSLOT", ItemID, HideID, "Create Slot Auction {STR}", NameItem);
}

bool ItemJob::OnParseVotingMenu(CPlayer *pPlayer, const char *CMD, const int VoteID, const int VoteID2, int Get, const char *GetText)
{
	const int ClientID = pPlayer->GetCID();

	// сортировка предметов
	if(PPSTR(CMD, "SORTEDINVENTORY") == 0)
	{
		pPlayer->m_SortTabs[SORTINVENTORY] = VoteID;
		GS()->VResetVotes(ClientID, MenuList::MENU_INVENTORY);
		return true;
	}

	// выброс предмета
	if(PPSTR(CMD, "IDROP") == 0)
	{
		if (!pPlayer->GetCharacter())
			return true;

		int AvailableCount = ActionItemCountAllowed(pPlayer, VoteID);
		if (AvailableCount <= 0)
			return true;

		if (Get > AvailableCount)
			Get = AvailableCount;

		ItemPlayer& PlItem = pPlayer->GetItem(VoteID);
		vec2 Force(pPlayer->GetCharacter()->m_Core.m_Input.m_TargetX, pPlayer->GetCharacter()->m_Core.m_Input.m_TargetY);
		if(length(Force) > 8.0f)
			Force = normalize(Force) * 8.0f;

		GS()->CreateDropItem(pPlayer->GetCharacter()->m_Core.m_Pos, -1, PlItem, Get, Force);

		GS()->SBL(ClientID, BroadcastPriority::BROADCAST_GAME_WARNING, 100, "You drop {STR}x{INT}", PlItem.Info().GetName(pPlayer), &Get);
		GS()->ResetVotes(ClientID, pPlayer->m_OpenVoteMenu);
		return true;
	}

	// использование предмета
	if(PPSTR(CMD, "IUSE") == 0)
	{
		int AvailableCount = ActionItemCountAllowed(pPlayer, VoteID);
		if (AvailableCount <= 0)
			return true;

		if (Get > AvailableCount)
			Get = AvailableCount;

		// проверяем если по функции он используется 1 раз
		ItemPlayer &PlItem = pPlayer->GetItem(VoteID);
		if(PlItem.Info().Function == FUNCTION_ONE_USED)
			Get = 1;

		UseItem(ClientID, VoteID, Get);
		return true;
	}

	// десинтез предмета
	if(PPSTR(CMD, "IDESYNTHESIS") == 0)
	{
		int AvailableCount = ActionItemCountAllowed(pPlayer, VoteID);
		if (AvailableCount <= 0)
			return true;

		if (Get > AvailableCount)
			Get = AvailableCount;

		ItemPlayer &PlItem = pPlayer->GetItem(VoteID);
		const int ItemGive = (PlItem.Info().Function == FUNCTION_PLANTS ? (int)itGoods : (int)itMaterial);
		ItemPlayer &ItemMaterial = pPlayer->GetItem(ItemGive); 

		int DesCount = PlItem.Info().Dysenthis * Get;
		if(PlItem.Remove(Get) && ItemMaterial.Add(DesCount))
		{
			GS()->Chat(ClientID, "Disassemble {STR}x{INT}, you receive {INT} {STR}(s)", 
				PlItem.Info().GetName(pPlayer), &Get, &DesCount, ItemMaterial.Info().GetName(pPlayer));
			GS()->ResetVotes(ClientID, pPlayer->m_OpenVoteMenu);
		}
		return true;
	}

	// настройка предмета
	if(PPSTR(CMD, "ISETTINGS") == 0)
	{
		ItemPlayer& EquipItem = pPlayer->GetItem(VoteID);
		EquipItem.EquipItem();
		if(EquipItem.Info().Function == EQUIP_DISCORD)
			GS()->Mmo()->SaveAccount(pPlayer, SaveType::SAVE_STATS);
		else if(EquipItem.Info().Type == ItemType::TYPE_EQUIP)
			GS()->ChangeEquipSkin(ClientID, VoteID);

		GS()->ResetVotes(ClientID, pPlayer->m_OpenVoteMenu);
		return true;
	}

	// зачарование
	if(PPSTR(CMD, "IENCHANT") == 0)
	{
		ItemPlayer &PlItem = pPlayer->GetItem(VoteID);
		if(PlItem.Enchant >= PlItem.Info().MaximalEnchant)
		{
			GS()->Chat(ClientID, "You enchant max level for this item!");
			return true;			
		}

		int Price = PlItem.EnchantMaterCount();
		ItemPlayer &PlMaterial = pPlayer->GetItem(itMaterial);
		if(Price > PlMaterial.Count)
		{
			GS()->Chat(ClientID, "You need {INT} Your {INT} materials!", &Price, &PlMaterial.Count);
			return true;
		}

		if(PlMaterial.Remove(Price, 0))
		{
			int EnchantLevel = PlItem.Enchant+1;
			int BonusID = PlItem.Info().BonusID;
			int BonusCount = PlItem.Info().BonusCount*(EnchantLevel+1);

			PlItem.SetEnchant(EnchantLevel);
			GS()->Chat(-1, "{STR} enchant {STR}+{INT}({STR} +{INT})", 
				GS()->Server()->ClientName(ClientID), PlItem.Info().GetName(), &EnchantLevel, pPlayer->AtributeName(BonusID), &BonusCount);
		
			if (EnchantLevel >= EFFECTENCHANT)
			{
				GS()->SendEquipItem(ClientID, -1);
			}
			GS()->ResetVotes(ClientID, pPlayer->m_OpenVoteMenu);
		}
		return true;
	}

	// сортировка надевания
	if(PPSTR(CMD, "SORTEDEQUIP") == 0)
	{
		pPlayer->m_SortTabs[SORTEQUIP] = VoteID;
		GS()->VResetVotes(ClientID, MenuList::MENU_EQUIPMENT);
		return true;				
	}

	return false;
}

bool ItemJob::OnPlayerHandleMainMenu(CPlayer* pPlayer, int Menulist, bool ReplaceMenu)
{
	const int ClientID = pPlayer->GetCID();
	if (ReplaceMenu)
	{
		return false;
	}

	if (Menulist == MenuList::MENU_SETTINGS)
	{
		pPlayer->m_LastVoteMenu = MenuList::MAIN_MENU;

		// Настройки
		bool FoundSettings = false;
		GS()->AVH(ClientID, TAB_SETTINGS, RED_COLOR, "Some of the settings becomes valid after death");
		for (const auto& it : Items[ClientID])
		{
			const ItemPlayer ItemData = it.second;
			if (ItemData.Info().Type != ItemType::TYPE_SETTINGS || ItemData.Count <= 0)
				continue;

			GS()->AVM(ClientID, "ISETTINGS", it.first, TAB_SETTINGS, "[{STR}] {STR}", (ItemData.Settings ? "Enable" : "Disable"), ItemData.Info().GetName(pPlayer));
			FoundSettings = true;
		}
		if (!FoundSettings)
		{
			GS()->AVM(ClientID, "null", NOPE, TAB_SETTINGS, "The list of settings is empty");
		}

		// Снаряжение
		FoundSettings = false;
		GS()->AV(ClientID, "null", "");
		GS()->AVH(ClientID, TAB_SETTINGS_MODULES, GREEN_COLOR, "Sub items settings.");
		for (const auto& it : Items[ClientID])
		{
			const ItemPlayer ItemData = it.second;
			if (ItemData.Count <= 0 || ItemData.Info().Type != ItemType::TYPE_MODULE)
				continue;

			int BonusCount = ItemData.Info().BonusCount * (ItemData.Enchant + 1);
			GS()->AVMI(ClientID, ItemData.Info().GetIcon(), "ISETTINGS", it.first, TAB_SETTINGS_MODULES, "{STR}({STR} +{INT}){STR}",
				ItemData.Info().GetName(pPlayer), pPlayer->AtributeName(ItemData.Info().BonusID), &BonusCount, (ItemData.Settings ? " ✔" : "\0"));
			FoundSettings = true;
		}
		if (!FoundSettings)
		{
			GS()->AVM(ClientID, "null", NOPE, TAB_SETTINGS_MODULES, "The list of equipment sub upgrades is empty");
		}
		GS()->AddBack(ClientID);
		return true;
	}

	if (Menulist == MenuList::MENU_INVENTORY)
	{
		pPlayer->m_LastVoteMenu = MenuList::MAIN_MENU;
		GS()->AVH(ClientID, TAB_INFO_INVENTORY, GREEN_COLOR, "Inventory Information");
		GS()->AVM(ClientID, "null", NOPE, TAB_INFO_INVENTORY, "Choose the type of items you want to show");
		GS()->AVM(ClientID, "null", NOPE, TAB_INFO_INVENTORY, "After, need select item to interact");
		GS()->AV(ClientID, "null", "");

		GS()->ShowPlayerStats(pPlayer);

		GS()->AVH(ClientID, TAB_INVENTORY_SELECT, RED_COLOR, "Inventory Select List");
		GS()->AVM(ClientID, "SORTEDINVENTORY", ItemType::TYPE_USED, TAB_INVENTORY_SELECT, "Used Items");
		GS()->AVM(ClientID, "SORTEDINVENTORY", ItemType::TYPE_CRAFT, TAB_INVENTORY_SELECT, "Craft Items");
		GS()->AVM(ClientID, "SORTEDINVENTORY", ItemType::TYPE_MODULE, TAB_INVENTORY_SELECT, "Modules Items");
		GS()->AVM(ClientID, "SORTEDINVENTORY", ItemType::TYPE_POTION, TAB_INVENTORY_SELECT, "Potion Items");
		GS()->AVM(ClientID, "SORTEDINVENTORY", ItemType::TYPE_OTHER, TAB_INVENTORY_SELECT, "Other Items");
		if (pPlayer->m_SortTabs[SORTINVENTORY])	
			ListInventory(pPlayer, pPlayer->m_SortTabs[SORTINVENTORY]);

		GS()->AddBack(ClientID);
		return true;
	}
	return false;
}

void ItemJob::UseItem(int ClientID, int ItemID, int Count)
{
	CPlayer *pPlayer = GS()->GetPlayer(ClientID, true, true);
	if(!pPlayer || pPlayer->GetItem(ItemID).Count < Count) 
		return;

	ItemPlayer &PlItem = pPlayer->GetItem(ItemID);
	if(ItemID == itPotionHealthRegen && PlItem.Remove(Count, 0))
	{
		pPlayer->GiveEffect("RegenHealth", 20);
		GS()->ChatFollow(ClientID, "You used {STR}x{INT}", PlItem.Info().GetName(pPlayer), &Count);
	}

	if(ItemID == itCapsuleSurvivalExperience && PlItem.Remove(Count, 0))
	{
		int UseCount = 500*Count;
		UseCount += rand()%UseCount;
		GS()->Chat(-1, "{STR} used {STR}x{INT} and got {INT} Survival Experience.", 
			"name", GS()->Server()->ClientName(ClientID), "item", PlItem.Info().GetName(pPlayer), "itemcount", &Count, "count", &UseCount);
		pPlayer->AddExp(UseCount);		
	}

	if(ItemID == itLittleBagGold && PlItem.Remove(Count, 0))
	{
		int UseCount = 10*Count;
		UseCount += (rand()%UseCount)*5;
		GS()->Chat(-1, "{STR} used {STR}x{INT} and got {INT} gold.", 
			GS()->Server()->ClientName(ClientID), PlItem.Info().GetName(), &Count, &UseCount);
		pPlayer->AddMoney(UseCount);				
	}
	GS()->VResetVotes(ClientID, MenuList::MENU_INVENTORY);
	return;
}

const char *ItemJob::ClassItemInformation::GetName(CPlayer *pPlayer) const
{
	if(!pPlayer) return iItemName;
	return pPlayer->GS()->Server()->Localization()->Localize(pPlayer->GetLanguage(), _(iItemName));	
}

const char *ItemJob::ClassItemInformation::GetDesc(CPlayer *pPlayer) const
{
	if(!pPlayer) return iItemDesc;
	return pPlayer->GS()->Server()->Localization()->Localize(pPlayer->GetLanguage(), _(iItemDesc));	
}

bool ItemJob::ClassItemInformation::IsEnchantable() const
{
	return (BonusID > 0 && CGS::AttributInfo.find(BonusID) != CGS::AttributInfo.end() && BonusCount > 0 && MaximalEnchant > 0 && iItemEnchantPrice > 0);
}

int ItemJob::ClassItems::EnchantMaterCount() const
{
	return ItemJob::ItemsInfo[itemid_].iItemEnchantPrice*(Enchant+1);
}

bool ItemJob::ClassItems::Add(int arg_count, int arg_settings, int arg_enchant, bool arg_message)
{
	if(arg_count < 1 || !pPlayer || !pPlayer->IsAuthed()) 
		return false;

	// все что потребуется для работы функции
	CGS *GameServer = pPlayer->GS();
	const int ClientID = pPlayer->GetCID();
	if(Info().IsEnchantable())
	{
		if(Count > 0) 
		{
			pPlayer->GS()->Chat(ClientID, "This item cannot have more than 1 item");
			return false;
		}
		arg_count = 1;
	}

	// проверить пустой слот если да тогда одеть предмет
	const bool AutoEquip = (Info().Type == ItemType::TYPE_EQUIP && pPlayer->GetItemEquip(Info().Function) <= 0) || (Info().Function == FUNCTION_SETTINGS && Info().IsEnchantable());
	if(AutoEquip)
	{
		GameServer->Chat(ClientID, "Auto equip {STR} ({STR} +{INT})!", Info().GetName(pPlayer), pPlayer->AtributeName(Info().BonusID), &Info().BonusCount);
		GameServer->Chat(ClientID, "For more detail see equip/inventory/settings in vote!");

		// обновить информацию по дискорд карточки
		if(Info().Function == EQUIP_DISCORD)  
			GameServer->Mmo()->SaveAccount(pPlayer, SaveType::SAVE_STATS);
	}

	// выдаем предмет
	GameServer->Mmo()->Item()->GiveItem(&pPlayer->m_SecurCheckCode, pPlayer, itemid_, arg_count, (AutoEquip ? 1 : arg_settings), arg_enchant);
	if(pPlayer->m_SecurCheckCode <= 0) 
		return false;

	// автообновление инвентаря если открыт
	GameServer->VResetVotes(ClientID, MenuList::MENU_INVENTORY);
	GameServer->VResetVotes(ClientID, MenuList::MENU_EQUIPMENT);

	// отправить смену скина
	if(AutoEquip) 
		GameServer->ChangeEquipSkin(ClientID, itemid_);

	// если тихий режим
	if(!arg_message || Info().Type == ItemType::TYPE_SETTINGS) 
		return true;

	// информация о получении себе предмета
	if(Info().Type != -1 && itemid_ != itMoney)
		GameServer->Chat(ClientID, "You got of the {STR}x{INT}!", Info().GetName(pPlayer), &arg_count);
	else if(Info().Notify)
	{
		GameServer->Chat(-1, "{STR} got of the {STR}x{INT}!", GameServer->Server()->ClientName(ClientID), Info().GetName(), &arg_count);
	}
	return true;
}

bool ItemJob::ClassItems::Remove(int arg_removecount, int arg_settings)
{
	if(Count <= 0 || arg_removecount < 1 || !pPlayer) 
		return false;

	if(Count < arg_removecount)
		arg_removecount = Count;

	if (IsEquipped())
	{
		Settings = 0;
		int ClientID = pPlayer->GetCID();
		pPlayer->GS()->ChangeEquipSkin(ClientID, itemid_);
	}

	pPlayer->GS()->Mmo()->Item()->RemoveItem(&pPlayer->m_SecurCheckCode, pPlayer, itemid_, arg_removecount, arg_settings);
	if(pPlayer->m_SecurCheckCode <= 0) 
		return false;
	return true;
}

void ItemJob::ClassItems::SetEnchant(int arg_enchantlevel)
{
	if(!Count || !pPlayer) 
		return;

	pPlayer->GS()->Mmo()->Item()->SetEnchant(pPlayer, itemid_, arg_enchantlevel);
}

bool ItemJob::ClassItems::SetSettings(int arg_settings)
{
	if(!Count || !pPlayer) 
		return false;

	pPlayer->GS()->Mmo()->Item()->SetSettings(pPlayer, itemid_, arg_settings);
	return true;
}

bool ItemJob::ClassItems::EquipItem()
{
	if(Count <= 0 || !pPlayer) 
		return false;

	// если снаряжение
	if(Info().Type == ItemType::TYPE_EQUIP)
	{
		const int EquipID = Info().Function;
		int EquipItemID = pPlayer->GetItemEquip(EquipID, itemid_);
		while (EquipItemID >= 1)
		{
			ItemJob::ItemPlayer &EquipItem = pPlayer->GetItem(EquipItemID);
			EquipItem.Settings = 0;
			EquipItem.SetSettings(0);
			EquipItemID = pPlayer->GetItemEquip(EquipID, itemid_);
		}
	}

	// обновляем
	Settings ^= true;
	pPlayer->GS()->Mmo()->Item()->SetSettings(pPlayer, itemid_, Settings);
	pPlayer->ShowInformationStats();

	// перестановка регена
	if((Info().BonusID == Stats::StAmmoRegen || Info().BonusID == Stats::StAmmoRegenQ) && pPlayer->GetCharacter())
		pPlayer->GetCharacter()->m_AmmoRegen = pPlayer->GetAttributeCount(Stats::StAmmoRegen, true);

	return true;
}

bool ItemJob::ClassItems::IsEquipped()
{
	if ((Info().Type == ItemType::TYPE_SETTINGS || Info().Type == ItemType::TYPE_EQUIP) && Settings)
		return true;
	return false;
}

void ItemJob::ClassItems::Save()
{
	if (!pPlayer)
		return;

	SJK.UD("tw_items", "ItemCount = '%d', ItemSettings = '%d', ItemEnchant = '%d' WHERE OwnerID = '%d' AND ItemID = '%d'",
		Count, Settings, Enchant, pPlayer->Acc().AuthID, itemid_);
}