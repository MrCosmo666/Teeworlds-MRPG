/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#include "ui_window.h"

#include <engine/keys.h>
#include <engine/textrender.h>
#include "gameclient.h"
#include "render.h"

#include <algorithm>

CUI* CWindowUI::m_pUI;
CRenderTools* CWindowUI::m_pRenderTools;
CWindowUI* CWindowUI::ms_pWindowHelper;
std::vector<CWindowUI*> CWindowUI::ms_aWindows;

static float s_BackgroundMargin = 2.0f;

// - - -- - -- - --
// The basic logic
void CWindowUI::RenderWindowWithoutBordure()
{
	CUIRect Workspace;
	m_WindowRect.Margin(s_BackgroundMargin, &Workspace);

	// background draw
	CUIRect MainBackground;
	Workspace.Margin(-s_BackgroundMargin, &MainBackground);
	m_pRenderTools->DrawUIRectMonochromeGradient(&MainBackground, DEFAULT_BACKGROUND_WINDOW_SHANDOW, CUI::CORNER_ALL, 2.0f);
	m_pRenderTools->DrawRoundRect(&Workspace, m_BackgroundColor, 2.0f);

	if(m_pCallback)
		m_pCallback(Workspace, *this);
}

void CWindowUI::RenderDefaultWindow()
{
	// logic bordour move window
	static float s_WindowSkipMovingX;
	static float s_WindowSkipMovingY;
	if(m_WindowMoving)
	{
		m_WindowRect.x = m_WindowBordure.x = clamp(m_pUI->MouseX() - s_WindowSkipMovingX, 0.0f, m_pUI->Screen()->w - m_WindowRect.w);
		m_WindowRect.y = m_WindowBordure.y = clamp(m_pUI->MouseY() - s_WindowSkipMovingY, 0.0f, m_pUI->Screen()->h - m_WindowRect.h);
		if(!m_pUI->KeyIsPressed(KEY_MOUSE_1))
		{
			m_WindowMoving = false;
			s_WindowSkipMovingX = 0.0f;
			s_WindowSkipMovingY = 0.0f;
		}
	}

	CUIRect Workspace;
	m_WindowRect.HSplitTop(20.0f, &m_WindowBordure, &Workspace);

	// background draw
	CUIRect ShadowBackground;
	m_WindowRect.Margin(-1.5f, &ShadowBackground);
	m_pRenderTools->DrawRoundRect(&ShadowBackground, DEFAULT_BACKGROUND_WINDOW_SHANDOW, 10.0f);
	
	const bool IsActiveWindow = IsActive();
	if(!m_WindowMinimize)
	{
		const float BackgroundFade = m_pUI->GetFade(&Workspace, IsActiveWindow, 0.4f);
		const vec4 Color = mix(m_BackgroundColor -= vec4(0.02f, 0.02f, 0.02f, 0.f), m_BackgroundColor, BackgroundFade);
		m_pRenderTools->DrawUIRectMonochromeGradient(&Workspace, Color, CUI::CORNER_ALL, 10.0f);
	}
	
	// bordour draw
	const float BordureFade = m_pUI->GetFade(&m_WindowBordure, IsActiveWindow);
	const vec4 Color = mix(vec4(0.15f, 0.15f, 0.15f, 1.0f), vec4(0.3f, 0.3f, 0.3f, 1.0f), BordureFade);
	m_pRenderTools->DrawUIRectMonochromeGradient(&m_WindowBordure, Color, m_WindowMinimize ? CUI::CORNER_ALL : CUI::CORNER_T | CUI::CORNER_IB, 10.0f);

	// window name
	CUIRect Label;
	m_WindowBordure.VSplitLeft(10.0f, 0, &Label);
	m_pUI->DoLabel(&Label, m_aWindowName, 12.0f, CUI::EAlignment::ALIGN_LEFT, -1.0f);
	
	// buttontop function
	auto CreateButtonTop = [this, &IsActiveWindow](CUIRect* pButtonRect, const char* pHintStr, vec4 ColorFade1, vec4 ColorFade2, const char* pSymbolUTF) -> bool
	{
		pButtonRect->x -= 24.0f;
		const vec4 ColorFinal = mix(ColorFade1, ColorFade2, m_pUI->GetFade(pButtonRect, false));
		m_pRenderTools->DrawUIRectMonochromeGradient(pButtonRect, ColorFinal, CUI::CORNER_ALL, 10.0f);
		m_pUI->DoLabel(pButtonRect, pSymbolUTF, 16.0f, CUI::ALIGN_CENTER);

		const int HideLogic = m_pUI->DoMouseEventLogic(pButtonRect, KEY_MOUSE_1);
		if(HideLogic & CUI::CButtonLogicEvent::EVENT_PRESS)
		{
			m_WindowMoving = false;
			return true;
		}

		if(IsActiveWindow && (HideLogic & CUI::CButtonLogicEvent::EVENT_HOVERED))
		{
			const char* HotKeyLabel = Localize(pHintStr);
			const float TextWidth = m_pUI->TextRender()->TextWidth(10.0f, HotKeyLabel, -1);
			CUIRect BackgroundKeyPress = { 0.f, 0.f, 10.0f + TextWidth, 20.f };
			m_pUI->MouseRectLimitMapScreen(&BackgroundKeyPress, 12.0f, CUI::RECTLIMITSCREEN_UP | CUI::RECTLIMITSCREEN_SKIP_BORDURE_UP);
			m_pRenderTools->DrawUIRectMonochromeGradient(&BackgroundKeyPress, vec4(0.1f, 0.1f, 0.1f, 0.5f), CUI::CORNER_ALL, 3.0f);

			CUIRect LabelKeyInfo = BackgroundKeyPress;
			LabelKeyInfo.Margin(s_BackgroundMargin, &LabelKeyInfo);
			m_pUI->DoLabel(&LabelKeyInfo, HotKeyLabel, 10.0f, CUI::ALIGN_CENTER);
		}
		return false;
	};


	CUIRect ButtonTop;
	m_WindowBordure.VSplitRight(24.0f, 0, &ButtonTop);
	ButtonTop.x += 24.0f;
	if(m_WindowFlags & CUI::WINDOWFLAG_CLOSE && CreateButtonTop(&ButtonTop, "Left Ctrl + Q - close active window.", vec4(0.f, 0.f, 0.f, 0.25f), vec4(0.7f, 0.1f, 0.1f, 0.75f), "\xE2\x9C\x95"))  // close button
			Close();

	if(m_WindowFlags & CUI::WINDOWFLAG_MINIMIZE && CreateButtonTop(&ButtonTop, "Left Ctrl + M - minimize active window.", vec4(0.f, 0.f, 0.f, 0.25f), vec4(0.2f, 0.2f, 0.7f, 0.75f),
			m_WindowMinimize ? "\xe2\x81\x82" : "\xe2\x80\xbb"))
		MinimizeWindow();

	if(m_pCallbackHelp && CreateButtonTop(&ButtonTop, "Left Ctrl + H - show attached help active window.",
			ms_pWindowHelper->IsOpenned() ? vec4(0.1f, 0.3f, 0.1f, 0.75f) : vec4(0.f, 0.f, 0.f, 0.25f), vec4(0.2f, 0.5f, 0.2f, 0.75f), "?"))
	{
		ms_pWindowHelper->Init(vec2(0, 0), this, m_pRenderDependence);
		ms_pWindowHelper->Register(m_pCallbackHelp);
		ms_pWindowHelper->Reverse();
	}
	else if(m_pUI->DoMouseEventLogic(&m_WindowBordure, KEY_MOUSE_1) & CUI::CButtonLogicEvent::EVENT_PRESS)
	{
		m_WindowMoving = true;
		s_WindowSkipMovingX = (m_pUI->MouseX() - m_WindowRect.x);
		s_WindowSkipMovingY = (m_pUI->MouseY() - m_WindowRect.y);
	}

	// callback function render
	if(!m_WindowMinimize && m_pCallback)
	{
		Workspace.Margin(s_BackgroundMargin, &Workspace);
		m_pCallback(Workspace, *this);
	}
}

void CWindowUI::Render()
{
	// don't render closed window
	if(!IsRenderAllowed())
		return;

	// start check only this window
	m_pUI->StartCheckWindow(this);

	// render window
	if(m_WindowFlags & CUI::WINDOW_WITHOUT_BORDURE)
		RenderWindowWithoutBordure();
	else
		RenderDefaultWindow();

	// close window when is unactive
	if(m_WindowFlags & CUI::WINDOW_CLOSE_CLICKING_OUTSIDE && !IsActive())
		Close();

	// end check only this window
	m_pUI->FinishCheckWindow();
}

void CWindowUI::MinimizeWindow()
{
	m_WindowMinimize ^= true;
	if(m_WindowMinimize)
	{
		m_WindowRectReserve = m_WindowRect;
		m_WindowRect = m_WindowBordure;
		return;
	}

	m_WindowRectReserve.x = clamp(m_WindowRect.x, 0.0f, m_pUI->Screen()->w - m_WindowRectReserve.w);
	m_WindowRectReserve.y = clamp(m_WindowRect.y, 0.0f, m_pUI->Screen()->h - m_WindowRectReserve.h);
	m_WindowRect = m_WindowRectReserve;
}

// - - -- - -- - -- - -- - -- - -- - -
// Functions for working with windows
void CWindowUI::Init(vec2 WindowSize, CWindowUI* pWindowDependent, bool* pRenderDependence)
{
	m_WindowBordure = { 0, 0, 0, 0 };
	m_WindowRect = { 0, 0, WindowSize.x, WindowSize.y };
	m_WindowRectReserve = m_WindowRect;
	m_WindowMinimize = false;
	m_WindowMoving = false;
	m_pRenderDependence = pRenderDependence;
	m_BackgroundColor = DEFAULT_BACKGROUND_WINDOW_COLOR;

	UpdateDependent(pWindowDependent);
}

const CUIRect& CWindowUI::GetRect() const
{
	return m_WindowRect;
}

CWindowUI::CWindowUI(const char* pWindowName, vec2 WindowSize, CWindowUI* pWindowDependent, bool* pRenderDependence, int WindowFlags)
{
	m_Openned = false;
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
	return (bool)(m_Openned && GetActiveWindow() == this);
}

void CWindowUI::Open()
{
	CUIRect NewWindowRect = { 0, 0, m_WindowRectReserve.w, m_WindowRectReserve.h };
	m_pUI->MouseRectLimitMapScreen(&NewWindowRect, 6.0f, CUI::RECTLIMITSCREEN_UP | CUI::RECTLIMITSCREEN_ALIGN_CENTER_X);

	m_WindowRect = NewWindowRect;
	m_Openned = true;
	m_WindowMoving = false;
	m_WindowMinimize = false;

	const auto pSearch = std::find_if(ms_aWindows.begin(), ms_aWindows.end(), [this](const CWindowUI* pWindow) { return this == pWindow;  });
	std::rotate(ms_aWindows.begin(), pSearch, pSearch + 1);
}

void CWindowUI::Close()
{
	m_Openned = false;
	for(auto& p : ms_aWindows)
	{
		if(str_comp_nocase(p->m_aWindowDependentName, m_aWindowName) == 0)
			p->Close();
	}
}

void CWindowUI::Reverse()
{
	if(m_Openned)
		Close();
	else
		Open();
}

void CWindowUI::Register(RenderWindowCallback pCallback)
{
	m_pCallback = std::move(pCallback);
}

void CWindowUI::RegisterHelpPage(RenderWindowCallback pCallback)
{
	m_pCallbackHelp = std::move(pCallback);
}

void CWindowUI::UpdateDependent(const char* pWindowName)
{
	str_copy(m_aWindowDependentName, pWindowName, sizeof(m_aWindowDependentName));
}

CWindowUI* CWindowUI::GetActiveWindow()
{
	const auto pItem = std::find_if(ms_aWindows.begin(), ms_aWindows.end(), [](const CWindowUI* pWindow) { return pWindow->IsRenderAllowed(); });
	return pItem != ms_aWindows.end() ? (*pItem) : nullptr;
}
