/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#include <engine/shared/config.h>
#include <game/server/gamecontext.h>
#include "HouseJob.h"

#include <game/server/mmocore/GameEntities/jobitems.h>

using namespace sqlstr;
std::map <int, HouseJob::HouseList> HouseJob::ms_aHouse;

void HouseJob::OnInitWorld(const char* pWhereLocalWorld) 
{
	// load house
	SJK.SDT("*", "tw_houses", [&](ResultSet* RES)
	{
		while(RES->next())
		{
			const int HouseID = RES->getInt("ID");
			ms_aHouse[HouseID].m_DoorX = RES->getInt("DoorX");
			ms_aHouse[HouseID].m_DoorY = RES->getInt("DoorY");
			ms_aHouse[HouseID].m_PosX = RES->getInt("PosX");
			ms_aHouse[HouseID].m_PosY = RES->getInt("PosY");
			ms_aHouse[HouseID].m_OwnerID = RES->getInt("OwnerID");
			ms_aHouse[HouseID].m_Price = RES->getInt("Price");
			ms_aHouse[HouseID].m_Bank = RES->getInt("HouseBank");
			ms_aHouse[HouseID].m_WorldID = RES->getInt("WorldID");
			str_copy(ms_aHouse[HouseID].m_aClass, RES->getString("Class").c_str(), sizeof(ms_aHouse[HouseID].m_aClass));
			ms_aHouse[HouseID].m_PlantID = RES->getInt("PlantID");
			ms_aHouse[HouseID].m_PlantPosX = RES->getInt("PlantX");
			ms_aHouse[HouseID].m_PlantPosY = RES->getInt("PlantY");
			if(GS()->GetWorldID() == ms_aHouse[HouseID].m_WorldID && ms_aHouse[HouseID].m_OwnerID > 0 && !ms_aHouse[HouseID].m_Door)
			{
				ms_aHouse[HouseID].m_Door = 0;
				ms_aHouse[HouseID].m_Door = new HouseDoor(&GS()->m_World, vec2(ms_aHouse[HouseID].m_DoorX, ms_aHouse[HouseID].m_DoorY));
			}
		}
		Job()->ShowLoadingProgress("Houses", ms_aHouse.size());
	}, pWhereLocalWorld);

	// load decoration
	if(ms_aDecorationHouse.empty())
	{
		SJK.SDT("*", "tw_houses_decorations", [&](ResultSet* DecoRES)
		{
			while(DecoRES->next())
			{
				const int DecoID = DecoRES->getInt("ID");
				ms_aDecorationHouse[DecoID] = new CDecorationHouses(&GS()->m_World, vec2(DecoRES->getInt("X"),
					DecoRES->getInt("Y")), DecoRES->getInt("HouseID"), DecoRES->getInt("DecoID"));
			}
			Job()->ShowLoadingProgress("Houses Decorations", ms_aDecorationHouse.size());
		}, pWhereLocalWorld);
	}
}

/*
	Later lead to quality standard code
*/
/* #########################################################################
	FUNCTIONS HOUSES PLANTS
######################################################################### */
void HouseJob::ChangePlantsID(int HouseID, int PlantID)
{
	if(ms_aHouse.find(HouseID) == ms_aHouse.end())
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
		ms_aHouse[HouseID].m_PlantID = PlantID;
		SJK.UD("tw_houses", "PlantID = '%d' WHERE ID = '%d'", PlantID, HouseID);
	}
}

/* #########################################################################
	FUNCTIONS HOUSES DECORATION
######################################################################### */
bool HouseJob::AddDecorationHouse(int DecoID, int HouseID, vec2 Position)
{
	if(HouseID <= 0)
		return false;

	vec2 PositionHouse = GetPositionHouse(HouseID);
	if(distance(PositionHouse, Position) > 400.0f)
		return false;

	std::shared_ptr<ResultSet> RES(SJK.SD("ID", "tw_houses_decorations", "WHERE HouseID = '%d'", HouseID));
	if((int)RES->rowsCount() >= g_Config.m_SvLimitDecoration) return false;

	std::shared_ptr<ResultSet> RES2(SJK.SD("ID", "tw_houses_decorations", "ORDER BY ID DESC LIMIT 1"));
	int InitID = (RES2->next() ? RES2->getInt("ID")+1 : 1); 

	SJK.ID("tw_houses_decorations", "(ID, DecoID, HouseID, X, Y, WorldID) VALUES ('%d', '%d', '%d', '%d', '%d', '%d')", 
		InitID, DecoID, HouseID, (int)Position.x, (int)Position.y, GS()->GetWorldID());

	ms_aDecorationHouse[InitID] = new CDecorationHouses(&GS()->m_World, Position, HouseID, DecoID);
	return true;
}

bool HouseJob::DeleteDecorationHouse(int ID)
{
	if(ms_aDecorationHouse.find(ID) != ms_aDecorationHouse.end())
	{
		if(ms_aDecorationHouse.at(ID))
		{
			delete ms_aDecorationHouse.at(ID);
			ms_aDecorationHouse.at(ID) = 0;
		}
		ms_aDecorationHouse.erase(ID);
		SJK.DD("tw_houses_decorations", "WHERE ID = '%d'", ID);
		return true;
	}
	return false;
}

void HouseJob::ShowDecorationList(CPlayer *pPlayer)
{
	int HouseID = PlayerHouseID(pPlayer);
	int ClientID = pPlayer->GetCID();
	for (auto deco = ms_aDecorationHouse.begin(); deco != ms_aDecorationHouse.end(); deco++)
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
	if(ms_aHouse.find(HouseID) != ms_aHouse.end())
		return ms_aHouse[HouseID].m_OwnerID;
	return -1;
}

int HouseJob::GetWorldID(int HouseID) const
{
	if(ms_aHouse.find(HouseID) != ms_aHouse.end())
		return ms_aHouse.at(HouseID).m_WorldID;
	return -1;
}

int HouseJob::GetHouse(vec2 Pos, bool Plants)
{
	float Distance = (Plants ? 300.0f : 128.0f);
	for(const auto& ihome : ms_aHouse)
	{
		if (ihome.second.m_WorldID != GS()->GetWorldID())
			continue;

		vec2 PosHome = (Plants ? vec2(ihome.second.m_PlantPosX, ihome.second.m_PlantPosY) : vec2(ihome.second.m_PosX, ihome.second.m_PosY));
		if(distance(PosHome, Pos) < Distance)
			return ihome.first;
	}
	return -1;
}

int HouseJob::GetHousePrice(int HouseID) const	
{	 
	if(ms_aHouse.find(HouseID) != ms_aHouse.end()) 
		return ms_aHouse.at(HouseID).m_Price;
 	return -1;
}

bool HouseJob::GetHouseDoor(int HouseID) const	
{	 
	if(ms_aHouse.find(HouseID) != ms_aHouse.end())	
	{
		bool DoorClose = (ms_aHouse.at(HouseID).m_Door);
		return DoorClose;
	}
	return false;
}

vec2 HouseJob::GetPositionHouse(int HouseID) const	
{ 	
	if(ms_aHouse.find(HouseID) != ms_aHouse.end()) 
		return vec2(ms_aHouse.at(HouseID).m_PosX, ms_aHouse.at(HouseID).m_PosY); 

	return vec2(0, 0);
}

int HouseJob::PlayerHouseID(CPlayer *pPlayer) const
{
	for (auto ihome = ms_aHouse.begin(); ihome != ms_aHouse.end(); ihome++) {
		if(ms_aHouse.at(ihome->first).m_OwnerID == pPlayer->Acc().m_AuthID)
			return ihome->first;
	}
	return -1;
}

int HouseJob::OwnerHouseID(int AuthID) const
{
	for (auto ihome = ms_aHouse.begin(); ihome != ms_aHouse.end(); ihome++) {
		if(ms_aHouse.at(ihome->first).m_OwnerID == AuthID)
			return ihome->first;
	}
	return -1;
}

int HouseJob::GetPlantsID(int HouseID) const
{
	if(ms_aHouse.find(HouseID) != ms_aHouse.end()) 
		return ms_aHouse.at(HouseID).m_PlantID;
	return -1;
}

const char *HouseJob::ClassName(int HouseID) const	
{	
	if(ms_aHouse.find(HouseID) != ms_aHouse.end()) 
		return ms_aHouse.at(HouseID).m_aClass; 
	return "None";
}

const char *HouseJob::OwnerName(int HouseID) 
{	
	if(ms_aHouse.find(HouseID) != ms_aHouse.end())
		return Job()->PlayerName(ms_aHouse[HouseID].m_OwnerID);
	return "No owner";
}

/* #########################################################################
	FUNCTIONS HOUSES 
######################################################################### */
void HouseJob::BuyHouse(int HouseID, CPlayer *pPlayer)
{
	const int ClientID = pPlayer->GetCID();
	if(PlayerHouseID(pPlayer) > 0)
	{
		GS()->Chat(ClientID, "You already have a home.");		
		return;
	}

	std::shared_ptr<ResultSet> RES(SJK.SD("OwnerID, Price", "tw_houses", "WHERE ID = '%d' AND OwnerID IS NULL", HouseID));
	if(RES->next())
	{
		const int Price = RES->getInt("Price");
		if(pPlayer->CheckFailMoney(Price))	
			return;

		ms_aHouse[HouseID].m_Bank = 0;
		ms_aHouse[HouseID].m_OwnerID = pPlayer->Acc().m_AuthID;
		SJK.UD("tw_houses", "OwnerID = '%d', HouseBank = '0' WHERE ID = '%d'", ms_aHouse[HouseID].m_OwnerID, HouseID);

		GS()->Chat(-1, "{STR} becomes the owner of the house class {STR}", GS()->Server()->ClientName(ClientID), ms_aHouse[HouseID].m_aClass);
		GS()->ChatDiscord(DC_SERVER_INFO, "Server information", "**{STR} becomes the owner of the house class {STR}**", GS()->Server()->ClientName(ClientID), ms_aHouse[HouseID].m_aClass);
		GS()->ResetVotes(ClientID, pPlayer->m_OpenVoteMenu);
		return;
	}

	GS()->Chat(ClientID, "House has already been purchased!");
}

void HouseJob::SellHouse(int HouseID)
{
	std::shared_ptr<ResultSet> RES(SJK.SD("HouseBank, OwnerID", "tw_houses", "WHERE ID = '%d' AND OwnerID IS NOT NULL", HouseID));
	if(RES->next())
	{
		const int OwnerID = RES->getInt("OwnerID");
		const int Price = ms_aHouse[HouseID].m_Price + RES->getInt("HouseBank");
		Job()->Inbox()->SendInbox(OwnerID, "House is sold", "Your house is sold !", itGold, Price, 0);
		SJK.UD("tw_houses", "OwnerID = NULL, HouseBank = '0' WHERE ID = '%d'", HouseID);

		const int ClientID = Job()->Account()->CheckOnlineAccount(OwnerID);
		if(ClientID >= 0)
		{
			GS()->ChatFollow(ClientID, "Your House is sold!");
			GS()->ResetVotes(ClientID, MenuList::MAIN_MENU);
		}
		GS()->Chat(-1, "House: {INT} have been is released!", &HouseID);
		GS()->ChatDiscord(DC_SERVER_INFO, "Server information", "**[House: {INT}] have been sold!**", &HouseID);
	}

	if(ms_aHouse[HouseID].m_Door)
	{
		delete ms_aHouse[HouseID].m_Door;
		ms_aHouse[HouseID].m_Door = 0;
	}

	ms_aHouse[HouseID].m_OwnerID = -1;
	ms_aHouse[HouseID].m_Bank = 0;
}

bool HouseJob::OnHandleTile(CCharacter* pChr, int IndexCollision)
{
	CPlayer* pPlayer = pChr->GetPlayer();
	const int ClientID = pPlayer->GetCID();

	if (pChr->GetHelper()->TileEnter(IndexCollision, TILE_PLAYER_HOUSE))
	{
		GS()->Chat(ClientID, "You can see menu in the votes!");
		pChr->m_Core.m_ProtectHooked = pChr->m_NoAllowDamage = true;
		GS()->ResetVotes(ClientID, pPlayer->m_OpenVoteMenu);
		return true;
	}
	else if (pChr->GetHelper()->TileExit(IndexCollision, TILE_PLAYER_HOUSE))
	{
		GS()->Chat(ClientID, "You left the active zone, menu is restored!");
		pChr->m_Core.m_ProtectHooked = pChr->m_NoAllowDamage = false;
		GS()->ResetVotes(ClientID, pPlayer->m_OpenVoteMenu);
		return true;
	}

	return false;
}

bool HouseJob::OnHandleMenulist(CPlayer* pPlayer, int Menulist, bool ReplaceMenu)
{
	const int ClientID = pPlayer->GetCID();
	if (ReplaceMenu)
	{
		CCharacter* pChr = pPlayer->GetCharacter();
		if (!pChr || !pChr->IsAlive())
			return false;

		if (pChr->GetHelper()->BoolIndex(TILE_PLAYER_HOUSE))
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
		pPlayer->m_LastVoteMenu = MenuList::MENU_HOUSE;
		const int HouseID = OwnerHouseID(pPlayer->Acc().m_AuthID);
		const int PlantItemID = GetPlantsID(HouseID);

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

void HouseJob::TakeFromSafeDeposit(CPlayer* pPlayer, int TakeCount)
{
	const int ClientID = pPlayer->GetCID();
	std::shared_ptr<ResultSet> RES(SJK.SD("ID, HouseBank", "tw_houses", "WHERE OwnerID = '%d'", pPlayer->Acc().m_AuthID));
	if(!RES->next())
		return;

	const int HouseID = RES->getInt("ID");
	const int Bank = (int)RES->getInt("HouseBank");
	if(Bank < TakeCount)
	{
		GS()->Chat(ClientID, "Acceptable for take {INT}gold", &Bank);
		return;
	}

	pPlayer->AddMoney(TakeCount);
	ms_aHouse[HouseID].m_Bank = Bank - TakeCount;
	SJK.UD("tw_houses", "HouseBank = '%d' WHERE ID = '%d'", ms_aHouse[HouseID].m_Bank, HouseID);
	GS()->Chat(ClientID, "You take {INT} gold in the safe {INT}!", &TakeCount, &ms_aHouse[HouseID].m_Bank);
}

void HouseJob::AddSafeDeposit(CPlayer *pPlayer, int Balance)
{
	const int ClientID = pPlayer->GetCID();
	std::shared_ptr<ResultSet> RES(SJK.SD("ID, HouseBank", "tw_houses", "WHERE OwnerID = '%d'", pPlayer->Acc().m_AuthID));
	if(!RES->next())
		return;

	const int HouseID = RES->getInt("ID");
	ms_aHouse[HouseID].m_Bank = RES->getInt("HouseBank") + Balance;            
	GS()->Chat(ClientID, "You put {INT} gold in the safe {INT}!", &Balance, &ms_aHouse[HouseID].m_Bank);
	SJK.UD("tw_houses", "HouseBank = '%d' WHERE ID = '%d'", ms_aHouse[HouseID].m_Bank, HouseID);
}

bool HouseJob::ChangeStateDoor(int HouseID)
{
	if(ms_aHouse.find(HouseID) == ms_aHouse.end() || ms_aHouse[HouseID].m_OwnerID <= 0) 
		return false;

	if(ms_aHouse[HouseID].m_WorldID != GS()->GetWorldID())
	{
		GS()->ChatAccountID(ms_aHouse[HouseID].m_OwnerID, "Change state door can only near your house.");
		return false;
	}

	if(ms_aHouse[HouseID].m_Door) 
	{
		delete ms_aHouse[HouseID].m_Door;
		ms_aHouse[HouseID].m_Door = 0;
	}
	else
		ms_aHouse[HouseID].m_Door = new HouseDoor(&GS()->m_World, vec2(ms_aHouse[HouseID].m_DoorX, ms_aHouse[HouseID].m_DoorY));

	const bool StateDoor = (ms_aHouse[HouseID].m_Door);
	GS()->ChatAccountID(ms_aHouse[HouseID].m_OwnerID, "You {STR} the house.", (StateDoor ? "closed" : "opened"));
	return true;
}

void HouseJob::ShowHouseMenu(CPlayer *pPlayer, int HouseID)
{
	const int ClientID = pPlayer->GetCID();
	GS()->AVH(ClientID, TAB_INFO_HOUSE, GREEN_COLOR, "House {INT} . {STR}", &HouseID, ms_aHouse[HouseID].m_aClass);
	GS()->AVM(ClientID, "null", NOPE, TAB_INFO_HOUSE, "Owner House: {STR}", Job()->PlayerName(ms_aHouse[HouseID].m_OwnerID));

	GS()->AV(ClientID, "null", "");
	GS()->ShowItemValueInformation(pPlayer, itGold);
	GS()->AV(ClientID, "null", "");
	
	pPlayer->m_Colored = LIGHT_GRAY_COLOR;
	if(ms_aHouse[HouseID].m_OwnerID <= 0)
		GS()->AVM(ClientID, "BUYHOUSE", HouseID, NOPE, "Buy this house. Price {INT}gold", &ms_aHouse[HouseID].m_Price);
	else
		GS()->AVM(ClientID, "null", HouseID, NOPE, "This house has already been purchased!");

	GS()->AV(ClientID, "null", "");
}

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
	GS()->AVH(ClientID, TAB_HOUSE_STAT, BLUE_COLOR, "House stats {INT} Class {STR} Door [{STR}]", &HouseID, ms_aHouse[HouseID].m_aClass, StateDoor ? "Closed" : "Opened");
	GS()->AVM(ClientID, "null", NOPE, TAB_HOUSE_STAT, "/doorhouse - interactive with door.");
	GS()->AVM(ClientID, "null", NOPE, TAB_HOUSE_STAT, "- - - - - - - - - -");
	GS()->AVM(ClientID, "null", NOPE, TAB_HOUSE_STAT, "Notes: Minimal operation house balance 100gold");
	GS()->AVM(ClientID, "null", NOPE, TAB_HOUSE_STAT, "In your safe is: {INT}gold", &ms_aHouse[HouseID].m_Bank);
	GS()->AV(ClientID, "null", "");
	//
	pPlayer->m_Colored = LIGHT_GRAY_COLOR;
	GS()->AVL(ClientID, "null", "◍ Your gold: {INT}gold", &pPlayer->GetItem(itGold).m_Count);
	pPlayer->m_Colored = SMALL_LIGHT_GRAY_COLOR;
	GS()->AVM(ClientID, "HOUSEADD", 1, NOPE, "Add to the safe gold. (Amount in a reason)");
	GS()->AVM(ClientID, "HOUSETAKE", 1, NOPE, "Take the safe gold. (Amount in a reason)");
	GS()->AV(ClientID, "null", "");
	//
	pPlayer->m_Colored = LIGHT_GRAY_COLOR;
	GS()->AVL(ClientID, "null", "▤ House system");
	pPlayer->m_Colored = SMALL_LIGHT_GRAY_COLOR;
	GS()->AVM(ClientID, "HOUSEDOOR", HouseID, NOPE, "Change state to [\"{STR}\"]", StateDoor ? "OPEN" : "CLOSED");
	GS()->AVM(ClientID, "HSPAWN", 1, NOPE, "Teleport to your house");
	GS()->AVM(ClientID, "HSELL", HouseID, NOPE, "Sell your house (in reason 777)");
	if(GS()->IsClientEqualWorldID(ClientID, ms_aHouse[HouseID].m_WorldID))
	{
		GS()->AVM(ClientID, "MENU", MenuList::MENU_HOUSE_DECORATION, NOPE, "Settings Decorations");
		GS()->AVM(ClientID, "MENU", MenuList::MENU_HOUSE_PLANTS, NOPE, "Settings Plants");
	}
	else
		GS()->AVM(ClientID, "null", MenuList::MENU_HOUSE_DECORATION, NOPE, "More settings allow, only on house zone");
}

bool HouseJob::OnVotingMenu(CPlayer *pPlayer, const char *CMD, const int VoteID, const int VoteID2, int Get, const char *GetText)
{
	const int ClientID = pPlayer->GetCID();
	if(PPSTR(CMD, "BUYHOUSE") == 0)
	{
		const int HouseID = VoteID;
		BuyHouse(HouseID, pPlayer);
		return true;
	}

	if(PPSTR(CMD, "HSPAWN") == 0)
	{
		if(!pPlayer->GetCharacter())
			return true;

		int HouseID = PlayerHouseID(pPlayer);
		int HouseWorldID = GetWorldID(HouseID);
		vec2 Position = GetPositionHouse(HouseID);
		if(!GS()->IsClientEqualWorldID(ClientID, HouseWorldID))
		{
			pPlayer->GetTempData().m_TempTeleportX = Position.x;
			pPlayer->GetTempData().m_TempTeleportY = Position.y;
			pPlayer->ChangeWorld(HouseWorldID);
			return true;
		}
		pPlayer->GetCharacter()->ChangePosition(Position);
		return true;
	}

	if (PPSTR(CMD, "HSELL") == 0)
	{
		const int HouseID = VoteID;
		if (HouseID <= 0 || Get != 777)
		{
			GS()->Chat(ClientID, "A verification number was entered incorrectly.");
			return true;
		}

		SellHouse(HouseID);
		GS()->ResetVotes(ClientID, MenuList::MAIN_MENU);
		return true;
	}

	if(PPSTR(CMD, "HOUSEADD") == 0)
	{
		if(Get < 100)
		{
			GS()->Chat(ClientID, "Minimal 100 gold.");
			return true;
		}
		
		if(pPlayer->CheckFailMoney(Get))
			return true;

		AddSafeDeposit(pPlayer, Get);
		GS()->UpdateVotes(ClientID, MenuList::MENU_HOUSE);
		return true;
	}

	if(PPSTR(CMD, "HOUSETAKE") == 0)
	{
		if(Get < 100)
		{
			GS()->Chat(ClientID, "Minimal 100 gold.");
			return true;
		}
		
		TakeFromSafeDeposit(pPlayer, Get);
		GS()->UpdateVotes(ClientID, MenuList::MENU_HOUSE);
		return true;
	}

	if(PPSTR(CMD, "HOUSEDOOR") == 0)
	{
		if(ChangeStateDoor(VoteID))
			GS()->UpdateVotes(ClientID, MenuList::MENU_HOUSE);
		return true;		
	}

	if(PPSTR(CMD, "DECOSTART") == 0)
	{
		const int HouseID = PlayerHouseID(pPlayer);
		const vec2 PositionHouse = GetPositionHouse(HouseID);
		if(HouseID <= 0 || distance(PositionHouse, pPlayer->GetCharacter()->m_Core.m_Pos) > 600)
		{
			GS()->Chat(ClientID, "Long distance from the house, or you do not own the house!");
			return true;
		}

		GS()->ClearVotes(ClientID);
		GS()->AV(ClientID, "null", "Please close vote and press Left Mouse,");
		GS()->AV(ClientID, "null", "on position where add decoration!");
		GS()->AddBack(ClientID);

		pPlayer->m_LastVoteMenu = MenuList::MENU_INVENTORY;
		pPlayer->GetTempData().m_TempDecoractionID = VoteID;
		pPlayer->GetTempData().m_TempDecorationType = DECOTYPE_HOUSE;
		return true;
	}

	if(PPSTR(CMD, "DECODELETE") == 0)
	{
		const int HouseID = PlayerHouseID(pPlayer);
		if(HouseID > 0 && DeleteDecorationHouse(VoteID))
		{
			InventoryItem &PlDecoItem = pPlayer->GetItem(VoteID2);
			GS()->Chat(ClientID, "You back to the backpack {STR}!", PlDecoItem.Info().GetName(pPlayer));
			PlDecoItem.Add(1);
		}
		GS()->UpdateVotes(ClientID, MenuList::MENU_HOUSE_DECORATION);
		return true;
	}

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
			GS()->Chat(ClientID, "Unfortunately plant did not take root!");
			GS()->ResetVotes(ClientID, pPlayer->m_OpenVoteMenu);
			return true;
		}

		GS()->Chat(-1, "Congratulations {STR}, planted at home {STR}!", GS()->Server()->ClientName(ClientID), GS()->GetItemInfo(ItemID).m_aName);
		ChangePlantsID(HouseID, ItemID);
		GS()->ResetVotes(ClientID, pPlayer->m_OpenVoteMenu);
	}

	return false;
}

HouseDoor::HouseDoor(CGameWorld *pGameWorld, vec2 Pos)
: CEntity(pGameWorld, CGameWorld::ENTTYPE_PLAYER_HOUSE_DOOR, Pos)
{
	m_Pos.y += 30;
	m_PosTo = GS()->Collision()->FindDirCollision(100, m_PosTo, 'y', '-');
	GameWorld()->InsertEntity(this);
}

void HouseDoor::Tick()
{
	for(CCharacter *pChar = (CCharacter*) GameWorld()->FindFirst(CGameWorld::ENTTYPE_CHARACTER); pChar; pChar = (CCharacter *)pChar->TypeNext())
	{
		vec2 IntersectPos = closest_point_on_line(m_Pos, m_PosTo, pChar->m_Core.m_Pos);
		const float Distance = distance(IntersectPos, pChar->m_Core.m_Pos);
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
	pObj->m_FromX = int(m_PosTo.x);
	pObj->m_FromY = int(m_PosTo.y);
	pObj->m_StartTick = Server()->Tick()-2;
}