/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#include <engine/shared/config.h>
#include <game/server/gamecontext.h>
#include "storage.h"

using namespace sqlstr;
std::map < int , StorageSql::SturctStorage > StorageSql::Storage;

void StorageSql::OnInitGlobal() 
{ 
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
			const int LifeTime = GS()->Server()->TickSpeed() - 5;
			vec2 Pos = vec2(x.second.TextX, x.second.TextY);
			GS()->CreateText(NULL, false, Pos, vec2(0, 0), LifeTime, std::to_string(Storage[x.first].Count).c_str(), x.second.WorldID);
		}
	}
}

void StorageSql::OnPaymentTime()
{
	boost::scoped_ptr<ResultSet> RES(SJK.SD("ID, OwnerID, Bank", "tw_storages", "WHERE OwnerID > '0'"));
	while(RES->next())
	{
		const int StorageID = (int)RES->getInt("ID");
		const int OwnerID = (int)RES->getInt("OwnerID"); 
		const int Bank = (int)RES->getInt("Bank");
		if(Bank >= g_Config.m_SvPaymentBussines)
		{
			Storage[StorageID].Bank = (int)(Bank-g_Config.m_SvPaymentBussines);
			SJK.UD("tw_storages", "Bank = '%d' WHERE ID = '%d'", Storage[StorageID].Bank, StorageID);
			SCO(StorageID, "Payment {INT} gold was successful {STR}", &g_Config.m_SvPaymentBussines, Storage[StorageID].Name, NULL);
			continue;
		}

		// продать склад
		SellStorage(OwnerID);
		GS()->Chat(-1, "Business {STR} sold nope payment!", Storage[StorageID].Name);
	}
}

bool StorageSql::OnPlayerHandleTile(CCharacter* pChr, int IndexCollision)
{
	CPlayer* pPlayer = pChr->GetPlayer();
	const int ClientID = pPlayer->GetCID();

	if (pChr->GetHelper()->TileEnter(IndexCollision, TILE_PLAYER_BUSSINES))
	{
		GS()->Chat(ClientID, "You can see list of store items in the votes!");
		pChr->m_Core.m_ProtectHooked = true;
		pChr->m_NoAllowDamage = true;
		GS()->ResetVotes(ClientID, MAINMENU);
		return true;
	}
	else if (pChr->GetHelper()->TileExit(IndexCollision, TILE_PLAYER_BUSSINES))
	{
		pChr->m_Core.m_ProtectHooked = true;
		pChr->m_NoAllowDamage = true;
		GS()->ResetVotes(ClientID, MAINMENU);
		return true;
	}

	return false;
}

bool StorageSql::OnParseVotingMenu(CPlayer* pPlayer, const char* CMD, const int VoteID, const int VoteID2, int Get, const char* GetText)
{
	const int ClientID = pPlayer->GetCID();

	// разгрузить со склада товары
	if (PPSTR(CMD, "UNLOADSTORAGE") == 0)
	{
		int Count = pPlayer->GetItem(itGoods).Count;
		if (Count <= 0)
		{
			GS()->Chat(ClientID, "You have no cargo to unload.");
			return true;
		}

		if (pPlayer->CheckFailMoney(Count, itGoods))
			return true;

		AddStorageGoods(VoteID, Count);
		pPlayer->GetItem(itMaterial).Add(Count);
		GS()->Chat(ClientID, "You sell Goodsx{INT}.", &Count);
		GS()->ChatDiscord(false, DC_SERVER_INFO, "Server information", "**{STR} was delivery {INT} goods.**", Storage[VoteID].Name, &Count);
		GS()->VResetVotes(ClientID, MAINMENU);
		return true;
	}

	// загрузить со склада товары
	if (PPSTR(CMD, "LOADSTORAGE") == 0)
	{
		const int PlayerCountItem = pPlayer->GetItem(itGoods).Count;
		const int StorageCount = Storage[VoteID].Count;
		if (PlayerCountItem >= 500)
		{
			GS()->Chat(ClientID, "You reach maximal goods in a backpack.");
			return true;
		}

		int Count = 500 - PlayerCountItem;
		if (StorageCount < Count) 
			Count = StorageCount;

		if (Count <= 0)
		{
			GS()->Chat(ClientID, "Storage is empty.");
			return true;
		}

		if (RemoveStorageGoods(VoteID, Count))
		{
			pPlayer->GetItem(itGoods).Add(Count);
			GS()->Chat(ClientID, "You load in backpack Goodsx{INT}.", &Count);
			GS()->VResetVotes(ClientID, MAINMENU);
		}
		return true;
	}

	// купить склад
	if (PPSTR(CMD, "BUYSTORAGE") == 0)
	{
		BuyStorages(pPlayer, VoteID);
		GS()->ResetVotes(ClientID, MAINMENU);
		return true;
	}

	// продать склад
	if (PPSTR(CMD, "SELLSTORAGE") == 0)
	{
		if (Get != 7)
		{
			GS()->Chat(ClientID, "For sell storage write on reason '7'.");
			return true;
		}

		SellStorage(pPlayer->Acc().AuthID);
		GS()->ResetVotes(ClientID, MAINMENU);
		return true;
	}

	// снять со склада золото
	if (PPSTR(CMD, "REMMSTORAGE") == 0)
	{
		const int StorageID = VoteID;
		if (Storage[StorageID].Bank < Get)
			Get = Storage[StorageID].Bank;

		if (Get < 100)
		{
			GS()->Chat(ClientID, "You dont remove less than 100.");
			return true;
		}

		// снимаем со клада деньги
		if (RemoveStorageMoney(StorageID, Get))
		{
			pPlayer->AddMoney(Get);
			GS()->Chat(ClientID, "You remove on your business {INT}.", &Get);
			GS()->ResetVotes(ClientID, MAINMENU);
		}
		return true;
	}

	// добавить в склад золото
	if (PPSTR(CMD, "ADDMSTORAGE") == 0)
	{
		const int StorageID = VoteID;
		if (Get < 100)
		{
			GS()->Chat(ClientID, "You dont add less than 100.");
			return true;
		}

		// снимаем золото
		if (pPlayer->CheckFailMoney(Get))
			return true;

		// добавляем на склад
		AddStorageMoney(StorageID, Get);
		GS()->Chat(ClientID, "You added on your business {INT}.", &Get);
		GS()->ResetVotes(ClientID, MAINMENU);
		return true;
	}

	// починка предметов
	if (PPSTR(CMD, "REPAIRITEMS") == 0)
	{
		Job()->Item()->RepairDurabilityFull(pPlayer);
		GS()->Chat(ClientID, "You repaired all your items.");
		return true;
	}
	return false;
}

void StorageSql::ShowStorageMenu(int ClientID, int StorageID)
{
	CPlayer *pPlayer = GS()->GetPlayer(ClientID);
	if(!pPlayer) return;

	if(StorageID < 0) 
		return GS()->AV(ClientID, "null", "Storage Don't work");

	const bool LoadStorage = (Storage[StorageID].MonsterSubType > 0);
	GS()->AVH(ClientID, HSTORAGEUSE, GOLDEN_COLOR, "Bussines [{STR}/{INT}]", Storage[StorageID].Name, &Storage[StorageID].Count);
	if(Storage[StorageID].OwnerID > 0)
		GS()->AVM(ClientID, "null", NOPE, HSTORAGEUSE, "{STR} Owner: {STR}", Storage[StorageID].Name, Job()->PlayerName(Storage[StorageID].OwnerID));	
	else if(!LoadStorage)
	{
		int Price = Storage[StorageID].Price + g_Config.m_SvPaymentBussines;
		GS()->AVM(ClientID, "BUYSTORAGE", StorageID, HSTORAGEUSE, "Buy this business. Price {INT}", &Price);	
	}

	if(Storage[StorageID].OwnerID == pPlayer->Acc().AuthID)
	{
		GS()->AVM(ClientID, "null", NOPE, HSTORAGEUSE, "Every day -{INT} from the bank, or sell", &g_Config.m_SvPaymentBussines);	
		GS()->AVM(ClientID, "null", NOPE, HSTORAGEUSE, "Your bank: {INT}", &Storage[StorageID].Bank);
		GS()->AVM(ClientID, "null", NOPE, HSTORAGEUSE, "- - - - - - - - - - - - - - - - - - -");	
		GS()->AVM(ClientID, "ADDMSTORAGE", StorageID, HSTORAGEUSE, "Add money in business");		
		GS()->AVM(ClientID, "REMMSTORAGE", StorageID, HSTORAGEUSE, "Remove money in business");
		GS()->AVM(ClientID, "SELLSTORAGE", StorageID, HSTORAGEUSE, "Sell storage in reason (7). Back half the cost.");		
	}

	GS()->AVM(ClientID, "REPAIRITEMS", StorageID, HSTORAGEUSE, "Repair all items - FREE");
	if(LoadStorage) 
		GS()->AVM(ClientID, "LOADSTORAGE", StorageID, HSTORAGEUSE, "Load goods");
	else 
		GS()->AVM(ClientID, "UNLOADSTORAGE", StorageID, HSTORAGEUSE, "Unload all goods");
}

bool StorageSql::BuyStorageItem(int ClientID, int StorageID, int Price)
{
	if (Storage.find(StorageID) == Storage.end())
		return false;

	int StoragePriceGoods = (int)(Price / g_Config.m_SvStorageFraction);
	if (!RemoveStorageGoods(StorageID, StoragePriceGoods))
	{
		GS()->Chat(ClientID, "This purchase will require {INT} goods in warehouse.", &StoragePriceGoods);
		return false;
	}

	AddStorageMoney(StorageID, Price);
	int StorageBank = Storage[StorageID].Bank;
	Job()->Storage()->SCO(StorageID, "{STR} something bought, your business Bank {INT}", GS()->Server()->ClientName(ClientID), &StorageBank);
	return true;
}

void StorageSql::AddStorageGoods(int StorageID, int Count)
{
	boost::scoped_ptr<ResultSet> RES(SJK.SD("Count", "tw_storages", "WHERE ID = '%d'", StorageID));
	if(RES->next()) return;
	
	const int ChangesValue = (int)RES->getInt("Count")+Count;
	Storage[StorageID].Count = ChangesValue;
	SJK.UD("tw_storages", "Count = '%d' WHERE ID = '%d'", ChangesValue, StorageID);	
}

bool StorageSql::RemoveStorageGoods(int StorageID, int Count)
{
	boost::scoped_ptr<ResultSet> RES(SJK.SD("Count", "tw_storages", "WHERE ID = '%d'", StorageID));
	if(!RES->next()) return false;

	const int StorageCount = (int)RES->getInt("Count");
	Storage[StorageID].Count = StorageCount;
	if (Storage[StorageID].Count < Count)
		return false;

	Storage[StorageID].Count -= Count;
	SJK.UD("tw_storages", "Count = '%d' WHERE ID = '%d'", Storage[StorageID].Count, StorageID);
	return true;
}

void StorageSql::AddStorageMoney(int StorageID, int Count)
{
	boost::scoped_ptr<ResultSet> RES(SJK.SD("Bank", "tw_storages", "WHERE OwnerID > '0' AND ID = '%d'", StorageID));
	if(!RES->next()) return;
	
	const int ChangesValue = (int)RES->getInt("Bank")+Count;
	Storage[StorageID].Bank = ChangesValue;
	SJK.UD("tw_storages", "Bank = '%d' WHERE ID = '%d'", ChangesValue, StorageID);
}

bool StorageSql::RemoveStorageMoney(int StorageID, int Count)
{
	boost::scoped_ptr<ResultSet> RES(SJK.SD("Bank", "tw_storages", "WHERE OwnerID > '0' AND ID = '%d'", StorageID));
	if(!RES->next()) return false;
	
	const int StorageBank = (int)RES->getInt("Bank");
	Storage[StorageID].Bank = StorageBank;
	if (Storage[StorageID].Bank < Count)
		return false;

	Storage[StorageID].Bank -= Count;
	SJK.UD("tw_storages", "Bank = '%d' WHERE ID = '%d'", Storage[StorageID].Bank, StorageID);
	return true;
}

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

	// чат информация о покупки
	// обновление информации и отправка в бд
	Storage[StorageID].Bank = g_Config.m_SvPaymentBussines;
	Storage[StorageID].OwnerID = pPlayer->Acc().AuthID;
	SJK.UD("tw_storages", "OwnerID = '%d', Bank = '%d' WHERE ID = '%d'", pPlayer->Acc().AuthID, g_Config.m_SvPaymentBussines, StorageID);
	GS()->Chat(-1, "{STR} becomes the owner {STR}", GS()->Server()->ClientName(ClientID), Storage[StorageID].Name);
	GS()->ChatDiscord(false, DC_PLAYER_INFO, "Server information", "**{STR} becomes the owner {STR}**", GS()->Server()->ClientName(ClientID), Storage[StorageID].Name);
}

void StorageSql::SellStorage(int AuthID)
{
	boost::scoped_ptr<ResultSet> RES(SJK.SD("ID, Price", "tw_storages", "WHERE OwnerID = '%d'", AuthID));
	if(!RES->next()) return;
	
	const int Price = RES->getInt("Price");
	Job()->Inbox()->SendInbox(AuthID, "Your business sell.",  "Your money sell business", itMoney, Price, 0);

	const int StorageID = RES->getInt("ID");
	Storage[StorageID].OwnerID = -1;
	Storage[StorageID].Bank = g_Config.m_SvPaymentBussines;
	SJK.UD("tw_storages", "OwnerID = '0', Bank = '%d' WHERE ID = '%d'", g_Config.m_SvPaymentBussines, StorageID);
}

void StorageSql::SCO(int StorageID, const char* pText, ...)
{
	if(Storage.find(StorageID) == Storage.end()) return;

	const int OwnerID = Storage[StorageID].OwnerID;
	const int ClientID = Job()->Account()->CheckOnlineAccount(OwnerID);

	if(OwnerID > 0 && GS()->GetPlayer(ClientID, true))
	{
		dynamic_string Buffer;

		va_list VarArgs;
		va_start(VarArgs, pText);

		CNetMsg_Sv_Chat Msg;
		Msg.m_Mode = CHAT_ALL;
		Msg.m_ClientID = -1;
		Msg.m_TargetID = ClientID;

		Buffer.append("[Bussines]");
		GS()->Server()->Localization()->Format_VL(Buffer, GS()->m_apPlayers[ClientID]->GetLanguage(), pText, VarArgs);
		Msg.m_pMessage = Buffer.buffer();
		GS()->Server()->SendPackMsg(&Msg, MSGFLAG_VITAL, ClientID);
		
		Buffer.clear();
		va_end(VarArgs);
	}
}

int StorageSql::GetStorageMonsterSub(int MonsterSubType) const
{
	for (const auto& st : Storage)
	{
		if (st.second.MonsterSubType != MonsterSubType) continue;
		return st.first;
	}
	return -1;
}

int StorageSql::GetStorageID(vec2 Pos) const
{
	for (const auto& st : Storage)
	{
		vec2 PosStorage = vec2(st.second.LoadPosX, st.second.LoadPosY);
		if (distance(PosStorage, Pos) > 200) continue;
		return st.first;
	}
	return -1;
}