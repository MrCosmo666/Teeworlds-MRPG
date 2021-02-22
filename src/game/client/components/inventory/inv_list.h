/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#ifndef GAME_CLIENT_COMPONENTS_INVENTORY_LIST_H
#define GAME_CLIENT_COMPONENTS_INVENTORY_LIST_H

#include <map>
#include <game/client/ui.h>

class CInventoryList
{
	// basic
	float m_WindowSkipX;
	float m_WindowSkipY;
	bool m_ActiveWindow;
	class CInventory* m_pInventory;
	bool m_Openned;
	CUIRect m_MainView;

	// slots
	class CInventorySlot* m_HoveredSlot;
	class CInventorySlot* m_SelectionSlot;
	class CInventorySlot* m_InteractiveSlot;

	// pages
	int m_ActivePage;
	int m_MaxSlotsWidth;
	int m_MaxSlotsHeight;
	std::map < int, class CInventoryPage* > m_aInventoryPages;

	// basic
	void WindowRender();

public:
	CInventoryList(CInventory* pInventory, CUIRect& pMainView, int MaxSlotsWidth, int MaxSlotsHeight);
	~CInventoryList();

	vec2 m_SlotInteractivePosition;

	CUIRect GetMainViewRect() const { return m_MainView; }
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
	void Open() { m_Openned = true; }
	void Close() { m_Openned = false; }
	bool IsOpenned() const { return m_Openned; }
	void Render();

	// slots
	void AddItem(int ItemID, int Count, const char* pName, const char* pDesc, const char* pIcon);

	// pages
	void RenderSelectionPage();
	void ScrollInventoryPage(int Page);

	// render tools
	void RenderTextRoundRect(vec2 Position, float Margin, float FontSize, const char* pText, float Rounding, bool* pHovored = nullptr);
};

#endif
