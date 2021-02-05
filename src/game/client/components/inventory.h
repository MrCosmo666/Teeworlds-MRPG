/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#ifndef GAME_CLIENT_COMPONENTS_INVENTORY_H
#define GAME_CLIENT_COMPONENTS_INVENTORY_H

#include <map>

#include <game/client/component.h>
#include <game/client/ui.h>

enum
{
	MAX_SLOTS_WIDTH = 5,
	MAX_SLOTS_HEIGHT = 8,
	MAX_ITEMS_PER_PAGE = MAX_SLOTS_WIDTH * MAX_SLOTS_HEIGHT
};

class CInventory : public CComponent
{
	class InventorySlot
	{
	public:
		InventorySlot()
		{
			m_SlotID = 0;
			m_ItemID = 0;
			m_Count = 0;
			m_aIcon[0] = '\0';
			m_RectSlot = CUIRect();
			m_InteractiveCount = 1;
		}
		char m_aName[64];
		char m_aDesc[256];
		char m_aIcon[64];

		int m_SlotID;
		int m_ItemID;
		int m_Count;
		int m_InteractiveCount;
		CUIRect m_RectSlot;

		// render
		void Render(CInventory* pInventory);
		void PostRender(CInventory* pInventory);

		// functions
		bool IsEmptySlot() const { return m_ItemID <= 0 || m_Count <= 0 || m_aIcon[0] == '\0'; }
		const char* GetHovoredDesc() const { return m_aDesc; }
	};

	struct CInventoryPage
	{
		InventorySlot m_Slot[MAX_ITEMS_PER_PAGE];
	};

	bool m_Active;
	CUIRect m_Screen;
	vec2 m_PositionMouse;

	// mouse events
	int m_MouseFlag;

	// control inventory
	int m_ActivePage;

	int m_HoveredSlotID;
	int m_SelectionSlotID;
	int m_InteractiveSlotID;

	vec2 m_SlotInteractivePosition;
	std::map < int , CInventoryPage > m_aInventoryPages;

public:
	CInventory();
	bool IsActive() const { return m_Active; }

	std::pair <int, int> FindSlotID(int ItemID);
	void AddItem(int ItemID, int Count, const char* pName, const char* pDesc, const char* pIcon);

	virtual void OnRender();
	virtual void OnMessage(int MsgType, void* pRawMsg);
	virtual void OnInit();
	virtual bool OnCursorMove(float x, float y, int CursorType);
	virtual bool OnInput(IInput::CEvent Event);

	void RenderSelectTab(CUIRect MainView);
	void RenderInventory();
	void RenderInventoryPage(CUIRect MainView);

	void RenderTextRoundRect(vec2 Position, float Margin, float FontSize, const char* pText, float Rounding, bool *pHovored = nullptr);


};

#endif
