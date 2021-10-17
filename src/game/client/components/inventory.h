/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#ifndef GAME_CLIENT_COMPONENTS_INVENTORY_H
#define GAME_CLIENT_COMPONENTS_INVENTORY_H

#include <game/client/component.h>
#include <game/client/ui.h>

class CInventory : public CComponent
{
	friend class CInventoryList;
	friend class CInventoryPage;
	friend class CInventorySlot;

	bool m_Active;
	vec2 m_PositionMouse;

	// rects
	CUIRect m_ScreenRestriction;

	// mouse events
	int m_MouseFlag;

public:
	CInventory();

	bool IsActive() const { return m_Active; }

	// events
	virtual void OnRender();
	virtual void OnMessage(int MsgType, void* pRawMsg);
	virtual void OnInit();
	virtual bool OnCursorMove(float x, float y, int CursorType);
	virtual bool OnInput(IInput::CEvent Event);
	virtual void OnStateChange(int NewState, int OldState);

	// render
	void RenderSelectTab(CUIRect MainView);
	void RenderInventory();

	// inventory list
	class CInventoryList *m_pCraftItems;
	class CInventoryList *m_pOtherItems;
	class CInventoryList *m_pSomeItems;
};

#endif
