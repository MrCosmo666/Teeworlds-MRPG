#include "elemnts_gui.h"

#include <game/client/components/menus.h>
#include <game/client/ui_window.h>

#include <utility>

IGraphics* CElementsGUI::m_pGraphics;
ITextRender* CElementsGUI::m_pTextRender;
CUI* CElementsGUI::m_pUI;
CMenus* CElementsGUI::m_pMenus;

CElementsGUI::~CElementsGUI()
{
	for(auto& p : m_aElements)
	{
		delete p->m_pWindow;
		delete p;
	}
}

template < class T, std::enable_if_t<std::is_convertible_v<T*, BaseElemGUI*>, bool> = true >
static void UpdateElement(std::vector<BaseElemGUI*>& paElements, T* pElement)
{
	const char* pName = pElement->m_pWindow->GetWindowName();
	const auto pItem = std::find_if(paElements.begin(), paElements.end(), [pName](const BaseElemGUI* p)
	{
		return str_comp(pName, p->m_pWindow->GetWindowName()) == 0;
	});

	if(pItem != paElements.end())
	{
		delete (*pItem);
		(*pItem) = pElement;
	}
	else
		paElements.push_back(pElement);
}

/* =====================================================================
 * Information	Box		|	 GUI
 * ===================================================================== */
BaseElemGUI* CElementsGUI::CreateInformationBoxElement(float Width, const char* pMessage)
{
	const CUIRect* pScreen = UI()->Screen();
	Graphics()->MapScreen(pScreen->x, pScreen->y, pScreen->w, pScreen->h);

	constexpr float FontSize = 10.0f;
	BaseElemGUI* pElement = new BaseElemGUI;
	pElement->m_Cursor.Reset();
	pElement->m_Cursor.m_Flags = TEXTFLAG_ALLOW_NEWLINE;
	pElement->m_Cursor.m_FontSize = FontSize;
	pElement->m_Cursor.m_MaxWidth = Width - (s_InformationBoxLabelSpace * 2.0f) - 1.0f;
	pElement->m_Cursor.m_MaxLines = -1;
	TextRender()->TextDeferred(&pElement->m_Cursor, pMessage, -1);
	return pElement;
}

void CElementsGUI::CreateInformationBox(const char* pWindowName, float Width, const char* pMessage, bool* pDepent)
{
	BaseElemGUI* pElement = CreateInformationBoxElement(Width, pMessage);
	pElement->m_pWindow = UI()->CreateWindow(pWindowName, vec2(Width, 100.0f + ((float)pElement->m_Cursor.LineCount() * pElement->m_Cursor.m_FontSize)), pDepent);
	pElement->m_pWindow->Register(WINREGISTER(&CElementsGUI::CallbackRenderInfoWindow, this));
	pElement->m_pWindow->Open();

	UpdateElement(m_aElements, pElement);
}

void CElementsGUI::CreateInformationBox(const char* pWindowName, float Width, const char* pMessage, CWindowUI* pWindow)
{
	BaseElemGUI* pElement = CreateInformationBoxElement(Width, pMessage);
	pElement->m_pWindow = pWindow->AddChild(pWindowName, vec2(Width, 100.0f + ((float)pElement->m_Cursor.LineCount() * pElement->m_Cursor.m_FontSize)));
	pElement->m_pWindow->Register(WINREGISTER(&CElementsGUI::CallbackRenderInfoWindow, this));
	pElement->m_pWindow->Open();

	UpdateElement(m_aElements, pElement);
}

void CElementsGUI::CallbackRenderInfoWindow(const CUIRect& pWindowRect, CWindowUI* pCurrentWindow)
{
	const auto pElement = std::find_if(m_aElements.begin(), m_aElements.end(), [&pCurrentWindow](const BaseElemGUI* p)
	{
		return (p->m_pWindow == pCurrentWindow);
	});

	CUIRect Label, ButtonAccept;
	pWindowRect.Margin(s_InformationBoxLabelSpace, &Label);
	Label.HSplitBottom(24.0f, &Label, &ButtonAccept);
	(*pElement)->m_Cursor.MoveTo(Label.x, Label.y);
	TextRender()->DrawTextPlain(&(*pElement)->m_Cursor);

	static CMenus::CButtonContainer s_ButtonAccept;
	if(m_pMenus->DoButton_Menu(&s_ButtonAccept, "Ok", false, &ButtonAccept, nullptr, CUI::CORNER_ALL, 8.0f))
		pCurrentWindow->Close();
}

/* =====================================================================
 * Popup	Box		|	 GUI
 * ===================================================================== */
PopupElemGUI* CElementsGUI::CreatePopupElement(float Width, const char* pMessage, PopupWindowCallback Callback)
{
	const CUIRect* pScreen = UI()->Screen();
	Graphics()->MapScreen(pScreen->x, pScreen->y, pScreen->w, pScreen->h);

	constexpr float FontSize = 10.0f;
	PopupElemGUI* pElement = new PopupElemGUI;
	pElement->m_Cursor.Reset();
	pElement->m_Cursor.m_Flags = TEXTFLAG_ALLOW_NEWLINE;
	pElement->m_Cursor.m_FontSize = FontSize;
	pElement->m_Cursor.m_MaxWidth = Width - (s_InformationBoxLabelSpace * 2.0f) - 1.0f;
	pElement->m_Cursor.m_MaxLines = -1;
	TextRender()->TextDeferred(&pElement->m_Cursor, pMessage, -1);
	pElement->m_pCallback = std::move(Callback);
	return pElement;
}

void CElementsGUI::CreatePopupBox(const char* pWindowName, float Width, const char* pMessage, PopupWindowCallback Callback, bool* pDepent)
{
	PopupElemGUI* pElement = CreatePopupElement(Width, pMessage, Callback);
	pElement->m_pWindow = UI()->CreateWindow(pWindowName, vec2(Width, 80.0f + ((float)pElement->m_Cursor.LineCount() * pElement->m_Cursor.m_FontSize)), pDepent);
	pElement->m_pWindow->Register(WINREGISTER(&CElementsGUI::CallbackRenderGuiPopupBox, this));
	pElement->m_pWindow->Open();

	UpdateElement(m_aElements, pElement);
}

void CElementsGUI::CreatePopupBox(const char* pWindowName, float Width, const char* pMessage, PopupWindowCallback Callback, CWindowUI* pWindow)
{
	PopupElemGUI* pElement = CreatePopupElement(Width, pMessage, Callback);
	pElement->m_pWindow = pWindow->AddChild(pWindowName, vec2(Width, 80.0f + ((float)pElement->m_Cursor.LineCount() * pElement->m_Cursor.m_FontSize)));
	pElement->m_pWindow->Register(WINREGISTER(&CElementsGUI::CallbackRenderGuiPopupBox, this));
	pElement->m_pWindow->Open();

	UpdateElement(m_aElements, pElement);
}

void CElementsGUI::CallbackRenderGuiPopupBox(const CUIRect& pWindowRect, CWindowUI* pCurrentWindow)
{
	const auto pElement = std::find_if(m_aElements.begin(), m_aElements.end(), [&pCurrentWindow](const BaseElemGUI* p)
	{
		return (p->m_pWindow == pCurrentWindow);
	});
	PopupElemGUI* pElemPopup = (PopupElemGUI*)(*pElement);

	CUIRect Label, ButtonAccept, ButtonDeny;
	pWindowRect.Margin(s_InformationBoxLabelSpace, &Label);
	Label.HSplitBottom(18.0f, &Label, &ButtonAccept);
	ButtonAccept.VSplitLeft(Label.w / 2.0f, &ButtonDeny, &ButtonAccept);
	(*pElement)->m_Cursor.MoveTo(Label.x, Label.y);
	TextRender()->DrawTextPlain(&(*pElement)->m_Cursor);

	ButtonAccept.VMargin(5.0f, &ButtonAccept);
	static CMenus::CButtonContainer s_ButtonAccept;
	if(m_pMenus->DoButton_Menu(&s_ButtonAccept, "Yes", false, &ButtonAccept, nullptr, CUI::CORNER_ALL, 8.0f))
	{
		if(pElemPopup->m_pCallback)
			pElemPopup->m_pCallback(pCurrentWindow, true);
		pCurrentWindow->Close();
	}
	ButtonDeny.VMargin(5.0f, &ButtonDeny);
	static CMenus::CButtonContainer s_ButtonDeny;
	if(m_pMenus->DoButton_Menu(&s_ButtonDeny, "No", false, &ButtonDeny, nullptr, CUI::CORNER_ALL, 8.0f))
	{
		if(pElemPopup->m_pCallback)
			pElemPopup->m_pCallback(pCurrentWindow, false);
		pCurrentWindow->Close();
	}
}
