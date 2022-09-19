/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#ifndef GAME_CLIENT_UI_WINDOW_H
#define GAME_CLIENT_UI_WINDOW_H

#include "ui.h"

#include <functional>

#define DEFAULT_WORKSPACE_SIZE -1.f
#define DEFAULT_BACKGROUND_WINDOW_SHANDOW vec4(0.4f, 0.4f, 0.4f, 0.95f)
#define DEFAULT_BACKGROUND_WINDOW_COLOR vec4(0.085f, 0.085f, 0.085f, 0.50f)
#define WINREGISTER(f, o)  std::bind(f, o, std::placeholders::_1, std::placeholders::_2)

class CWindowUI
{
	static std::vector<CWindowUI*> ms_aWindows;
	std::vector<CWindowUI*> m_paChildrenWindows;

	friend class CUI;
	friend class CWindowsRender;

	static CUI* m_pUI;
	static class CRenderTools* m_pRenderTools;

	static CWindowUI* ms_pWindowHelper;
	using RenderWindowCallback = std::function<void(const CUIRect&, CWindowUI*)>;
	RenderWindowCallback m_pCallback;
	RenderWindowCallback m_pCallbackHelp;

	bool m_Openned;
	vec4 m_BackgroundColor;
	char m_aWindowName[128];
	CUIRect m_WindowRect;
	CUIRect m_WindowBordure;
	CUIRect m_WindowRectReserve;
	CUIRect m_DefaultWindowRect;
	bool* m_pRenderDependence;

	int m_WindowFlags;
	bool m_WindowMinimize;
	bool m_WindowMoving;

	CWindowUI() = default;
	CWindowUI(const char* pWindowName, vec2 WindowSize, bool* pRenderDependence = nullptr, int WindowFlags = CUI::WINDOWFLAG_ALL);

	bool IsRenderAllowed() const { return m_Openned && m_pCallback && (m_pRenderDependence == nullptr || (m_pRenderDependence && *m_pRenderDependence == true)); }

	void RenderWindowWithoutBordure();
	void RenderDefaultWindow();
	void Render();

public:
	CWindowUI(const CWindowUI& pWindow) = delete;

	/*
	 * Add child for window
	 */
	CWindowUI* AddChild(CWindowUI* pElem)
	{
		const auto pSearch = std::find_if(m_paChildrenWindows.begin(), m_paChildrenWindows.end(), [&pElem](const CWindowUI* pItem) { return str_comp(pElem->GetWindowName(), pItem->GetWindowName()) == 0;  });
		if(pSearch == m_paChildrenWindows.end())
			m_paChildrenWindows.emplace_back(pElem);
		return pElem;
	}

	CWindowUI* AddChild(const char* pChildName, vec2 WindowSize, int WindowFlags = CUI::WINDOWFLAG_ALL)
	{
		char aChildNameBuf[64];
		GetFullChildWindowName(pChildName, aChildNameBuf, sizeof(aChildNameBuf));
		const auto pSearch = std::find_if(ms_aWindows.begin(), ms_aWindows.end(), [&aChildNameBuf](const CWindowUI* pItem) { return str_comp(aChildNameBuf, pItem->GetWindowName()) == 0;  });
		if(pSearch != ms_aWindows.end())
			return AddChild((*pSearch));
		
		CWindowUI* pElem = m_pUI->CreateWindow(aChildNameBuf, WindowSize, m_pRenderDependence, WindowFlags);
		return AddChild(pElem);
	}

	/*
	 * Get child window
	 */
	CWindowUI* GetChild(const char* pChildName)
	{
		char aChildNameBuf[64];
		GetFullChildWindowName(pChildName, aChildNameBuf, sizeof(aChildNameBuf));
		const auto pSearch = std::find_if(m_paChildrenWindows.begin(), m_paChildrenWindows.end(), [aChildNameBuf](const CWindowUI* pItem) { return str_comp(aChildNameBuf, pItem->GetWindowName()) == 0;  });
		dbg_assert(pSearch != m_paChildrenWindows.end(), "window does not exist");
		return (*pSearch);
	}

	/*
	 * Get fully child window name
	 */
	void GetFullChildWindowName(const char* pChildName, char* aBuf, int Size)
	{
		str_format(aBuf, Size, "%s : %s", m_aWindowName, pChildName);
	}

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
			ms_pWindowHelper = CUI::CreateWindow("Help", vec2(0, 0), nullptr, CUI::WINDOWFLAG_CLOSE);
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
	void Init(vec2 WindowSize, bool* pRenderDependence = nullptr);

	/*
		Function: Register -> void
			- Registers the callback function for render.
		Parameters:
			- pCallback - Callback function.
			- pCallbackHelp - Callback helppage function.
		Remarks:
			- WINREGISTER(function ref, object) is used to register the callback function.
	*/
	void Register(RenderWindowCallback pCallback, RenderWindowCallback pCallbackHelp = nullptr);

	/*
		Function: SetDefaultWorkspace -> void
			- Set default workspace rect.
	*/
	void SetDefaultWorkspace()
	{
		m_WindowRect.w = m_DefaultWindowRect.w;
		m_WindowRect.h = m_DefaultWindowRect.h;
	}

	/*
		Function: SetBackgroundColor -> void
			- Set background color.
		Parameters:
			- Color - vector4 color.
	*/
	void SetBackgroundColor(vec4 Color)
	{
		if(Color.a > 0.0f)
			m_BackgroundColor = Color;
	}
	
	/*
		Function: MarginWorkspace -> void
			- Update workspace window.
	*/
	void MarginWorkspace(float Width, float Height)
	{
		SetWorkspace({ m_WindowRect.w - Width, m_WindowRect.h - Height });
		
	}

	/*
		Function: UpdateWorkspace -> void
			- Update workspace window.
		Parameters:
			- WorkspaceSize - Window size (Width Height).
	*/
	void SetWorkspace(float WorkspaceX = DEFAULT_WORKSPACE_SIZE, float WorkspaceY = DEFAULT_WORKSPACE_SIZE)
	{
		SetWorkspace({WorkspaceX, WorkspaceY});
	}

	void SetWorkspace(vec2 WorkspaceSize = vec2(DEFAULT_WORKSPACE_SIZE, DEFAULT_WORKSPACE_SIZE))
	{
		float Width = WorkspaceSize.x == DEFAULT_WORKSPACE_SIZE ? m_WindowRect.w : WorkspaceSize.x;
		float Height = WorkspaceSize.y == DEFAULT_WORKSPACE_SIZE ? m_WindowRect.h : WorkspaceSize.y;

		CUIRect NewWindowRect = { 0, 0, Width, Height };
		if(NewWindowRect.w != m_WindowRect.w || NewWindowRect.h != m_WindowRect.h)
		{
			m_pUI->MouseRectLimitMapScreen(&NewWindowRect, 6.0f, CUI::RECTLIMITSCREEN_UP | CUI::RECTLIMITSCREEN_ALIGN_CENTER_X);
			m_WindowRect.w = NewWindowRect.w;
			m_WindowRect.h = NewWindowRect.h;
			m_WindowRectReserve.w = NewWindowRect.w;
			m_WindowRectReserve.h = NewWindowRect.h;
		}
	}

private:
	static void SetActiveWindow(CWindowUI* pWindow);
	static CWindowUI* GetActiveWindow();
};


#endif
