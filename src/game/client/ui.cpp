/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#include "ui.h"
#include "ui_window.h"

#include <generated/client_data.h>
#include <engine/client.h>
#include <engine/graphics.h>
#include <engine/input.h>
#include <engine/keys.h>
#include <engine/textrender.h>
#include <engine/shared/config.h>

#include <algorithm>

/********************************************************
 UI
*********************************************************/
const vec4 CUI::ms_DefaultTextColor(1.0f, 1.0f, 1.0f, 1.0f);
const vec4 CUI::ms_DefaultTextOutlineColor(0.0f, 0.0f, 0.0f, 0.3f);
const vec4 CUI::ms_HighlightTextColor(0.0f, 0.0f, 0.0f, 1.0f);
const vec4 CUI::ms_HighlightTextOutlineColor(1.0f, 1.0f, 1.0f, 0.25f);
const vec4 CUI::ms_TransparentTextColor(1.0f, 1.0f, 1.0f, 0.5f);

CUI::CUI()
{
	m_pHotItem = 0;
	m_pActiveItem = 0;
	m_pLastActiveItem = 0;
	m_pBecommingHotItem = 0;

	m_MouseX = 0;
	m_MouseY = 0;
	m_MouseWorldX = 0;
	m_MouseWorldY = 0;
	m_MouseButtons = 0;
	m_LastMouseButtons = 0;
	m_Enabled = true;

	m_Screen.x = 0;
	m_Screen.y = 0;

	m_NumClips = 0;

	m_AnimFades.clear();
}

CUI::~CUI()
{
	WindowsClear();
}

void CUI::Update(float MouseX, float MouseY, float MouseWorldX, float MouseWorldY)
{
	unsigned MouseButtons = 0;
	if(Enabled())
	{
		if(Input()->KeyIsPressed(KEY_MOUSE_1)) MouseButtons |= 1;
		if(Input()->KeyIsPressed(KEY_MOUSE_2)) MouseButtons |= 2;
		if(Input()->KeyIsPressed(KEY_MOUSE_3)) MouseButtons |= 4;
	}

	m_MouseX = MouseX;
	m_MouseY = MouseY;
	m_MouseWorldX = MouseWorldX;
	m_MouseWorldY = MouseWorldY;
	m_LastMouseButtons = m_MouseButtons;
	m_MouseButtons = MouseButtons;
	m_pHotItem = m_pBecommingHotItem;
	if(m_pActiveItem)
		m_pHotItem = m_pActiveItem;
	m_pBecommingHotItem = 0;
}

void CUI::MouseRectLimitMapScreen(CUIRect* pRect, float Indent, int LimitRectFlag)
{
	const float MaxWidth = Screen()->w;
	const float MaxHeight = Screen()->h;
	const float SpaceWidthX = Screen()->w / 24.f;
	const float SpaceHeightY = Screen()->h / 24.f;

	pRect->x = MouseX();
	if(pRect->x < MaxWidth / 2.0f)
	{
		pRect->x -= pRect->w + Indent;
		if(LimitRectFlag & RECTLIMITSCREEN_ALIGN_CENTER_X)
			pRect->x += pRect->w / 2.0f;
		if((pRect->x - SpaceWidthX) < Screen()->x)
			pRect->x = Indent + SpaceWidthX;
	}
	else
	{
		if(LimitRectFlag & RECTLIMITSCREEN_ALIGN_CENTER_X)
			pRect->x -= pRect->w / 2.0f;
		if((pRect->x + (pRect->w + SpaceWidthX)) > MaxWidth)
			pRect->x = (MaxWidth - pRect->w) - (Indent + SpaceWidthX);
	}

	pRect->y = MouseY();
	if((pRect->y < MaxHeight / 2.0f && LimitRectFlag & RECTLIMITSCREEN_DOWN) || LimitRectFlag == RECTLIMITSCREEN_ALL)
	{
		if(LimitRectFlag & RECTLIMITSCREEN_SKIP_BORDURE_DOWN)
			pRect->y += Indent;
		else if((pRect->y + (pRect->h + SpaceHeightY)) > MaxHeight)
			pRect->y = (MaxHeight - pRect->h) - (Indent + SpaceHeightY);
	}
	else if(LimitRectFlag & RECTLIMITSCREEN_UP || LimitRectFlag == RECTLIMITSCREEN_ALL)
	{
		if(LimitRectFlag & RECTLIMITSCREEN_SKIP_BORDURE_UP)
			pRect->y -= pRect->h + Indent;
		else
		{
			pRect->y -= pRect->h + Indent;
			if((pRect->y - SpaceHeightY) < Screen()->y)
				pRect->y = Indent + SpaceHeightY;
		}
	}
}

bool CUI::KeyPress(int Key) const
{
	return Enabled() && Input()->KeyPress(Key);
}

bool CUI::KeyIsPressed(int Key) const
{
	return Enabled() && Input()->KeyIsPressed(Key);
}

bool CUI::MouseHovered(const CUIRect* pRect) const
{
	if(g_Config.m_ClEditor || (m_pHoveredWindow && m_pCheckWindow && m_pCheckWindow == m_pHoveredWindow && MouseInside(&m_pHoveredWindow->m_WindowRect)))
		return MouseInside(pRect) && MouseInsideClip();

	// this logic ignores all MouseHovered excluding its area and available area.
	// So that each window is unique and clicks or selections don't look weird
	if(!m_pCheckWindow)
	{
		for(auto &p : CWindowUI::ms_aWindows)
		{
			if(p->IsRenderAllowed() && MouseInside(&p->m_WindowRect))
				return false;
		}
		return MouseInside(pRect) && MouseInsideClip();
	}
	return false;
}

void CUI::ConvertCursorMove(float* pX, float* pY, int CursorType) const
{
	float Factor = 1.0f;
	switch(CursorType)
	{
	case IInput::CURSOR_MOUSE:
		Factor = g_Config.m_UiMousesens / 100.0f;
		break;
	case IInput::CURSOR_JOYSTICK:
		Factor = g_Config.m_UiJoystickSens / 100.0f;
		break;
	}
	*pX *= Factor;
	*pY *= Factor;
}

const CUIRect* CUI::Screen()
{
	m_Screen.h = 600;
	m_Screen.w = Graphics()->ScreenAspect() * m_Screen.h;
	return &m_Screen;
}

float CUI::PixelSize()
{
	return Screen()->w / Graphics()->ScreenWidth();
}

void CUI::ClipEnable(const CUIRect* pRect)
{
	if(IsClipped())
	{
		dbg_assert(m_NumClips < MAX_CLIP_NESTING_DEPTH, "max clip nesting depth exceeded");
		const CUIRect* pOldRect = ClipArea();
		CUIRect Intersection;
		Intersection.x = max(pRect->x, pOldRect->x);
		Intersection.y = max(pRect->y, pOldRect->y);
		Intersection.w = min(pRect->x + pRect->w, pOldRect->x + pOldRect->w) - pRect->x;
		Intersection.h = min(pRect->y + pRect->h, pOldRect->y + pOldRect->h) - pRect->y;
		m_aClips[m_NumClips] = Intersection;
	}
	else
	{
		m_aClips[m_NumClips] = *pRect;
	}
	m_NumClips++;
	UpdateClipping();
}

void CUI::ClipDisable()
{
	dbg_assert(m_NumClips > 0, "no clip region");
	m_NumClips--;
	UpdateClipping();
}

const CUIRect* CUI::ClipArea() const
{
	dbg_assert(m_NumClips > 0, "no clip region");
	return &m_aClips[m_NumClips - 1];
}

void CUI::UpdateClipping()
{
	if(IsClipped())
	{
		const CUIRect* pRect = ClipArea();
		const float XScale = Graphics()->ScreenWidth() / Screen()->w;
		const float YScale = Graphics()->ScreenHeight() / Screen()->h;
		Graphics()->ClipEnable((int)(pRect->x * XScale), (int)(pRect->y * YScale), (int)(pRect->w * XScale), (int)(pRect->h * YScale));
	}
	else
	{
		Graphics()->ClipDisable();
	}
}

void CUIRect::HSplitMid(CUIRect* pTop, CUIRect* pBottom, float Spacing) const
{
	CUIRect r = *this;
	const float Cut = r.h / 2;
	const float HalfSpacing = Spacing / 2;

	if(pTop)
	{
		pTop->x = r.x;
		pTop->y = r.y;
		pTop->w = r.w;
		pTop->h = Cut - HalfSpacing;
	}

	if(pBottom)
	{
		pBottom->x = r.x;
		pBottom->y = r.y + Cut + HalfSpacing;
		pBottom->w = r.w;
		pBottom->h = r.h - Cut - HalfSpacing;
	}
}

void CUIRect::HSplitTop(float Cut, CUIRect* pTop, CUIRect* pBottom) const
{
	CUIRect r = *this;

	if(pTop)
	{
		pTop->x = r.x;
		pTop->y = r.y;
		pTop->w = r.w;
		pTop->h = Cut;
	}

	if(pBottom)
	{
		pBottom->x = r.x;
		pBottom->y = r.y + Cut;
		pBottom->w = r.w;
		pBottom->h = r.h - Cut;
	}
}

void CUIRect::HSplitBottom(float Cut, CUIRect* pTop, CUIRect* pBottom) const
{
	CUIRect r = *this;

	if(pTop)
	{
		pTop->x = r.x;
		pTop->y = r.y;
		pTop->w = r.w;
		pTop->h = r.h - Cut;
	}

	if(pBottom)
	{
		pBottom->x = r.x;
		pBottom->y = r.y + r.h - Cut;
		pBottom->w = r.w;
		pBottom->h = Cut;
	}
}


void CUIRect::VSplitMid(CUIRect* pLeft, CUIRect* pRight, float Spacing) const
{
	CUIRect r = *this;
	const float Cut = r.w / 2;
	const float HalfSpacing = Spacing / 2;

	if(pLeft)
	{
		pLeft->x = r.x;
		pLeft->y = r.y;
		pLeft->w = Cut - HalfSpacing;
		pLeft->h = r.h;
	}

	if(pRight)
	{
		pRight->x = r.x + Cut + HalfSpacing;
		pRight->y = r.y;
		pRight->w = r.w - Cut - HalfSpacing;
		pRight->h = r.h;
	}
}

void CUIRect::VSplitLeft(float Cut, CUIRect* pLeft, CUIRect* pRight) const
{
	CUIRect r = *this;

	if(pLeft)
	{
		pLeft->x = r.x;
		pLeft->y = r.y;
		pLeft->w = Cut;
		pLeft->h = r.h;
	}

	if(pRight)
	{
		pRight->x = r.x + Cut;
		pRight->y = r.y;
		pRight->w = r.w - Cut;
		pRight->h = r.h;
	}
}

void CUIRect::VSplitRight(float Cut, CUIRect* pLeft, CUIRect* pRight) const
{
	CUIRect r = *this;

	if(pLeft)
	{
		pLeft->x = r.x;
		pLeft->y = r.y;
		pLeft->w = r.w - Cut;
		pLeft->h = r.h;
	}

	if(pRight)
	{
		pRight->x = r.x + r.w - Cut;
		pRight->y = r.y;
		pRight->w = Cut;
		pRight->h = r.h;
	}
}

void CUIRect::Margin(float Cut, CUIRect* pOtherRect) const
{
	CUIRect r = *this;

	pOtherRect->x = r.x + Cut;
	pOtherRect->y = r.y + Cut;
	pOtherRect->w = r.w - 2 * Cut;
	pOtherRect->h = r.h - 2 * Cut;
}

void CUIRect::VMargin(float Cut, CUIRect* pOtherRect) const
{
	CUIRect r = *this;

	pOtherRect->x = r.x + Cut;
	pOtherRect->y = r.y;
	pOtherRect->w = r.w - 2 * Cut;
	pOtherRect->h = r.h;
}

void CUIRect::HMargin(float Cut, CUIRect* pOtherRect) const
{
	CUIRect r = *this;

	pOtherRect->x = r.x;
	pOtherRect->y = r.y + Cut;
	pOtherRect->w = r.w;
	pOtherRect->h = r.h - 2 * Cut;
}

bool CUIRect::Inside(float x, float y) const
{
	return x >= this->x
		&& x < this->x + this->w
		&& y >= this->y
		&& y < this->y + this->h;
}

bool CUI::DoButtonLogic(const void* pID, const CUIRect* pRect, int Button)
{
	// logic
	bool Clicked = false;
	static int s_LastButton = -1;
	const bool Hovered = MouseHovered(pRect);

	if(CheckActiveItem(pID))
	{
		if(s_LastButton == Button && !MouseButton(s_LastButton))
		{
			if(Hovered)
				Clicked = true;
			SetActiveItem(0);
			s_LastButton = -1;
		}
	}
	else if(HotItem() == pID)
	{
		if(MouseButton(Button))
		{
			SetActiveItem(pID);
			s_LastButton = Button;
		}
	}

	if(Hovered && !MouseButton(Button))
		SetHotItem(pID);

	return Clicked;
}

int CUI::DoPickerLogic(const void *pID, const CUIRect *pRect, float *pX, float *pY)
{
	if(CheckActiveItem(pID))
	{
		if(!MouseButton(0))
			SetActiveItem(0);
	}
	else if(HotItem() == pID)
	{
		if(MouseButton(0))
			SetActiveItem(pID);
	}

	if(MouseHovered(pRect))
		SetHotItem(pID);

	if(!CheckActiveItem(pID))
		return 0;

	if(pX)
		*pX = clamp(m_MouseX - pRect->x, 0.0f, pRect->w);
	if(pY)
		*pY = clamp(m_MouseY - pRect->y, 0.0f, pRect->h);

	return 1;
}

void CUI::DoLabel(const CUIRect* pRect, const char* pText, float FontSize, EAlignment Align, float LineWidth, bool MultiLine) const
{
	// TODO: FIX ME!!!!
	//Graphics()->BlendNormal();

	static CTextCursor s_Cursor;
	s_Cursor.Reset();
	s_Cursor.m_FontSize = FontSize;
	s_Cursor.m_MaxWidth = LineWidth;
	s_Cursor.m_MaxLines = 1;
	s_Cursor.m_Align = Align;

	if(MultiLine)
	{
		s_Cursor.m_Flags |= TEXTFLAG_ALLOW_NEWLINE;
		s_Cursor.m_MaxLines = -1;
	}

	switch(Align)
	{
	case CUI::ALIGN_LEFT:
		s_Cursor.m_Align = TEXTALIGN_LEFT;
		s_Cursor.MoveTo(pRect->x, pRect->y);
		break;
	case CUI::ALIGN_CENTER:
		s_Cursor.m_Align = TEXTALIGN_CENTER;
		s_Cursor.MoveTo(pRect->x + pRect->w / 2.0f, pRect->y);
		break;
	case CUI::ALIGN_RIGHT:
		s_Cursor.m_Align = TEXTALIGN_RIGHT;
		s_Cursor.MoveTo(pRect->x + pRect->w, pRect->y);
		break;
	}

	TextRender()->TextOutlined(&s_Cursor, pText, -1);
}

void CUI::DoLabelColored(const CUIRect* pRect, const char* pText, float FontSize, EAlignment Align, vec4 Color, float LineWidth, bool MultiLine) const
{
	TextRender()->TextColor(Color);
	DoLabel(pRect, pText, FontSize, Align, LineWidth, MultiLine);
	TextRender()->TextColor(ms_DefaultTextColor);
}

void CUI::DoLabelHighlighted(const CUIRect* pRect, const char* pText, const char* pHighlighted, float FontSize, const vec4& TextColor, const vec4& HighlightColor) const
{
	static CTextCursor s_Cursor;
	s_Cursor.Reset();
	s_Cursor.m_FontSize = FontSize;
	s_Cursor.m_MaxWidth = pRect->w;
	s_Cursor.MoveTo(pRect->x, pRect->y);

	TextRender()->TextColor(TextColor);
	const char* pMatch = pHighlighted && pHighlighted[0] ? str_find_nocase(pText, pHighlighted) : 0;
	if(pMatch)
	{
		TextRender()->TextDeferred(&s_Cursor, pText, (int)(pMatch - pText));
		TextRender()->TextColor(HighlightColor);
		TextRender()->TextDeferred(&s_Cursor, pMatch, str_length(pHighlighted));
		TextRender()->TextColor(TextColor);
		TextRender()->TextDeferred(&s_Cursor, pMatch + str_length(pHighlighted), -1);
	}
	else
		TextRender()->TextDeferred(&s_Cursor, pText, -1);

	TextRender()->DrawTextOutlined(&s_Cursor);
}

// TODO: improve
// I have no idea how to avoid static data and make unique ID
// in each area regardless of the class and still work directly at runtime
float CUI::GetFade(CUIRect *pRect, bool Checked, float Seconds)
{
	const bool Hovered = MouseHovered(pRect);
	m_AnimFades.erase(std::remove_if(m_AnimFades.begin(), m_AnimFades.end(), [&](const AnimFade& pFade)
		{ return (pFade.m_StartTime + pFade.m_Seconds) < m_pClient->LocalTime(); }), m_AnimFades.end());
	auto pItem = std::find_if(m_AnimFades.begin(), m_AnimFades.end(), [&pRect](const AnimFade& pFade)
		{ return pRect->x == pFade.m_Rect.x && pRect->y == pFade.m_Rect.y && pRect->w == pFade.m_Rect.w && pRect->h == pFade.m_Rect.h; });

	if(Hovered || Checked)
	{
		if(pItem != m_AnimFades.end())
		{
			pItem->m_StartTime = m_pClient->LocalTime();
			pItem->m_Seconds = Seconds;
		}
		else
		{
			AnimFade Fade;
			Fade.m_Rect = *pRect;
			Fade.m_StartTime = m_pClient->LocalTime();
			Fade.m_Seconds = Seconds;
			m_AnimFades.push_back(Fade);
		}
		return 1.0f;
	}

	if(pItem != m_AnimFades.end())
	{
		float Progression = max(0.0f, pItem->m_StartTime - m_pClient->LocalTime() + pItem->m_Seconds) / pItem->m_Seconds;
		return Progression;
	}
	return 0.0f;
}

int CUI::DoMouseEventLogic(const CUIRect* pRect, int Button) const
{
	static bool IsPressed = false;
	int Event = CButtonLogicEvent::EMPTY;
	bool Hovered = MouseHovered(pRect);
	if(Hovered)
	{
		Event = CButtonLogicEvent::EVENT_HOVERED;
		if(KeyPress(Button))
		{
			Event |= CButtonLogicEvent::EVENT_PRESS;
			IsPressed = true;
		}
		else if(KeyIsPressed(Button))
			Event |= CButtonLogicEvent::EVENT_PRESSED;
		else if(IsPressed)
		{
			Event |= CButtonLogicEvent::EVENT_RELEASE;
			IsPressed = false;
		}
	}

	if(!Hovered && IsPressed)
	{
		Event = CButtonLogicEvent::EVENT_RELEASE;
		IsPressed = false;
	}

	return Event;
}

CWindowUI* CUI::CreateWindow(const char* pWindowName, vec2 WindowSize, CWindowUI* pDependentWindow, bool* pRenderDependence, int WindowFlags)
{
	const auto pSearch = std::find_if(CWindowUI::ms_aWindows.begin(), CWindowUI::ms_aWindows.end(), [pWindowName](const CWindowUI* pWindow) { return str_comp(pWindowName, pWindow->GetWindowName()) == 0;  });
	if(pSearch != CWindowUI::ms_aWindows.end())
		return (*pSearch);

	CWindowUI* pWindow = new CWindowUI(pWindowName, WindowSize, pDependentWindow, pRenderDependence, WindowFlags);
	CWindowUI::ms_aWindows.push_back(pWindow);
	return pWindow;
}

void CUI::WindowRender()
{
	/*const auto RenderAllowed = [](const CWindowUI* pWindow) -> bool
	{
		return (pWindow->m_Openned && (pWindow->m_pRenderDependence == nullptr || (pWindow->m_pRenderDependence && *pWindow->m_pRenderDependence == true)));
	};*/

	// update hovered the active highlighted area
	for(auto it = CWindowUI::ms_aWindows.rbegin(); it != CWindowUI::ms_aWindows.rend(); ++it)
	{
		if((*it)->IsRenderAllowed() && MouseInside(&(*it)->m_WindowRect))
			m_pHoveredWindow = (*it);
	}

	// draw in reverse order as they are sorted here
	bool ShowCursor = false;
	const CUIRect ScreenMap = *Screen();
	Graphics()->MapScreen(ScreenMap.x, ScreenMap.y, ScreenMap.w, ScreenMap.h);
	for(auto it = CWindowUI::ms_aWindows.rbegin(); it != CWindowUI::ms_aWindows.rend(); ++it)
	{
		if((*it)->IsRenderAllowed())
		{
			(*it)->Render();
			ShowCursor = true;
		}
	}

	// update the sorting in case of a change of the active window
	for(auto it = CWindowUI::ms_aWindows.rbegin(); it != CWindowUI::ms_aWindows.rend(); ++it)
	{
		if((*it)->IsRenderAllowed())
		{
			// start check only this window
			StartCheckWindow((*it));
			if(CWindowUI::GetActiveWindow() != (*it) && (DoMouseEventLogic(&(*it)->m_WindowRect, KEY_MOUSE_1) & CUI::CButtonLogicEvent::EVENT_PRESS))
			{
				auto Iterator = std::find_if(CWindowUI::ms_aWindows.begin(), CWindowUI::ms_aWindows.end(), [=](const CWindowUI* pWindow) { return pWindow == (*it);  });
				if(Iterator != CWindowUI::ms_aWindows.end())
				{
					std::rotate(CWindowUI::ms_aWindows.begin(), Iterator, Iterator + 1);
					break;
				}
			}

			// end check only this window
			FinishCheckWindow();
		}
	}

	// clear hovered active highlighted area
	m_pHoveredWindow = nullptr;

	// hotkeys
	CWindowUI* pWindowActive = CWindowUI::GetActiveWindow();
	if(pWindowActive)
	{
		if((pWindowActive->m_WindowFlags & WINDOWFLAG_CLOSE) && Input()->KeyIsPressed(KEY_LCTRL) && Input()->KeyPress(KEY_Q))
			pWindowActive->Close();
		if((pWindowActive->m_WindowFlags & WINDOWFLAG_MINIMIZE) && Input()->KeyIsPressed(KEY_LCTRL) && Input()->KeyPress(KEY_M))
			pWindowActive->MinimizeWindow();
		if((pWindowActive->m_pCallbackHelp) && Input()->KeyIsPressed(KEY_LCTRL) && Input()->KeyPress(KEY_H))
		{
			CWindowUI::ms_pWindowHelper->Init(vec2(0, 0), pWindowActive, pWindowActive->m_pRenderDependence);
			CWindowUI::ms_pWindowHelper->Register(pWindowActive->m_pCallbackHelp);
			CWindowUI::ms_pWindowHelper->Open();
		}
	}

	// render cursor
	if(ShowCursor)
	{
		Graphics()->TextureSet(g_pData->m_aImages[IMAGE_CURSOR].m_Id);
		Graphics()->WrapClamp();
		Graphics()->QuadsBegin();
		IGraphics::CQuadItem QuadItem(MouseX(), MouseY(), 24, 24);
		Graphics()->QuadsDrawTL(&QuadItem, 1);
		Graphics()->QuadsEnd();
		Graphics()->WrapNormal();
	}
}

void CUI::WindowsClear()
{
	for(auto* p : CWindowUI::ms_aWindows)
		delete p;
	CWindowUI::ms_aWindows.clear();
}