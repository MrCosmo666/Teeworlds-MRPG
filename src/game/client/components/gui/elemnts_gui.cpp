#include "elemnts_gui.h"

#include <game/client/components/menus.h>
#include <game/client/ui_window.h>

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

/* =====================================================================
 * Information	Box		|	 GUI
 * ===================================================================== */
static float s_InformationBoxLabelSpace = 8.0f;
void CElementsGUI::CreateInformationBox(const char* pWindowName, CWindowUI* pDependentWindow, float Width, const char* pMessage, bool* pRequires)
{
	const CUIRect* pScreen = UI()->Screen();
	Graphics()->MapScreen(pScreen->x, pScreen->y, pScreen->w, pScreen->h);

	auto EnableWindow = [=](BaseElemGUI* m_pElemGUI)
	{
		const float FontSize = 10.0f;
		m_pElemGUI->m_Cursor.Reset();
		m_pElemGUI->m_Cursor.m_Flags = TEXTFLAG_ALLOW_NEWLINE;
		m_pElemGUI->m_Cursor.m_FontSize = FontSize;
		m_pElemGUI->m_Cursor.m_MaxWidth = Width - (s_InformationBoxLabelSpace * 2.0f) - 1.0f;
		m_pElemGUI->m_Cursor.m_MaxLines = -1;
		TextRender()->TextDeferred(&m_pElemGUI->m_Cursor, pMessage, -1);
		m_pElemGUI->m_pWindow = UI()->CreateWindow(pWindowName, vec2(0, 0), pDependentWindow, pRequires);
		m_pElemGUI->m_pWindow->Register(WINREGISTER(&CElementsGUI::CallbackRenderInfoWindow, this));
		m_pElemGUI->m_pWindow->Init(vec2(Width, 80.0f + ((float)m_pElemGUI->m_Cursor.LineCount() * FontSize)), pDependentWindow, pRequires);
		m_pElemGUI->m_pWindow->Open();
	};

	const auto pElemGUI = std::find_if(m_aElements.begin(), m_aElements.end(), [pWindowName](const BaseElemGUI* p) { return str_comp(pWindowName, p->m_pWindow->GetWindowName()) == 0; });
	if(pElemGUI != m_aElements.end())
	{
		EnableWindow((*pElemGUI));
		return;
	}
	BaseElemGUI* pElement = new BaseElemGUI;
	EnableWindow(pElement);
	m_aElements.push_back(pElement);
}

void CElementsGUI::CallbackRenderInfoWindow(const CUIRect& pWindowRect, CWindowUI& pCurrentWindow)
{
	const auto pElemGUI = std::find_if(m_aElements.begin(), m_aElements.end(), [&pCurrentWindow](const BaseElemGUI* p) { return (*p->m_pWindow == pCurrentWindow); });

	CUIRect Label, ButtonAccept;
	pWindowRect.Margin(s_InformationBoxLabelSpace, &Label);
	Label.HSplitBottom(24.0f, &Label, &ButtonAccept);
	(*pElemGUI)->m_Cursor.MoveTo(Label.x, Label.y);
	TextRender()->DrawTextPlain(&(*pElemGUI)->m_Cursor);

	static CMenus::CButtonContainer s_ButtonAccept;
	if(m_pMenus->DoButton_Menu(&s_ButtonAccept, "Ok", false, &ButtonAccept, nullptr, CUI::CORNER_ALL, 8.0f))
		pCurrentWindow.Close();
}

/* =====================================================================
 * Popup	Box		|	 GUI
 * ===================================================================== */
void CElementsGUI::CreatePopupBox(const char* pWindowName, CWindowUI* pDependentWindow, float Width, const char* pMessage, PopupWindowCallback Callback, bool* pRequires)
{
	const CUIRect* pScreen = UI()->Screen();
	Graphics()->MapScreen(pScreen->x, pScreen->y, pScreen->w, pScreen->h);

	auto EnableWindow = [=](PopupElemGUI* m_pElemGUI)
	{
		const float FontSize = 10.0f;
		m_pElemGUI->m_Cursor.Reset();
		m_pElemGUI->m_Cursor.m_Flags = TEXTFLAG_ALLOW_NEWLINE;
		m_pElemGUI->m_Cursor.m_FontSize = FontSize;
		m_pElemGUI->m_Cursor.m_MaxWidth = Width - (s_InformationBoxLabelSpace * 2.0f) - 1.0f;
		m_pElemGUI->m_Cursor.m_MaxLines = -1;
		TextRender()->TextDeferred(&m_pElemGUI->m_Cursor, pMessage, -1);
		m_pElemGUI->m_pCallback = Callback;
		m_pElemGUI->m_pWindow = UI()->CreateWindow(pWindowName, vec2(0, 0), pDependentWindow, pRequires);
		m_pElemGUI->m_pWindow->Register(WINREGISTER(&CElementsGUI::CallbackRenderGuiPopupBox, this));
		m_pElemGUI->m_pWindow->Init(vec2(Width, 80.0f + ((float)m_pElemGUI->m_Cursor.LineCount() * FontSize)), pDependentWindow, pRequires);
		m_pElemGUI->m_pWindow->Open();
	};

	const auto pElemGUI = std::find_if(m_aElements.begin(), m_aElements.end(), [pWindowName](const BaseElemGUI* p) { return str_comp(pWindowName, p->m_pWindow->GetWindowName()) == 0; });
	if(pElemGUI != m_aElements.end())
	{
		EnableWindow((PopupElemGUI*)(*pElemGUI));
		return;
	}
	PopupElemGUI* pElement = new PopupElemGUI;
	EnableWindow(pElement);
	m_aElements.push_back(pElement);
}


void CElementsGUI::CallbackRenderGuiPopupBox(const CUIRect& pWindowRect, CWindowUI& pCurrentWindow)
{
	const auto pElemGUI = std::find_if(m_aElements.begin(), m_aElements.end(), [&pCurrentWindow](const BaseElemGUI* p) { return (*p->m_pWindow == pCurrentWindow); });
	PopupElemGUI* pElemPopup = (PopupElemGUI*)(*pElemGUI);

	CUIRect Label, ButtonAccept, ButtonDeny;
	pWindowRect.Margin(s_InformationBoxLabelSpace, &Label);
	Label.HSplitBottom(18.0f, &Label, &ButtonAccept);
	ButtonAccept.VSplitLeft(Label.w / 2.0f, &ButtonDeny, &ButtonAccept);
	(*pElemGUI)->m_Cursor.MoveTo(Label.x, Label.y);
	TextRender()->DrawTextPlain(&(*pElemGUI)->m_Cursor);

	ButtonAccept.VMargin(5.0f, &ButtonAccept);
	static CMenus::CButtonContainer s_ButtonAccept;
	if(m_pMenus->DoButton_Menu(&s_ButtonAccept, "Yes", false, &ButtonAccept, nullptr, CUI::CORNER_ALL, 8.0f))
	{
		if(pElemPopup->m_pCallback)
			pElemPopup->m_pCallback(&pCurrentWindow, true);
		pCurrentWindow.Close();
	}
	ButtonDeny.VMargin(5.0f, &ButtonDeny);
	static CMenus::CButtonContainer s_ButtonDeny;
	if(m_pMenus->DoButton_Menu(&s_ButtonDeny, "No", false, &ButtonDeny, nullptr, CUI::CORNER_ALL, 8.0f))
	{
		if(pElemPopup->m_pCallback)
			pElemPopup->m_pCallback(&pCurrentWindow, false);
		pCurrentWindow.Close();
	}
}