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
	CUIRect Bordure;
	m_WindowRect.HSplitTop(16.0f, &Bordure, 0);
	int MoveLogic = m_pUI->DoMouseEventLogic(&Bordure, KEY_MOUSE_1);
	if(MoveLogic & CUI::CButtonLogicEvent::EVENT_PRESS)
	{
		m_WindowMoved = true;
		m_WindowSkipX = (m_pUI->MouseX() - m_WindowRect.x);
		m_WindowSkipY = (m_pUI->MouseY() - m_WindowRect.y);
	}
	if(m_WindowMoved)
	{
		float NewPositionX = m_pUI->MouseX() - m_WindowSkipX;
		float NewPositionY = m_pUI->MouseY() - m_WindowSkipY;
		NewPositionX = clamp(NewPositionX, 0.0f, m_pUI->Screen()->w - m_WindowRect.w);
		NewPositionY = clamp(NewPositionY, 0.0f, m_pUI->Screen()->h - m_WindowRect.h);

		Bordure.x = NewPositionX;
		Bordure.y = NewPositionY;
		m_WindowRect.x = NewPositionX;
		m_WindowRect.y = NewPositionY;
	
		if(!m_pUI->KeyIsPressed(KEY_MOUSE_1))
		{
			m_WindowMoved = false;
			m_WindowSkipX = 0.0f;
			m_WindowSkipY = 0.0f;
		}
	}

	if(!m_WindowHidden)
	{
		// background draw
		CUIRect MoreBackground = m_WindowRect;
		MoreBackground.Margin(4.0f, &MoreBackground);
		m_pRenderTools->DrawRoundRect(&MoreBackground, vec4(0.3f, 0.3f, 0.3f, 0.9f), 12.0f);
		m_pRenderTools->DrawRoundRect(&m_WindowRect, vec4(0.1f, 0.1f, 0.1f, 0.50f), 12.0f);

		// callback function render
		if(m_pCallback)
		{
			m_pCallback(m_WindowRect, *this);
			m_pCallback = nullptr;
		}
	}

	// bordour draw
	vec4 Color = mix(vec4(0.1f, 0.1f, 0.1f, 1.0f), vec4(0.4f, 0.4f, 0.4f, 1.0f), m_pUI->GetFade(&Bordure, IsActive()));
	m_pRenderTools->DrawUIRect(&Bordure, Color, CUI::CORNER_ALL, 8.0f);

	CUIRect Label;
	Bordure.VSplitLeft(10.0f, 0, &Label);
	m_pUI->DoLabel(&Label, m_aWindowName, 12.0f, CUI::EAlignment::ALIGN_LEFT, -1.0f);

	// close button
	if(m_WindowFlags & CWindowFlags::WINDOW_CLOSE)
	{
		CUIRect ButtonClose;
		Bordure.VSplitRight(24.0f, 0, &ButtonClose);
		Color = mix(vec4(0.f, 0.f, 0.f, 0.25f), vec4(0.7f, 0.1f, 0.1f, 0.75f), m_pUI->GetFade(&ButtonClose, false));
		m_pRenderTools->DrawUIRect(&ButtonClose, Color, CUI::CORNER_ALL, 8.0f);
		m_pUI->DoLabel(&ButtonClose, "\xE2\x9C\x95", 14.0f, CUI::ALIGN_CENTER);
		const int CloseLogic = m_pUI->DoMouseEventLogic(&ButtonClose, KEY_MOUSE_1);
		if(CloseLogic & CUI::CButtonLogicEvent::EVENT_PRESS)
			m_Openned = false;
	}

	// hide button
	if(m_WindowFlags & CWindowFlags::WINDOW_MINIMIZE)
	{
		CUIRect ButtonHide;
		Bordure.VSplitRight(24.0f, 0, &ButtonHide);
		if(m_WindowFlags & CWindowFlags::WINDOW_CLOSE)
			ButtonHide.x -= 24.0f;

		Color = mix(vec4(0.f, 0.f, 0.f, 0.25f), vec4(0.2f, 0.2f, 0.5f, 0.75f), m_pUI->GetFade(&ButtonHide, false));
		m_pRenderTools->DrawUIRect(&ButtonHide, Color, CUI::CORNER_ALL, 8.0f);
		m_pUI->DoLabel(&ButtonHide, m_WindowHidden ? "\xe2\x80\xbb" : "\xe2\x81\x82", 14.0f, CUI::ALIGN_CENTER);
		const int HideLogic = m_pUI->DoMouseEventLogic(&ButtonHide, KEY_MOUSE_1);
		if(HideLogic & CUI::CButtonLogicEvent::EVENT_PRESS)
		{
			m_WindowHidden ^= true;
			if(m_WindowHidden)
			{
				m_WindowRectOld = m_WindowRect;
				m_WindowRect = Bordure;
			}
			else
				m_WindowRect = m_WindowRectOld;
		}
	}

	// end check only this window
	m_pUI->FinishCheckWindow();
}

// - - -- - -- - -- - -- - -- - -- - -
// Functions for working with windows
void CWindowUI::Init(const char* pWindowName, CUIRect WindowRect, int WindowFlags)
{
	CWindowUI* pWindow = GetWindow(pWindowName);
	if(pWindow)
	{
		(*this) = *pWindow;
		return;
	}

	pWindow = new CWindowUI();
	str_copy(pWindow->m_aWindowName, pWindowName, sizeof(pWindow->m_aWindowName));
	pWindow->m_WindowFlags = WindowFlags;
	pWindow->m_WindowRect = WindowRect;
	pWindow->m_WindowHidden = false;
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
