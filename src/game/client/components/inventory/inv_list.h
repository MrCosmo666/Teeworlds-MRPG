/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#ifndef GAME_CLIENT_COMPONENTS_INVENTORY_LIST_H
#define GAME_CLIENT_COMPONENTS_INVENTORY_LIST_H

#include <game/client/ui.h>
#include <game/client/ui_window.h>

#include <map>

class CInventoryList
{
	// window
	CWindowUI* m_pWindowItemsList;

	// basic
	class CInventory* m_pInventory;
	CUIRect m_MainView;

	// slots
	class CInventorySlot* m_HoveredSlot;
	class CInventorySlot* m_SelectionSlot;
	class CInventorySlot* m_InteractiveSlot;

	// pages
	int m_ActivePage;
	int m_MaxSlotsWidth;
	int m_MaxSlotsHeight;
	std::map< int, class CInventoryPage* > m_aInventoryPages;

public:
	CInventoryList(CInventory* pInventory, const char* pInventoryListName, CUIRect& pMainView, int MaxSlotsWidth, int MaxSlotsHeight);
	~CInventoryList();

	vec2 m_SlotInteractivePosition;

	const CUIRect &GetMainViewRect() { return m_pWindowItemsList->GetRect(); }
	CInventorySlot* GetHoveredSlot() const { return m_HoveredSlot; }
	void SetHoveredSlot(CInventorySlot* pSlot) { m_HoveredSlot = pSlot; }
	CInventorySlot* GetSelectedSlot() const { return m_SelectionSlot; }
	void SetSelectedSlot(CInventorySlot* pSlot) { m_SelectionSlot = pSlot; }
	CInventorySlot* GetInteractiveSlot() const { return m_InteractiveSlot; }
	void SetInteractiveSlot(CInventorySlot* pSlot) { m_InteractiveSlot = pSlot; }
	int GetPageMaxSlots() const { return m_MaxSlotsWidth * m_MaxSlotsHeight; }
	int GetSlotsWidth() const { return m_MaxSlotsWidth; }
	int GetSlotsHeight() const { return m_MaxSlotsHeight; }

	// basic
	void Render();

	// slots
	void AddItem(int ItemID, int Count, const char* pName, const char* pDesc, const char* pIcon);

	// pages
	void RenderSelectionPage();
	void ScrollInventoryPage(int Page);

	// render tools
	void RenderTextRoundRect(CUIRect Rect, float Margin, float FontSize, const char* pText, float Rounding, bool* pHovored = nullptr);
};

#endif
