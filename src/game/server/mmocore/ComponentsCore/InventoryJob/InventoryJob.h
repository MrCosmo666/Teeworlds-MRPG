/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#ifndef GAME_SERVER_SQLITEM_H
#define GAME_SERVER_SQLITEM_H

#include "ItemInformation.h"
#include "ItemInventory.h"

#include <game/server/mmocore/MmoComponent.h>

class InventoryJob : public MmoComponent
{
	int SecureCheck(CPlayer *pPlayer, int ItemID, int Count, int Settings, int Enchant);
	int DeSecureCheck(CPlayer *pPlayer, int ItemID, int Count, int Settings);

public:
	static std::map < int, ItemInformation > ms_aItemsInfo;
	static std::map < int, std::map < int, InventoryItem > > ms_aItems;

	virtual void OnInit();
	virtual void OnInitAccount(CPlayer *pPlayer);
	virtual void OnResetClient(int ClientID);
	virtual bool OnVotingMenu(CPlayer* pPlayer, const char* CMD, const int VoteID, const int VoteID2, int Get, const char* GetText);
	virtual bool OnHandleMenulist(CPlayer* pPlayer, int Menulist, bool ReplaceMenu);

	// primary
	void ListInventory(CPlayer *pPlayer, int TypeList, bool SortedFunction = false);
	int GiveItem(CPlayer *pPlayer, int ItemID, int Count, int Settings, int Enchant);
	int RemoveItem(CPlayer *pPlayer, int ItemID, int Count, int Settings);
	void ItemSelected(CPlayer *pPlayer, const InventoryItem& pItemPlayer, bool Dress = false);
	int ActionItemCountAllowed(CPlayer* pPlayer, int ItemID);

	void RepairDurabilityFull(CPlayer *pPlayer);
	int GetCountItemsType(CPlayer* pPlayer, int Type) const;

	// TODO: FIX IT (lock .. unlock)
	void AddItemSleep(int AccountID, int ItemID, int GiveCount, int Milliseconds);
};

#endif