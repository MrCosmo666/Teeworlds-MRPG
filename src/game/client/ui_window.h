/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#ifndef GAME_CLIENT_UI_WINDOW_H
#define GAME_CLIENT_UI_WINDOW_H

#include "ui.h"

#include <functional>

#define WINREGISTER(f, o)  std::bind(f, o, std::placeholders::_1, std::placeholders::_2)

class CWindowUI
{
	friend class CUI;
	friend class CWindowsRender;

	static CUI* m_pUI;
	static class CRenderTools* m_pRenderTools;

	typedef std::function<void(const CUIRect&, CWindowUI&)> RenderWindowCallback;
	RenderWindowCallback m_pCallback;

	static CWindowUI* ms_pWindowHelper;
	RenderWindowCallback m_pCallbackHelp;

	static std::vector<CWindowUI*> ms_aWindows;
	bool* m_pRenderDependence;

	bool m_Openned;
	vec4 m_HighlightColor;
	char m_aWindowName[128];
	char m_aWindowDependentName[128];
	CUIRect m_WindowRect;
	CUIRect m_WindowBordure;
	CUIRect m_WindowRectProtected;

	int m_WindowFlags;
	bool m_WindowMinimize;
	bool m_WindowMoving;

	CWindowUI() = default;
	CWindowUI(const char* pWindowName, vec2 WindowSize, CWindowUI* pWindowDependent = nullptr, bool* pRenderDependence = nullptr, int WindowFlags = CUI::WINDOWFLAG_ALL);

	bool IsRenderAllowed() const { return m_Openned && m_pCallback && (m_pRenderDependence == nullptr || (m_pRenderDependence && *m_pRenderDependence == true)); }

	void RenderHighlightArea(const CUIRect& AreaRect) const;
	void RenderWindowWithoutBordure();
	void RenderDefaultWindow();
	void Render();

public:
	CWindowUI(const CWindowUI& pWindow) = delete;

	/*
		Static function: InitComponents -> void
			- Initializes components.
	*/
	static void InitComponents(class CUI* pUI, class CRenderTools* pRenderTools)
	{
		m_pUI = pUI;
		m_pRenderTools = pRenderTools;
		if(!ms_pWindowHelper)
			ms_pWindowHelper = CUI::CreateWindow("Help", vec2(0, 0), nullptr, nullptr, CUI::WINDOWFLAG_CLOSE);
	}

	/*
		Function: IsOpenned -> bool
			- Returns the window status.
	*/
	bool IsOpenned() const;

	/*
		Function: IsActive -> bool
			- Returns whether the window is active.
	*/
	bool IsActive() const;

	/*
		Function: GetRect -> const CUIRect&
			- Returns the window area.
	*/
	const CUIRect& GetRect() const;

	/*
		Function: GetWindowName -> const char
			- Returns the window name.
	*/
	const char* GetWindowName() const { return m_aWindowName; }

	/*
		Function: SetWorkspaceSize -> void
			- Set workspace window size.
		Parameters:
			- WorkspaceSize - Window size (Width Height).
	*/
	void SetWorkspaceSize(vec2 WorkspaceSize)
	{
		CUIRect NewWindowRect = { 0, 0, WorkspaceSize.x, WorkspaceSize.y + m_WindowBordure.h };
		if(NewWindowRect.w != m_WindowRect.w || NewWindowRect.h != m_WindowRect.h)
		{
			m_pUI->MouseRectLimitMapScreen(&NewWindowRect, 6.0f, CUI::RECTLIMITSCREEN_UP | CUI::RECTLIMITSCREEN_ALIGN_CENTER_X);
			m_WindowRect = NewWindowRect;
			m_WindowRectProtected = NewWindowRect;
		}
	}

	/*
		Function: Open -> void
			- Opens a window.
		Remarks:
			- If window already openned, it will be reopened.
	*/
	void Open();

	/*
		Function: Close -> void
			- Closes the window.
		Remarks:
			- Including all windows dependent on this window.
	*/
	void Close();

	/*
		Function: Reverse -> void
			- If the window is openned it will be closed.
			- If the window is closed it will be open.
	*/
	void Reverse();

	/*
		Function: MinimizeWindow -> void
			- Minimize window or maximize recursive.
	*/
	void MinimizeWindow();

	/*
		Function: Init -> void
			- Initializes the window.
		Parameters:
			- WindowSize - Window size (Width Height).
			- pWindowDependent - Window will depend on transferred pointer.
			- pRenderDependence - If the pointer is true or nullptr the windows will render.
	*/
	void Init(vec2 WindowSize, CWindowUI* pWindowDependent = nullptr, bool* pRenderDependence = nullptr);

	/*
		Function: Register -> void
			- Registers the callback function for render.
		Parameters:
			- pCallback - Callback function.
		Remarks:
			- WINREGISTER(function ref, object) is used to register the callback function.
	*/
	void Register(RenderWindowCallback pCallback);

	/*
		Function: RegisterHelp -> void
			- Registers the callback function for window information.
		Parameters:
			- pCallback - Callback function.
		Remarks:
			- WINREGISTER(function ref, object) is used to register the callback function.
			If the callback function is set then the button on the bordure will be shown with the '?'.
	*/
	void RegisterHelp(RenderWindowCallback pCallback);

	/*
		Function: HighlightEnable -> void
			- Turns on the window light.
		Parameters:
			- Color - vector4 Color.
			- DependentToo - Including dependent windows.
	*/
	void HighlightEnable(vec4 Color, bool DependentToo = false);

	/*
		Function: HighlightDisable -> void
			- Disables window illumination including dependent windows.
	*/
	void HighlightDisable();

	/*
		Function: SetDependent -> void
			- Sets the window on which to depend.
		Parameters:
			- pDependentWindow - the window on which will depend.
	*/
	void SetDependent(CWindowUI* pDependentWindow)
	{
		if(pDependentWindow)
			SetDependent(pDependentWindow->GetWindowName());
	}

	/*
		Function: SetDependent -> void
			- Sets the window on which to depend.
		Parameters:
			- pWindowName - the window on which will depend.
	*/
	void SetDependent(const char* pWindowName);

private:
	static CWindowUI* GetActiveWindow();
};


#endif
