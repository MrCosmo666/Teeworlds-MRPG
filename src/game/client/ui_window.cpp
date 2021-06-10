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
void CWindowUI::RenderHighlightArea(const CUIRect& AreaRect) const
{
	if(m_HighlightColor.a <= 0.0f)
		return;

	CUIRect HighlightActive = AreaRect;
	const float HighlightMargin = -1.2f;
	const vec4 ColorHighlight = m_HighlightColor;
	HighlightActive.Margin(HighlightMargin, &HighlightActive);
	m_pRenderTools->DrawUIRectMonochromeGradient(&HighlightActive, ColorHighlight, CUI::CORNER_ALL, 8.0f);
}

void CWindowUI::RenderWindowWithoutBordure()
{
	CUIRect Workspace = m_WindowRect;
	Workspace.Margin(s_BackgroundMargin, &Workspace);
	RenderHighlightArea(Workspace);

	// background draw
	CUIRect MainBackground;
	Workspace.Margin(-s_BackgroundMargin, &MainBackground);
	m_pRenderTools->DrawUIRectMonochromeGradient(&MainBackground, vec4(0.3f, 0.3f, 0.3f, 0.95f), CUI::CORNER_ALL, 2.0f);
	m_pRenderTools->DrawRoundRect(&Workspace, vec4(0.1f, 0.1f, 0.1f, 0.5f), 2.0f);

	if(m_pCallback)
		m_pCallback(Workspace, *this);
}

void CWindowUI::RenderDefaultWindow()
{
	static float s_WindowSkipMovingX;
	static float s_WindowSkipMovingY;

	// highlight
	CUIRect Workspace;
	m_WindowRect.HSplitTop(20.0f, &m_WindowBordure, &Workspace);
	if(m_WindowMinimize)
		RenderHighlightArea(m_WindowBordure);
	else
		RenderHighlightArea(m_WindowRect);

	// background draw
	const bool IsActiveWindow = IsActive();
	if(!m_WindowMinimize)
	{
		CUIRect ShadowBackground = { m_WindowRect.x - 3.0f, m_WindowRect.y + 3.0f, m_WindowRect.w, m_WindowRect.h };
		const float BackgroundFade = m_pUI->GetFade(&Workspace, IsActiveWindow, 0.4f);
		const vec4 Color = mix(vec4(0.12f, 0.12f, 0.12f, 0.97f), vec4(0.15f, 0.15f, 0.15f, 0.97f), BackgroundFade);

		m_pRenderTools->DrawRoundRect(&ShadowBackground, vec4(0.1f, 0.1f, 0.1f, 0.3f), 10.0f);
		m_pRenderTools->DrawUIRectMonochromeGradient(&Workspace, Color, CUI::CORNER_ALL, 10.0f);
	}

	// bordour draw
	const float BordureFade = m_pUI->GetFade(&m_WindowBordure, IsActiveWindow);
	vec4 Color = mix(vec4(0.15f, 0.15f, 0.15f, 1.0f), vec4(0.3f, 0.3f, 0.3f, 1.0f), BordureFade);
	const int BordureCornerFlag = m_WindowMinimize ? CUI::CORNER_ALL : CUI::CORNER_T | CUI::CORNER_IB;
	m_pRenderTools->DrawUIRectMonochromeGradient(&m_WindowBordure, Color, BordureCornerFlag, 10.0f);

	// window name
	CUIRect Label;
	m_WindowBordure.VSplitLeft(10.0f, 0, &Label);
	m_pUI->DoLabel(&Label, m_aWindowName, 12.0f, CUI::EAlignment::ALIGN_LEFT, -1.0f);

	// close button
	if(m_WindowFlags & CUI::WINDOWFLAG_CLOSE)
	{
		CUIRect ButtonClose;
		m_WindowBordure.VSplitRight(24.0f, 0, &ButtonClose);
		Color = mix(vec4(0.f, 0.f, 0.f, 0.25f), vec4(0.7f, 0.1f, 0.1f, 0.75f), m_pUI->GetFade(&ButtonClose, false));

		m_pRenderTools->DrawUIRectMonochromeGradient(&ButtonClose, Color, CUI::CORNER_ALL, 10.0f);
		m_pUI->DoLabel(&ButtonClose, "\xE2\x9C\x95", 16.0f, CUI::ALIGN_CENTER);

		const int CloseLogic = m_pUI->DoMouseEventLogic(&ButtonClose, KEY_MOUSE_1);
		if(CloseLogic & CUI::CButtonLogicEvent::EVENT_PRESS)
			Close();

		if(IsActiveWindow && (CloseLogic & CUI::CButtonLogicEvent::EVENT_HOVERED))
		{
			const char* HotKeyLabel = Localize("Left Ctrl + Q - close active window.");
			const float TextWidth = m_pUI->TextRender()->TextWidth(10.0f, HotKeyLabel, -1);
			CUIRect BackgroundKeyPress = { 0.f, 0.f, 10.0f + TextWidth, 20.f };
			m_pUI->MouseRectLimitMapScreen(&BackgroundKeyPress, 12.0f, CUI::RECTLIMITSCREEN_UP | CUI::RECTLIMITSCREEN_SKIP_BORDURE_UP);
			m_pRenderTools->DrawUIRectMonochromeGradient(&BackgroundKeyPress, vec4(0.1f, 0.1f, 0.1f, 0.5f), CUI::CORNER_ALL, 3.0f);

			CUIRect LabelKeyInfo = BackgroundKeyPress;
			LabelKeyInfo.Margin(s_BackgroundMargin, &LabelKeyInfo);
			m_pUI->DoLabel(&LabelKeyInfo, HotKeyLabel, 10.0f, CUI::ALIGN_CENTER);
		}
	}

	// hide button
	if(m_WindowFlags & CUI::WINDOWFLAG_MINIMIZE)
	{
		CUIRect ButtonHide;
		m_WindowBordure.VSplitRight(24.0f, 0, &ButtonHide);
		ButtonHide.x -= m_WindowFlags & CUI::WINDOWFLAG_CLOSE ? 24.0f : 0.0f;
		Color = mix(vec4(0.f, 0.f, 0.f, 0.25f), vec4(0.2f, 0.2f, 0.7f, 0.75f), m_pUI->GetFade(&ButtonHide, false));

		m_pRenderTools->DrawUIRectMonochromeGradient(&ButtonHide, Color, CUI::CORNER_ALL, 10.0f);
		m_pUI->DoLabel(&ButtonHide, m_WindowMinimize ? "\xe2\x81\x82" : "\xe2\x80\xbb", 16.0f, CUI::ALIGN_CENTER);

		const int HideLogic = m_pUI->DoMouseEventLogic(&ButtonHide, KEY_MOUSE_1);
		if(HideLogic & CUI::CButtonLogicEvent::EVENT_PRESS)
			MinimizeWindow();

		if(IsActiveWindow && (HideLogic & CUI::CButtonLogicEvent::EVENT_HOVERED))
		{
			const char* HotKeyLabel = Localize("Left Ctrl + M - minimize active window.");
			const float TextWidth = m_pUI->TextRender()->TextWidth(10.0f, HotKeyLabel, -1);
			CUIRect BackgroundKeyPress = { 0.f, 0.f, 10.0f + TextWidth, 20.f };
			m_pUI->MouseRectLimitMapScreen(&BackgroundKeyPress, 12.0f, CUI::RECTLIMITSCREEN_UP|CUI::RECTLIMITSCREEN_SKIP_BORDURE_UP);
			m_pRenderTools->DrawUIRectMonochromeGradient(&BackgroundKeyPress, vec4(0.1f, 0.1f, 0.1f, 0.5f), CUI::CORNER_ALL, 3.0f);

			CUIRect LabelKeyInfo = BackgroundKeyPress;
			LabelKeyInfo.Margin(s_BackgroundMargin, &LabelKeyInfo);
			m_pUI->DoLabel(&LabelKeyInfo, HotKeyLabel, 10.0f, CUI::ALIGN_CENTER);
		}
	}

	// help button
	if(m_pCallbackHelp)
	{
		CUIRect ButtonHelp;
		m_WindowBordure.VSplitRight(24.0f, 0, &ButtonHelp);
		ButtonHelp.x -= m_WindowFlags & CUI::WINDOWFLAG_CLOSE ? 24.0f + (m_WindowFlags & CUI::WINDOWFLAG_MINIMIZE ? 24.0f : 0.0f)  : 0.0f;
		Color = mix((ms_pWindowHelper->IsOpenned() ? vec4(0.1f, 0.3f, 0.1f, 0.75f) : vec4(0.f, 0.f, 0.f, 0.25f)), vec4(0.2f, 0.5f, 0.2f, 0.75f), m_pUI->GetFade(&ButtonHelp, false));

		m_pRenderTools->DrawUIRectMonochromeGradient(&ButtonHelp, Color, CUI::CORNER_ALL, 10.0f);
		m_pUI->DoLabel(&ButtonHelp, "?", 16.0f, CUI::ALIGN_CENTER);

		const int HelpLogic = m_pUI->DoMouseEventLogic(&ButtonHelp, KEY_MOUSE_1);
		if(HelpLogic & CUI::CButtonLogicEvent::EVENT_PRESS)
		{
			ms_pWindowHelper->Init(vec2(0, 0), this, m_pRenderDependence);
			ms_pWindowHelper->Register(m_pCallbackHelp);
			ms_pWindowHelper->Reverse();
		}
		if(IsActiveWindow && (HelpLogic & CUI::CButtonLogicEvent::EVENT_HOVERED))
		{
			const char* HotKeyLabel = Localize("Left Ctrl + H - show attached help active window.");
			const float TextWidth = m_pUI->TextRender()->TextWidth(10.0f, HotKeyLabel, -1);
			CUIRect BackgroundKeyPress = { 0.f, 0.f, 10.0f + TextWidth, 20.f };
			m_pUI->MouseRectLimitMapScreen(&BackgroundKeyPress, 12.0f, CUI::RECTLIMITSCREEN_UP | CUI::RECTLIMITSCREEN_SKIP_BORDURE_UP);
			m_pRenderTools->DrawUIRectMonochromeGradient(&BackgroundKeyPress, vec4(0.1f, 0.1f, 0.1f, 0.5f), CUI::CORNER_ALL, 3.0f);

			CUIRect LabelKeyInfo = BackgroundKeyPress;
			LabelKeyInfo.Margin(s_BackgroundMargin, &LabelKeyInfo);
			m_pUI->DoLabel(&LabelKeyInfo, HotKeyLabel, 10.0f, CUI::ALIGN_CENTER);
		}
	}

	// logic bordour move window
	const int MoveLogic = m_pUI->DoMouseEventLogic(&m_WindowBordure, KEY_MOUSE_1);
	if(MoveLogic & CUI::CButtonLogicEvent::EVENT_PRESS)
	{
		m_WindowMoving = true;
		s_WindowSkipMovingX = (m_pUI->MouseX() - m_WindowRect.x);
		s_WindowSkipMovingY = (m_pUI->MouseY() - m_WindowRect.y);
	}
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
		m_WindowRectProtected = m_WindowRect;
		m_WindowRect = m_WindowBordure;
		return;
	}

	m_WindowRectProtected.x = clamp(m_WindowRect.x, 0.0f, m_pUI->Screen()->w - m_WindowRectProtected.w);
	m_WindowRectProtected.y = clamp(m_WindowRect.y, 0.0f, m_pUI->Screen()->h - m_WindowRectProtected.h);
	m_WindowRect = m_WindowRectProtected;
}

// - - -- - -- - -- - -- - -- - -- - -
// Functions for working with windows
void CWindowUI::Init(vec2 WindowSize, CWindowUI* pWindowDependent, bool* pRenderDependence)
{
	m_WindowBordure = { 0, 0, 0, 0 };
	m_WindowRect = { 0, 0, WindowSize.x, WindowSize.y };
	m_WindowRectProtected = m_WindowRect;
	m_WindowMinimize = false;
	m_WindowMoving = false;
	m_pRenderDependence = pRenderDependence;

	SetDependent(pWindowDependent);
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
	return (bool)(m_Openned && CWindowUI::GetActiveWindow() == this);
}

void CWindowUI::Open()
{
	if(IsOpenned())
		Close();

	const CUIRect WindowRect = m_WindowRectProtected;
	CUIRect NewWindowRect = { 0, 0, WindowRect.w, WindowRect.h };
	m_pUI->MouseRectLimitMapScreen(&NewWindowRect, 6.0f, CUI::RECTLIMITSCREEN_UP | CUI::RECTLIMITSCREEN_ALIGN_CENTER_X);

	m_WindowRect = NewWindowRect;
	m_Openned = true;
	m_WindowMoving = false;
	m_WindowMinimize = false;

	const auto pSearch = std::find_if(CWindowUI::ms_aWindows.begin(), CWindowUI::ms_aWindows.end(), [this](const CWindowUI* pWindow) { return str_comp(m_aWindowName, pWindow->GetWindowName()) == 0;  });
	std::rotate(CWindowUI::ms_aWindows.begin(), pSearch, pSearch + 1);
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

void CWindowUI::RegisterHelp(RenderWindowCallback pCallback)
{
	m_pCallbackHelp = std::move(pCallback);
}

void CWindowUI::HighlightEnable(vec4 Color, bool DependentToo)
{
	m_HighlightColor = Color;
	for(auto& p : ms_aWindows)
	{
		if(str_comp_nocase(p->m_aWindowDependentName, m_aWindowName) == 0)
			p->HighlightEnable(Color, DependentToo);
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

CWindowUI* CWindowUI::GetActiveWindow()
{
	const auto pItem = std::find_if(ms_aWindows.begin(), ms_aWindows.end(), [](const CWindowUI* pWindow) { return pWindow->IsRenderAllowed(); });
	return pItem != ms_aWindows.end() ? (*pItem) : nullptr;
}
