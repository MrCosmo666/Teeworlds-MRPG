/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#include <engine/shared/config.h>
#include <game/server/gamecontext.h>
#include "shopeat.h"

using namespace sqlstr;

/* Information NON STATIC DATA LOAD ONLY DATABASE (MultiWorld Work)*/
std::map < int , ShopEatSql::StructShopEat > ShopEatSql::ShopEat;

// Инициализация класса
void ShopEatSql::OnInitGlobal() 
{ 
	// загрузить еду
	boost::scoped_ptr<ResultSet> RES(SJK.SD("*", "tw_shopeat ORDER BY Price"));
	while(RES->next())
	{
		const int ID = (int)RES->getInt("ID");
		ShopEat[ ID ].Eat = (int)RES->getInt("Eat");
		ShopEat[ ID ].Price = (int)RES->getInt("Price");
		ShopEat[ ID ].StorageID = (int)RES->getInt("StorageID");
		str_copy(ShopEat[ ID ].EatName, RES->getString("EatName").c_str(), sizeof(ShopEat[ ID ].EatName));
	}
}

/* #########################################################################
	GET CHECK SHOP EAT
######################################################################### */
// Проверить склад существует ли он и в магазине указан он
bool ShopEatSql::CheckShopEatStorage(int StorageID)
{
	for(const auto & sheat : ShopEat)
	{
		if(StorageID == sheat.second.StorageID)
			return true;
	}
	return false;
}

/* #########################################################################
	FUNCTIONS SHOP EAT 
######################################################################### */
 // Действия покупка в кафе
void ShopEatSql::BuyEatItem(CPlayer *pPlayer, int EatID)
{
	const int ClientID = pPlayer->GetCID();
	const int StorageID = ShopEat[EatID].StorageID;

	// проверяем голод игрока
	if(pPlayer->Acc().Hungry >= 100)
		return GS()->SBL(ClientID, PRELEGENDARY, 100, "You are not hungry.");							
	
	// проверяем деньги и склад на доступность покупки
	const int Price = ShopEat[EatID].Price;
	if(!Job()->Storage()->BuyStorageItem(true, ClientID, StorageID, Price) || pPlayer->CheckFailMoney(Price)) return;	

	// даем + к голоду и пишем сообщение
	short Eat = ShopEat[EatID].Eat;
	pPlayer->Acc().Hungry = clamp(pPlayer->Acc().Hungry + Eat, 0, 100);

	// текст и покупаем предмет взаимодействуя со складом
	GS()->SBL(ClientID, PRELEGENDARY, 100, "Satiety +{INT}%. Curret {INT}%.", &Eat, &pPlayer->Acc().Hungry);	
	Job()->Storage()->BuyStorageItem(false, ClientID, StorageID, Price);

	// сохраняем аккаунт и обновляем голосование
	Job()->SaveAccount(pPlayer, SAVESTATS);
	GS()->VResetVotes(ClientID, MAINMENU);
}

// Список предметов в кафе продаваемых
void ShopEatSql::ShowListShopEat(CPlayer *pPlayer, int StorageID)
{
	if(!CheckShopEatStorage(StorageID)) return;
		
	// выводим кол-во денег и название склада
	const int ClientID = pPlayer->GetCID();
	GS()->AV(ClientID, "null", "");
	GS()->AVH(ClientID, HCAFELIST, vec3(40,40,40), "{STR} : Backpack [{INT} gold]", Job()->Storage()->StorageName(StorageID), &pPlayer->GetItem(itMoney).Count);

	// имщем список предметов в каффе
	const int StorageShopEat = Job()->Storage()->GetCountStorage(StorageID);
    for(const auto& sheat : ShopEat)
	{
		if(StorageID != sheat.second.StorageID) continue;
		int Available = StorageShopEat / Job()->Storage()->ValidGoodsPrice(sheat.second.Price); 
		GS()->AVMI(ClientID, "eat", "BUYSHOPEAT", sheat.first, HCAFELIST, "[{INT}gold +{INT} Eat][Available: {INT}] {STR}",
			&sheat.second.Price, &sheat.second.Eat, &Available, sheat.second.EatName);
    }
}

// Парсинг голосований кафе
bool ShopEatSql::OnParseVotingMenu(CPlayer *pPlayer, const char *CMD, const int VoteID, const int VoteID2, int Get, const char *GetText)
{
	const int ClientID = pPlayer->GetCID();
	if(PPSTR(CMD, "BUYSHOPEAT") == 0)
	{
		BuyEatItem(pPlayer, VoteID);
		return true;
	}	
	return false;
}