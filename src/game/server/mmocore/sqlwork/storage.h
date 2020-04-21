/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#ifndef GAME_SERVER_SQLSTORAGE_H
#define GAME_SERVER_SQLSTORAGE_H

#include "../component.h"

class StorageSql : public CMmoComponent
{
	struct SturctStorage
	{
		char Name[32];
		int TextX;
		int TextY;
		int LoadPosX;
		int LoadPosY;
		int Count;
		int WorldID;
		int OwnerID;
		int Bank;
		int Price;
		int MonsterSubType;
	};
	typedef std::map < int , SturctStorage > StorageType;
	static StorageType Storage;

public:
	virtual void OnInitGlobal();
	virtual void OnTick();
	virtual void OnPaymentTime();
	virtual bool OnParseVotingMenu(CPlayer* pPlayer, const char* CMD, const int VoteID, const int VoteID2, int Get, const char* GetText);
	
	void ShowStorageMenu(int ClientID, int StorageID);
	bool BuyStorageItem(int ClientID, int StorageID, int Price);
	void AddStorageGoods(int StorageID, int Count);
	bool RemoveStorageGoods(int StorageID, int Count);
	void AddStorageMoney(int StorageID, int Count);
	bool RemoveStorageMoney(int StorageID, int Count);

	int GetStorageMonsterSub(int MonsterSubType) const;
	int GetStorageID(vec2 Pos) const;

private:
	void BuyStorages(CPlayer *pPlayer, int StorageID);
	void SellStorage(int AuthID);

public:
	void SCO(int StorageID, const char* pText, ...);
};

#endif