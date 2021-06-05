/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#include "ui_window.h"

#include <engine/keys.h>
#include "render.h"

#include <algorithm>

std::vector<CWindowUI*> CWindowUI::ms_aWindows;
CUI* CWindowUI::m_pUI;
CRenderTools* CWindowUI::m_pRenderTools;

// - - -- - -- - --
// The basic logic
void CWindowUI::Render()
{
	// don't render closed window
	if(!m_Openned || !m_pCallback || m_pRenderDependence && (*m_pRenderDependence) == false)
		return;

	// start check only this window
	m_pUI->StartCheckWindow(this);

	static float s_BackgroundMargin = 2.0f;
	static float s_WindowSkipMovingX;
	static float s_WindowSkipMovingY;
	CUIRect Bordure, Workspace;
	m_WindowRect.HSplitTop(20.0f, &Bordure, &Workspace);

	// logic bordour move window
	const int MoveLogic = m_pUI->DoMouseEventLogic(&Bordure, KEY_MOUSE_1);
	if(MoveLogic & CUI::CButtonLogicEvent::EVENT_PRESS)
	{
		m_WindowMoving = true;
		s_WindowSkipMovingX = (m_pUI->MouseX() - m_WindowRect.x);
		s_WindowSkipMovingY = (m_pUI->MouseY() - m_WindowRect.y);
	}
	if(m_WindowMoving)
	{
		m_WindowRect.x = Bordure.x = clamp(m_pUI->MouseX() - s_WindowSkipMovingX, 0.0f, m_pUI->Screen()->w - m_WindowRect.w);
		m_WindowRect.y = Bordure.y = clamp(m_pUI->MouseY() - s_WindowSkipMovingY, 0.0f, m_pUI->Screen()->h - m_WindowRect.h);

		if(!m_pUI->KeyIsPressed(KEY_MOUSE_1))
		{
			m_WindowMoving = false;
			s_WindowSkipMovingX = 0.0f;
			s_WindowSkipMovingY = 0.0f;
		}
	}

	// highlight
	if(m_HighlightColor.a > 0.0f)
	{
		CUIRect HighlightActive;
		const float HighlightMargin = -1.2f;
		const vec4 ColorHighlight = m_HighlightColor;

		if(m_WindowMinimize)
			HighlightActive = Bordure;
		else
			HighlightActive = m_WindowRect;

		HighlightActive.Margin(HighlightMargin, &HighlightActive);
		m_pRenderTools->DrawUIRectMonochromeGradient(&HighlightActive, ColorHighlight, CUI::CORNER_ALL, 8.0f);
	}

	// background draw
	const bool IsActiveWindow = IsActive();
	if(!m_WindowMinimize)
	{
		CUIRect MainBackground = m_WindowRect;
		const float BackgroundFade = m_pUI->GetFade(&m_WindowRect, IsActiveWindow, 0.4f);
		const vec4 Color = mix(vec4(0.15f, 0.15f, 0.15f, 0.95f), vec4(0.2f, 0.2f, 0.2f, 0.95f), BackgroundFade);

		MainBackground.Margin(s_BackgroundMargin, &MainBackground);
		m_pRenderTools->DrawUIRectMonochromeGradient(&MainBackground, Color, CUI::CORNER_ALL, 12.0f);
		m_pRenderTools->DrawRoundRect(&m_WindowRect, vec4(0.1f, 0.1f, 0.1f, 0.5f), 12.0f);
	}

	// bordour draw
	const float BordureFade = m_pUI->GetFade(&Bordure, IsActiveWindow);
	vec4 Color = mix(vec4(0.15f, 0.15f, 0.15f, 1.0f), vec4(0.3f, 0.3f, 0.3f, 1.0f), BordureFade);
	m_pRenderTools->DrawUIRectMonochromeGradient(&Bordure, Color, CUI::CORNER_ALL, 10.0f);

	// window name
	CUIRect Label;
	Bordure.VSplitLeft(10.0f, 0, &Label);
	m_pUI->DoLabel(&Label, m_aWindowName, 12.0f, CUI::EAlignment::ALIGN_LEFT, -1.0f);

	// close button
	if(m_WindowFlags & CUI::WINDOWFLAG_CLOSE)
	{
		CUIRect ButtonClose;
		Bordure.VSplitRight(24.0f, 0, &ButtonClose);
		Color = mix(vec4(0.f, 0.f, 0.f, 0.25f), vec4(0.7f, 0.1f, 0.1f, 0.75f), m_pUI->GetFade(&ButtonClose, false));

		m_pRenderTools->DrawUIRectMonochromeGradient(&ButtonClose, Color, CUI::CORNER_ALL, 10.0f);
		m_pUI->DoLabel(&ButtonClose, "\xE2\x9C\x95", 16.0f, CUI::ALIGN_CENTER);

		const int CloseLogic = m_pUI->DoMouseEventLogic(&ButtonClose, KEY_MOUSE_1);
		if(CloseLogic & CUI::CButtonLogicEvent::EVENT_PRESS)
			Close();
	}

	// hide button
	if(m_WindowFlags & CUI::WINDOWFLAG_MINIMIZE)
	{
		CUIRect ButtonHide;
		Bordure.VSplitRight(24.0f, 0, &ButtonHide);
		ButtonHide.x -= m_WindowFlags & CUI::WINDOWFLAG_CLOSE ? 24.0f : 0.0f;
		Color = mix(vec4(0.f, 0.f, 0.f, 0.25f), vec4(0.2f, 0.2f, 0.7f, 0.75f), m_pUI->GetFade(&ButtonHide, false));

		m_pRenderTools->DrawUIRectMonochromeGradient(&ButtonHide, Color, CUI::CORNER_ALL, 10.0f);
		m_pUI->DoLabel(&ButtonHide, m_WindowMinimize ? "\xe2\x81\x82" : "\xe2\x80\xbb", 16.0f, CUI::ALIGN_CENTER);

		const int HideLogic = m_pUI->DoMouseEventLogic(&ButtonHide, KEY_MOUSE_1);
		if(HideLogic & CUI::CButtonLogicEvent::EVENT_PRESS)
		{
			m_WindowMinimize ^= true;
			if(m_WindowMinimize)
			{
				m_WindowRectProtected = m_WindowRect;
				m_WindowRect = Bordure;
			}
			else
				m_WindowRect = m_WindowRectProtected;
		}
	}

	// callback function render
	if(!m_WindowMinimize && m_pCallback)
	{
		CUIRect DrawWindowRect = m_WindowRect;
		DrawWindowRect.Margin(s_BackgroundMargin, &DrawWindowRect);
		DrawWindowRect.HSplitTop(Bordure.h, 0, &DrawWindowRect);
		m_pCallback(DrawWindowRect, *this);
	}

	// end check only this window
	m_pUI->FinishCheckWindow();
}

// - - -- - -- - -- - -- - -- - -- - -
// Functions for working with windows
void CWindowUI::Init(vec2 WindowSize, CWindowUI* pWindowDependent, bool* pRenderDependence)
{
	m_WindowRect = { 0, 0, WindowSize.x, WindowSize.y };
	m_WindowRectProtected = m_WindowRect;
	m_WindowMinimize = false;
	m_WindowMoving = false;
	m_Openned = false;
	m_pRenderDependence = pRenderDependence;

	SetDependent(pWindowDependent);
}

const CUIRect& CWindowUI::GetRect() const
{
	return m_WindowRect;
}

CWindowUI::CWindowUI(const char* pWindowName, vec2 WindowSize, CWindowUI* pWindowDependent, bool* pRenderDependence, int WindowFlags)
{
	m_WindowFlags = WindowFlags;
	str_copy(m_aWindowName, pWindowName, sizeof(m_aWindowName));
	Init(WindowSize, pWindowDependent, pRenderDependence);
}

bool CWindowUI::IsOpenned() const
{
	return m_Openned;
}

bool CWindowUI::IsActive() const
{
	return (bool)(m_Openned && CWindowUI::GetActiveWindow() == this);
}

void CWindowUI::Open()
{
	if(IsOpenned())
		Close();

	const CUIRect WindowRect = m_WindowRectProtected;
	CUIRect NewWindowRect = { m_pUI->MouseX(), m_pUI->MouseY(), WindowRect.w, WindowRect.h };
	m_pUI->MouseRectLimitMapScreen(&NewWindowRect, 6.0f, CUI::RECTLIMITSCREEN_UP);

	m_WindowRect = NewWindowRect;
	m_Openned = true;
	m_WindowMoving = false;
	m_WindowMinimize = false;

	const auto pSearch = std::find_if(CWindowUI::ms_aWindows.begin(), CWindowUI::ms_aWindows.end(), [this](const CWindowUI* pWindow) { return str_comp(m_aWindowName, pWindow->GetWindowName()) == 0;  });
	std::rotate(CWindowUI::ms_aWindows.begin(), pSearch, pSearch + 1);
}

void CWindowUI::Close()
{
	for(auto& p : ms_aWindows)
	{
		if(str_comp_nocase(p->m_aWindowDependentName, m_aWindowName) == 0)
			p->Close();
	}
	m_Openned = false;
}

void CWindowUI::Reverse()
{
	if(m_Openned)
		Close();
	else
		Open();
}

void CWindowUI::RegisterCallback(RenderWindowCallback pCallback)
{
	m_pCallback = pCallback;
}

void CWindowUI::HighlightEnable(vec4 Color, bool DependentsToo)
{
	m_HighlightColor = Color;
	if(DependentsToo)
	{
		for(auto& p : ms_aWindows)
		{
			if(str_comp_nocase(p->m_aWindowDependentName, m_aWindowName) == 0)
				p->HighlightEnable(Color, DependentsToo);
		}
	}
}

void CWindowUI::HighlightDisable()
{
	m_HighlightColor = vec4(-1, -1, -1, -1);
	for(auto& p : ms_aWindows)
	{
		if(str_comp_nocase(p->m_aWindowDependentName, m_aWindowName) == 0)
			p->HighlightDisable();
	}
}

void CWindowUI::SetDependent(const char* pWindowName)
{
	str_copy(m_aWindowDependentName, pWindowName, sizeof(m_aWindowDependentName));
}
