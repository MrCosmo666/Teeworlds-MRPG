/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#ifndef GAME_CLIENT_COMPONENTS_INVENTORY_PAGE_H
#define GAME_CLIENT_COMPONENTS_INVENTORY_PAGE_H

enum
{
	MAX_SLOTS_WIDTH = 5,
	MAX_SLOTS_HEIGHT = 8,
	MAX_ITEMS_PER_PAGE = MAX_SLOTS_WIDTH * MAX_SLOTS_HEIGHT
};

class CInventoryPage
{
	class CInventory* m_pInventory;
	class CInventorySlot* m_Slot[MAX_ITEMS_PER_PAGE];

public:
	CInventoryPage(CInventory* pInventory, int Page);

	CInventorySlot*GetSlot(int SlotID) const { return m_Slot[SlotID]; }
};

#endif
