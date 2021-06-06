/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#include <engine/textrender.h>

#include <game/client/components/menus.h>
#include <game/client/components/inventory.h>

#include "inv_page.h"
#include "inv_slot.h"
#include "inv_list.h"

CInventoryList::CInventoryList(CInventory* pInventory, const char *pInventoryListName, CUIRect& pMainView, int MaxSlotsWidth, int MaxSlotsHeight)
	: m_pInventory(pInventory)
{
	m_HoveredSlot = nullptr;
	m_SelectionSlot = nullptr;
	m_InteractiveSlot = nullptr;
	m_MaxSlotsWidth = MaxSlotsWidth;
	m_MaxSlotsHeight = MaxSlotsHeight;

	m_pWindowItemsList = CUI::CreateWindow(pInventoryListName, vec2(200, 200));

	// by default, the first page should exist
	m_ActivePage = 0;
	m_aInventoryPages[0] = new CInventoryPage(m_pInventory, this, 0);

	// test
	int ID = 0;
	AddItem(++ID, random_int()%1500, "Hello suka", "Info tipo blea", "ignot_r");
	AddItem(++ID, random_int() % 1500, "Hello suka", "Info tipo blea", "ignot_b");
	AddItem(++ID, random_int() % 1500, "Hello suka", "Info tipo blea", "ignot_g");
	AddItem(++ID, random_int()%1500, "Hello suka", "Info tipo blea", "ignot_r");
	AddItem(++ID, random_int() % 1500, "Hello suka", "Info tipo blea", "ignot_b");
	AddItem(++ID, random_int() % 1500, "Hello suka", "Info tipo blea", "ignot_g");
	AddItem(++ID, random_int()%1500, "Hello suka", "Info tipo blea", "ignot_r");
	AddItem(++ID, random_int() % 1500, "Hello suka", "Info tipo blea", "ignot_b");
	AddItem(++ID, random_int() % 1500, "Hello suka", "Info tipo blea", "ignot_g");
	AddItem(++ID, random_int()%1500, "Hello suka", "Info tipo blea", "ignot_r");
	AddItem(++ID, random_int() % 1500, "Hello suka", "Info tipo blea", "ignot_b");
	AddItem(++ID, random_int() % 1500, "Hello suka", "Info tipo blea", "ignot_g");
	AddItem(++ID, random_int()%1500, "Hello suka", "Info tipo blea", "ignot_r");
	AddItem(++ID, random_int() % 1500, "Hello suka", "Info tipo blea", "ignot_b");
	AddItem(++ID, random_int() % 1500, "Hello suka", "Info tipo blea", "ignot_g");
	AddItem(++ID, random_int()%1500, "Hello suka", "Info tipo blea", "ignot_r");
	AddItem(++ID, random_int() % 1500, "Hello suka", "Info tipo blea", "ignot_b");
	AddItem(++ID, random_int() % 1500, "Hello suka", "Info tipo blea", "ignot_g");
	AddItem(++ID, random_int()%1500, "Hello suka", "Info tipo blea", "ignot_r");
	AddItem(++ID, random_int() % 1500, "Hello suka", "Info tipo blea", "ignot_b");
	AddItem(++ID, random_int() % 1500, "Hello suka", "Info tipo blea", "ignot_g");
	AddItem(++ID, random_int()%1500, "Hello suka", "Info tipo blea", "ignot_r");
	AddItem(++ID, random_int() % 1500, "Hello suka", "Info tipo blea", "ignot_b");
	AddItem(++ID, random_int() % 1500, "Hello suka", "Info tipo blea", "ignot_g");
	AddItem(++ID, random_int()%1500, "Hello suka", "Info tipo blea", "ignot_r");
	AddItem(++ID, random_int() % 1500, "Hello suka", "Info tipo blea", "ignot_b");
	AddItem(++ID, random_int() % 1500, "Hello suka", "Info tipo blea", "ignot_g");
	AddItem(++ID, random_int()%1500, "Hello suka", "Info tipo blea", "ignot_r");
	AddItem(++ID, random_int() % 1500, "Hello suka", "Info tipo blea", "ignot_b");
	AddItem(++ID, random_int() % 1500, "Hello suka", "Info tipo blea", "ignot_g");
	AddItem(++ID, random_int()%1500, "Hello suka", "Info tipo blea", "ignot_r");
	AddItem(++ID, random_int() % 1500, "Hello suka", "Info tipo blea", "ignot_b");
	AddItem(++ID, random_int() % 1500, "Hello suka", "Info tipo blea", "ignot_g");
	AddItem(++ID, random_int()%1500, "Hello suka", "Info tipo blea", "ignot_r");
	AddItem(++ID, random_int() % 1500, "Hello suka", "Info tipo blea", "ignot_b");
	AddItem(++ID, random_int() % 1500, "Hello suka", "Info tipo blea", "ignot_g");
	AddItem(++ID, random_int()%1500, "Hello suka", "Info tipo blea", "ignot_r");
	AddItem(++ID, random_int() % 1500, "Hello suka", "Info tipo blea", "ignot_b");
	AddItem(++ID, random_int() % 1500, "Hello suka", "Info tipo blea", "ignot_g");
	AddItem(++ID, random_int()%1500, "Hello suka", "Info tipo blea", "ignot_r");
	AddItem(++ID, random_int() % 1500, "Hello suka", "Info tipo blea", "ignot_b");
	AddItem(++ID, random_int() % 1500, "Hello suka", "Info tipo blea", "ignot_g");
	AddItem(++ID, random_int()%1500, "Hello suka", "Info tipo blea", "ignot_r");
	AddItem(++ID, random_int() % 1500, "Hello suka", "Info tipo blea", "ignot_b");
	AddItem(++ID, random_int() % 1500, "Hello suka", "Info tipo blea", "ignot_g");
	AddItem(++ID, random_int()%1500, "Hello suka", "Info tipo blea", "ignot_r");
	AddItem(++ID, random_int() % 1500, "Hello suka", "Info tipo blea", "ignot_b");
	AddItem(++ID, random_int() % 1500, "Hello suka", "Info tipo blea", "ignot_g");
	AddItem(++ID, random_int()%1500, "Hello suka", "Info tipo blea", "ignot_r");
	AddItem(++ID, random_int() % 1500, "Hello suka", "Info tipo blea", "ignot_b");
	AddItem(++ID, random_int() % 1500, "Hello suka", "Info tipo blea", "ignot_g");
	AddItem(++ID, random_int()%1500, "Hello suka", "Info tipo blea", "ignot_r");
	AddItem(++ID, random_int() % 1500, "Hello suka", "Info tipo blea", "ignot_b");
	AddItem(++ID, random_int() % 1500, "Hello suka", "Info tipo blea", "ignot_g");
	AddItem(++ID, random_int()%1500, "Hello suka", "Info tipo blea", "ignot_r");
	AddItem(++ID, random_int() % 1500, "Hello suka", "Info tipo blea", "ignot_b");
	AddItem(++ID, random_int() % 1500, "Hello suka", "Info tipo blea", "ignot_g");
	AddItem(++ID, random_int()%1500, "Hello suka", "Info tipo blea", "ignot_r");
	AddItem(++ID, random_int() % 1500, "Hello suka", "Info tipo blea", "ignot_b");
	AddItem(++ID, random_int() % 1500, "Hello suka", "Info tipo blea", "ignot_g");
	AddItem(++ID, random_int()%1500, "Hello suka", "Info tipo blea", "ignot_r");
	AddItem(++ID, random_int() % 1500, "Hello suka", "Info tipo blea", "ignot_b");
	AddItem(++ID, random_int() % 1500, "Hello suka", "Info tipo blea", "ignot_g");
	AddItem(++ID, random_int()%1500, "Hello suka", "Info tipo blea", "ignot_r");
	AddItem(++ID, random_int() % 1500, "Hello suka", "Info tipo blea", "ignot_b");
	AddItem(++ID, random_int() % 1500, "Hello suka", "Info tipo blea", "ignot_g");
	AddItem(++ID, random_int()%1500, "Hello suka", "Info tipo blea", "ignot_r");
	AddItem(++ID, random_int() % 1500, "Hello suka", "Info tipo blea", "ignot_b");
	AddItem(++ID, random_int() % 1500, "Hello suka", "Info tipo blea", "ignot_g");
	AddItem(++ID, random_int()%1500, "Hello suka", "Info tipo blea", "ignot_r");
	AddItem(++ID, random_int() % 1500, "Hello suka", "Info tipo blea", "ignot_b");
	AddItem(++ID, random_int() % 1500, "Hello suka", "Info tipo blea", "ignot_g");
}

CInventoryList::~CInventoryList()
{
	for(int i = 0; i < (int)m_aInventoryPages.size(); i++)
		delete m_aInventoryPages[i];
	m_aInventoryPages.clear();
}

void CInventoryList::Render()
{
	if(!m_pWindowItemsList->IsOpenned())
		return;

	m_pWindowItemsList->Register([&](const CUIRect& pWindowRect, CWindowUI& pCurrentWindow)
	{
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
	});
}

void CInventoryList::RenderSelectionPage()
{
	bool HoveredLeft;
	bool HoveredRight;
	static float FontSize = 16.0f;
	static float FontDescSize = 10.0f;
	static float SpaceOption = 32.0f;

	const CUIRect &MainView = GetMainViewRect();
	CUIRect SelectionRect = MainView;
	SelectionRect.HSplitBottom(INVSLOT_BOXSIZE, 0, &SelectionRect);
	SelectionRect.VMargin((float)(MainView.w / 2.0f) - SpaceOption, &SelectionRect);
	{
		// left arrow
		RenderTextRoundRect(SelectionRect, 0.0f, FontSize, "<", 6.0f, &HoveredLeft);
		if((m_pInventory->m_MouseFlag & MouseEvent::M_WHEEL_DOWN && m_pInventory->UI()->MouseInside(&m_MainView))
			|| (m_pInventory->m_MouseFlag & MouseEvent::M_LEFT_CLICKED && HoveredLeft && !m_InteractiveSlot))
			ScrollInventoryPage(m_ActivePage - 1);
	}

	{ // information

		char aPageBuf[16];
		str_format(aPageBuf, sizeof(aPageBuf), "%d", (m_ActivePage + 1));

		CUIRect Label = SelectionRect;
		m_pInventory->UI()->DoLabel(&Label, aPageBuf, FontSize, CUI::EAlignment::ALIGN_CENTER);

		Label.HSplitBottom((float)20.0f, 0, &Label);
		m_pInventory->TextRender()->TextColor(0.85f, 0.85f, 0.85f, 0.85f);
		m_pInventory->UI()->DoLabel(&Label, "Can scroll with the mouse wheel.", FontDescSize, CUI::EAlignment::ALIGN_CENTER);
		m_pInventory->TextRender()->TextColor(CUI::ms_DefaultTextColor);
	}

	SelectionRect.x += (SpaceOption * 2.0f);
	{ // right arrow

		RenderTextRoundRect(SelectionRect, 0.0f, FontSize, ">", 6.0f, &HoveredRight);
		if((m_pInventory->m_MouseFlag & MouseEvent::M_WHEEL_UP && m_pInventory->UI()->MouseInside(&m_MainView)) ||
			(m_pInventory->m_MouseFlag & MouseEvent::M_LEFT_CLICKED && HoveredRight && !m_InteractiveSlot))
			ScrollInventoryPage(m_ActivePage + 1);
	}
}


void CInventoryList::AddItem(int ItemID, int Count, const char* pName, const char* pDesc, const char* pIcon)
{
	// TODO: rework and add clamping items
	CInventorySlot* pSlot = nullptr;
	for(int i = 0; i < (int)m_aInventoryPages.size(); i++)
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

void CInventoryList::RenderTextRoundRect(CUIRect Rect, float Margin, float FontSize, const char* pText, float Rounding, bool* pHovored /* = nullptr */)
{
	const float TextWeidth = (float)m_pInventory->TextRender()->TextWidth(FontSize, pText, -1);
	Rect.x -= (TextWeidth / 2.0f);
	Rect.w = (TextWeidth + (Margin * 2.0f));
	Rect.h = (FontSize + Rounding);

	if(pHovored)
	{
		*pHovored = m_pInventory->UI()->MouseHovered(&Rect);
		if(*pHovored)
			m_pInventory->RenderTools()->DrawRoundRect(&Rect, vec4(0.5f, 0.5f, 0.5f, 0.4f), Rounding);
		else
			m_pInventory->RenderTools()->DrawRoundRect(&Rect, vec4(0.1f, 0.1f, 0.1f, 0.4f), Rounding);
	}
	else
		m_pInventory->RenderTools()->DrawRoundRect(&Rect, vec4(0.1f, 0.1f, 0.1f, 0.2f), Rounding);

	Rect.Margin(Margin, &Rect);
	m_pInventory->TextRender()->TextSecondaryColor(0.0f, 0.0f, 0.0f, 0.1f);
	static CTextCursor Cursor(FontSize, Rect.x, Rect.y);
	m_pInventory->TextRender()->TextDeferred(&Cursor, pText, -1.0f);
	m_pInventory->TextRender()->TextSecondaryColor(CUI::ms_DefaultTextOutlineColor);
}

void CInventoryList::ScrollInventoryPage(int Page)
{
	const CUIRect MainView = GetMainViewRect();
	if(!m_pInventory->UI()->MouseHovered(&MainView) || m_InteractiveSlot != nullptr)
		return;

	int NewPage = clamp(Page, 0, (int)m_aInventoryPages.size() - 1);
	if(NewPage != m_ActivePage)
	{
		m_HoveredSlot = nullptr;
		m_ActivePage = NewPage;
	}
}