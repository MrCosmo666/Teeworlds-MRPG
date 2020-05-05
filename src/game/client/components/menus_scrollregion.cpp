/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#include <math.h>

#include <base/system.h>
#include <base/math.h>
#include <base/vmath.h>

#include <engine/config.h>
#include <engine/keys.h>
#include <engine/shared/config.h>

#include "menus.h"

CMenus::CScrollRegion::CScrollRegion(CMenus *pMenus)
{
	m_pMenus = pMenus;
	m_ScrollY = 0;
	m_ContentH = 0;
	m_RequestScrollY = -1;
	m_ContentScrollOff = vec2(0,0);
	m_WasClipped = false;
	m_Params = CScrollRegionParams();
}

void CMenus::CScrollRegion::Begin(CUIRect* pClipRect, vec2* pOutOffset, const CScrollRegionParams* pParams)
{
	if(pParams)
		m_Params = *pParams;

	m_WasClipped = m_pMenus->UI()->IsClipped();
	m_OldClipRect = *m_pMenus->UI()->ClipArea();

	const bool ContentOverflows = m_ContentH > pClipRect->h;
	const bool ForceShowScrollbar = m_Params.m_Flags&CScrollRegionParams::FLAG_CONTENT_STATIC_WIDTH;

	CUIRect ScrollBarBg;
	CUIRect* pModifyRect = (ContentOverflows || ForceShowScrollbar) ? pClipRect : 0;
	pClipRect->VSplitRight(m_Params.m_ScrollbarWidth, pModifyRect, &ScrollBarBg);
	ScrollBarBg.Margin(m_Params.m_ScrollbarMargin, &m_RailRect);

	// only show scrollbar if required
	if(ContentOverflows || ForceShowScrollbar)
	{
		if(m_Params.m_ScrollbarBgColor.a > 0)
			m_pMenus->RenderTools()->DrawRoundRect(&ScrollBarBg, m_Params.m_ScrollbarBgColor, 4.0f);
		if(m_Params.m_RailBgColor.a > 0)
			m_pMenus->RenderTools()->DrawRoundRect(&m_RailRect, m_Params.m_RailBgColor, m_RailRect.w/2.0f);
	}
	if(!ContentOverflows)
		m_ContentScrollOff.y = 0;

	if(m_Params.m_ClipBgColor.a > 0)
		m_pMenus->RenderTools()->DrawRoundRect(pClipRect, m_Params.m_ClipBgColor, 4.0f);

	CUIRect ClipRect = *pClipRect;
	if(m_WasClipped)
	{
		CUIRect Intersection;
		Intersection.x = max(ClipRect.x, m_OldClipRect.x);
		Intersection.y = max(ClipRect.y, m_OldClipRect.y);
		Intersection.w = min(ClipRect.x+ClipRect.w, m_OldClipRect.x+m_OldClipRect.w) - ClipRect.x;
		Intersection.h = min(ClipRect.y+ClipRect.h, m_OldClipRect.y+m_OldClipRect.h) - ClipRect.y;
		ClipRect = Intersection;
	}

	m_pMenus->UI()->ClipEnable(&ClipRect);

	m_ClipRect = *pClipRect;
	m_ContentH = 0;
	*pOutOffset = m_ContentScrollOff;
}

void CMenus::CScrollRegion::End()
{
	m_pMenus->UI()->ClipDisable();
	if(m_WasClipped)
		m_pMenus->UI()->ClipEnable(&m_OldClipRect);

	// only show scrollbar if content overflows
	if(m_ContentH <= m_ClipRect.h)
		return;

	// scroll wheel
	CUIRect RegionRect = m_ClipRect;
	RegionRect.w += m_Params.m_ScrollbarWidth;
	if(m_pMenus->UI()->MouseInside(&RegionRect))
	{
		if(m_pMenus->Input()->KeyPress(KEY_MOUSE_WHEEL_UP))
			m_ScrollY -= m_Params.m_ScrollSpeed;
		else if(m_pMenus->Input()->KeyPress(KEY_MOUSE_WHEEL_DOWN))
			m_ScrollY += m_Params.m_ScrollSpeed;
	}

	const float SliderHeight = max(m_Params.m_SliderMinHeight,
		m_ClipRect.h/m_ContentH * m_RailRect.h);

	CUIRect Slider = m_RailRect;
	Slider.h = SliderHeight;
	const float MaxScroll = m_RailRect.h - SliderHeight;

	if(m_RequestScrollY >= 0)
	{
		m_ScrollY = m_RequestScrollY/(m_ContentH - m_ClipRect.h) * MaxScroll;
		m_RequestScrollY = -1;
	}

	m_ScrollY = clamp(m_ScrollY, 0.0f, MaxScroll);
	Slider.y += m_ScrollY;

	bool Hovered = false;
	bool Grabbed = false;
	const void* pID = &m_ScrollY;
	int Inside = m_pMenus->UI()->MouseInside(&Slider);

	if(Inside)
	{
		m_pMenus->UI()->SetHotItem(pID);

		if(!m_pMenus->UI()->CheckActiveItem(pID) && m_pMenus->UI()->MouseButtonClicked(0))
		{
			m_pMenus->UI()->SetActiveItem(pID);
			m_MouseGrabStart.y = m_pMenus->UI()->MouseY();
		}

		Hovered = true;
	}

	if(m_pMenus->UI()->CheckActiveItem(pID) && !m_pMenus->UI()->MouseButton(0))
		m_pMenus->UI()->SetActiveItem(0);

	// move slider
	if(m_pMenus->UI()->CheckActiveItem(pID) && m_pMenus->UI()->MouseButton(0))
	{
		float my = m_pMenus->UI()->MouseY();
		m_ScrollY += my - m_MouseGrabStart.y;
		m_MouseGrabStart.y = my;

		Grabbed = true;
	}

	m_ScrollY = clamp(m_ScrollY, 0.0f, MaxScroll);
	m_ContentScrollOff.y = -m_ScrollY/MaxScroll * (m_ContentH - m_ClipRect.h);

	vec4 SliderColor = m_Params.m_SliderColor;
	if(Grabbed)
		SliderColor = m_Params.m_SliderColorGrabbed;
	else if(Hovered)
		SliderColor = m_Params.m_SliderColorHover;

	m_pMenus->RenderTools()->DrawRoundRect(&Slider, SliderColor, Slider.w/2.0f);
}

void CMenus::CScrollRegion::AddRect(CUIRect Rect)
{
	vec2 ContentPos = vec2(m_ClipRect.x, m_ClipRect.y);
	ContentPos.x += m_ContentScrollOff.x;
	ContentPos.y += m_ContentScrollOff.y;
	m_LastAddedRect = Rect;
	m_ContentH = max(Rect.y + Rect.h - ContentPos.y, m_ContentH);
}

void CMenus::CScrollRegion::ScrollHere(int Option)
{
	const float MinHeight = min(m_ClipRect.h, m_LastAddedRect.h);
	const float TopScroll = m_LastAddedRect.y - (m_ClipRect.y + m_ContentScrollOff.y);

	switch(Option)
	{
		case CScrollRegion::SCROLLHERE_TOP:
			m_RequestScrollY = TopScroll;
			break;

		case CScrollRegion::SCROLLHERE_BOTTOM:
			m_RequestScrollY = TopScroll - (m_ClipRect.h - MinHeight);
			break;

		case CScrollRegion::SCROLLHERE_KEEP_IN_VIEW:
		default: {
			const float dy = m_LastAddedRect.y - m_ClipRect.y;

			if(dy < 0)
				m_RequestScrollY = TopScroll;
			else if(dy > (m_ClipRect.h-MinHeight))
				m_RequestScrollY = TopScroll - (m_ClipRect.h - MinHeight);
		} break;
	}
}

bool CMenus::CScrollRegion::IsRectClipped(const CUIRect& Rect) const
{
	return (m_ClipRect.x > (Rect.x + Rect.w)
		|| (m_ClipRect.x + m_ClipRect.w) < Rect.x
		|| m_ClipRect.y > (Rect.y + Rect.h)
		|| (m_ClipRect.y + m_ClipRect.h) < Rect.y);
}

bool CMenus::CScrollRegion::IsScrollbarShown() const
{
	return m_ContentH > m_ClipRect.h;
} 