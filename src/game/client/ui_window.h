/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#ifndef GAME_CLIENT_UI_WINDOW_H
#define GAME_CLIENT_UI_WINDOW_H

#include "ui.h"

#include <functional>

#define DEFAULT_BACKGROUND_WINDOW_SHANDOW vec4(0.3f, 0.3f, 0.3f, 0.95f)
#define DEFAULT_BACKGROUND_WINDOW_COLOR vec4(0.085f, 0.085f, 0.085f, 0.50f)
#define WINREGISTER(f, o)  std::bind(f, o, std::placeholders::_1, std::placeholders::_2)

class CWindowUI
{
	friend class CUI;
	friend class CWindowsRender;

	static CUI* m_pUI;
	static class CRenderTools* m_pRenderTools;

	static CWindowUI* ms_pWindowHelper;
	using RenderWindowCallback = std::function<void(const CUIRect&, CWindowUI&)>;
	RenderWindowCallback m_pCallback;
	RenderWindowCallback m_pCallbackHelp;

	static std::vector<CWindowUI*> ms_aWindows;
	bool* m_pRenderDependence;

	bool m_Openned;
	vec4 m_BackgroundColor;
	char m_aWindowName[128];
	char m_aWindowDependentName[128];
	CUIRect m_WindowRect;
	CUIRect m_WindowBordure;
	CUIRect m_WindowRectReserve;

	int m_WindowFlags;
	bool m_WindowMinimize;
	bool m_WindowMoving;

	CWindowUI() = default;
	CWindowUI(const char* pWindowName, vec2 WindowSize, CWindowUI* pWindowDependent = nullptr, bool* pRenderDependence = nullptr, int WindowFlags = CUI::WINDOWFLAG_ALL);

	bool IsRenderAllowed() const { return m_Openned && m_pCallback && (m_pRenderDependence == nullptr || (m_pRenderDependence && *m_pRenderDependence == true)); }

	void RenderWindowWithoutBordure();
	void RenderDefaultWindow();
	void Render();

public:
	CWindowUI(const CWindowUI& pWindow) = delete;

	/*
		Operator ==
	 */
	bool operator==(const CWindowUI& p) const { return str_comp(m_aWindowName, p.m_aWindowName) == 0; }
	
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
		Function: RegisterHelpPage -> void
			- Registers the callback function for window information.
		Parameters:
			- pCallback - Callback function.
		Remarks:
			- WINREGISTER(function ref, object) is used to register the callback function.
			If the callback function is set then the button on the bordure will be shown with the '?'.
	*/
	void RegisterHelpPage(RenderWindowCallback pCallback);

	/*
		Function: UpdateDependent -> void
			- Update the window on which to depend.
		Parameters:
			- pDependentWindow - the window on which will depend.
	*/
	void UpdateDependent(CWindowUI* pDependentWindow)
	{
		if(pDependentWindow)
			UpdateDependent(pDependentWindow->GetWindowName());
	}

	/*
		Function: UpdateDependent -> void
			- Update the window on which to depend.
		Parameters:
			- pWindowName - the window on which will depend.
	*/
	void UpdateDependent(const char* pWindowName);

	/*
		Function: UpdateWorkspace -> void
			- Update workspace window.
		Parameters:
			- WorkspaceSize - Window size (Width Height).
			- BackgroundColor - Window background color.
	*/
	void UpdateWorkspace(vec2 WorkspaceSize, vec4 BackgroundColor = vec4())
	{
		CUIRect NewWindowRect = { 0, 0, WorkspaceSize.x, WorkspaceSize.y + m_WindowBordure.h };
		if(NewWindowRect.w != m_WindowRect.w || NewWindowRect.h != m_WindowRect.h)
		{
			m_pUI->MouseRectLimitMapScreen(&NewWindowRect, 6.0f, CUI::RECTLIMITSCREEN_UP | CUI::RECTLIMITSCREEN_ALIGN_CENTER_X);
			m_WindowRect = NewWindowRect;
			m_WindowRectReserve = NewWindowRect;
		}
		if(BackgroundColor.a > 0.0f)
			m_BackgroundColor = BackgroundColor;
	}

private:
	static CWindowUI* GetActiveWindow();
};


#endif
