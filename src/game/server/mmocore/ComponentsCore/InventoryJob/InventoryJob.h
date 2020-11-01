/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#ifndef GAME_SERVER_SQLITEM_H
#define GAME_SERVER_SQLITEM_H

#include <game/server/mmocore/MmoComponent.h>

class InventoryJob : public MmoComponent
{
	void OnInit() override;
	void OnInitAccount(CPlayer* pPlayer) override;
	void OnResetClient(int ClientID) override;
	bool OnHandleVoteCommands(CPlayer* pPlayer, const char* CMD, const int VoteID, const int VoteID2, int Get, const char* GetText) override;
	bool OnHandleMenulist(CPlayer* pPlayer, int Menulist, bool ReplaceMenu) override;

	int SecureCheck(CPlayer *pPlayer, int ItemID, int Count, int Settings, int Enchant);
	int DeSecureCheck(CPlayer *pPlayer, int ItemID, int Count, int Settings);

public:
	static std::map < int, ItemInformation > ms_aItemsInfo;
	static std::map < int, std::map < int, InventoryItem > > ms_aItems;

	// primary
	void ListInventory(CPlayer *pPlayer, int TypeList, bool SortedFunction = false);
	void ItemSelected(CPlayer* pPlayer, const InventoryItem& pItemPlayer, bool Dress = false);
	int GetUnfrozenItemCount(CPlayer* pPlayer, int ItemID);

	void RepairDurabilityItems(CPlayer *pPlayer);
	int GetCountItemsType(CPlayer* pPlayer, int Type) const;

	void AddItemSleep(int AccountID, int ItemID, int Count, int Milliseconds);
	int GiveItem(CPlayer* pPlayer, int ItemID, int Count, int Settings, int Enchant);
	int RemoveItem(CPlayer* pPlayer, int ItemID, int Count, int Settings);
};

#endif