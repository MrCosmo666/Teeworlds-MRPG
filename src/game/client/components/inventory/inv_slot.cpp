/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#include <engine/textrender.h>

#include <game/client/components/menus.h>
#include <game/client/components/inventory.h>

#include "inv_list.h"
#include "inv_slot.h"

CInventorySlot::CInventorySlot(CInventory* pInventory, CInventoryList* pInventoryList, int Page, int SlotID)
: m_Page(Page), m_SlotID(SlotID), m_pInventory(pInventory), m_pInventoryList(pInventoryList)
{
	m_ItemID = 0;
	m_Count = 0;
	m_aIcon[0] = '\0';
	m_RectSlot = CUIRect();
	m_InteractiveCount = 1;
}

void CInventorySlot::UpdateEvents()
{
	if(m_pInventoryList->GetInteractiveSlot())
		return;

	// update hovered and selection and interactive slots
	if(m_pInventory->UI()->MouseHovered(&m_RectSlot))
	{
		m_pInventoryList->SetHoveredSlot(this);

		if(!IsEmptySlot())
		{
			if(m_pInventory->m_MouseFlag & MouseEvent::M_LEFT_CLICKED)
			{
				m_pInventoryList->SetSelectedSlot(this);
			}
			else if(m_pInventory->m_MouseFlag & MouseEvent::M_RIGHT_CLICKED)
			{
				m_pInventoryList->SetInteractiveSlot(this);
				m_pInventoryList->m_SlotInteractivePosition = m_pInventory->m_PositionMouse;
				m_InteractiveCount = min(m_InteractiveCount, m_Count);
			}
		}
	}
}

void CInventorySlot::Render()
{
	// icon and count near icon
	if(!IsEmptySlot())
	{
		char aCountBuf[32];
		str_format(aCountBuf, sizeof(aCountBuf), "%d%s", min(999, m_Count), (m_Count > 999 ? "+" : "\0"));
		m_pInventory->m_pClient->m_pMenus->DoItemIcon(m_aIcon, m_RectSlot, INVSLOT_BOXSIZE);

		CTextCursor Cursor(10.0f, m_RectSlot.x, m_RectSlot.y);
		m_pInventory->TextRender()->TextDeferred(&Cursor, aCountBuf, -1.0f);
	}
}

void CInventorySlot::OnInteractiveSlot()
{
	if(m_pInventoryList->GetInteractiveSlot() != this)
		return;

	CUIRect BackgroundRect = m_pInventoryList->GetMainViewRect();
	BackgroundRect.w = 150.0f;
	BackgroundRect.h = 120.0f;
	BackgroundRect.x = m_pInventoryList->m_SlotInteractivePosition.x - (BackgroundRect.w / 2.0f);
	BackgroundRect.y = m_pInventoryList->m_SlotInteractivePosition.y;
	m_pInventory->RenderTools()->DrawRoundRect(&BackgroundRect, vec4(0.1f, 0.1f, 0.1f, 0.8f), 16.0f);

	/*
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
	*/
	if(m_pInventory->m_MouseFlag & (MouseEvent::M_LEFT_CLICKED | MouseEvent::M_RIGHT_CLICKED) && !m_pInventory->UI()->MouseHovered(&BackgroundRect))
		m_pInventoryList->SetInteractiveSlot(nullptr);
}

void CInventorySlot::OnSelectedSlot()
{
	if(m_pInventoryList->GetSelectedSlot() != this)
		return;

	// selected slot move to mouse
	if(m_pInventory->m_MouseFlag & MouseEvent::M_LEFT_PRESSED)
	{
		m_RectSlot.x = m_pInventory->m_PositionMouse.x;
		m_RectSlot.y = m_pInventory->m_PositionMouse.y;
	}

	// render slot
	Render();

	// swap event slots
	if(m_pInventory->m_MouseFlag & MouseEvent::M_LEFT_RELEASE)
	{
		if(m_pInventoryList->GetHoveredSlot())
			tl_swap(*m_pInventoryList->GetSelectedSlot(), *m_pInventoryList->GetHoveredSlot());
		m_pInventoryList->SetSelectedSlot(nullptr);
	}
}

void CInventorySlot::OnHoveredSlot()
{
	if(m_pInventoryList->GetHoveredSlot() != this)
		return;

	if(!IsEmptySlot())
	{
		const float FontDescSize = 10.0f;
		const float TextNameWidth = m_pInventory->TextRender()->TextWidth(FontDescSize, m_aName, -1);
		const float TextDescWidth = m_pInventory->TextRender()->TextWidth(FontDescSize, m_aDesc, -1);
		const float BackgroundWidth = 50.0f + TextNameWidth;
		const float BackgroundHeight = 25.0f + (float)ceil(TextDescWidth / BackgroundWidth) * FontDescSize;
		CUIRect Background = { m_pInventory->m_PositionMouse.x, m_pInventory->m_PositionMouse.y - BackgroundHeight, BackgroundWidth, BackgroundHeight };
		m_pInventory->RenderTools()->DrawRoundRect(&Background, vec4(0.0f, 0.0f, 0.0f, 0.4f), 8.0f);

		CTextCursor Cursor(14.0f, Background.x, Background.y);
		m_pInventory->TextRender()->TextDeferred(&Cursor, m_aName, -1.0f);

		CUIRect Label;
		Background.HSplitTop(15.0f, 0, &Label);

		Cursor.Reset();
		Cursor.MoveTo(Label.x, Label.y);
		Cursor.m_MaxWidth = Background.w;
		Cursor.m_MaxLines = ceil(Background.h / FontDescSize);
		m_pInventory->TextRender()->TextDeferred(&Cursor, m_aDesc, -1);
	}
}