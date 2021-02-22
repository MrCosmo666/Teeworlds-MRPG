/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#ifndef GAME_CLIENT_COMPONENTS_INVENTORY_H
#define GAME_CLIENT_COMPONENTS_INVENTORY_H

#include <map>

#include <game/client/component.h>
#include <game/client/ui.h>

class CInventory : public CComponent
{
	friend class CInventorySlot;
	friend class CInventoryPage;
	CInventorySlot* m_HoveredSlot;
	CInventorySlot* m_SelectionSlot;
	CInventorySlot* m_InteractiveSlot;
	std::map < int, CInventoryPage* > m_aInventoryPages;

	bool m_Active;
	vec2 m_PositionMouse;

	// rects
	CUIRect m_Screen;
	CUIRect m_InventoryBackground;

	// mouse events
	int m_MouseFlag;

	// control inventory
	CInventorySlot& FindSlot(int ItemID);

	int m_ActivePage;
	vec2 m_SlotInteractivePosition;

public:
	CInventory();
	bool IsActive() const { return m_Active; }

	void AddItem(int ItemID, int Count, const char* pName, const char* pDesc, const char* pIcon);

	// events
	virtual void OnRender();
	virtual void OnMessage(int MsgType, void* pRawMsg);
	virtual void OnInit();
	virtual bool OnCursorMove(float x, float y, int CursorType);
	virtual bool OnInput(IInput::CEvent Event);
	virtual void OnStateChange(int NewState, int OldState);
	virtual void OnConsoleInit();
	static void ConToggleInventoryMRPG(IConsole::IResult* pResult, void* pUser);

	// pages
	void ScrollInventoryPage(int Page, CUIRect* pHoveredRect = nullptr);

	// render
	void RenderSelectTab(CUIRect MainView);
	void RenderInventory();
	void RenderInventoryPage(CUIRect MainView);

	void RenderTextRoundRect(vec2 Position, float Margin, float FontSize, const char* pText, float Rounding, bool *pHovored = nullptr);


};

#endif
