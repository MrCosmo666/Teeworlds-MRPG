/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#include <engine/textrender.h>
#include <generated/client_data.h>

#include <game/client/components/inventory.h>

#include "inv_list.h"
#include "inv_slot.h"
#include "inv_page.h"

CInventoryPage::CInventoryPage(CInventory* pInventory, CInventoryList* pInventoryList, int Page)
	: m_pInventory(pInventory), m_pInventoryList(pInventoryList)
{
	int NewSlots = 0;
	for(int i = 0; i < m_pInventoryList->GetPageMaxSlots(); i++)
		m_Slot.add(new CInventorySlot(pInventory, pInventoryList, Page, NewSlots++));
}

CInventoryPage::~CInventoryPage()
{
	for(int i = 0; i < m_pInventoryList->GetPageMaxSlots(); i++)
		delete m_Slot[i];
	m_Slot.clear();
}

void CInventoryPage::Render()
{
	// empty grid
	for(int i = 0; i < m_pInventoryList->GetPageMaxSlots(); i++)
	{
		CUIRect SlotRect = m_pInventoryList->GetMainViewRect();
		SlotRect.VSplitLeft(INVSLOT_BOXSIZE / 2.0f, 0, &SlotRect);
		SlotRect.HSplitTop(INVSLOT_BOXSIZE, 0, &SlotRect);

		CalculateSlotPosition(i, &SlotRect);
		if(!m_pInventoryList->GetInteractiveSlot() && m_pInventory->UI()->MouseHovered(&SlotRect))
			m_pInventory->RenderTools()->DrawRoundRect(&SlotRect, vec4(0.7f, 0.7f, 0.7f, 0.5f), 8.0f);
		else
			m_pInventory->RenderTools()->DrawRoundRect(&SlotRect, vec4(0.35f, 0.35f, 0.35f, 0.4f), 8.0f);

		m_Slot[i]->m_RectSlot = SlotRect;
	}

	// render inventory
	for(int i = 0; i < m_pInventoryList->GetPageMaxSlots(); i++)
	{
		CInventorySlot* pSlot = GetSlot(i);
		if(!pSlot || m_pInventoryList->GetSelectedSlot() == pSlot)
			continue;

		GetSlot(i)->Render();
		GetSlot(i)->UpdateEvents();
	}
}

void CInventoryPage::CalculateSlotPosition(int SlotID, CUIRect* SlotRect)
{
	float OldPositionX = SlotRect->x;
	for(int CheckingSlot = 1; CheckingSlot <= SlotID; CheckingSlot++)
	{
		if(CheckingSlot % m_pInventoryList->GetSlotsWidth() != 0)
		{
			SlotRect->x += (INVSLOT_BOXSIZE + INVSLOT_SPACING);
			continue;
		}
		SlotRect->x = OldPositionX;
		SlotRect->y += (INVSLOT_BOXSIZE + INVSLOT_SPACING);
	}
	SlotRect->w = INVSLOT_BOXSIZE;
	SlotRect->h = INVSLOT_BOXSIZE;
}