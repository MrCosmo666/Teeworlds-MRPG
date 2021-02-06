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
	m_MouseFlag = 0;
	m_ActivePage = 0;
	m_HoveredSlotID = -1;
	m_SelectionSlotID = -1;
	m_InteractiveSlotID = -1;
}

std::pair <int, int> CInventory::FindSlotID(int ItemID)
{
	for(auto &pPages : m_aInventoryPages)
	{
		for(int p = 0; p < MAX_ITEMS_PER_PAGE; p++)
		{
			if(pPages.second.m_Slot[p].IsEmptySlot())
				return std::pair < int, int >(pPages.first, p);
		}
	}
	const int NewPage = (int)m_aInventoryPages.size();
	return std::pair < int, int > (NewPage, 0);
}

void CInventory::AddItem(int ItemID, int Count, const char *pName, const char *pDesc, const char *pIcon)
{
	std::pair<int, int> Position = FindSlotID(ItemID);
	const int FreeSlotID = Position.second;
	const int Page = Position.first;

	CUIRect SlotRect = { m_Screen.w / 1.25f, m_Screen.h / 4.0f, BoxWidth, BoxHeight };
	CalculateSlotPosition(FreeSlotID, &SlotRect);

	// add slot
	InventorySlot SlotItem;
	SlotItem.m_SlotID = FreeSlotID;
	SlotItem.m_ItemID = ItemID;
	SlotItem.m_Count = Count;
	SlotItem.m_RectSlot = SlotRect;
	str_copy(SlotItem.m_aName, pName, sizeof(SlotItem.m_aName));
	str_copy(SlotItem.m_aDesc, pDesc, sizeof(SlotItem.m_aDesc));
	str_copy(SlotItem.m_aIcon, pIcon, sizeof(SlotItem.m_aIcon));
	m_aInventoryPages[Page].m_Slot[FreeSlotID] = SlotItem;
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
	m_HoveredSlotID = -1;
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
	CUIRect MainView = { m_Screen.w / 1.25f, m_Screen.h / 4.0f, m_Screen.w / 4.0f, m_Screen.h / 1.2f };
	RenderSelectTab(MainView);
	CUIRect Background = { MainView.x - 20.0f, MainView.y - 20.0f, BoxWidth * (MAX_SLOTS_WIDTH + 2), BoxHeight * (MAX_SLOTS_HEIGHT + 3) };
	RenderTools()->DrawRoundRect(&Background, vec4(0.4f, 0.4f, 0.4f, 0.7f), 16.0f);

	// - - - - - -
	// render pages
	RenderInventoryPage(MainView);
	CInventoryPage& pInventoryActivePage = m_aInventoryPages[m_ActivePage];

	// - - - - - -
	// empty grid
	for(int i = 0; i < MAX_ITEMS_PER_PAGE; i++)
	{
		CUIRect SlotRect = MainView;
		CalculateSlotPosition(i, &SlotRect);
		pInventoryActivePage.m_Slot[i].m_SlotID = i;
		pInventoryActivePage.m_Slot[i].m_RectSlot = SlotRect;
		if(m_InteractiveSlotID == -1 && UI()->MouseHovered(&SlotRect))
			RenderTools()->DrawRoundRect(&SlotRect, vec4(0.5f, 0.5f, 0.5f, 0.5f), 8.0f);
		else
			RenderTools()->DrawRoundRect(&SlotRect, vec4(0.2f, 0.2f, 0.2f, 0.4f), 8.0f);
	}

	// - - - - - - - - 
	// render inventory
	for(int i = 0; i < MAX_ITEMS_PER_PAGE; i++)
	{	
		// skip selections slot and render it on post
		if(m_MouseFlag & MouseEvent::M_LEFT_PRESSED && m_SelectionSlotID == i)
		{
			pInventoryActivePage.m_Slot[i].m_RectSlot.x = m_PositionMouse.x;
			pInventoryActivePage.m_Slot[i].m_RectSlot.y = m_PositionMouse.y;
			continue;
		}
		pInventoryActivePage.m_Slot[i].Render(this);
	}

	// - - - - - - - - - - -
	// post render inventory
	for(int i = 0; i < MAX_ITEMS_PER_PAGE; i++)
	{
		pInventoryActivePage.m_Slot[i].PostRender(this);
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
	if(m_MouseFlag & MouseEvent::M_LEFT_CLICKED && HoveredLeft && m_InteractiveSlotID == -1)
	{
		m_HoveredSlotID = -1;
		m_SelectionSlotID = -1;
		m_ActivePage = max(0, m_ActivePage - 1);
	}

	// information
	char aPageBuf[16];
	str_format(aPageBuf, sizeof(aPageBuf), "%d", (m_ActivePage + 1));
	SelectRect.VSplitLeft(25.0f, 0, &SelectRect);
	RenderTextRoundRect(vec2(SelectRect.x, SelectRect.y), 3.0f, 12.0f, aPageBuf, 6.0f);

	// right
	SelectRect.VSplitLeft(25.0f, 0, &SelectRect);
	RenderTextRoundRect(vec2(SelectRect.x, SelectRect.y), 2.0f, 12.0f, ">", 6.0f, &HoveredRight);
	if(m_MouseFlag & MouseEvent::M_LEFT_CLICKED && HoveredRight && m_InteractiveSlotID == -1)
	{
		m_HoveredSlotID = -1;
		m_SelectionSlotID = -1;
		m_ActivePage = min((int)m_aInventoryPages.size(), m_ActivePage + 1);
	}
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
	m_Screen = *UI()->Screen();
	m_Active = false;
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
	return false;
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

void CInventory::InventorySlot::Render(CInventory* pInventory)
{
	// hovered
	if(pInventory->m_InteractiveSlotID == -1 && pInventory->UI()->MouseHovered(&m_RectSlot))
	{
		pInventory->m_HoveredSlotID = m_SlotID;
		if(!IsEmptySlot())
		{
			// select slot after left mouse
			if(pInventory->m_MouseFlag & MouseEvent::M_LEFT_CLICKED)
				pInventory->m_SelectionSlotID = m_SlotID;
			// open interactive menu
			else if(pInventory->m_MouseFlag & MouseEvent::M_RIGHT_CLICKED)
			{
				pInventory->m_InteractiveSlotID = m_SlotID;
				pInventory->m_SlotInteractivePosition = pInventory->m_PositionMouse;
				m_InteractiveCount = min(m_InteractiveCount, m_Count);
			}
		}
	}
	else
	{
		pInventory->RenderTools()->DrawRoundRect(&m_RectSlot, vec4(0.2f, 0.2f, 0.2f, 0.4f), 8.0f);
	}

	// icon and count near icon
	if(!IsEmptySlot())
	{
		char aCountBuf[32];
		str_format(aCountBuf, sizeof(aCountBuf), "%d%s", min(999, m_Count), (m_Count > 999 ? "+" : "\0"));
		pInventory->m_pClient->m_pMenus->DoItemIcon(m_aIcon, m_RectSlot, 32.0f);
		pInventory->TextRender()->Text(0x0, m_RectSlot.x, m_RectSlot.y, 10.0f, aCountBuf, -1.0f);
	}
}

void CInventory::InventorySlot::PostRender(CInventory* pInventory)
{
	CInventoryPage& pInventoryActivePage = pInventory->m_aInventoryPages[pInventory->m_ActivePage];

	// - - - - - - - - - - - -
	// render slot interactive
	if(pInventory->m_InteractiveSlotID == m_SlotID)
	{
		static float Space = 20.0f;
		CUIRect InteractiveRect = pInventory->m_Screen;
		vec2 Position = vec2(pInventory->m_SlotInteractivePosition.x, pInventory->m_SlotInteractivePosition.y);
		InteractiveRect.w = 150.0f;
		InteractiveRect.h = 120.0f;
		InteractiveRect.x = pInventory->m_SlotInteractivePosition.x - (InteractiveRect.w / 2.0f);
		InteractiveRect.y = pInventory->m_SlotInteractivePosition.y;
		pInventory->RenderTools()->DrawRoundRect(&InteractiveRect, vec4(0.1f, 0.1f, 0.1f, 0.8f), 16.0f);

		char aBuf[256];
		str_format(aBuf, sizeof(aBuf), "Interaction with %s", m_aName);
		pInventory->RenderTextRoundRect(Position, 2.0f, 12.0f, aBuf, 5.0f);

		char aCountBuf[32];
		CUIRect ItemRect = InteractiveRect;
		ItemRect.HSplitTop(25.0f, 0, &ItemRect);
		ItemRect.VSplitLeft(10.0f, 0, &ItemRect);
		ItemRect.w = BoxWidth;
		ItemRect.h = BoxHeight;
		str_format(aCountBuf, sizeof(aCountBuf), "%d%s", min(999, m_Count), (m_Count > 999 ? "+" : "\0"));
		pInventory->RenderTools()->DrawRoundRect(&ItemRect, vec4(0.2f, 0.2f, 0.2f, 0.4f), 8.0f);
		pInventory->m_pClient->m_pMenus->DoItemIcon(m_aIcon, ItemRect, 32.0f);
		pInventory->TextRender()->Text(0x0, ItemRect.x, ItemRect.y, 10.0f, aCountBuf, -1.0f);

		// interactions
		bool HoveredUseItem;
		Position = vec2(Position.x, Position.y + Space);
		pInventory->RenderTextRoundRect(Position, 2.0f, 10.0f, "Use item", 6.0f, &HoveredUseItem);
		if(pInventory->m_MouseFlag & MouseEvent::M_LEFT_CLICKED && HoveredUseItem)
		{
			// todo use item
		}

		bool HoveredDropItem;
		Position = vec2(Position.x, Position.y + Space);
		pInventory->RenderTextRoundRect(Position, 2.0f, 10.0f, "Drop item", 6.0f, &HoveredDropItem);
		if(pInventory->m_MouseFlag & MouseEvent::M_LEFT_CLICKED && HoveredDropItem)
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
		SelectRect.x = pInventory->m_SlotInteractivePosition.x;
		SelectRect.x -= 10.0f;
		pInventory->RenderTextRoundRect(vec2(SelectRect.x, SelectRect.y), 2.0f, 10.0f, "<", 4.0f, &HoveredLeft);
		if(pInventory->m_MouseFlag & MouseEvent::M_LEFT_CLICKED && HoveredLeft)
			m_InteractiveCount = max(1, m_InteractiveCount - 1);

		// min
		SelectRect.x -= 20.0f;
		pInventory->RenderTextRoundRect(vec2(SelectRect.x, SelectRect.y), 2.0f, 10.0f, "MIN", 4.0f, &HoveredMin);
		if(pInventory->m_MouseFlag & MouseEvent::M_LEFT_CLICKED && HoveredMin)
			m_InteractiveCount = 1;

		// right
		SelectRect.x = pInventory->m_SlotInteractivePosition.x;
		SelectRect.x += 10.0f;
		pInventory->RenderTextRoundRect(vec2(SelectRect.x, SelectRect.y), 2.0f, 10.0f, ">", 4.0f, &HoveredRight);
		if(pInventory->m_MouseFlag & MouseEvent::M_LEFT_CLICKED && HoveredRight)
			m_InteractiveCount = min(m_Count, m_InteractiveCount + 1);

		// max
		SelectRect.x += 20.0f;
		pInventory->RenderTextRoundRect(vec2(SelectRect.x, SelectRect.y), 2.0f, 10.0f, "MAX", 4.0f, &HoveredMax);
		if(pInventory->m_MouseFlag & MouseEvent::M_LEFT_CLICKED && HoveredMax)
			m_InteractiveCount = m_Count;

		// information
		char aNumberBuf[16];
		str_format(aNumberBuf, sizeof(aNumberBuf), "%d", m_InteractiveCount);
		SelectRect.HSplitTop(25.0f, 0, &SelectRect);
		SelectRect.x = pInventory->m_SlotInteractivePosition.x;
		pInventory->RenderTextRoundRect(vec2(SelectRect.x, SelectRect.y), 3.0f, 16.0f, aNumberBuf, 8.0f);

		if(pInventory->m_MouseFlag & (MouseEvent::M_LEFT_CLICKED | MouseEvent::M_RIGHT_CLICKED) && !pInventory->UI()->MouseHovered(&InteractiveRect))
			pInventory->m_InteractiveSlotID = -1;
		return;
	}

	// - - - - - - - -
	// swap event slots
	if(pInventory->m_SelectionSlotID == m_SlotID && pInventory->m_MouseFlag & M_LEFT_RELEASE)
	{
		if(pInventory->m_HoveredSlotID != -1)
		{
			tl_swap(pInventoryActivePage.m_Slot[pInventory->m_SelectionSlotID], pInventoryActivePage.m_Slot[pInventory->m_HoveredSlotID]);
		}
		pInventory->m_SelectionSlotID = -1;
		pInventory->m_HoveredSlotID = -1;
		return;
	}

	// - - - - - - - - - - - - - - - -
	// render information hovered item
	if(((pInventory->m_HoveredSlotID == m_SlotID && pInventory->m_SelectionSlotID == -1) || pInventory->m_SelectionSlotID == m_SlotID) && !IsEmptySlot())
	{
		const float FontDescSize = 10.0f;
		const float TextNameWidth = pInventory->TextRender()->TextWidth(0, FontDescSize, m_aName, -1, -1.0f);
		const float TextDescWidth = pInventory->TextRender()->TextWidth(0, FontDescSize, m_aDesc, -1, -1.0f);
		const float BackgroundWidth = 50.0f + TextNameWidth;
		const float BackgroundHeight = 25.0f + (float)ceil(TextDescWidth / BackgroundWidth) * FontDescSize;
		CUIRect Background = { pInventory->m_PositionMouse.x, pInventory->m_PositionMouse.y - BackgroundHeight, BackgroundWidth, BackgroundHeight };
		pInventory->RenderTools()->DrawRoundRect(&Background, vec4(0.0f, 0.0f, 0.0f, 0.4f), 8.0f);
		pInventory->TextRender()->Text(nullptr, Background.x, Background.y, 14.0f, m_aName, -1.0f);

		CUIRect Label;
		CTextCursor Cursor;
		Background.HSplitTop(15.0f, 0, &Label);
		pInventory->TextRender()->SetCursor(&Cursor, Label.x, Label.y, FontDescSize, TEXTFLAG_RENDER);
		Cursor.m_LineWidth = Background.w;
		Cursor.m_MaxLines = ceil(Background.h / FontDescSize);
		pInventory->TextRender()->TextEx(&Cursor, m_aDesc, -1);
	}

	// - - - - - - - - - - - -
	// render selection slot
	if(pInventory->m_SelectionSlotID == m_SlotID && !IsEmptySlot())
	{
		Render(pInventory);
	}
}
