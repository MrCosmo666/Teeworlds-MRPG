/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#ifndef GAME_CLIENT_COMPONENTS_INVENTORY_SLOT_H
#define GAME_CLIENT_COMPONENTS_INVENTORY_SLOT_H

#include <game/client/ui.h>

enum MouseEvent
{
	M_LEFT_CLICKED = 1,
	M_LEFT_RELEASE = 2,
	M_LEFT_PRESSED = 4,
	M_RIGHT_CLICKED = 8,
	M_RIGHT_RELEASE = 16,
	M_RIGHT_PRESSED = 32,
};

// basic set of variables for configuration
static float BoxWidth = 32.0f;
static float BoxHeight = 32.0f;
static int SpacingSlot = 3.0f;

class CInventorySlot
{
	CInventory* m_pInventory;

public:
	CInventorySlot(CInventory* pInventory);
	char m_aName[64];
	char m_aDesc[256];
	char m_aIcon[64];

	int m_Page;
	int m_SlotID;
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
