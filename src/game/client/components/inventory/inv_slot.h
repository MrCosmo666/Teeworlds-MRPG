/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#ifndef GAME_CLIENT_COMPONENTS_INVENTORY_SLOT_H
#define GAME_CLIENT_COMPONENTS_INVENTORY_SLOT_H

#include <game/client/ui.h>

enum MouseEvent
{
	M_LEFT_CLICKED = 1 << 0,
	M_LEFT_RELEASE = 1 << 1,
	M_LEFT_PRESSED = 1 << 2,
	M_RIGHT_CLICKED = 1 << 3,
	M_RIGHT_RELEASE = 1 << 4,
	M_RIGHT_PRESSED = 1 << 5,
	M_WHEEL_UP = 1 << 6,
	M_WHEEL_DOWN = 1 << 7,
};

// basic set of variables for configuration
#define INVSLOT_BOXSIZE 40.0f
#define INVSLOT_SPACING 3.0f

class CInventorySlot
{
	int m_Page;
	int m_SlotID;
	class CInventory* m_pInventory;
	class CInventoryList* m_pInventoryList;

public:
	CInventorySlot(CInventory* pInventory, CInventoryList* pInventoryList, int Page, int SlotID);
	char m_aName[64];
	char m_aDesc[256];
	char m_aIcon[64];

	int m_ItemID;
	int m_Count;
	int m_InteractiveCount;
	CUIRect m_RectSlot;

	void UpdateEvents();
	void Render();
	void OnSelectedSlot();
	void OnInteractiveSlot();
	void OnHoveredSlot();

	// functions
	bool IsEmptySlot() const { return m_ItemID <= 0 || m_Count <= 0 || m_aIcon[0] == '\0'; }
	const char* GetHovoredDesc() const { return m_aDesc; }
};

#endif
