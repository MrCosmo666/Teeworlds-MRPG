/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#ifndef GAME_CLIENT_COMPONENTS_INVENTORY_PAGE_H
#define GAME_CLIENT_COMPONENTS_INVENTORY_PAGE_H

class CInventoryPage
{
	class CInventory* m_pInventory;
	class CInventoryList* m_pInventoryList;
	array<CInventorySlot*> m_Slot;

public:
	CInventoryPage(CInventory* pInventory, CInventoryList* pInventoryList, int Page);
	~CInventoryPage();

	CInventorySlot *GetSlot(int SlotID) const { return m_Slot[SlotID]; }

	void Render();
	void CalculateSlotPosition(int SlotID, CUIRect* SlotRect);
};

#endif
