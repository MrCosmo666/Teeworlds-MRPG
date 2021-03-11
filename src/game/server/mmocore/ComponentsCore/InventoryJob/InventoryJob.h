/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#ifndef GAME_SERVER_SQLITEM_H
#define GAME_SERVER_SQLITEM_H
#include <game/server/mmocore/MmoComponent.h>

#include <game/server/mmocore/ComponentsCore/InventoryJob/Item.h>
#include <map>

class InventoryJob : public MmoComponent
{
	~InventoryJob()
	{
		ms_aItemsInfo.clear();
		ms_aItems.clear();
	};

	void OnPrepareInformation(class IStorageEngine* pStorage, class CDataFileWriter* pDataFile) override;
	void OnInit() override;
	void OnInitAccount(class CPlayer* pPlayer) override;
	void OnResetClient(int ClientID) override;
	bool OnHandleVoteCommands(class CPlayer* pPlayer, const char* CMD, const int VoteID, const int VoteID2, int Get, const char* GetText) override;
	bool OnHandleMenulist(class CPlayer* pPlayer, int Menulist, bool ReplaceMenu) override;

	int SecureCheck(class CPlayer *pPlayer, int ItemID, int Count, int Settings, int Enchant);
	int DeSecureCheck(class CPlayer *pPlayer, int ItemID, int Count, int Settings);

public:
	static std::map < int, ItemInformation > ms_aItemsInfo;
	static std::map < int, std::map < int, InventoryItem > > ms_aItems;

	// primary
	void ListInventory(class CPlayer *pPlayer, int TypeList, bool SortedFunction = false);
	void ItemSelected(class CPlayer* pPlayer, const InventoryItem& pItemPlayer, bool Dress = false);
	int GetUnfrozenItemCount(class CPlayer* pPlayer, int ItemID) const;

	void RepairDurabilityItems(class CPlayer *pPlayer);
	int GetCountItemsType(class CPlayer* pPlayer, int Type) const;

	void AddItemSleep(int AccountID, int ItemID, int Count, int Milliseconds);
	int GiveItem(class CPlayer* pPlayer, int ItemID, int Count, int Settings, int Enchant);
	int RemoveItem(class CPlayer* pPlayer, int ItemID, int Count, int Settings);
};

#endif