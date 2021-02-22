/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#include <base/vmath.h>
#include <engine/keys.h>
#include <engine/storage.h>
#include <engine/textrender.h>

#include <engine/shared/config.h>
#include <generated/client_data.h>

#include <game/client/render.h>

#include "menus.h"
#include "console.h"
#include "inventory.h"

// TODO: ever complete the inventory

// basic set of variables for configuration
static float BoxWidth = 32.0f;
static float BoxHeight = 32.0f;
static int SpacingSlot = 3.0f;

enum MouseEvent
{
	M_LEFT_CLICKED = 1,
	M_LEFT_RELEASE = 2,
	M_LEFT_PRESSED = 4,
	M_RIGHT_CLICKED = 8,
	M_RIGHT_RELEASE = 16,
	M_RIGHT_PRESSED = 32,
};

static void CalculateSlotPosition(int SlotID, CUIRect *SlotRect)
{
	float OldPositionX = SlotRect->x;
	for(int CheckingSlot = 1; CheckingSlot <= SlotID; CheckingSlot++)
	{
		if(CheckingSlot % MAX_SLOTS_WIDTH != 0)
		{
			SlotRect->x += (BoxWidth + SpacingSlot);
			continue;
		}
		SlotRect->x = OldPositionX;
		SlotRect->y += (BoxHeight + SpacingSlot);
	}
	SlotRect->w = BoxWidth;
	SlotRect->h = BoxHeight;
}

CInventory::CInventory()
{
	m_aInventoryPages[0] = new CInventoryPage(this, 0);
	m_InteractiveSlot = nullptr;
	m_SelectionSlot = nullptr;
	m_HoveredSlot = nullptr;

	m_MouseFlag = 0;
	m_ActivePage = 0;
}

CInventory::InventorySlot &CInventory::FindSlot(int ItemID)
{
	for(int i = 0; i < m_aInventoryPages.size(); i++)
	{
		CInventoryPage* pInventoryPage = m_aInventoryPages[i];
		if(pInventoryPage)
		{
			for(int p = 0; p < MAX_ITEMS_PER_PAGE; p++)
			{
				if(pInventoryPage->m_Slot[p]->IsEmptySlot())
				{
					pInventoryPage->m_Slot[p]->m_Page = i;
					pInventoryPage->m_Slot[p]->m_SlotID = p;
					return *pInventoryPage->m_Slot[p];
				}
			}
		}
	}

	const int NewPage = (int)m_aInventoryPages.size();
	m_aInventoryPages[NewPage] = new CInventoryPage(this, NewPage);
	m_aInventoryPages[NewPage]->m_Slot[0]->m_Page = NewPage;
	m_aInventoryPages[NewPage]->m_Slot[0]->m_SlotID = 0;
	return *m_aInventoryPages[NewPage]->m_Slot[0];
}

void CInventory::AddItem(int ItemID, int Count, const char *pName, const char *pDesc, const char *pIcon)
{
	CUIRect SlotRect = { m_Screen.w / 1.25f, m_Screen.h / 4.0f, BoxWidth, BoxHeight };

	InventorySlot& NewSlot = FindSlot(ItemID);
	CalculateSlotPosition(NewSlot.m_SlotID, &SlotRect);
	NewSlot.m_ItemID = ItemID;
	NewSlot.m_Count = Count;
	NewSlot.m_RectSlot = SlotRect;
	str_copy(NewSlot.m_aName, pName, sizeof(NewSlot.m_aName));
	str_copy(NewSlot.m_aDesc, pDesc, sizeof(NewSlot.m_aDesc));
	str_copy(NewSlot.m_aIcon, pIcon, sizeof(NewSlot.m_aIcon));
}

void CInventory::OnRender()
{
	// skip menus
	if(m_pClient->m_pMenus->IsActive() || !m_Active)
		return;

	UI()->StartCheck();
	Graphics()->MapScreen(m_Screen.x, m_Screen.y, m_Screen.w, m_Screen.h);
	UI()->Update(m_PositionMouse.x, m_PositionMouse.y, m_PositionMouse.x * 3.0f, m_PositionMouse.y * 3.0f);

	// render inventory
	RenderInventory();

	// render cursor
	Graphics()->TextureSet(g_pData->m_aImages[IMAGE_CURSOR].m_Id);
	Graphics()->WrapClamp();
	Graphics()->QuadsBegin();
	IGraphics::CQuadItem QuadItem(m_PositionMouse.x, m_PositionMouse.y, 24, 24);
	Graphics()->QuadsDrawTL(&QuadItem, 1);
	Graphics()->QuadsEnd();
	Graphics()->WrapNormal();
	UI()->FinishCheck();

	// reset events
	int OldEvent = 0;
	if(m_MouseFlag & MouseEvent::M_LEFT_PRESSED)
		OldEvent |= MouseEvent::M_LEFT_PRESSED;
	if(m_MouseFlag & MouseEvent::M_RIGHT_PRESSED)
		OldEvent |= MouseEvent::M_RIGHT_PRESSED;
	m_MouseFlag = OldEvent;
	m_HoveredSlot = nullptr;
}

void CInventory::RenderSelectTab(CUIRect MainView)
{/*
	CUIRect TabRect = MainView;
	TabRect.w = 32.0f;
	TabRect.h = 32.0f;
	//TabRect.(&TabRect, 0, g_Config.m_JoystickSens);

	// hovered
	if(UI()->MouseHovered(&TabRect))
		RenderTools()->DrawRoundRect(&TabRect, vec4(0.5f, 0.5f, 0.5f, 0.5f), 8.0f);
	else
		RenderTools()->DrawRoundRect(&TabRect, vec4(0.2f, 0.2f, 0.2f, 0.4f), 8.0f);

	// icon and count near icon
	m_pClient->m_pMenus->DoItemIcon("module", TabRect, 32.0f);
*/
}

void CInventory::RenderInventory()
{
	if(m_pClient->m_pGameConsole->IsConsoleActive() || !IsActive())
		return;

	// background
	m_InventoryBackground = { m_Screen.w / 1.25f, m_Screen.h / 4.0f, m_Screen.w / 4.0f, m_Screen.h / 1.2f };
	RenderSelectTab(m_InventoryBackground);
	CUIRect Background = { m_InventoryBackground.x - 20.0f, m_InventoryBackground.y - 20.0f, BoxWidth * (MAX_SLOTS_WIDTH + 2), BoxHeight * (MAX_SLOTS_HEIGHT + 3) };
	RenderTools()->DrawRoundRect(&Background, vec4(0.4f, 0.4f, 0.4f, 0.7f), 16.0f);

	// render pages
	RenderInventoryPage(m_InventoryBackground);
	CInventoryPage *pInventoryActivePage = m_aInventoryPages[m_ActivePage];

	// empty grid
	for(int i = 0; i < MAX_ITEMS_PER_PAGE; i++)
	{
		CUIRect SlotRect = m_InventoryBackground;
		CalculateSlotPosition(i, &SlotRect);
		if(!m_InteractiveSlot && UI()->MouseHovered(&SlotRect))
			RenderTools()->DrawRoundRect(&SlotRect, vec4(0.5f, 0.5f, 0.5f, 0.5f), 8.0f);
		else
			RenderTools()->DrawRoundRect(&SlotRect, vec4(0.2f, 0.2f, 0.2f, 0.4f), 8.0f);

		// set positions to every slot on page
		pInventoryActivePage->m_Slot[i]->m_RectSlot = SlotRect;
	}

	// render inventory	
	for(int i = 0; i < MAX_ITEMS_PER_PAGE; i++)
	{
		if(m_SelectionSlot == pInventoryActivePage->m_Slot[i])
			continue;
		pInventoryActivePage->m_Slot[i]->Render();
	}

	// post render inventory
	for(int i = 0; i < MAX_ITEMS_PER_PAGE; i++)
	{
		if(m_SelectionSlot == pInventoryActivePage->m_Slot[i])
			continue;
		pInventoryActivePage->m_Slot[i]->PostRender();
	}

	dbg_msg("test", "%p", m_SelectionSlot);
	if(m_SelectionSlot)
	{
		// selected slot move to mouse
		if(m_MouseFlag & MouseEvent::M_LEFT_PRESSED)
		{
			m_SelectionSlot->m_RectSlot.x = m_PositionMouse.x;
			m_SelectionSlot->m_RectSlot.y = m_PositionMouse.y;
		}

		// render selected slot
		m_SelectionSlot->Render();
		m_SelectionSlot->PostRender();

		// swap event slots
		if(m_MouseFlag & MouseEvent::M_LEFT_RELEASE)
		{
			if(m_HoveredSlot)
				tl_swap(*m_SelectionSlot, *m_HoveredSlot);
			m_SelectionSlot = nullptr;
		}
	}
}

void CInventory::RenderInventoryPage(CUIRect MainView)
{
	bool HoveredLeft;
	bool HoveredRight;

	// left
	CUIRect SelectRect = MainView;
	SelectRect.HSplitTop(BoxHeight * MAX_SLOTS_HEIGHT + 30.0f, 0, &SelectRect);
	SelectRect.VMargin((BoxWidth * MAX_SLOTS_WIDTH) / 2.0f - 20.0f, &SelectRect);
	RenderTextRoundRect(vec2(SelectRect.x, SelectRect.y), 2.0f, 12.0f, "<", 6.0f, &HoveredLeft);
	if(m_MouseFlag & MouseEvent::M_LEFT_CLICKED && HoveredLeft && !m_InteractiveSlot)
		ScrollInventoryPage(m_ActivePage - 1);

	// information
	char aPageBuf[16];
	str_format(aPageBuf, sizeof(aPageBuf), "%d", (m_ActivePage + 1));
	SelectRect.VSplitLeft(25.0f, 0, &SelectRect);
	RenderTextRoundRect(vec2(SelectRect.x, SelectRect.y), 3.0f, 12.0f, aPageBuf, 6.0f);

	// right
	SelectRect.VSplitLeft(25.0f, 0, &SelectRect);
	RenderTextRoundRect(vec2(SelectRect.x, SelectRect.y), 2.0f, 12.0f, ">", 6.0f, &HoveredRight);
	if(m_MouseFlag & MouseEvent::M_LEFT_CLICKED && HoveredRight && !m_InteractiveSlot)
		ScrollInventoryPage(m_ActivePage + 1);
}

void CInventory::RenderTextRoundRect(vec2 Position, float Margin, float FontSize, const char *pText, float Rounding, bool* pHovored)
{
	const float TextWeidth = (float)TextRender()->TextWidth(nullptr, FontSize, pText, -1, -1.0f);
	CUIRect Rect;
	Rect.x = Position.x - (TextWeidth / 2.0f);
	Rect.y = Position.y;
	Rect.w = (TextWeidth + (Margin * 2.0f));
	Rect.h = (FontSize + Rounding);

	if(pHovored)
	{
		*pHovored = UI()->MouseHovered(&Rect);
		if(*pHovored)
			RenderTools()->DrawRoundRect(&Rect, vec4(0.2f, 0.2f, 0.2f, 0.5f), Rounding);
		else
			RenderTools()->DrawRoundRect(&Rect, vec4(0.0f, 0.0f, 0.0f, 0.5f), Rounding);
	}
	else
		RenderTools()->DrawRoundRect(&Rect, vec4(0.0f, 0.0f, 0.0f, 0.5f), Rounding);

	Rect.Margin(Margin, &Rect);
	TextRender()->TextOutlineColor(0.0f, 0.0f, 0.0f, 0.1f);
	TextRender()->Text(0x0, Rect.x, Rect.y, FontSize, pText, -1.0f);
	TextRender()->TextOutlineColor(CUI::ms_DefaultTextOutlineColor);

}

void CInventory::OnMessage(int MsgType, void* pRawMsg)
{
	// skip demo
	if(Client()->State() == IClient::STATE_DEMOPLAYBACK)
		return;

	//
}

void CInventory::OnInit()
{
	int ID = 0;
	AddItem(++ID, 1000, "Hello suka", "Info tipo blea", "ignot_r");
	AddItem(++ID, 1000, "Hello suka", "Info tipo blea", "ignot_b");
	AddItem(++ID, 1000, "Hello suka", "Info tipo blea", "ignot_g");
	AddItem(++ID, 1000, "Hello suka", "Info tipo blea", "ignot_r");
	AddItem(++ID, 1000, "Hello suka", "Info tipo blea", "ignot_b");
	AddItem(++ID, 1000, "Hello suka", "Info tipo blea", "ignot_g");
	AddItem(++ID, 1000, "Hello suka", "Info tipo blea", "ignot_r");
	AddItem(++ID, 1000, "Hello suka", "Info tipo blea", "ignot_b");
	AddItem(++ID, 1000, "Hello suka", "Info tipo blea", "ignot_g");
	AddItem(++ID, 1000, "Hello suka", "Info tipo blea", "ignot_r");
	AddItem(++ID, 1000, "Hello suka", "Info tipo blea", "ignot_b");
	AddItem(++ID, 1000, "Hello suka", "Info tipo blea", "ignot_g");
	AddItem(++ID, 1000, "Hello suka", "Info tipo blea", "ignot_r");
	AddItem(++ID, 1000, "Hello suka", "Info tipo blea", "ignot_b");
	AddItem(++ID, 1000, "Hello suka", "Info tipo blea", "ignot_g");
	AddItem(++ID, 1000, "Hello suka", "Info tipo blea", "ignot_r");
	AddItem(++ID, 1000, "Hello suka", "Info tipo blea", "ignot_b");
	AddItem(++ID, 1000, "Hello suka", "Info tipo blea", "ignot_g");
	AddItem(++ID, 1000, "Hello suka", "Info tipo blea", "ignot_r");
	AddItem(++ID, 1000, "Hello suka", "Info tipo blea", "ignot_b");
	AddItem(++ID, 1000, "Hello suka", "Info tipo blea", "ignot_g");
	AddItem(++ID, 1000, "Hello suka", "Info tipo blea", "ignot_r");
	AddItem(++ID, 1000, "Hello suka", "Info tipo blea", "ignot_b");
	AddItem(++ID, 1000, "Hello suka", "Info tipo blea", "ignot_g");
	AddItem(++ID, 1000, "Hello suka", "Info tipo blea", "ignot_r");
	AddItem(++ID, 1000, "Hello suka", "Info tipo blea", "ignot_b");
	AddItem(++ID, 1000, "Hello suka", "Info tipo blea", "ignot_g");
	AddItem(++ID, 1000, "Hello suka", "Info tipo blea", "ignot_r");
	AddItem(++ID, 1000, "Hello suka", "Info tipo blea", "ignot_b");
	AddItem(++ID, 1000, "Hello suka", "Info tipo blea", "ignot_g");
	AddItem(++ID, 1000, "Hello suka", "Info tipo blea", "ignot_r");
	AddItem(++ID, 1000, "Hello suka", "Info tipo blea", "ignot_b");
	AddItem(++ID, 1000, "Hello suka", "Info tipo blea", "ignot_g");
	AddItem(++ID, 1000, "Hello suka", "Info tipo blea", "ignot_r");
	AddItem(++ID, 1000, "Hello suka", "Info tipo blea", "ignot_b");
	AddItem(++ID, 1000, "Hello suka", "Info tipo blea", "ignot_g");
	AddItem(++ID, 1000, "Hello suka", "Info tipo blea", "ignot_r");
	AddItem(++ID, 1000, "Hello suka", "Info tipo blea", "ignot_b");
	AddItem(++ID, 1000, "Hello suka", "Info tipo blea", "ignot_g");
	AddItem(++ID, 1000, "Hello suka", "Info tipo blea", "ignot_r");
	AddItem(++ID, 1000, "Hello suka", "Info tipo blea", "ignot_b");
	AddItem(++ID, 1000, "Hello suka", "Info tipo blea", "ignot_g");
	AddItem(++ID, 1000, "Hello suka", "Info tipo blea", "ignot_r");
	AddItem(++ID, 1000, "Hello suka", "Info tipo blea", "ignot_b");
	AddItem(++ID, 1000, "Hello suka", "Info tipo blea", "ignot_g");

	m_Screen = *UI()->Screen();
	m_Active = true;
}

bool CInventory::OnInput(IInput::CEvent Event)
{
	if(m_pClient->m_pGameConsole->IsConsoleActive() || !IsActive())
		return false;

	if(Event.m_Key == KEY_MOUSE_1)
	{
		if(Event.m_Flags & IInput::FLAG_PRESS)
		{
			m_MouseFlag |= M_LEFT_PRESSED;
			m_MouseFlag |= M_LEFT_CLICKED;
		}
		else if(Event.m_Flags & IInput::FLAG_RELEASE)
		{
			m_MouseFlag |= M_LEFT_RELEASE;
			m_MouseFlag ^= M_LEFT_PRESSED;
		}
		return true;
	}
	else if(Event.m_Key == KEY_MOUSE_2)
	{
		if(Event.m_Flags & IInput::FLAG_PRESS)
		{
			m_MouseFlag |= M_RIGHT_PRESSED;
			m_MouseFlag |= M_RIGHT_CLICKED;
		}
		else if(Event.m_Flags & IInput::FLAG_RELEASE)
		{
			m_MouseFlag |= M_RIGHT_RELEASE;
			m_MouseFlag ^= M_RIGHT_PRESSED;
		}
		return true;
	}
	// scroll pages
	else if(Event.m_Key == KEY_MOUSE_WHEEL_UP)
	{
		ScrollInventoryPage(m_ActivePage + 1);
		return true;
	}
	else if(Event.m_Key == KEY_MOUSE_WHEEL_DOWN)
	{
		ScrollInventoryPage(m_ActivePage - 1);
		return true;
	}
	return false;
}

void CInventory::OnStateChange(int NewState, int OldState)
{
	if(NewState != IClient::STATE_ONLINE)
		m_Active = false;
}

void CInventory::OnConsoleInit()
{
	Console()->Register("toggle_inventory_mrpg", "", CFGFLAG_CLIENT, ConToggleInventoryMRPG, this, "Toggle inventory mrpg");
}

void CInventory::ConToggleInventoryMRPG(IConsole::IResult* pResult, void* pUser)
{
	CInventory* pInventory = static_cast<CInventory*>(pUser);
	if(pInventory->Client()->State() != IClient::STATE_ONLINE)
		return;

	pInventory->m_Active ^= true;
}

bool CInventory::OnCursorMove(float x, float y, int CursorType)
{
	if(!m_Active)
		return false;

	UI()->ConvertCursorMove(&x, &y, CursorType);
	m_PositionMouse += vec2(x, y);

	if(m_PositionMouse.x < 0) m_PositionMouse.x = 0;
	if(m_PositionMouse.y < 0) m_PositionMouse.y = 0;
	if(m_PositionMouse.x > m_Screen.w) m_PositionMouse.x = m_Screen.w;
	if(m_PositionMouse.y > m_Screen.h) m_PositionMouse.y = m_Screen.h;
	return true;
}

void CInventory::InventorySlot::Render()
{
	// hovered
	if(m_pInventory->m_SelectionSlot != this && !m_pInventory->m_InteractiveSlot)
	{
		if(m_pInventory->UI()->MouseHovered(&m_RectSlot))
		{
			m_pInventory->m_HoveredSlot = this;

			// select slot events mouse
			if(!IsEmptySlot())
			{
				if(m_pInventory->m_MouseFlag & MouseEvent::M_LEFT_CLICKED)
				{
					m_pInventory->m_SelectionSlot = this;
				}
				else if(m_pInventory->m_MouseFlag & MouseEvent::M_RIGHT_CLICKED)
				{
					m_pInventory->m_InteractiveSlot = this;
					m_pInventory->m_SlotInteractivePosition = m_pInventory->m_PositionMouse;
					m_InteractiveCount = min(m_InteractiveCount, m_Count);
				}
			}
		}
		else
		{
			m_pInventory->RenderTools()->DrawRoundRect(&m_RectSlot, vec4(0.2f, 0.2f, 0.2f, 0.4f), 8.0f);
		}
	}

	// icon and count near icon
	if(!IsEmptySlot())
	{
		char aCountBuf[32];
		str_format(aCountBuf, sizeof(aCountBuf), "%d%s %d", min(999, m_Count), (m_Count > 999 ? "+" : "\0"), m_Page);
		m_pInventory->m_pClient->m_pMenus->DoItemIcon(m_aIcon, m_RectSlot, 32.0f);
		m_pInventory->TextRender()->Text(0x0, m_RectSlot.x, m_RectSlot.y, 10.0f, aCountBuf, -1.0f);
	}
}

void CInventory::InventorySlot::PostRender()
{
	CInventoryPage *pInventoryActivePage = m_pInventory->m_aInventoryPages[m_pInventory->m_ActivePage];

	// - - - - - - - - - - - -
	// render slot interactive
	if(m_pInventory->m_InteractiveSlot == this)
	{
		static float Space = 20.0f;
		CUIRect InteractiveRect = m_pInventory->m_Screen;
		vec2 Position = vec2(m_pInventory->m_SlotInteractivePosition.x, m_pInventory->m_SlotInteractivePosition.y);
		InteractiveRect.w = 150.0f;
		InteractiveRect.h = 120.0f;
		InteractiveRect.x = m_pInventory->m_SlotInteractivePosition.x - (InteractiveRect.w / 2.0f);
		InteractiveRect.y = m_pInventory->m_SlotInteractivePosition.y;
		m_pInventory->RenderTools()->DrawRoundRect(&InteractiveRect, vec4(0.1f, 0.1f, 0.1f, 0.8f), 16.0f);

		char aBuf[256];
		str_format(aBuf, sizeof(aBuf), "Interaction with %s", m_aName);
		m_pInventory->RenderTextRoundRect(Position, 2.0f, 12.0f, aBuf, 5.0f);

		char aCountBuf[32];
		CUIRect ItemRect = InteractiveRect;
		ItemRect.HSplitTop(25.0f, 0, &ItemRect);
		ItemRect.VSplitLeft(10.0f, 0, &ItemRect);
		ItemRect.w = BoxWidth;
		ItemRect.h = BoxHeight;
		str_format(aCountBuf, sizeof(aCountBuf), "%d%s", min(999, m_Count), (m_Count > 999 ? "+" : "\0"));
		m_pInventory->RenderTools()->DrawRoundRect(&ItemRect, vec4(0.2f, 0.2f, 0.2f, 0.4f), 8.0f);
		m_pInventory->m_pClient->m_pMenus->DoItemIcon(m_aIcon, ItemRect, 32.0f);
		m_pInventory->TextRender()->Text(0x0, ItemRect.x, ItemRect.y, 10.0f, aCountBuf, -1.0f);

		// interactions
		bool HoveredUseItem;
		Position = vec2(Position.x, Position.y + Space);
		m_pInventory->RenderTextRoundRect(Position, 2.0f, 10.0f, "Use item", 6.0f, &HoveredUseItem);
		if(m_pInventory->m_MouseFlag & MouseEvent::M_LEFT_CLICKED && HoveredUseItem)
		{
			// todo use item
		}

		bool HoveredDropItem;
		Position = vec2(Position.x, Position.y + Space);
		m_pInventory->RenderTextRoundRect(Position, 2.0f, 10.0f, "Drop item", 6.0f, &HoveredDropItem);
		if(m_pInventory->m_MouseFlag & MouseEvent::M_LEFT_CLICKED && HoveredDropItem)
		{
			// todo drop item
		}

		bool HoveredMin;
		bool HoveredLeft;
		bool HoveredRight;
		bool HoveredMax;
		CUIRect SelectRect = InteractiveRect;
		SelectRect.y = Position.y;
		SelectRect.HSplitTop(25.0f, 0, &SelectRect);

		// left
		SelectRect.x = m_pInventory->m_SlotInteractivePosition.x;
		SelectRect.x -= 10.0f;
		m_pInventory->RenderTextRoundRect(vec2(SelectRect.x, SelectRect.y), 2.0f, 10.0f, "<", 4.0f, &HoveredLeft);
		if(m_pInventory->m_MouseFlag & MouseEvent::M_LEFT_CLICKED && HoveredLeft)
			m_InteractiveCount = max(1, m_InteractiveCount - 1);

		// min
		SelectRect.x -= 20.0f;
		m_pInventory->RenderTextRoundRect(vec2(SelectRect.x, SelectRect.y), 2.0f, 10.0f, "MIN", 4.0f, &HoveredMin);
		if(m_pInventory->m_MouseFlag & MouseEvent::M_LEFT_CLICKED && HoveredMin)
			m_InteractiveCount = 1;

		// right
		SelectRect.x = m_pInventory->m_SlotInteractivePosition.x;
		SelectRect.x += 10.0f;
		m_pInventory->RenderTextRoundRect(vec2(SelectRect.x, SelectRect.y), 2.0f, 10.0f, ">", 4.0f, &HoveredRight);
		if(m_pInventory->m_MouseFlag & MouseEvent::M_LEFT_CLICKED && HoveredRight)
			m_InteractiveCount = min(m_Count, m_InteractiveCount + 1);

		// max
		SelectRect.x += 20.0f;
		m_pInventory->RenderTextRoundRect(vec2(SelectRect.x, SelectRect.y), 2.0f, 10.0f, "MAX", 4.0f, &HoveredMax);
		if(m_pInventory->m_MouseFlag & MouseEvent::M_LEFT_CLICKED && HoveredMax)
			m_InteractiveCount = m_Count;

		// information
		char aNumberBuf[16];
		str_format(aNumberBuf, sizeof(aNumberBuf), "%d", m_InteractiveCount);
		SelectRect.HSplitTop(25.0f, 0, &SelectRect);
		SelectRect.x = m_pInventory->m_SlotInteractivePosition.x;
		m_pInventory->RenderTextRoundRect(vec2(SelectRect.x, SelectRect.y), 3.0f, 16.0f, aNumberBuf, 8.0f);

		if(m_pInventory->m_MouseFlag & (MouseEvent::M_LEFT_CLICKED | MouseEvent::M_RIGHT_CLICKED) && !m_pInventory->UI()->MouseHovered(&InteractiveRect))
			m_pInventory->m_InteractiveSlot = nullptr;
		return;
	}

	// render information hovered item
	if(((m_pInventory->m_HoveredSlot == this && !m_pInventory->m_SelectionSlot) || m_pInventory->m_SelectionSlot == this) && !IsEmptySlot())
	{
		const float FontDescSize = 10.0f;
		const float TextNameWidth = m_pInventory->TextRender()->TextWidth(0, FontDescSize, m_aName, -1, -1.0f);
		const float TextDescWidth = m_pInventory->TextRender()->TextWidth(0, FontDescSize, m_aDesc, -1, -1.0f);
		const float BackgroundWidth = 50.0f + TextNameWidth;
		const float BackgroundHeight = 25.0f + (float)ceil(TextDescWidth / BackgroundWidth) * FontDescSize;
		CUIRect Background = { m_pInventory->m_PositionMouse.x, m_pInventory->m_PositionMouse.y - BackgroundHeight, BackgroundWidth, BackgroundHeight };
		m_pInventory->RenderTools()->DrawRoundRect(&Background, vec4(0.0f, 0.0f, 0.0f, 0.4f), 8.0f);
		m_pInventory->TextRender()->Text(nullptr, Background.x, Background.y, 14.0f, m_aName, -1.0f);

		CUIRect Label;
		CTextCursor Cursor;
		Background.HSplitTop(15.0f, 0, &Label);
		m_pInventory->TextRender()->SetCursor(&Cursor, Label.x, Label.y, FontDescSize, TEXTFLAG_RENDER);
		Cursor.m_LineWidth = Background.w;
		Cursor.m_MaxLines = ceil(Background.h / FontDescSize);
		m_pInventory->TextRender()->TextEx(&Cursor, m_aDesc, -1);
	}
}

// inventory pages
void CInventory::ScrollInventoryPage(int Page, CUIRect* pHoveredRect)
{
	if(pHoveredRect && !UI()->MouseHovered(pHoveredRect) || m_InteractiveSlot != nullptr)
		return;

	int NewPage = clamp(Page, 0, (int)m_aInventoryPages.size() - 1);
	if(NewPage != m_ActivePage)
	{
		m_HoveredSlot = nullptr;
		m_ActivePage = NewPage;
	}
}