/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#include <engine/shared/config.h>
#include <game/server/gamecontext.h>
#include "storage.h"

using namespace sqlstr;
std::map < int , StorageSql::SturctStorage > StorageSql::Storage;

void StorageSql::OnInitGlobal() 
{ 
	// загрузить склады
	boost::scoped_ptr<ResultSet> RES(SJK.SD("*", "tw_storages", "WHERE ID > '0'"));
	while(RES->next())
	{
		const int ID = (int)RES->getInt("ID");
		Storage[ID].TextX = (int)RES->getInt("TextX");
		Storage[ID].TextY = (int)RES->getInt("TextY");
		Storage[ID].LoadPosX = (int)RES->getInt("LoadPosX");
		Storage[ID].LoadPosY = (int)RES->getInt("LoadPosY");
		Storage[ID].Count = (int)RES->getInt("Count");
		Storage[ID].WorldID = (int)RES->getInt("WorldID");
		Storage[ID].OwnerID = (int)RES->getInt("OwnerID");
		Storage[ID].Price = (int)RES->getInt("Price");
		Storage[ID].Bank = (int)RES->getInt("Bank");
		Storage[ID].MonsterSubType = (int)RES->getInt("MonsterSubType");
		str_copy(Storage[ID].Name, RES->getString("Name").c_str(), sizeof(Storage[ID].Name));
	}
}

void StorageSql::OnTick()
{
	if(GS()->Server()->Tick() % GS()->Server()->TickSpeed() == 0)
	{
		for (const auto& x : Storage)
		{
			// пропускаем нулевые и склады которые не с этого мира
			if(x.second.Count < 0 || GS()->GetWorldID() != x.second.WorldID) 
				continue;
			
			// выводим текст о кол-ве на складах
			vec2 Pos = vec2(x.second.TextX, x.second.TextY);
			GS()->CreateText(NULL, false, Pos, vec2(0, 0), 248, std::to_string(Storage[x.first].Count).c_str(), x.second.WorldID);
		}
	}
}

void StorageSql::OnPaymentTime()
{
	boost::scoped_ptr<ResultSet> RES(SJK.SD("ID, OwnerID, Bank", "tw_storages", "WHERE OwnerID > '0'"));
	while(RES->next())
	{
		// ищим если на складу больше денег оплачиваем и проверяем следующий склад
		const int StorageID = (int)RES->getInt("ID"), OwnerID = (int)RES->getInt("OwnerID"), Bank = (int)RES->getInt("Bank");
		if(Bank >= g_Config.m_SvPaymentBussines)
		{
			Storage[StorageID].Bank = Bank-g_Config.m_SvPaymentBussines;
			SJK.UD("tw_storages", "Bank = '%d' WHERE ID = '%d'", Storage[StorageID].Bank, StorageID);
			SCO(StorageID, "[Business] Payment {INT} gold was successful {STR}", &g_Config.m_SvPaymentBussines, Storage[StorageID].Name, NULL);
			continue;
		}

		// продать склад
		SellStorage(OwnerID);
		GS()->Chat(-1, "[Business] {STR} sold nope payment!", Storage[StorageID].Name);
	}
}

/* #########################################################################
	GLOBAL STORAGE CLASS 
######################################################################### */
// показать действия с складом
void StorageSql::ShowStorageMenu(int ClientID, int StorageID)
{
	CPlayer *pPlayer = GS()->GetPlayer(ClientID);
	if(!pPlayer) return;

	// если склад не работает
	if(StorageID < 0) return GS()->AV(ClientID, "null", "Storage Don't work");

	// ремонт и имя склада
	GS()->AVH(ClientID, HSTORAGEUSE, vec3(40,50,60), "Bussines [{STR}/{INT}]", Storage[StorageID].Name, &Storage[StorageID].Count);

	// выводим владельца склада
	const bool LoadStorage = (Storage[StorageID].MonsterSubType > 0);
	if(Storage[StorageID].OwnerID > 0)
	{
		GS()->AVM(ClientID, "null", NOPE, HSTORAGEUSE, "{STR} Owner: {STR}", Storage[StorageID].Name, Job()->PlayerName(Storage[StorageID].OwnerID));	
	}
	else if(!LoadStorage)
	{
		int Price = Storage[StorageID].Price + g_Config.m_SvPaymentBussines;
		GS()->AVM(ClientID, "BUYSTORAGE", StorageID, HSTORAGEUSE, "Buy this business. Price {INT}", &Price);	
	}

	// если владелец бизнеса
	if(Storage[StorageID].OwnerID == pPlayer->Acc().AuthID)
	{
		GS()->AVM(ClientID, "null", NOPE, HSTORAGEUSE, "Every day -{INT} from the bank, or sell", &g_Config.m_SvPaymentBussines);	
		GS()->AVM(ClientID, "null", NOPE, HSTORAGEUSE, "Your bank: {INT}", &Storage[StorageID].Bank);
		GS()->AVM(ClientID, "null", NOPE, HSTORAGEUSE, "- - - - - - - - - - - - - - - - - - -");	
		GS()->AVM(ClientID, "ADDMSTORAGE", StorageID, HSTORAGEUSE, "Add money in business");		
		GS()->AVM(ClientID, "REMMSTORAGE", StorageID, HSTORAGEUSE, "Remove money in business");
		GS()->AVM(ClientID, "SELLSTORAGE", StorageID, HSTORAGEUSE, "Sell storage in reason (7). Back half the cost.");		
	}

	// если мобы для этого склада есть и починка предметов
	GS()->AVM(ClientID, "REPAIRITEMS", StorageID, HSTORAGEUSE, "Repair all items - FREE");
	if(LoadStorage) GS()->AVM(ClientID, "LOADSTORAGE", StorageID, HSTORAGEUSE, "Load goods");
	else GS()->AVM(ClientID, "UNLOADSTORAGE", StorageID, HSTORAGEUSE, "Unload all goods");
}

/* #########################################################################
	HELPER STORAGE CLASS 
######################################################################### */
// имя бизнеса
const char *StorageSql::StorageName(int StorageID) const
{
	if(Storage.find(StorageID) != Storage.end()) return Storage.at(StorageID).Name;
	return "Unknown";
}

// получить количество на складе по айди
int StorageSql::GetCountStorage(int StorageID) const 
{ 
	if(Storage.find(StorageID) != Storage.end()) return Storage.at(StorageID).Count;
	return -1;
}

// получить деньги на складе по айди
int StorageSql::GetBankStorage(int StorageID) const 
{ 
	if(Storage.find(StorageID) != Storage.end()) return Storage.at(StorageID).Bank;
	return -1;
}

// получить айди по типу монстра
int StorageSql::GetStorageMonsterSub(int MonsterSubType) const
{
	for(const auto& st : Storage)
	{
		if(st.second.MonsterSubType != MonsterSubType) continue;
		return st.first;
	}
	return -1;
}

// получить айди склада по позиции
int StorageSql::GetLoadStorage(vec2 Pos) const
{ 
	for(const auto& st : Storage)
	{
		vec2 PosStorage = vec2(st.second.LoadPosX, st.second.LoadPosY);
		if(distance(PosStorage, Pos) > 200) continue;
		return st.first; 
	} 
	return -1;
}

// получить валидное кол-во требуемое goods со склада
int StorageSql::ValidGoodsPrice(int Price) const { return (int)(Price/g_Config.m_SvStorageFraction); }

/* #########################################################################
	FUNCTION STORAGE CLASS 
######################################################################### */
// купить в складе предмет
bool StorageSql::BuyStorageItem(bool CheckStorageCount, int BoughtCID, int StorageID, int Price)
{
	// проверяем если стоит проверять
	int StorageGoods = ValidGoodsPrice(Price);
	if(CheckStorageCount)
	{
		if(GetCountStorage(StorageID) < StorageGoods)
		{
			GS()->Chat(BoughtCID, "This purchase will require {INT} goods in warehouse.", &StorageGoods);
			return false;
		}
		return true;
	}

	// если без проверки то забераем и все
	RemoveStorage(StorageID, StorageGoods);
	AddStorageMoney(StorageID, Price);
	
	// пишем что кто-то что-то купил
	int StorageBank = GetBankStorage(StorageID);
	Job()->Storage()->SCO(StorageID, "{STR} something bought, your business Bank {INT}", GS()->Server()->ClientName(BoughtCID), &StorageBank);
	return true;
}

// добавить в склад по айди количество
void StorageSql::AddStorage(int StorageID, int Count)
{
	// ищим есть ли такой склад
	ResultSet* RES = SJK.SD("Count", "tw_storages", "WHERE ID = '%d'", StorageID);
	if(RES->next()) return;
	
	// добавляем на склад товары
	const int ChangesValue = (int)RES->getInt("Count")+Count;
	Storage[StorageID].Count = ChangesValue;
	SJK.UD("tw_storages", "Count = '%d' WHERE ID = '%d'", ChangesValue, StorageID);	
}

// снять со склада по айди количество
void StorageSql::RemoveStorage(int StorageID, int Count)
{
	// ищим есть ли такой склад
	boost::scoped_ptr<ResultSet> RES(SJK.SD("Count", "tw_storages", "WHERE ID = '%d'", StorageID));
	if(!RES->next()) return;

	// снимаем со склада товары
	const int ChangesValue = (int)RES->getInt("Count")-Count;
	Storage[StorageID].Count = ChangesValue;
	SJK.UD("tw_storages", "Count = '%d' WHERE ID = '%d'", ChangesValue, StorageID);		
}

// добавить в склад по айди количество
void StorageSql::AddStorageMoney(int StorageID, int Count)
{
	// ищим есть ли такой склад
	boost::scoped_ptr<ResultSet> RES(SJK.SD("Bank", "tw_storages", "WHERE OwnerID > '0' AND ID = '%d'", StorageID));
	if(!RES->next()) return;
	
	// добовляем на склад золото
	const int ChangesValue = (int)RES->getInt("Bank")+Count;
	Storage[StorageID].Bank = ChangesValue;
	SJK.UD("tw_storages", "Bank = '%d' WHERE ID = '%d'", ChangesValue, StorageID);
}

// снять со склада по айди количество
void StorageSql::RemoveStorageMoney(int StorageID, int Count)
{
	// ищим есть ли такой склад
	boost::scoped_ptr<ResultSet> RES(SJK.SD("Bank", "tw_storages", "WHERE OwnerID > '0' AND ID = '%d'", StorageID));
	if(!RES->next()) return;
	
	// снимаем со склада золото
	const int ChangesValue = (int)RES->getInt("Bank")-Count;
	Storage[StorageID].Bank = ChangesValue;
	SJK.UD("tw_storages", "Bank = '%d' WHERE ID = '%d'", ChangesValue, StorageID);
}

// купить склад
void StorageSql::BuyStorages(CPlayer *pPlayer, int StorageID)
{
	// проверяем если уже владелец другого склада
	const int ClientID = pPlayer->GetCID();
	boost::scoped_ptr<ResultSet> RES(SJK.SD("ID", "tw_storages", "WHERE OwnerID = '%d'", pPlayer->Acc().AuthID));
	if(RES->next())	return GS()->Chat(ClientID, "You already owner another business.");		

	// ищим есть ли такой склад и владелеца нету
	boost::scoped_ptr<ResultSet> RES2(SJK.SD("*", "tw_storages", "WHERE OwnerID < '1' AND ID = '%d'", StorageID));
	if(!RES2->next()) return;

	const int Price = ((int)RES2->getInt("Price") + g_Config.m_SvPaymentBussines);
	if(pPlayer->CheckFailMoney(Price)) return;
	
	// обновление информации и отправка в бд
	Storage[StorageID].Bank = g_Config.m_SvPaymentBussines;
	Storage[StorageID].OwnerID = pPlayer->Acc().AuthID;
	SJK.UD("tw_storages", "OwnerID = '%d', Bank = '%d' WHERE ID = '%d'", pPlayer->Acc().AuthID, g_Config.m_SvPaymentBussines, StorageID);

	// чат информация о покупки
	GS()->Chat(-1, "{STR} becomes the owner {STR}", GS()->Server()->ClientName(ClientID), Storage[StorageID].Name);
	GS()->ChatDiscord(false, DC_PLAYER_INFO, "Server information", "**{STR} becomes the owner {STR}**", GS()->Server()->ClientName(ClientID), Storage[StorageID].Name);
}

// продать склад
void StorageSql::SellStorage(int AuthID)
{
	// ищим свой склад
	boost::scoped_ptr<ResultSet> RES(SJK.SD("ID, Price", "tw_storages", "WHERE OwnerID = '%d'", AuthID));
	if(!RES->next()) return;
	
	// отпровляем письмо о продажи
	const int Price = RES->getInt("Price");
	Job()->Inbox()->SendInbox(AuthID, "Your business sell.",  "Your money sell business", itMoney, Price, 0);

	// сбрасываем владельца склада
	const int StorageID = RES->getInt("ID");
	Storage[StorageID].OwnerID = -1;
	Storage[StorageID].Bank = g_Config.m_SvPaymentBussines;
	SJK.UD("tw_storages", "OwnerID = '0', Bank = '%d' WHERE ID = '%d'", g_Config.m_SvPaymentBussines, StorageID);
}

/* #########################################################################
	ANOTHER STORAGE CLASS 
######################################################################### */
// Отправить текст владельцу
void StorageSql::SCO(int StorageID, const char* pText, ...)
{
	if(Storage.find(StorageID) == Storage.end()) return;

	const int OwnerID = Storage[StorageID].OwnerID;
	const int ClientID = Job()->Account()->CheckOnlineAccount(OwnerID);

	// проверяем владельца 
	if(OwnerID > 0 && GS()->GetPlayer(ClientID, true))
	{
		dynamic_string Buffer;

		va_list VarArgs;
		va_start(VarArgs, pText);

		CNetMsg_Sv_Chat Msg;
		Msg.m_Mode = CHAT_ALL;
		Msg.m_ClientID = -1;
		Msg.m_TargetID = ClientID;

		GS()->Server()->Localization()->Format_VL(Buffer, GS()->m_apPlayers[ClientID]->GetLanguage(), pText, VarArgs);
		Msg.m_pMessage = Buffer.buffer();

		GS()->Server()->SendPackMsg(&Msg, MSGFLAG_VITAL, ClientID);
		
		Buffer.clear();
		va_end(VarArgs);
	}
}

// Парсинг голосований кафе
bool StorageSql::OnParseVotingMenu(CPlayer *pPlayer, const char *CMD, const int VoteID, const int VoteID2, int Get, const char *GetText)
{
	const int ClientID = pPlayer->GetCID();

	// разгрузить со склада товары
	if(PPSTR(CMD, "UNLOADSTORAGE") == 0)
	{
		int Count = pPlayer->GetItem(itGoods).Count;

		// проверяем есть возмож
		if(Count <= 0)
		{
			GS()->Chat(ClientID, "You have no cargo to unload.");
			return true;
		}

		// проверяем оплату товара
		if(pPlayer->CheckFailMoney(Count, itGoods))
			return true;

		pPlayer->GetItem(itMaterial).Add(Count);
		AddStorage(VoteID, Count);
		GS()->Chat(ClientID, "You sell Goodsx{INT}.", &Count);
		GS()->ChatDiscord(false, DC_SERVER_INFO, "Server information", "**{STR} was delivery {INT} goods.**", Storage[VoteID].Name, &Count);
		GS()->VResetVotes(ClientID, MAINMENU);
		return true;
	}
	
	// загрузить со склада товары
	if(PPSTR(CMD, "LOADSTORAGE") == 0)
	{
		const int CountItem = pPlayer->GetItem(itGoods).Count;
		const int StorageCount = GetCountStorage(VoteID);

		// проверяем в инвентаре полное кол-во максимальное или нет
		if(CountItem >= 500) 
		{
			GS()->Chat(ClientID, "You reach maximal goods in a backpack.");
			return true;
		}
	
		// устанавливаем и проверяем на складе сколько можно снять
		int Count = 500 - CountItem;
		if(StorageCount < Count) { Count = StorageCount; }

		// проверяем если на складе 0
		if(Count <= 0)
		{
			GS()->Chat(ClientID, "Storage is empty.");					
			return true;
		}

		// снимаем со склада и получаем товары
		pPlayer->GetItem(itGoods).Add(Count);
		RemoveStorage(VoteID, Count);
		GS()->Chat(ClientID, "You load in backpack Goodsx{INT}.", &Count);
		GS()->VResetVotes(ClientID, MAINMENU);
		return true;
	}

	// купить склад
	if(PPSTR(CMD, "BUYSTORAGE") == 0)
	{
		// покупка склада
		BuyStorages(pPlayer, VoteID);
		GS()->ResetVotes(ClientID, MAINMENU);
		return true;
	}

	// продать склад
	if(PPSTR(CMD, "SELLSTORAGE") == 0)
	{
		// проверить если проверочное число не равно 7
		if(Get != 7)
		{
			GS()->Chat(ClientID, "For sell storage write on reason '7'.");
			return true;			
		}

		// продажа склада
		SellStorage(pPlayer->Acc().AuthID);
		GS()->ResetVotes(ClientID, MAINMENU);
		return true;
	}

	// снять со склада золото
	if(PPSTR(CMD, "REMMSTORAGE") == 0)
	{
		// устанавливаем минимальное снятие равное банку гильдии
		const int StorageID = VoteID;
		if(Storage[StorageID].Bank < Get)
			Get = Storage[StorageID].Bank;

		// проверяем является ли снятие меньш 100
		if(Get < 100)
		{
			GS()->Chat(ClientID, "You dont remove less than 100.");
			return true;
		}

		// снимаем со клада деньги
		pPlayer->AddMoney(Get);
		RemoveStorageMoney(StorageID, Get);
		GS()->Chat(ClientID, "You remove on your business {INT}.", &Get);
		GS()->ResetVotes(ClientID, MAINMENU);
		return true;
	}

	// добавить в склад золото
	if(PPSTR(CMD, "ADDMSTORAGE") == 0)
	{
		// проверяем является ли снятие меньш 100
		if(Get < 100)
		{
			GS()->Chat(ClientID, "You dont add less than 100.");
			return true;
		}

		// снимаем золото
		if(pPlayer->CheckFailMoney(Get))
			return true;
		
		// добавляем на склад
		AddStorageMoney(VoteID, Get);
		GS()->Chat(ClientID, "You added on your business {INT}.", &Get);
		GS()->ResetVotes(ClientID, MAINMENU);
		return true;
	}

	// починка предметов
	if(PPSTR(CMD, "REPAIRITEMS") == 0)
	{
		Job()->Item()->RepairDurability(pPlayer);
		GS()->Chat(ClientID, "You repaired all your items.");
		return true;
	}
	return false;
}