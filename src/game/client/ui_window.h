/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#ifndef GAME_CLIENT_UI_WINDOW_H
#define GAME_CLIENT_UI_WINDOW_H

#include "ui.h"

#include <functional>

#define WINDOW_REGISTER(f, o)  std::bind(f, o, std::placeholders::_1, std::placeholders::_2)

class CWindowUI
{
	friend class CUI;
	friend class CWindowsRender;

	static CUI* m_pUI;
	static class CRenderTools* m_pRenderTools;

	typedef std::function<void(const CUIRect&, CWindowUI&)> RenderWindowCallback;
	RenderWindowCallback m_pCallback;

	static std::vector<CWindowUI*> ms_aWindows;
	bool* m_pRenderDependence;

	bool m_Openned;
	vec4 m_HighlightColor;
	char m_aWindowName[128];
	char m_aWindowDependentName[128];
	CUIRect m_WindowRect;
	CUIRect m_WindowRectProtected;

	int m_WindowFlags;
	bool m_WindowMinimize;
	bool m_WindowMoving;

	CWindowUI() = default;
	CWindowUI(const char* pWindowName, vec2 WindowSize, CWindowUI* pWindowDependent = nullptr, bool* pRenderDependence = nullptr, int WindowFlags = CUI::WINDOWFLAG_ALL);

public:
	CWindowUI(const CWindowUI& pWindow) = delete;

	bool IsOpenned() const;
	bool IsActive() const;
	const CUIRect& GetRect() const;
	const char* GetWindowName() const { return m_aWindowName; }

	void Open();
	void Close();
	void Reverse();

	void Init(vec2 WindowSize, CWindowUI* pWindowDependent = nullptr, bool* pRenderDependence = nullptr);
	void RegisterCallback(RenderWindowCallback pCallback);

	void HighlightEnable(vec4 Color, bool DaughtersToo = false);
	void HighlightDisable();

	void SetDependent(CWindowUI* pDependentWindow)
	{
		if(pDependentWindow)
			SetDependent(pDependentWindow->GetWindowName());
	}
	void SetDependent(const char* pWindowName);

private:
	void Render();
	static CWindowUI* GetActiveWindow()
	{
		if(!CWindowUI::ms_aWindows.empty())
			return CWindowUI::ms_aWindows.front();
		return nullptr;
	}
};


#endif
