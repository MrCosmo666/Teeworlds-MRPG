/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#ifndef GAME_SERVER_SQLSTORAGE_H
#define GAME_SERVER_SQLSTORAGE_H

#include "../component.h"

class StorageSql : public CMmoComponent
{
/* #########################################################################
	VAR STORAGE CLASS 
######################################################################### */
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

/* #########################################################################
	GLOBAL STORAGE CLASS 
######################################################################### */
public:
	virtual void OnInitGlobal();
	virtual void OnTick();
	virtual void OnPaymentTime();
	
	void ShowStorageMenu(int ClientID, int StorageID);

/* #########################################################################
	HELPER STORAGE CLASS 
######################################################################### */
	const char *StorageName(int StorageID) const;
	int GetCountStorage(int StorageID) const;
	int GetBankStorage(int StorageID) const;
	int GetStorageMonsterSub(int MonsterSubType) const;
	int GetLoadStorage(vec2 Pos) const;
	int ValidGoodsPrice(int Price) const;

/* #########################################################################
	FUNCTION STORAGE CLASS 
######################################################################### */
	bool BuyStorageItem(bool CheckStorageCount, int BoughtCID, int StorageID, int Price);
	void AddStorage(int StorageID, int Count);
	void RemoveStorage(int StorageID, int Count);
	void AddStorageMoney(int StorageID, int Count);
	void RemoveStorageMoney(int StorageID, int Count);

	void BuyStorages(CPlayer *pPlayer, int StorageID);
	void SellStorage(int AuthID);
/* #########################################################################
	ANOTHER STORAGE CLASS 
######################################################################### */
	void SCO(int StorageID, const char* pText, ...);
	virtual bool OnParseVotingMenu(CPlayer *pPlayer, const char *CMD, const int VoteID, const int VoteID2, int Get, const char *GetText);
};

#endif