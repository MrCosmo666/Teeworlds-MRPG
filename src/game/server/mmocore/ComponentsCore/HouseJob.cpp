/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#include <engine/shared/config.h>
#include <game/server/gamecontext.h>
#include "HouseJob.h"

#include <game/server/mmocore/GameEntities/jobitems.h>

using namespace sqlstr;
std::map < int , HouseJob::HouseList > HouseJob::Home;

// Инициализация класса
void HouseJob::OnInitWorld(const char* pWhereLocalWorld) 
{ 
	// загрузка домов
	boost::scoped_ptr<ResultSet> RES(SJK.SD("*", "tw_houses", pWhereLocalWorld));
	while(RES->next())
	{
		int HouseID = RES->getInt("ID");
		Home[HouseID].m_DoorX = RES->getInt("DoorX");
		Home[HouseID].m_DoorY = RES->getInt("DoorY");
		Home[HouseID].m_PosX = RES->getInt("PosX");
		Home[HouseID].m_PosY = RES->getInt("PosY");
		Home[HouseID].m_OwnerID = RES->getInt("OwnerID");
		Home[HouseID].m_Price = RES->getInt("Price");
		Home[HouseID].m_Bank = RES->getInt("HouseBank");
		Home[HouseID].m_WorldID = RES->getInt("WorldID");
		str_copy(Home[HouseID].m_Class, RES->getString("Class").c_str(), sizeof(Home[HouseID].m_Class));
		Home[HouseID].m_PlantID = RES->getInt("PlantID");
		Home[HouseID].m_PlantPosX = RES->getInt("PlantX");
		Home[HouseID].m_PlantPosY = RES->getInt("PlantY");
		if (GS()->GetWorldID() == Home[HouseID].m_WorldID && Home[HouseID].m_OwnerID > 0 && !Home[HouseID].m_Door)
		{
			Home[HouseID].m_Door = 0;
			Home[HouseID].m_Door = new HouseDoor(&GS()->m_World, vec2(Home[HouseID].m_DoorX, Home[HouseID].m_DoorY));
		}
	}

	// загружаем декорации
	if (m_DecorationHouse.size() <= 0)
	{
		boost::scoped_ptr<ResultSet> DecoLoadingRES(SJK.SD("*", "tw_houses_decorations", pWhereLocalWorld));
		while (DecoLoadingRES->next())
		{
			const int DecoID = DecoLoadingRES->getInt("ID");
			m_DecorationHouse[DecoID] = new CDecorationHouses(&GS()->m_World, vec2(DecoLoadingRES->getInt("X"),
				DecoLoadingRES->getInt("Y")), DecoLoadingRES->getInt("HouseID"), DecoLoadingRES->getInt("DecoID"));
		}
	}
	Job()->ShowLoadingProgress("Houses", Home.size());
	Job()->ShowLoadingProgress("Houses Decorations", m_DecorationHouse.size());
}

/*
	Later lead to quality standard code
*/
/* #########################################################################
	FUNCTIONS HOUSES PLANTS
######################################################################### */
// Изменить расстения у дома
void HouseJob::ChangePlantsID(int HouseID, int PlantID)
{
	if(Home.find(HouseID) == Home.end())
		return;

	bool Changes = false;
	for(CJobItems* pPlant = (CJobItems*)GS()->m_World.FindFirst(CGameWorld::ENTTYPE_JOBITEMS); pPlant; pPlant = (CJobItems*)pPlant->TypeNext())
	{
		if(pPlant->m_HouseID == HouseID)
		{
			pPlant->m_ItemID = PlantID;
			Changes = true;
		}
	}
	if(Changes) 
	{
		Home[HouseID].m_PlantID = PlantID;
		SJK.UD("tw_houses", "PlantID = '%d' WHERE ID = '%d'", PlantID, HouseID);
	}
}

/* #########################################################################
	FUNCTIONS HOUSES DECORATION
######################################################################### */
// добавить декорацию для дома
bool HouseJob::AddDecorationHouse(int DecoID, int HouseID, vec2 Position)
{
	if(HouseID <= 0)
		return false;

	vec2 PositionHouse = GetPositionHouse(HouseID);
	if(distance(PositionHouse, Position) > 400.0f)
		return false;

	boost::scoped_ptr<ResultSet> RES(SJK.SD("ID", "tw_houses_decorations", "WHERE HouseID = '%d'", HouseID));
	if((int)RES->rowsCount() >= g_Config.m_SvLimitDecoration) return false;

	boost::scoped_ptr<ResultSet> RES2(SJK.SD("ID", "tw_houses_decorations", "ORDER BY ID DESC LIMIT 1"));
	int InitID = (RES2->next() ? RES2->getInt("ID")+1 : 1); 

	SJK.ID("tw_houses_decorations", "(ID, DecoID, HouseID, X, Y, WorldID) VALUES ('%d', '%d', '%d', '%d', '%d', '%d')", 
		InitID, DecoID, HouseID, (int)Position.x, (int)Position.y, GS()->GetWorldID());

	m_DecorationHouse[InitID] = new CDecorationHouses(&GS()->m_World, Position, HouseID, DecoID);
	return true;
}
// Удалить декорацию
bool HouseJob::DeleteDecorationHouse(int ID)
{
	if(m_DecorationHouse.find(ID) != m_DecorationHouse.end())
	{
		if(m_DecorationHouse.at(ID))
		{
			delete m_DecorationHouse.at(ID);
			m_DecorationHouse.at(ID) = 0;
		}
		m_DecorationHouse.erase(ID);
		SJK.DD("tw_houses_decorations", "WHERE ID = '%d'", ID);
		return true;
	}
	return false;
}
// Показать меню декораций
void HouseJob::ShowDecorationList(CPlayer *pPlayer)
{
	int HouseID = PlayerHouseID(pPlayer);
	int ClientID = pPlayer->GetCID();
	for (auto deco = m_DecorationHouse.begin(); deco != m_DecorationHouse.end(); deco++)
	{
		if(deco->second && deco->second->m_HouseID == HouseID) 
		{
			GS()->AVD(ClientID, "DECODELETE", deco->first, deco->second->m_DecoID, 1, "{STR}:{INT} back to the inventory", 
				GS()->GetItemInfo(deco->second->m_DecoID).GetName(pPlayer), &deco->first);
		}
	}
}

/* #########################################################################
	GET CHECK HOUSES 
######################################################################### */
int HouseJob::GetOwnerHouse(int HouseID)
{
	if(Home.find(HouseID) != Home.end())
		return Home[HouseID].m_OwnerID;
	return -1;
}

// Узнать мир где находится дом
int HouseJob::GetWorldID(int HouseID) const
{
	if(Home.find(HouseID) != Home.end())
		return Home.at(HouseID).m_WorldID;
	return -1;
}
// Получить дом по позиции
int HouseJob::GetHouse(vec2 Pos, bool Plants)
{
	float Distance = (Plants ? 300.0f : 128.0f);
	for(const auto& ihome : Home)
	{
		if (ihome.second.m_WorldID != GS()->GetWorldID())
			continue;

		vec2 PosHome = (Plants ? vec2(ihome.second.m_PlantPosX, ihome.second.m_PlantPosY) : vec2(ihome.second.m_PosX, ihome.second.m_PosY));
		if(distance(PosHome, Pos) < Distance)
			return ihome.first;
	}
	return -1;
}
// Узнать стоимость дома по айди дома
int HouseJob::GetHousePrice(int HouseID) const	
{	 
	if(Home.find(HouseID) != Home.end()) 
		return Home.at(HouseID).m_Price;
 	return -1;
}

// Проверить открыта дверь или нет по айди дома
bool HouseJob::GetHouseDoor(int HouseID) const	
{	 
	if(Home.find(HouseID) != Home.end())	
	{
		bool DoorClose = (Home.at(HouseID).m_Door);
		return DoorClose;
	}
	return false;
}
// Позиция дома
vec2 HouseJob::GetPositionHouse(int HouseID) const	
{ 	
	if(Home.find(HouseID) != Home.end()) 
		return vec2(Home.at(HouseID).m_PosX, Home.at(HouseID).m_PosY); 

	return vec2(0, 0);
}
// Узнать имеет ли игрок дом
int HouseJob::PlayerHouseID(CPlayer *pPlayer) const
{
	for (auto ihome = Home.begin(); ihome != Home.end(); ihome++) {
		if(Home.at(ihome->first).m_OwnerID == pPlayer->Acc().AuthID)
			return ihome->first;
	}
	return -1;
}
// Узнать имеет ли игрок под айди дом
int HouseJob::OwnerHouseID(int AuthID) const
{
	for (auto ihome = Home.begin(); ihome != Home.end(); ihome++) {
		if(Home.at(ihome->first).m_OwnerID == AuthID)
			return ihome->first;
	}
	return -1;
}
// Узнать имеет ли игрок под айди дом
int HouseJob::GetPlantsID(int HouseID) const
{
	if(Home.find(HouseID) != Home.end()) 
		return Home.at(HouseID).m_PlantID;
	return -1;
}
// Класс дома
const char *HouseJob::ClassName(int HouseID) const	
{	
	if(Home.find(HouseID) != Home.end()) 
		return Home.at(HouseID).m_Class; 
	return "None";
}
// Владелец дома
const char *HouseJob::OwnerName(int HouseID) 
{	
	if(Home.find(HouseID) != Home.end())
		return Job()->PlayerName(Home[HouseID].m_OwnerID);
	return "No owner";
}

/* #########################################################################
	FUNCTIONS HOUSES 
######################################################################### */
// Покупка дома
bool HouseJob::BuyHouse(int HouseID, CPlayer *pPlayer)
{
	int ClientID = pPlayer->GetCID();
	if(PlayerHouseID(pPlayer) > 0)
	{
		GS()->Chat(ClientID, "You already have a home.");		
		return false;
	}

	boost::scoped_ptr<ResultSet> RES(SJK.SD("OwnerID, Price", "tw_houses", "WHERE ID = '%d';", HouseID));
	if(RES->next())
	{
		if(RES->getInt("OwnerID") > 0) 
			return false;

		const int Price = RES->getInt("Price");
		if(pPlayer->CheckFailMoney(Price))	
			return false;

		Home[HouseID].m_Bank = 0;
		Home[HouseID].m_OwnerID = pPlayer->Acc().AuthID;
		SJK.UD("tw_houses", "OwnerID = '%d', HouseBank = '0' WHERE ID = '%d'", Home[HouseID].m_OwnerID, HouseID);
		return true;
	}	
	return false;
}

// Продажа дома
void HouseJob::SellHouse(int HouseID)
{
	boost::scoped_ptr<ResultSet> RES(SJK.SD("OwnerID", "tw_houses", "WHERE ID = '%d'", HouseID));
	if(RES->next())
	{
		const int OwnerID = RES->getInt("OwnerID");
		const int ClientID = Job()->Account()->CheckOnlineAccount(OwnerID);
		if(ClientID >= 0)
		{
			GS()->ChatFollow(ClientID, "Your House is sold !");
			GS()->Chat(-1, "House: {INT} have been is released!", &HouseID);
			GS()->ChatDiscord(DC_SERVER_INFO, "Server information", "**[House: {INT}] have been sold!**", &HouseID);
		}
		const int Price = Home[HouseID].m_Price;
		Job()->Inbox()->SendInbox(OwnerID, "House is sold", "Your house is sold !", itGold, Price, 0);

		if(Home[HouseID].m_Door)
		{
			delete Home[HouseID].m_Door;
			Home[HouseID].m_Door = 0;
		}
		Home[HouseID].m_Bank = 50000;
		Home[HouseID].m_OwnerID = -1;
		
		// обновить голосования
		if(ClientID >= 0)
			GS()->ResetVotes(ClientID, MenuList::MAIN_MENU);

		SJK.UD("tw_houses", "OwnerID = NULL, HouseBank = '50000' WHERE ID = '%d'", HouseID);
	}
}

bool HouseJob::OnHandleTile(CCharacter* pChr, int IndexCollision)
{
	CPlayer* pPlayer = pChr->GetPlayer();
	const int ClientID = pPlayer->GetCID();

	if (pChr->GetHelper()->TileEnter(IndexCollision, TILE_PLAYER_HOUSE))
	{
		const int HouseID = GS()->Mmo()->House()->GetHouse(pChr->m_Core.m_Pos);
		if (HouseID > 0)
		{
			GS()->Chat(ClientID, "List of house, you can see on vote!");
			const int PriceHouse = GS()->Mmo()->House()->GetHousePrice(HouseID);
			GS()->SBL(ClientID, BroadcastPriority::BROADCAST_GAME_INFORMATION, 200, "House Price: {INT}gold \n"
				" Owner: {STR}.\nInformation load in vote.", &PriceHouse, GS()->Mmo()->House()->OwnerName(HouseID));
			GS()->ResetVotes(ClientID, MenuList::MAIN_MENU);
		}
		pChr->m_Core.m_ProtectHooked = pChr->m_NoAllowDamage = true;
		return true;
	}
	else if (pChr->GetHelper()->TileExit(IndexCollision, TILE_PLAYER_HOUSE))
	{
		GS()->Chat(ClientID, "You have left the active zone, menu is restored!");
		GS()->ResetVotes(ClientID, MenuList::MAIN_MENU);
		pChr->m_Core.m_ProtectHooked = pChr->m_NoAllowDamage = false;
		return true;
	}

	return false;
}

bool HouseJob::OnHandleMenulist(CPlayer* pPlayer, int Menulist, bool ReplaceMenu)
{
	int ClientID = pPlayer->GetCID();
	if (ReplaceMenu)
	{
		CCharacter* pChr = pPlayer->GetCharacter();
		if (!pChr || !pChr->IsAlive())
			return false;

		if (Menulist == MenuList::MAIN_MENU && pChr->GetHelper()->BoolIndex(TILE_PLAYER_HOUSE))
		{
			const int HouseID = GetHouse(pChr->m_Core.m_Pos);
			if (HouseID > 0)
				ShowHouseMenu(pPlayer, HouseID);

			return true;
		}
		return false;
	}

	if (Menulist == MenuList::MENU_HOUSE_DECORATION)
	{
		pPlayer->m_LastVoteMenu = MenuList::MENU_HOUSE;
		GS()->AVH(ClientID, TAB_INFO_DECORATION, GREEN_COLOR, "Decorations Information");
		GS()->AVM(ClientID, "null", NOPE, TAB_INFO_DECORATION, "Add: Select your item in list. Select (Add to house),");
		GS()->AVM(ClientID, "null", NOPE, TAB_INFO_DECORATION, "later press (ESC) and mouse select position");
		GS()->AVM(ClientID, "null", NOPE, TAB_INFO_DECORATION, "Return in inventory: Select down your decorations");
		GS()->AVM(ClientID, "null", NOPE, TAB_INFO_DECORATION, "and press (Back to inventory).");

		Job()->Item()->ListInventory(pPlayer, ItemType::TYPE_DECORATION);
		GS()->AV(ClientID, "null", "");
		ShowDecorationList(pPlayer);
		GS()->AddBack(ClientID);
		return true;
	}
	if (Menulist == MenuList::MENU_HOUSE)
	{
		pPlayer->m_LastVoteMenu = MenuList::MAIN_MENU;

		ShowPersonalHouse(pPlayer);
		GS()->AddBack(ClientID);
		return true;
	}
	if (Menulist == MenuList::MENU_HOUSE_PLANTS)
	{
		const int HouseID = OwnerHouseID(pPlayer->Acc().AuthID);
		const int PlantItemID = GetPlantsID(HouseID);
		pPlayer->m_LastVoteMenu = MenuList::MENU_HOUSE;

		GS()->AVH(ClientID, TAB_INFO_HOUSE_PLANT, GREEN_COLOR, "Plants Information");
		GS()->AVM(ClientID, "null", NOPE, TAB_INFO_HOUSE_PLANT, "Select item and in tab select 'To plant'");
		GS()->AV(ClientID, "null", "");

		GS()->AVM(ClientID, "null", NOPE, NOPE, "Housing Active Plants: {STR}", GS()->GetItemInfo(PlantItemID).GetName(pPlayer));

		GS()->Mmo()->Item()->ListInventory(pPlayer, FUNCTION_PLANTS, true);
		GS()->AddBack(ClientID);
		return true;
	}
	return false;
}

// Снять баланс с фарм счета
void HouseJob::TakeFromSafeDeposit(CPlayer* pPlayer, int TakeCount)
{
	const int ClientID = pPlayer->GetCID();
	boost::scoped_ptr<ResultSet> RES(SJK.SD("ID, HouseBank", "tw_houses", "WHERE OwnerID = '%d'", pPlayer->Acc().AuthID));
	if(!RES->next())
		return;

	const int HouseID = RES->getInt("ID");
	const int Bank = (int)RES->getInt("HouseBank");
	if(Bank < TakeCount)
	{
		GS()->Chat(ClientID, "Acceptable for take {INT}gold", &Bank);
		return;
	}
	Home[HouseID].m_Bank = Bank - TakeCount;
	GS()->Chat(ClientID, _("You take gold in the safe (+{VAL}){VAL}!"), &TakeCount, &Home[HouseID].m_Bank);
	SJK.UD("tw_houses", "HouseBank = '%d' WHERE ID = '%d'", Home[HouseID].m_Bank, HouseID);
}


// Добавление баланса
void HouseJob::AddSafeDeposit(CPlayer *pPlayer, int Balance)
{
	const int ClientID = pPlayer->GetCID();
	boost::scoped_ptr<ResultSet> RES(SJK.SD("ID, HouseBank", "tw_houses", "WHERE OwnerID = '%d'", pPlayer->Acc().AuthID));
	if(!RES->next())
		return;

	const int HouseID = RES->getInt("ID");
	Home[HouseID].m_Bank = RES->getInt("HouseBank") + Balance;            
	GS()->Chat(ClientID, _("You put gold in the safe (+{VAL}){VAL}!"), &Balance, &Home[HouseID].m_Bank);
	SJK.UD("tw_houses", "HouseBank = '%d' WHERE ID = '%d'", Home[HouseID].m_Bank, HouseID);
}

// Действия над дверью
void HouseJob::ChangeStateDoor(int HouseID)
{
	if(Home.find(HouseID) == Home.end() || Home[HouseID].m_OwnerID <= 0) return;

	// если мир не равен данному
	if(Home[HouseID].m_WorldID != GS()->GetWorldID())
	{
		GS()->ChatAccountID(Home[HouseID].m_OwnerID, "Change state door can only near your house.");
		return;
	}

	// изменяем стату двери
	if(Home[HouseID].m_Door) 
	{
		// дверь удаляем
		delete Home[HouseID].m_Door;
		Home[HouseID].m_Door = 0;
	}
	else
	{
		// создаем дверь
		Home[HouseID].m_Door = new HouseDoor(&GS()->m_World, vec2(Home[HouseID].m_DoorX, Home[HouseID].m_DoorY));
	}

	// надпись если найдется игрок
	bool StateDoor = (Home[HouseID].m_Door);
	GS()->ChatAccountID(Home[HouseID].m_OwnerID, "You {STR} the house.", (StateDoor ? "closed" : "opened"));
}

// Показ меню дома
void HouseJob::ShowHouseMenu(CPlayer *pPlayer, int HouseID)
{
	const int ClientID = pPlayer->GetCID();
	GS()->AVH(ClientID, TAB_INFO_HOUSE, GREEN_COLOR, "House {INT} . {STR}", &HouseID, Home[HouseID].m_Class);
	GS()->AVM(ClientID, "null", NOPE, TAB_INFO_HOUSE, "Owner House: {STR}", Job()->PlayerName(Home[HouseID].m_OwnerID));
	GS()->AV(ClientID, "null", "");

	if(Home[HouseID].m_OwnerID <= 0)
		GS()->AVM(ClientID, "BUYHOUSE", HouseID, NOPE, "Buy this house. Price {INT}gold", &Home[HouseID].m_Price);
}
// Показ меню дома персонально
void HouseJob::ShowPersonalHouse(CPlayer *pPlayer)
{
	const int ClientID = pPlayer->GetCID();
	const int HouseID = PlayerHouseID(pPlayer);
	if(HouseID <= 0)
	{
		GS()->AVL(ClientID, "null", "You not owner home!");
		return;
	}
	const bool StateDoor = GetHouseDoor(HouseID);
	GS()->AVH(ClientID, TAB_HOUSE_STAT, BLUE_COLOR, "House stats {INT} Class {STR} Door [{STR}]", &HouseID, Home[HouseID].m_Class, StateDoor ? "Closed" : "Opened");
	GS()->AVM(ClientID, "null", NOPE, TAB_HOUSE_STAT, "/doorhouse - interactive with door.");
	GS()->AVM(ClientID, "null", NOPE, TAB_HOUSE_STAT, "/sellhouse - sell house to world.");
	GS()->AVM(ClientID, "null", NOPE, TAB_HOUSE_STAT, "- - - - - - - - - -");
	GS()->AVM(ClientID, "null", NOPE, TAB_HOUSE_STAT, "Notes: Minimal operation house balance 100gold");
	GS()->AVM(ClientID, "null", NOPE, TAB_HOUSE_STAT, "In your safe is: {INT}gold", &Home[HouseID].m_Bank);
	GS()->AV(ClientID, "null", "");
	//
	pPlayer->m_Colored = LIGHT_GRAY_COLOR;
	GS()->AVL(ClientID, "null", "◍ Your gold: {INT}gold", &pPlayer->GetItem(itGold).Count);
	pPlayer->m_Colored = SMALL_LIGHT_GRAY_COLOR;
	GS()->AVM(ClientID, "HOUSEADD", 1, NOPE, "Add to the safe gold. (Amount in a reason)");
	GS()->AVM(ClientID, "HOUSETAKE", 1, NOPE, "Take the safe gold. (Amount in a reason)");
	GS()->AV(ClientID, "null", "");
	//
	pPlayer->m_Colored = LIGHT_GRAY_COLOR;
	GS()->AVL(ClientID, "null", "▤ House system");
	pPlayer->m_Colored = SMALL_LIGHT_GRAY_COLOR;
	GS()->AVM(ClientID, "HOUSEDOOR", HouseID, NOPE, "Change state to [\"{STR} door\"]", StateDoor ? "Open" : "Close");
	GS()->AVM(ClientID, "HSPAWN", 1, NOPE, "Teleport to your house");
	if(Home[HouseID].m_WorldID == GS()->Server()->GetWorldID(ClientID))
	{
		GS()->AVM(ClientID, "MENU", MenuList::MENU_HOUSE_DECORATION, NOPE, "Settings Decorations");
		GS()->AVM(ClientID, "MENU", MenuList::MENU_HOUSE_PLANTS, NOPE, "Settings Plants");
	}
	else
	{
		GS()->AVM(ClientID, "null", MenuList::MENU_HOUSE_DECORATION, NOPE, "More settings allow, only on house zone");
	}
}

// Парсинг голосований
bool HouseJob::OnVotingMenu(CPlayer *pPlayer, const char *CMD, const int VoteID, const int VoteID2, int Get, const char *GetText)
{
	const int ClientID = pPlayer->GetCID();
	if(PPSTR(CMD, "BUYHOUSE") == 0)
	{
		const int HouseID = VoteID;
		if(BuyHouse(HouseID, pPlayer))
		{
			GS()->Chat(-1, "{STR} becomes the owner of the house class {STR}", GS()->Server()->ClientName(ClientID), Home[VoteID].m_Class);
			GS()->ChatDiscord(DC_SERVER_INFO, "Server information", "**{STR} becomes the owner of the house class {STR}**", GS()->Server()->ClientName(ClientID), Home[HouseID].m_Class);
			GS()->ChatFollow(ClientID, "Do not forget to top up the balance at home, now the balance {INT}G", &Home[HouseID].m_Bank);
			GS()->ResetVotes(ClientID, pPlayer->m_OpenVoteMenu);
		}
		return true;
	}
	// переместится домой
	if(PPSTR(CMD, "HSPAWN") == 0)
	{
		if(!pPlayer->GetCharacter())
			return true;

		int HouseID = PlayerHouseID(pPlayer);
		int HouseWorldID = GetWorldID(HouseID);
		vec2 Position = GetPositionHouse(HouseID);
		if(!GS()->IsClientEqualWorldID(ClientID, HouseWorldID))
		{
			pPlayer->GetTempData().TempTeleportX = Position.x;
			pPlayer->GetTempData().TempTeleportY = Position.y;
			pPlayer->ChangeWorld(HouseWorldID);
			return true;
		}
		pPlayer->GetCharacter()->ChangePosition(Position);
		return true;
	}

	// Оплата дома
	if(PPSTR(CMD, "HOUSEADD") == 0)
	{
		if(Get < 100 || pPlayer->CheckFailMoney(Get))
			return true;

		AddSafeDeposit(pPlayer, Get);
		GS()->VResetVotes(ClientID, MenuList::MENU_HOUSE);
		return true;
	}

	// Снятие сбережения
	if(PPSTR(CMD, "HOUSETAKE") == 0)
	{
		if(Get < 100)
			return true;

		TakeFromSafeDeposit(pPlayer, Get);
		GS()->VResetVotes(ClientID, MenuList::MENU_HOUSE);
		return true;
	}

	// Дверь дома
	if(PPSTR(CMD, "HOUSEDOOR") == 0)
	{
		ChangeStateDoor(VoteID);
		GS()->VResetVotes(ClientID, MenuList::MENU_HOUSE);
		return true;		
	}

	// начала расстановки декорации
	if(PPSTR(CMD, "DECOSTART") == 0)
	{
		int HouseID = PlayerHouseID(pPlayer);
		vec2 PositionHouse = GetPositionHouse(HouseID);
		if(HouseID <= 0 || distance(PositionHouse, pPlayer->GetCharacter()->m_Core.m_Pos) > 600)
		{
			GS()->Chat(ClientID, "Long distance from the house, or you do not own the house!");
			return true;
		}

		// информация
		GS()->ClearVotes(ClientID);
		GS()->AV(ClientID, "null", "Please close vote and press Left Mouse,");
		GS()->AV(ClientID, "null", "on position where add decoration!");
		GS()->AddBack(ClientID);

		pPlayer->m_LastVoteMenu = MenuList::MENU_INVENTORY;
		pPlayer->GetTempData().TempDecoractionID = VoteID;
		pPlayer->GetTempData().TempDecorationType = DECOTYPE_HOUSE;
		return true;
	}

	// декорации
	if(PPSTR(CMD, "DECODELETE") == 0)
	{
		// дом проверка
		int HouseID = PlayerHouseID(pPlayer);
		if(HouseID > 0 && DeleteDecorationHouse(VoteID))
		{
			ItemJob::InventoryItem &PlDecoItem = pPlayer->GetItem(VoteID2);
			GS()->Chat(ClientID, "You back to the backpack {STR}!", PlDecoItem.Info().GetName(pPlayer));
			PlDecoItem.Add(1);
		}
		GS()->VResetVotes(ClientID, MenuList::MENU_HOUSE_DECORATION);
		return true;
	}

	// смена расстения дома
	if(PPSTR(CMD, "HOMEPLANTSET") == 0)
	{
		const int HouseID = PlayerHouseID(pPlayer);
		if(HouseID < 0)
		{
			GS()->Chat(ClientID,"You not owner home!");
			return true;
		}

		if(GetPlantsID(HouseID) <= 0)
		{
			GS()->Chat(ClientID, "Your home does not support plants!");
			return true;		
		}

		const int ItemID = VoteID;
		if(pPlayer->CheckFailMoney(1, ItemID))
			return true;

		const int ChanceSuccesful = VoteID2;
		if(ChanceSuccesful != 0)
		{
			GS()->Chat(ClientID, "Unfortunately the plant did not take root!");
			GS()->ResetVotes(ClientID, pPlayer->m_OpenVoteMenu);
			return true;
		}

		ChangePlantsID(HouseID, VoteID);
		GS()->ResetVotes(ClientID, pPlayer->m_OpenVoteMenu);
	}

	return false;
}

// Двери
HouseDoor::HouseDoor(CGameWorld *pGameWorld, vec2 Pos)
: CEntity(pGameWorld, CGameWorld::ENTTYPE_HOUSEDOOR, Pos), m_To(Pos)
{
	m_Pos.y += 30;
	m_To = GS()->Collision()->FindDirCollision(100, m_To, 'y', '-');
	GameWorld()->InsertEntity(this);
}

void HouseDoor::Tick()
{
	for(CCharacter *pChar = (CCharacter*) GameWorld()->FindFirst(CGameWorld::ENTTYPE_CHARACTER); pChar; pChar = (CCharacter *)pChar->TypeNext())
	{
		vec2 IntersectPos = closest_point_on_line(m_Pos, m_To, pChar->m_Core.m_Pos);
		float Distance = distance(IntersectPos, pChar->m_Core.m_Pos);
		if (Distance <= g_Config.m_SvDoorRadiusHit)
			pChar->m_DoorHit = true;
	}
}

void HouseDoor::Snap(int SnappingClient)
{
	if (NetworkClipped(SnappingClient))
		return;
	
	CNetObj_Laser *pObj = static_cast<CNetObj_Laser *>(Server()->SnapNewItem(NETOBJTYPE_LASER, GetID(), sizeof(CNetObj_Laser)));
	if (!pObj)
		return;

	pObj->m_X = int(m_Pos.x);
	pObj->m_Y = int(m_Pos.y);
	pObj->m_FromX = int(m_To.x);
	pObj->m_FromY = int(m_To.y);
	pObj->m_StartTick = Server()->Tick()-2;
}