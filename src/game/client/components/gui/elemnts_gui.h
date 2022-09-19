#ifndef GAME_CLIENT_ELEMENTS_GUI_H
#define GAME_CLIENT_ELEMENTS_GUI_H

#include <engine/textrender.h>
#include <game/client/ui_window.h>

#include <functional>

#define POPUP_REGISTER(f, o)  std::bind(f, o, std::placeholders::_1, std::placeholders::_2)
using PopupWindowCallback = std::function<void(CWindowUI*, bool)>;

struct BaseElemGUI
{
	CTextCursor m_Cursor;
	CWindowUI* m_pWindow;
};

struct PopupElemGUI : BaseElemGUI
{
	PopupWindowCallback m_pCallback;
};

class CElementsGUI
{
	static class IGraphics* m_pGraphics;
	static class ITextRender* m_pTextRender;
	static class CUI* m_pUI;
	static class CMenus* m_pMenus;

	static class CUI* UI() { return m_pUI; }
	static class IGraphics* Graphics() { return m_pGraphics; }
	static class ITextRender* TextRender() { return m_pTextRender; }
	
	std::vector<BaseElemGUI*> m_aElements;
	
public:
	~CElementsGUI();
	
	static void Init(IGraphics* pGraphics, ITextRender* pTextRender, CUI* pUI, CMenus* pMenus)
	{
		m_pGraphics = pGraphics;
		m_pTextRender = pTextRender;
		m_pUI = pUI;
		m_pMenus = pMenus;
	}
	
	BaseElemGUI* CreateInformationBoxElement(float Width, const char* pMessage);
	void CreateInformationBox(const char* pWindowName, float Width, const char* pMessage, CWindowUI* pWindow);
	void CreateInformationBox(const char* pWindowName, float Width, const char* pMessage, bool* pDepent = nullptr);

	PopupElemGUI* CreatePopupElement(float Width, const char* pMessage, PopupWindowCallback Callback);
	void CreatePopupBox(const char* pWindowName, float Width, const char* pMessage, PopupWindowCallback Callback, CWindowUI* pWindow);
	void CreatePopupBox(const char* pWindowName, float Width, const char* pMessage, PopupWindowCallback Callback, bool* pDepent = nullptr);

private:
	void CallbackRenderInfoWindow(const CUIRect& pWindowRect, CWindowUI* pCurrentWindow);
	void CallbackRenderGuiPopupBox(const CUIRect& pWindowRect, CWindowUI* pCurrentWindow);
};

#endif