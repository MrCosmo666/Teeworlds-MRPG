/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#include <base/system.h>
#include <engine/keys.h>

#include "render.h"
#include "ui_window.h"

std::vector<CWindowUI*> CWindowUI::ms_aWindows;
CUI* CWindowUI::m_pUI;
CRenderTools* CWindowUI::m_pRenderTools;

// - - -- - -- - --
// The basic logic
void CWindowUI::Render()
{
	// don't render closed window
	if(!m_Openned)
		return;

	// start check only this window
	m_pUI->StartCheckWindow(this);

	// logic bordour move window
	CUIRect Bordure = m_WindowRect;
	Bordure.h = 24.0f;

	int MoveLogic = m_pUI->DoMouseEventLogic(&Bordure, KEY_MOUSE_1);
	if(MoveLogic & CUI::CButtonLogicEvent::EVENT_PRESS)
	{
		m_WindowMove = true;
		m_WindowSkipX = (m_pUI->MouseX() - m_WindowRect.x);
		m_WindowSkipY = (m_pUI->MouseY() - m_WindowRect.y);
	}
	if(m_WindowMove)
	{
		float NewPositionX = m_pUI->MouseX() - m_WindowSkipX;
		float NewPositionY = m_pUI->MouseY() - m_WindowSkipY;
		NewPositionX = clamp(NewPositionX, 0.0f, m_pUI->Screen()->w - m_WindowRect.w);
		NewPositionY = clamp(NewPositionY, 0.0f, m_pUI->Screen()->h - m_WindowRect.h);

		Bordure.x = NewPositionX;
		Bordure.y = NewPositionY;
		m_WindowRect.x = NewPositionX;
		m_WindowRect.y = NewPositionY;
		// m_pUI->Update(m_WindowRect.x + m_WindowSkipX, m_WindowRect.y + m_WindowSkipY, 0, 0);
	
		if(!m_pUI->KeyIsPressed(KEY_MOUSE_1))
		{
			m_WindowMove = false;
			m_WindowSkipX = 0.0f;
			m_WindowSkipY = 0.0f;
		}
	}

	// background draw
	CUIRect MoreBackground = m_WindowRect;
	MoreBackground.Margin(5.0f, &MoreBackground);
	m_pRenderTools->DrawRoundRect(&MoreBackground, vec4(0.3f, 0.3f, 0.3f, 0.9f), 16.0f);
	m_pRenderTools->DrawRoundRect(&m_WindowRect, vec4(0.1f, 0.1f, 0.1f, 0.50f), 16.0f);

	// bordour draw
	vec4 Color = mix(vec4(0.1f, 0.1f, 0.1f, 1.0f), vec4(0.4f, 0.4f, 0.4f, 1.0f), m_pUI->GetFade(&Bordure, IsActive()));
	m_pRenderTools->DrawUIRect(&Bordure, Color, CUI::CORNER_ALL, 12.0f);
	m_pUI->DoLabel(&Bordure, m_aWindowName, 16.0f, CUI::EAlignment::ALIGN_CENTER, -1.0f);

	// callback function render
	if(m_pCallback)
	{
		m_pCallback(m_WindowRect, *this);
		m_pCallback = nullptr;
	}

	// close button
	CUIRect ButtonClose;
	Bordure.VSplitRight(24.0f, 0, &ButtonClose);
	Color = mix(vec4(0.f, 0.f, 0.f, 0.25f), vec4(0.7f, 0.1f, 0.1f, 0.75f), m_pUI->GetFade(&ButtonClose, false));
	m_pRenderTools->DrawUIRect(&ButtonClose, Color, CUI::CORNER_ALL, 12.0f);
	m_pUI->DoLabel(&ButtonClose, "\xE2\x9C\x95", 18.0f, CUI::ALIGN_CENTER);
	int CloseLogic = m_pUI->DoMouseEventLogic(&ButtonClose, KEY_MOUSE_1);
	if(CloseLogic & CUI::CButtonLogicEvent::EVENT_PRESS)
		m_Openned = false;

	// end check only this window
	m_pUI->FinishCheckWindow();
}

// - - -- - -- - -- - -- - -- - -- - -
// Functions for working with windows
void CWindowUI::Init(const char* pWindowName, CUIRect WindowRect)
{
	CWindowUI* pWindow = GetWindow(pWindowName);
	if(pWindow)
	{
		(*this) = *pWindow;
		return;
	}

	pWindow = new CWindowUI();
	str_copy(pWindow->m_aWindowName, pWindowName, sizeof(pWindow->m_aWindowName));
	pWindow->m_WindowRect = WindowRect;
	pWindow->m_Openned = true;
	ms_aWindows.push_back(pWindow);
	(*this) = *pWindow;
}

CUIRect& CWindowUI::GetRect()
{
	CWindowUI* pWindow = GetWindow(m_aWindowName);
	return (pWindow ? pWindow->m_WindowRect : m_WindowRect);
}

bool CWindowUI::IsOpenned() const
{
	CWindowUI* pWindow = GetWindow(m_aWindowName);
	return (pWindow && pWindow->m_Openned);
}

bool CWindowUI::IsActive() const
{
	CWindowUI* pWindow = GetWindow(m_aWindowName);
	return (bool)(pWindow && pWindow->m_Openned && CWindowUI::GetActiveWindow() == pWindow);
}

void CWindowUI::OnRenderWindow(RenderWindowCallback pCallback)
{
	CWindowUI* pWindow = GetWindow(m_aWindowName);
	if(pWindow)
		pWindow->m_pCallback = pCallback;
}

CWindowUI* CWindowUI::GetWindow(const char* pWindowName) const
{
	auto pSearch = std::find_if(CWindowUI::ms_aWindows.begin(), CWindowUI::ms_aWindows.end(), [pWindowName](const CWindowUI* pWindow) { return str_comp(pWindowName, pWindow->GetWindowName()) == 0;  });
	if(pSearch != CWindowUI::ms_aWindows.end())
		return (*pSearch);
	return nullptr;
}
