/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#include <engine/textrender.h>
#include <engine/shared/config.h>

#include <game/client/components/menus.h>
#include <game/client/components/inventory.h>

#include "inv_page.h"
#include "inv_slot.h"
#include "inv_list.h"

CInventoryList::CInventoryList(CInventory* pInventory, CUIRect& pMainView, int MaxSlotsWidth, int MaxSlotsHeight) : m_pInventory(pInventory)
{
	m_WindowSkipX = 0.0f;
	m_ActiveWindow = false;
	m_ActivePage = 0;
	m_HoveredSlot = nullptr;
	m_SelectionSlot = nullptr;
	m_InteractiveSlot = nullptr;
	m_MaxSlotsWidth = MaxSlotsWidth;
	m_MaxSlotsHeight = MaxSlotsHeight;
	m_MainView = pMainView;
	m_MainView.y = max(pMainView.y, 20.0f);
	m_MainView.w = ((BoxSize + SpacingSlot) * (MaxSlotsWidth + 1)) - (SpacingSlot * 2.0f);
	m_MainView.h = ((BoxSize + SpacingSlot) * (MaxSlotsHeight + 2)) - (SpacingSlot * 2.0f);

	// by default, the first page should exist
	m_aInventoryPages[0] = new CInventoryPage(m_pInventory, this, 0);

	// test
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
}

CInventoryList::~CInventoryList()
{
	for(int i = 0; i < m_aInventoryPages.size(); i++)
		delete m_aInventoryPages[i];
	m_aInventoryPages.clear();
}

void CInventoryList::WindowRender()
{
	// hovered window
	CUIRect Bordure = m_MainView;
	Bordure.h = 24.0f;
	if(m_pInventory->m_MouseFlag & MouseEvent::M_LEFT_CLICKED && m_pInventory->UI()->MouseInside(&Bordure))
	{
		m_ActiveWindow = true;
		m_WindowSkipX = (m_pInventory->m_PositionMouse.x - m_MainView.x);
		m_WindowSkipY = (m_pInventory->m_PositionMouse.y - m_MainView.y);
	}
	else if(m_pInventory->m_MouseFlag & MouseEvent::M_LEFT_RELEASE)
	{
		m_ActiveWindow = false;
		m_WindowSkipX = 0.0f;
		m_WindowSkipY = 0.0f;
	}

	if(m_ActiveWindow)
	{
		float NewPositionX = m_pInventory->m_PositionMouse.x - m_WindowSkipX;
		float NewPositionY = m_pInventory->m_PositionMouse.y - m_WindowSkipY;
		NewPositionX = clamp(NewPositionX, 0.0f, m_pInventory->m_ScreenRestriction.w - m_MainView.w);
		NewPositionY = clamp(NewPositionY, 0.0f, m_pInventory->m_ScreenRestriction.h - m_MainView.h);

		m_MainView.x = NewPositionX;
		m_MainView.y = NewPositionY;
		Bordure.x = NewPositionX;
		Bordure.y = NewPositionY;
		m_pInventory->m_PositionMouse = vec2(m_MainView.x + m_WindowSkipX, m_MainView.y + m_WindowSkipY);
	}

	// background
	m_pInventory->RenderTools()->DrawRoundRect(&m_MainView, vec4(0.4f, 0.4f, 0.4f, 0.85f), 16.0f);
	m_pInventory->RenderTools()->DrawUIRect(&Bordure, vec4(0.0f, 0.0f, 0.0f, 1.0f), CUI::CORNER_T, 12.0f);
}

void CInventoryList::Render()
{
	if(!m_Openned)
		return;

	WindowRender();

	// render pages
	CInventoryPage* pInventoryActivePage = m_aInventoryPages[m_ActivePage];
	if(!pInventoryActivePage)
		return;

	pInventoryActivePage->Render();
	RenderSelectionPage();

	// on hovered slot
	if(m_HoveredSlot)
		m_HoveredSlot->OnHoveredSlot();

	// on interactive slot
	if(m_InteractiveSlot)
		m_InteractiveSlot->OnInteractiveSlot();

	// on selected slot
	if(m_SelectionSlot)
		m_SelectionSlot->OnSelectedSlot();

	// reset hovered item
	SetHoveredSlot(nullptr);
}

void CInventoryList::RenderSelectionPage()
{
	bool HoveredLeft;
	bool HoveredRight;
	static float FontSize = 16.0f;
	static float SpaceOption = 32.0f;

	char aPageBuf[16];
	str_format(aPageBuf, sizeof(aPageBuf), "%d", (m_ActivePage + 1));
	float tw = m_pInventory->TextRender()->TextWidth(0, FontSize, aPageBuf, -1, -1.0f);

	// left
	CUIRect SelectionRect = m_MainView;
	SelectionRect.HSplitBottom((float)BoxSize, 0, &SelectionRect);
	SelectionRect.VMargin((float)(m_MainView.w / 2.0f) - ((tw / 2.0f) + SpaceOption), &SelectionRect);
	RenderTextRoundRect(vec2(SelectionRect.x, SelectionRect.y), 0.0f, FontSize, "<", 6.0f, &HoveredLeft);
	if((m_pInventory->m_MouseFlag & MouseEvent::M_WHEEL_DOWN && m_pInventory->UI()->MouseInside(&m_MainView))
			|| (m_pInventory->m_MouseFlag & MouseEvent::M_LEFT_CLICKED && HoveredLeft && !m_InteractiveSlot))
		ScrollInventoryPage(m_ActivePage - 1);

	// information
	SelectionRect.x += SpaceOption;
	RenderTextRoundRect(vec2(SelectionRect.x, SelectionRect.y), 1.0f, FontSize, aPageBuf, 6.0f);

	// right
	SelectionRect.x += SpaceOption;
	RenderTextRoundRect(vec2(SelectionRect.x, SelectionRect.y), 0.0f, FontSize, ">", 6.0f, &HoveredRight);
	if((m_pInventory->m_MouseFlag & MouseEvent::M_WHEEL_UP && m_pInventory->UI()->MouseInside(&m_MainView)) ||
		(m_pInventory->m_MouseFlag & MouseEvent::M_LEFT_CLICKED && HoveredRight && !m_InteractiveSlot))
		ScrollInventoryPage(m_ActivePage + 1);
}


void CInventoryList::AddItem(int ItemID, int Count, const char* pName, const char* pDesc, const char* pIcon)
{
	// TODO: rework and add clamping items
	CInventorySlot* pSlot = nullptr;
	for(int i = 0; i < m_aInventoryPages.size(); i++)
	{
		CInventoryPage* pInventoryPage = m_aInventoryPages[i];
		if(pInventoryPage)
		{
			for(int p = 0; p < GetPageMaxSlots(); p++)
			{
				if(!pInventoryPage->GetSlot(p)->IsEmptySlot())
					continue;

				pSlot = pInventoryPage->GetSlot(p);
				break;
			}
		}
	}

	if(!pSlot)
	{
		const int NewPage = (int)m_aInventoryPages.size();
		m_aInventoryPages[NewPage] = new CInventoryPage(m_pInventory, this, NewPage);
		pSlot = m_aInventoryPages[NewPage]->GetSlot(0);
	}

	pSlot->m_ItemID = ItemID;
	pSlot->m_Count = Count;
	str_copy(pSlot->m_aName, pName, sizeof(pSlot->m_aName));
	str_copy(pSlot->m_aDesc, pDesc, sizeof(pSlot->m_aDesc));
	str_copy(pSlot->m_aIcon, pIcon, sizeof(pSlot->m_aIcon));
}

void CInventoryList::RenderTextRoundRect(vec2 Position, float Margin, float FontSize, const char* pText, float Rounding, bool* pHovored /* = nullptr */)
{
	const float TextWeidth = (float)m_pInventory->TextRender()->TextWidth(nullptr, FontSize, pText, -1, -1.0f);
	CUIRect Rect;
	Rect.x = Position.x - (TextWeidth / 2.0f);
	Rect.y = Position.y;
	Rect.w = (TextWeidth + (Margin * 2.0f));
	Rect.h = (FontSize + Rounding);

	if(pHovored)
	{
		*pHovored = m_pInventory->UI()->MouseHovered(&Rect);
		if(*pHovored)
			m_pInventory->RenderTools()->DrawRoundRect(&Rect, vec4(0.2f, 0.2f, 0.2f, 0.4f), Rounding);
		else
			m_pInventory->RenderTools()->DrawRoundRect(&Rect, vec4(0.0f, 0.0f, 0.0f, 0.4f), Rounding);
	}
	else
		m_pInventory->RenderTools()->DrawRoundRect(&Rect, vec4(0.0f, 0.0f, 0.0f, 0.2f), Rounding);

	Rect.Margin(Margin, &Rect);
	m_pInventory->TextRender()->TextOutlineColor(0.0f, 0.0f, 0.0f, 0.1f);
	m_pInventory->TextRender()->Text(0x0, Rect.x, Rect.y, FontSize, pText, -1.0f);
	m_pInventory->TextRender()->TextOutlineColor(CUI::ms_DefaultTextOutlineColor);
}

void CInventoryList::ScrollInventoryPage(int Page)
{
	if(!m_pInventory->UI()->MouseHovered(&m_MainView) || m_InteractiveSlot != nullptr)
		return;

	int NewPage = clamp(Page, 0, (int)m_aInventoryPages.size() - 1);
	if(NewPage != m_ActivePage)
	{
		m_HoveredSlot = nullptr;
		m_ActivePage = NewPage;
	}
}