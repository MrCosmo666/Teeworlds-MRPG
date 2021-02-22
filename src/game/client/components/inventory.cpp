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

#include "inventory/inv_list.h"
#include "inventory/inv_page.h"
#include "inventory/inv_slot.h"

// TODO: ever complete the inventory
CInventory::CInventory()
{
	m_MouseFlag = 0;
}

void CInventory::OnRender()
{
	// skip menus
	if(m_pClient->m_pMenus->IsActive() || !m_Active)
		return;

	UI()->StartCheck();
	Graphics()->MapScreen(m_ScreenRestriction.x, m_ScreenRestriction.y, m_ScreenRestriction.w, m_ScreenRestriction.h);
	UI()->Update(m_PositionMouse.x, m_PositionMouse.y, m_PositionMouse.x * 3.0f, m_PositionMouse.y * 3.0f);

	// render inventory
	m_pCraftItems->Render();

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
	m_ScreenRestriction = *UI()->Screen();
	m_Active = true;

	m_pCraftItems = new CInventoryList(this, m_ScreenRestriction, 5, 6);
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
	else if(Event.m_Key == KEY_MOUSE_WHEEL_UP)
	{
		m_MouseFlag |= M_WHEEL_UP;
		return true;
	}
	else if(Event.m_Key == KEY_MOUSE_WHEEL_DOWN)
	{
		m_MouseFlag |= M_WHEEL_DOWN;
		return true;
	}
	return false;
}

void CInventory::OnStateChange(int NewState, int OldState)
{
	if(NewState != IClient::STATE_ONLINE)
	{
		m_Active = false;
	}
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
	if(m_PositionMouse.x > m_ScreenRestriction.w) m_PositionMouse.x = m_ScreenRestriction.w;
	if(m_PositionMouse.y > m_ScreenRestriction.h) m_PositionMouse.y = m_ScreenRestriction.h;
	return true;
}