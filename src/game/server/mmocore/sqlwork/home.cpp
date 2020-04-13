/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#include <engine/shared/config.h>
#include <game/server/gamecontext.h>
#include "home.h"

#include <game/server/entities/jobitems.h>

using namespace sqlstr;
std::map < int , HouseSql::HouseList > HouseSql::Home;

// Инициализация класса
void HouseSql::OnInitLocal(const char *pLocal) 
{ 
	{ // загрузка домов
		boost::scoped_ptr<ResultSet> RES(SJK.SD("*", "tw_homes", pLocal));
		while(RES->next())
		{
			int HID = RES->getInt("ID");

			// двери
			Home[HID].hDoorX = RES->getInt("DoorX");
			Home[HID].hDoorY = RES->getInt("DoorY");
			if(!Home[HID].m_Door) { Home[HID].m_Door = 0; }

			// дом информация
			Home[HID].hX = RES->getInt("HomeX");
			Home[HID].hY = RES->getInt("HomeY");
			Home[HID].hOwnerID = RES->getInt("OwnerID");
			Home[HID].hPrice = RES->getInt("Price");
			Home[HID].hBank = RES->getInt("HouseBank");
			Home[HID].hFarm = RES->getInt("Farm");
			Home[HID].hFarmLevel = RES->getInt("FarmLevel");
			Home[HID].hWorldID = RES->getInt("WorldID");
			str_copy(Home[HID].hClass, RES->getString("Class").c_str(), sizeof(Home[HID].hClass));

			// плантации
			Home[HID].hPlantID = RES->getInt("PlantID");
			Home[HID].hPlantX = RES->getInt("PlantX");
			Home[HID].hPlantY = RES->getInt("PlantY");

			// создаем дверь если ее нету
			if(GS()->GetWorldID() == Home[HID].hWorldID && Home[HID].hOwnerID > 0 && !Home[HID].m_Door)
				Home[HID].m_Door = new HouseDoor(&GS()->m_World, vec2(Home[HID].hDoorX, Home[HID].hDoorY));
		}
		Job()->ShowLoadingProgress("Houses", Home.size());	
	}
	
	// пропускаем загрузку если там есть элементы
	if(m_decorations.size() > 0) return;

	// загрузка декораций		
	boost::scoped_ptr<ResultSet> RES(SJK.SD("*", "tw_homes_decorations", "WHERE WorldID = '%d'", GS()->GetWorldID()));
	while(RES->next())
	{
		const int DID = RES->getInt("ID");
		m_decorations[DID] = new DecoHouse(&GS()->m_World, vec2(RES->getInt("X"), 
									RES->getInt("Y")), RES->getInt("HouseID"), RES->getInt("DecoID"));
	}
}

void HouseSql::OnPaymentTime()
{
	boost::scoped_ptr<ResultSet> RES(SJK.SD("ID, HouseBank, OwnerID", "tw_homes", "WHERE OwnerID > '0'"));
	while(RES->next())
	{	
		int HouseID = RES->getInt("ID"), HouseBank = RES->getInt("HouseBank"), OwnerID = RES->getInt("OwnerID");
		if(Home.find(HouseID) != Home.end()) 
		{
			if(Home[HouseID].hBank < g_Config.m_SvHousePriceUse)
			{
				SellHouse(HouseID);
				continue;
			}

			Home[HouseID].hBank = HouseBank-g_Config.m_SvHousePriceUse;
			Home[HouseID].hFarm += Home[HouseID].hFarmLevel;
			int ClientID = Job()->Account()->CheckOnlineAccount(OwnerID);                                           
			if(ClientID >= 0)
			{
				GS()->ChatFollow(ClientID, "House Farming Bank now: {INT}gold", &Home[HouseID].hFarm);
				GS()->ChatFollow(ClientID, "Now House Balance: {INT} Payment: -{INT}gold", &Home[HouseID].hBank, &g_Config.m_SvHousePriceUse);
			}
		}
	}
	SJK.UD("tw_homes", "HouseBank = HouseBank - '%d', Farm = Farm + FarmLevel WHERE OwnerID > '0'", g_Config.m_SvHousePriceUse);
}

/*
	Later lead to quality standard code
*/
/* #########################################################################
	FUNCTIONS HOUSES PLANTS
######################################################################### */
// Изменить расстения у дома
void HouseSql::ChangePlantsID(int HouseID, int PlantID)
{
	if(Home.find(HouseID) != Home.end()) {
		bool Changes = false;
		for(CJobItems *pPlant = (CJobItems*) GS()->m_World.FindFirst(CGameWorld::ENTTYPE_JOBITEMS); pPlant; 
			pPlant = (CJobItems *)pPlant->TypeNext()) { 
			if(HouseID > 0 && pPlant->m_HouseID == HouseID) {
				pPlant->m_ItemID = PlantID;
				Changes = true;
			}
		}
		if(Changes)	{
			Home[HouseID].hPlantID = PlantID;
			SJK.UD("tw_homes", "PlantID = '%d' WHERE ID = '%d'", PlantID, HouseID);
		}
	}
}

/* #########################################################################
	FUNCTIONS HOUSES DECORATION
######################################################################### */
// добавить декорацию для дома
bool HouseSql::AddDecorationHouse(int DecoID, int HouseID, vec2 Position)
{
	if(HouseID <= 0)
		return false;

	vec2 PositionHouse = GetPositionHouse(HouseID);
	if(distance(PositionHouse, Position) > 600)
		return false;

	boost::scoped_ptr<ResultSet> RES(SJK.SD("ID", "tw_homes_decorations", "WHERE HouseID = '%d'", HouseID));
	if(RES->rowsCount() >= g_Config.m_SvLimitDecoration) return false;

	boost::scoped_ptr<ResultSet> RES2(SJK.SD("ID", "tw_homes_decorations", "ORDER BY ID DESC LIMIT 1"));
	int InitID = (RES2->next() ? RES2->getInt("ID")+1 : 1); 

	SJK.ID("tw_homes_decorations", "(ID, DecoID, HouseID, X, Y, WorldID) VALUES ('%d', '%d', '%d', '%d', '%d', '%d')", 
		InitID, DecoID, HouseID, (int)Position.x, (int)Position.y, GS()->GetWorldID());

	m_decorations[InitID] = new DecoHouse(&GS()->m_World, Position, HouseID, DecoID);
	return true;
}
// Удалить декорацию
bool HouseSql::DeleteDecorationHouse(int ID)
{
	if(m_decorations.find(ID) != m_decorations.end())
	{
		if(m_decorations.at(ID))
		{
			delete m_decorations.at(ID);
			m_decorations.at(ID) = 0;
		}
		m_decorations.erase(ID);
		SJK.DD("tw_homes_decorations", "WHERE ID = '%d'", ID);
		return true;
	}
	return false;
}
// Показать меню декораций
void HouseSql::ShowDecorationList(CPlayer *pPlayer)
{
	int HouseID = PlayerHouseID(pPlayer);
	int ClientID = pPlayer->GetCID();
	for (auto deco = m_decorations.begin(); deco != m_decorations.end(); deco++)  
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
// Узнать мир где находится дом
int HouseSql::GetWorldID(int HouseID) const
{
	if(Home.find(HouseID) != Home.end())
		return Home.at(HouseID).hWorldID;
	return 0;
}
// Получить дом по позиции
int HouseSql::GetHouse(vec2 Pos, bool Plants)
{
	int HouseID = -1, Distance = (Plants ? 300 : 100);
	for (auto ihome = Home.begin(); ihome != Home.end(); ihome++)
	{
		vec2 PosHome = (Plants ? vec2(Home.at(ihome->first).hPlantX, Home.at(ihome->first).hPlantY) 
							   : vec2(Home.at(ihome->first).hX, Home.at(ihome->first).hY));
		if(distance(PosHome, Pos) < Distance)
			HouseID =  ihome->first;
	}
	return HouseID;
}
// Узнать стоимость дома по айди дома
int HouseSql::GetHousePrice(int HouseID) const	
{	 
	if(Home.find(HouseID) != Home.end()) 
		return Home.at(HouseID).hPrice;
 	return -1;
}

// Проверить открыта дверь или нет по айди дома
bool HouseSql::GetHouseDoor(int HouseID) const	
{	 
	if(Home.find(HouseID) != Home.end())	
	{
		bool DoorClose = (Home.at(HouseID).m_Door);
		return DoorClose;
	}
	return false;
}
// Позиция дома
vec2 HouseSql::GetPositionHouse(int HouseID) const	
{ 	
	if(Home.find(HouseID) != Home.end()) 
		return vec2(Home.at(HouseID).hX, Home.at(HouseID).hY); 

	return vec2(0, 0);
}
// Узнать имеет ли игрок дом
int HouseSql::PlayerHouseID(CPlayer *pPlayer) const
{
	for (auto ihome = Home.begin(); ihome != Home.end(); ihome++) {
		if(Home.at(ihome->first).hOwnerID == pPlayer->Acc().AuthID)
			return ihome->first;
	}
	return -1;
}
// Узнать имеет ли игрок под айди дом
int HouseSql::OwnerHouseID(int AuthID) const
{
	for (auto ihome = Home.begin(); ihome != Home.end(); ihome++) {
		if(Home.at(ihome->first).hOwnerID == AuthID)
			return ihome->first;
	}
	return -1;
}
// Узнать имеет ли игрок под айди дом
int HouseSql::GetPlantsID(int HouseID) const
{
	if(Home.find(HouseID) != Home.end()) 
		return Home.at(HouseID).hPlantID;
	return -1;
}
// Класс дома
const char *HouseSql::ClassName(int HouseID) const	
{	
	if(Home.find(HouseID) != Home.end()) 
		return Home.at(HouseID).hClass; 
	return "None";
}
// Владелец дома
const char *HouseSql::OwnerName(int HouseID) 
{	
	if(Home.find(HouseID) != Home.end())
		return Job()->PlayerName(Home[HouseID].hOwnerID);
	return "No owner";
}

/* #########################################################################
	FUNCTIONS HOUSES 
######################################################################### */
// Покупка дома
bool HouseSql::BuyHouse(int HouseID, CPlayer *pPlayer)
{
	int ClientID = pPlayer->GetCID();
	if(PlayerHouseID(pPlayer) > 0)
	{
		GS()->Chat(ClientID, "You already have a home.");		
		return false;
	}

	boost::scoped_ptr<ResultSet> RES(SJK.SD("OwnerID, Price", "tw_homes", "WHERE ID = '%d';", HouseID));
	if(RES->next())
	{
		if(RES->getInt("OwnerID") > 0) return false;

		int Price = RES->getInt("Price") + g_Config.m_SvHousePriceUse;
		if(pPlayer->CheckFailMoney(Price))	
			return false;

		Home[HouseID].hBank = g_Config.m_SvHousePriceUse;
		Home[HouseID].hOwnerID = pPlayer->Acc().AuthID;
		SJK.UD("tw_homes", "OwnerID = '%d', HouseBank = '%d' WHERE ID = '%d'", Home[HouseID].hOwnerID, g_Config.m_SvHousePriceUse, HouseID);
		return true;
	}	
	return false;
}

// Продажа кому то дома
void HouseSql::SellToHouse(int SellerID, int BuyightID, int Price)
{
	// check player in world
	CPlayer *pSeller = GS()->GetPlayer(SellerID, true);
	CPlayer *pBuyight = GS()->GetPlayer(BuyightID, true);
	if(!pSeller || !pBuyight)
		return;

	// checl buyight ownerhouses and release
	boost::scoped_ptr<ResultSet> RES(SJK.SD("ID", "tw_homes", "WHERE OwnerID = '%d'", pBuyight->Acc().AuthID));
	if(RES->next())
	{
		GS()->Chat(BuyightID, "You are already a home owner!");	
		return;
	}

	// check owner seller
	boost::scoped_ptr<ResultSet> RES2(SJK.SD("ID", "tw_homes", "WHERE OwnerID = '%d'", pSeller->Acc().AuthID));
	if(RES2->next())
	{
		// check money for buy this house and release
		if(pBuyight->CheckFailMoney(Price))	
		{	
			GS()->Chat(BuyightID, "You have so much money!");
			GS()->Chat(SellerID, "The player {STR} doesn't have that much money", GS()->Server()->ClientName(BuyightID));
			return;
		}
	
		// send money who sell house
		int SellerAuthID = pSeller->Acc().AuthID;
		Job()->Inbox()->SendInbox(SellerAuthID, "House is sold", "Your house is sold !", itMoney, Price, 0);

		// update house information
		int HouseID = RES2->getInt("ID");
		int BuyightAuthID = pBuyight->Acc().AuthID;
		SJK.UD("tw_homes", "OwnerID = '%d' WHERE ID = '%d'", BuyightAuthID, HouseID);
		
		// если есть двери удаляем их
		Home[HouseID].hOwnerID = BuyightAuthID;
	
		// send message
		GS()->Chat(-1, "Made a deal {STR} and {STR} at home {VAL} G", 
			GS()->Server()->ClientName(SellerID), GS()->Server()->ClientName(BuyightID), &Price);

		GS()->ChatDiscord(false, DC_SERVER_INFO, "Server information", "**Made a deal [{STR} and {STR}] at home [{INT}G]**", 
			GS()->Server()->ClientName(SellerID), GS()->Server()->ClientName(BuyightID), &Price);	
		
		GS()->ResetVotes(SellerID, MAINMENU);
		GS()->ResetVotes(BuyightID, MAINMENU);
		return;
	}	
	return;
}
// Продажа дома
void HouseSql::SellHouse(int HouseID)
{
	boost::scoped_ptr<ResultSet> RES(SJK.SD("OwnerID, Function", "tw_homes", "WHERE ID = '%d'", HouseID));
	if(RES->next())
	{
		int OwnerID = RES->getInt("OwnerID");
		int Function = RES->getInt("Function");
		int ClientID = Job()->Account()->CheckOnlineAccount(OwnerID); 

		if(Function == FUNCTIONHOUSE)
		{                                          
			if(ClientID >= 0)
			{
				GS()->ChatFollow(ClientID, "Your House is sold !");
				GS()->Chat(-1, "House: {INT} have been is released!", &HouseID);
				GS()->ChatDiscord(false, DC_SERVER_INFO, "Server information", "**[House: {INT}] have been sold!**", &HouseID);
			}
			int Price = Home[HouseID].hPrice;
			Job()->Inbox()->SendInbox(OwnerID, "House is sold", "Your house is sold !", itMoney, Price, 0);
		}

		// удалить двери если есть и очистить инфу
		if(Home[HouseID].m_Door)
		{
			delete Home[HouseID].m_Door;
			Home[HouseID].m_Door = 0;
		}
		Home[HouseID].hBank = 50000;
		Home[HouseID].hOwnerID = -1;
		
		// обновить голосования
		if(ClientID >= 0)
			GS()->ResetVotes(ClientID, MAINMENU);

		SJK.UD("tw_homes", "OwnerID = '0', HouseBank = '50000' WHERE ID = '%d'", HouseID);
	}
}

// Снять баланс с фарм счета
void HouseSql::TakeFarmMoney(CPlayer *pPlayer, int TakeCount)
{
	int ClientID = pPlayer->GetCID(), HouseID = -1;
	boost::scoped_ptr<ResultSet> RES(SJK.SD("ID, Farm", "tw_homes", "WHERE OwnerID = '%d'", pPlayer->Acc().AuthID));
	if(RES->next())
	{
		HouseID = RES->getInt("ID");
		Home[HouseID].hFarm = RES->getInt("Farm");
	} else return;

	if(Home[HouseID].hFarm <= 0) return;
	if(TakeCount > Home[HouseID].hFarm)
		TakeCount = Home[HouseID].hFarm;

	Home[HouseID].hFarm -= TakeCount;
	GS()->SendInbox(ClientID, "You banking operation", "You have withdrawn savings from your home account", itMoney, TakeCount);
	SJK.UD("tw_homes", "Farm = '%d' WHERE ID = '%d'", Home[HouseID].hFarm, HouseID);
}

// Добавление баланса
void HouseSql::AddBalance(CPlayer *pPlayer, int Balance)
{
	int ClientID = pPlayer->GetCID(), HouseID = -1;
	boost::scoped_ptr<ResultSet> RES(SJK.SD("ID, HouseBank, FarmLevel", "tw_homes", "WHERE OwnerID = '%d'", pPlayer->Acc().AuthID));
	if(RES->next())
	{
		HouseID = RES->getInt("ID");
		Home[HouseID].hFarmLevel = RES->getInt("FarmLevel")+(Balance/1000);
		Home[HouseID].hBank = RES->getInt("HouseBank")+Balance;
	} else return;
                
	GS()->Chat(ClientID, "Current House [Farming Level {INT}]!", &Home[HouseID].hFarmLevel);
	GS()->Chat(ClientID, false, _("Payment: +{INT} Now House Balance: {INT}!"), &Balance, &Home[HouseID].hBank);
	SJK.UD("tw_homes", "HouseBank = '%d', FarmLevel = '%d' WHERE ID = '%d'", Home[HouseID].hBank, Home[HouseID].hFarmLevel, HouseID);
}

// Действия над дверью
void HouseSql::ChangeStateDoor(int HouseID)
{
	if(Home.find(HouseID) == Home.end() || Home[HouseID].hOwnerID <= 0) return;

	// если мир не равен данному
	if(Home[HouseID].hWorldID != GS()->GetWorldID())
	{
		GS()->ChatAccountID(Home[HouseID].hOwnerID, "Change state door can only near your house.");
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
		Home[HouseID].m_Door = new HouseDoor(&GS()->m_World, vec2(Home[HouseID].hDoorX, Home[HouseID].hDoorY));
	}

	// надпись если найдется игрок
	bool StateDoor = (Home[HouseID].m_Door);
	GS()->ChatAccountID(Home[HouseID].hOwnerID, "You {STR} the house.", (StateDoor ? "closed" : "opened"));
}

/* #########################################################################
	MENUS HOUSES 
######################################################################### */
// Показ меню дома
void HouseSql::ShowHouseMenu(CPlayer *pPlayer, int HouseID)
{
	int ClientID = pPlayer->GetCID();
	GS()->AVH(ClientID, HHOMEINFO, vec3(35,80,40), "House {INT} . {STR}", &HouseID, Home[HouseID].hClass);
	GS()->AVL(ClientID, "null", "Owner House: {STR}", Job()->PlayerName(Home[HouseID].hOwnerID));
	GS()->AVM(ClientID, "null", NOPE, HHOMEINFO, "House information : Helper house system");
	GS()->AVM(ClientID, "null", NOPE, HHOMEINFO, "Every game day, you got check payment house.");
	GS()->AVM(ClientID, "null", NOPE, HHOMEINFO, "If there is no money left in the account at home.");
	GS()->AVM(ClientID, "null", NOPE, HHOMEINFO, "The house is automatically sold and you get");
	GS()->AVM(ClientID, "null", NOPE, HHOMEINFO, "Mail with attached money");
	GS()->AV(ClientID, "null", "");

	GS()->AVH(ClientID, HHOMECOMMAND, vec3(35,80,40), "House command");
	GS()->AVM(ClientID, "null", NOPE, HHOMECOMMAND, "/doorhouse - Open or close door your house");
	GS()->AVM(ClientID, "null", NOPE, HHOMECOMMAND, "/sellhouse - To sell the house to the city");
	GS()->AVM(ClientID, "null", NOPE, HHOMECOMMAND, "/selltohouse <clientid> <price> - To sell the house to player");
	GS()->AVM(ClientID, "null", NOPE, HHOMECOMMAND, "Another in House Menu, available only owner ");
	GS()->AV(ClientID, "null", "");

	if(Home[HouseID].hOwnerID <= 0)
	{
		int Price =  Home[HouseID].hPrice + g_Config.m_SvHousePriceUse;
		GS()->AVM(ClientID, "BUYHOUSE", HouseID, NOPE, "Buy this house. Price {INT}gold", &Price);
	}
}
// Показ меню дома персонально
void HouseSql::ShowPersonalHouse(CPlayer *pPlayer)
{
	int ClientID = pPlayer->GetCID();
	int HouseID = PlayerHouseID(pPlayer);
	if(HouseID <= 0) return GS()->AVL(ClientID, "null", "You not owner home!");

	bool StateDoor = GetHouseDoor(HouseID);
	GS()->AVH(ClientID, HHOUSESTATS, vec3(52,26,80), "House stats {INT} Class {STR} Door [{STR}]", &HouseID, Home[HouseID].hClass, StateDoor ? "Closed" : "Opened");
	GS()->AVM(ClientID, "null", NOPE, HHOUSESTATS, "Farming level: every day +{INT}gold", &Home[HouseID].hFarmLevel);
	GS()->AVM(ClientID, "null", NOPE, HHOUSESTATS, "Bank farming house {INT}gold", &Home[HouseID].hFarm);
	GS()->AVM(ClientID, "null", NOPE, HHOUSESTATS, "- - - - - - - - - -");
	GS()->AVM(ClientID, "null", NOPE, HHOUSESTATS, "Your money: {INT}gold", &pPlayer->GetItem(itMoney).Count);
	GS()->AVM(ClientID, "null", NOPE, HHOUSESTATS, "House {INT}. Every day -{INT}gold", &Home[HouseID].hBank, &g_Config.m_SvHousePriceUse);
	GS()->AVM(ClientID, "null", NOPE, HHOUSESTATS, "- - - - - - - - - -");
	GS()->AVM(ClientID, "null", NOPE, HHOUSESTATS, "Notes: Minimal operation house balance 100gold");
	GS()->AVM(ClientID, "null", NOPE, HHOUSESTATS, "Sell house for player. You gived your house bank.");
	GS()->AV(ClientID, "null", "");
	GS()->AVM(ClientID, "HOUSEMONEY", 1, NOPE, "Add in house money. (Amount in a reason)");
	GS()->AVM(ClientID, "HOUSETAKE", 1, NOPE, "Take in house farming bank. (Amount in a reason)");
	GS()->AVM(ClientID, "HOUSEDOOR", HouseID, NOPE, "Change state [\"{STR} door\"]", StateDoor ? "Open" : "Close");
	GS()->AVM(ClientID, "HSPAWN", 1, NOPE, "Teleport to your house");
	if(Home[HouseID].hWorldID == GS()->Server()->GetWorldID(ClientID))
	{
		GS()->AVM(ClientID, "MENU", HOUSEDECORATION, NOPE, "Settings Decorations");
		GS()->AVM(ClientID, "MENU", HOUSEPLANTS, NOPE, "Settings Plants");
	}
	else GS()->AVM(ClientID, "null", HOUSEDECORATION, NOPE, "More settings allow only own House World");
}

/* #########################################################################
	PARSING HOUSES 
######################################################################### */
// Парсинг голосований
bool HouseSql::OnParseVotingMenu(CPlayer *pPlayer, const char *CMD, const int VoteID, const int VoteID2, int Get, const char *GetText)
{
	int ClientID = pPlayer->GetCID();
	if(PPSTR(CMD, "BUYHOUSE") == 0)
	{
		if(BuyHouse(VoteID, pPlayer))
		{
			GS()->Chat(-1, "{STR} becomes the owner of the house class {STR} HID {INT}", GS()->Server()->ClientName(ClientID), Home[VoteID].hClass, &VoteID);
			GS()->ChatDiscord(false, DC_SERVER_INFO, "Server information", "**{STR} becomes the owner of the house class [{STR} number {INT}]**",
					GS()->Server()->ClientName(ClientID), Home[VoteID].hClass, &VoteID);
			GS()->ChatFollow(ClientID, "Do not forget to top up the balance at home, now the balance {INT}G", &Home[VoteID].hBank);
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
		if(HouseWorldID != GS()->Server()->GetWorldID(ClientID))
		{
			pPlayer->Acc().TeleportX = Position.x;
			pPlayer->Acc().TeleportY = Position.y;
			GS()->Server()->ChangeWorld(ClientID, HouseWorldID);
			return true;
		}
		else 
		{
			pPlayer->GetCharacter()->ChangePosition(Position);
		}
		return true;
	}

	// Оплата дома
	if(PPSTR(CMD, "HOUSEMONEY") == 0)
	{
		if(Get < 100 || pPlayer->CheckFailMoney(Get))
			return true;

		AddBalance(pPlayer, Get);
		GS()->VResetVotes(ClientID, HOUSEMENU);
		return true;
	}

	// Снятие сбережения
	if(PPSTR(CMD, "HOUSETAKE") == 0)
	{
		if(Get < 100)
			return true;

		TakeFarmMoney(pPlayer, Get);
		GS()->VResetVotes(ClientID, HOUSEMENU);
		return true;
	}

	// Дверь дома
	if(PPSTR(CMD, "HOUSEDOOR") == 0)
	{
		ChangeStateDoor(VoteID);
		GS()->VResetVotes(ClientID, HOUSEMENU);
		return true;		
	}

	// начала расстановки декорации
	if(PPSTR(CMD, "DECOSTART") == 0)
	{
		if(!pPlayer->GetCharacter())
			return true;

		// дом проверка
		int HouseID = PlayerHouseID(pPlayer);
		if(HouseID < 0)
		{
			GS()->Chat(ClientID, "You not owner home!");
			return true;
		}

		// дастанция проверка
		vec2 PositionHouse = GetPositionHouse(HouseID);
		if(distance(PositionHouse, pPlayer->GetCharacter()->m_Core.m_Pos) > 600)
		{
			GS()->Chat(ClientID, "Maximum distance between your home 600!");
			return true;
		}

		// пишем о добавлении
		int *DecoID = &CGS::InteractiveSub[ClientID].TempID;
		
		// очищаем голосования
		GS()->ClearVotes(ClientID);

		// информация
		GS()->AV(ClientID, "null", "Please close vote and press Left Mouse,");
		GS()->AV(ClientID, "null", "on position where add decoration!");
		GS()->AddBack(ClientID);

		pPlayer->m_LastVoteMenu = INVENTORY;
		*DecoID = VoteID;
		return true;
	}

	// декорации
	if(PPSTR(CMD, "DECODELETE") == 0)
	{
		// дом проверка
		int HouseID = PlayerHouseID(pPlayer);
		if(HouseID < 0)
		{
			GS()->Chat(ClientID, "You not owner home!");
			return true;
		}

		if(DeleteDecorationHouse(VoteID))
		{
			ItemSql::ItemPlayer &PlDecoItem = pPlayer->GetItem(VoteID2);
			GS()->Chat(ClientID, "You back to the backpack {STR}!", PlDecoItem.Info().GetName(pPlayer));
			PlDecoItem.Add(1);
		}
		GS()->VResetVotes(ClientID, HOUSEDECORATION);
		return true;
	}

	// смена расстения дома
	if(PPSTR(CMD, "HOMEPLANTSET") == 0)
	{
		// дом проверка
		int HouseID = PlayerHouseID(pPlayer);
		if(HouseID < 0)
		{
			GS()->Chat(ClientID,"You not owner home!");
			return true;
		}

		// не разрешать менять если нет поддержки
		if(GetPlantsID(HouseID) <= 0)
		{
			GS()->Chat(ClientID, "Your home does not support plants!");
			return true;		
		}

		// проверка хватит ли расстений на рассадку дома
		if(pPlayer->CheckFailMoney(VoteID2, VoteID))
			return true;

		ChangePlantsID(HouseID, VoteID);
		GS()->ResetVotes(ClientID, pPlayer->m_OpenVoteMenu);
	}
	return false;
}

// Двери
HouseDoor::HouseDoor(CGameWorld *pGameWorld, vec2 Pos)
: CEntity(pGameWorld, CGameWorld::ENTTYPE_HOUSEDOOR, Pos)
{
	m_To = vec2(Pos.x, Pos.y-140);
	m_Pos.y += 30;

	GameWorld()->InsertEntity(this);
}
HouseDoor::~HouseDoor(){}

void HouseDoor::Tick()
{
	for(CCharacter *pChar = (CCharacter*) GameWorld()->FindFirst(CGameWorld::ENTTYPE_CHARACTER); pChar; pChar = (CCharacter *)pChar->TypeNext())
	{
		vec2 IntersectPos = closest_point_on_line(m_Pos, m_To, pChar->m_Core.m_Pos);
		float Distance = distance(IntersectPos, pChar->m_Core.m_Pos);
		
		// снижаем скокрость
		if(Distance < 64.0f && length(pChar->m_Core.m_Vel) >= 64.0)
			pChar->m_Core.m_Vel = vec2(0,0);

		// проверяем дистанцию
		if(Distance < 30.0f) 
		{
			vec2 Dir = normalize(pChar->m_Core.m_Pos - IntersectPos);
			float a = (30.0f*1.45f - Distance);
			float Velocity = 0.5f;
			if (length(pChar->m_Core.m_Vel) > 0.0001)
				Velocity = 1-(dot(normalize(pChar->m_Core.m_Vel), Dir)+1)/4;
		
			pChar->m_Core.m_Vel += (Dir*a*(Velocity*0.75f))*0.85f;
			pChar->m_Core.m_Pos = pChar->m_OldPos + Dir;
		}
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

// Декорации
DecoHouse::DecoHouse(CGameWorld *pGameWorld, vec2 Pos, int HouseID, int DecoID)
: CEntity(pGameWorld, CGameWorld::ENTTYPE_DECOHOUSE, Pos)
{
	m_HouseID = HouseID;
	m_DecoID = DecoID;

	GameWorld()->InsertEntity(this);

	if(SwitchToObject(true) >= 0) {
		for(int i=0; i < NUM_IDS; i++)
			m_IDs[i] = Server()->SnapNewID();
	}
}
DecoHouse::~DecoHouse()
{
	if(SwitchToObject(true) >= 0) {
		for(int i=0; i<NUM_IDS; i++)
			Server()->SnapFreeID(m_IDs[i]);
	}
}

int DecoHouse::SwitchToObject(bool Data)
{
	switch(m_DecoID)
	{
		case itDecoHealth: return (Data ? -1 : (int)PICKUP_HEALTH);
		case itDecoArmor: return (Data ? -1 : (int)PICKUP_ARMOR);
		case itEliteDecoHealth: return (Data ? (int)WEAPON_SHOTGUN : (int)PICKUP_HEALTH);
		case itEliteDecoNinja: return (Data ? -1 : (int)PICKUP_NINJA);
	}
	return -1;
}

void DecoHouse::Tick()
{
}

void DecoHouse::Snap(int SnappingClient)
{
	if (NetworkClipped(SnappingClient))
		return;
	
	if(SwitchToObject(true) <= -1)
	{
		CNetObj_Pickup *pP = static_cast<CNetObj_Pickup *>(Server()->SnapNewItem(NETOBJTYPE_PICKUP, GetID(), sizeof(CNetObj_Pickup)));
		if(!pP)
			return;

		pP->m_X = (int)m_Pos.x;
		pP->m_Y = (int)m_Pos.y;
		pP->m_Type = SwitchToObject(false);
		return;
	}

	float AngleStart = (2.0f * pi * Server()->Tick()/static_cast<float>(Server()->TickSpeed()))/10.0f;
	float AngleStep = 2.0f * pi / DecoHouse::BODY;
	float Radius = 30.0f;
	for(int i=0; i < DecoHouse::BODY; i++)
	{
		vec2 PosStart = m_Pos + vec2(Radius * cos(AngleStart + AngleStep*i), Radius * sin(AngleStart + AngleStep*i));
		CNetObj_Projectile *pObj = static_cast<CNetObj_Projectile *>(Server()->SnapNewItem(NETOBJTYPE_PROJECTILE, m_IDs[i], sizeof(CNetObj_Projectile)));
		if(!pObj)
			return;

		pObj->m_X = (int)PosStart.x;
		pObj->m_Y = (int)PosStart.y;
		pObj->m_VelX = 0;
		pObj->m_VelY = 0;
		pObj->m_StartTick = Server()->Tick()-1;
		pObj->m_Type = SwitchToObject(true);
	}


	CNetObj_Pickup *pP = static_cast<CNetObj_Pickup *>(Server()->SnapNewItem(NETOBJTYPE_PICKUP, m_IDs[BODY], sizeof(CNetObj_Pickup)));
	if(!pP)
		return;

	pP->m_X = (int)m_Pos.x;
	pP->m_Y = (int)m_Pos.y;
	pP->m_Type = SwitchToObject(false);
}