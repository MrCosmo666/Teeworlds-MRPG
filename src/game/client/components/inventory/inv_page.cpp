/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#include <base/vmath.h>
#include <engine/keys.h>
#include <engine/storage.h>
#include <engine/textrender.h>

#include <engine/shared/config.h>
#include <generated/client_data.h>

#include <game/client/components/inventory.h>
#include "inv_page.h"
#include "inv_slot.h"

CInventoryPage::CInventoryPage(CInventory* pInventory, int Page) : m_pInventory(pInventory)
{
	for(int i = 0; i < MAX_ITEMS_PER_PAGE; i++)
	{
		m_Slot[i] = new CInventorySlot(pInventory);
		m_Slot[i]->m_Page = Page;
	}
}