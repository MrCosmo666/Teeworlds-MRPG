/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#ifndef GAME_CLIENT_UI_WINDOW_H
#define GAME_CLIENT_UI_WINDOW_H

#include "ui.h"

#include <functional>

enum CWindowFlags
{
	WINDOW_MINIMIZE = 1 << 0,
	WINDOW_CLOSE = 1 << 1,
	WINDOW_ALL = WINDOW_MINIMIZE | WINDOW_CLOSE
};

class CWindowUI
{
	friend class CUI;
	friend class CWindowsRender;

	static CUI* m_pUI;
	static class CRenderTools* m_pRenderTools;
	typedef std::function<void(const CUIRect&, CWindowUI&)> RenderWindowCallback;
	static std::vector<CWindowUI*> ms_aWindows;
	RenderWindowCallback m_pCallback;
	int m_SkippedRenderFrames;

	bool m_Openned;
	vec4 m_HighlightColor;
	char m_aWindowName[128];
	CUIRect m_WindowRect;
	CUIRect m_WindowRectGuardian;
	std::vector<CWindowUI> m_DaughtersWindows;

	int m_WindowFlags;
	bool m_WindowHidden;
	bool m_WindowMoved;
	float m_WindowSkipX;
	float m_WindowSkipY;

public:
	CWindowUI() = default;
	CWindowUI(const CWindowUI& pWindow) = default;
	CWindowUI(const char* pWindowName, CUIRect WindowRect, bool DefaultClose = false, std::vector<CWindowUI> DaughtersWindows = {}, int WindowFlags = CWindowFlags::WINDOW_ALL);

	bool IsOpenned() const;
	bool IsActive() const;
	const CUIRect& GetRect();
	const char* GetWindowName() const { return m_aWindowName; }
	std::vector<CWindowUI> GetDaughtersWindows() const { return m_DaughtersWindows; }

	void Open();
	void Close() const;
	void CloseOpen();
	void Init(const char* pWindowName, CUIRect WindowRect, bool DefaultClose = false, std::vector<CWindowUI> DaughtersWindows = {}, int WindowFlags = CWindowFlags::WINDOW_ALL);
	void ReInitRect(CUIRect WindowRect) { m_WindowRect = WindowRect; }
	void OnRenderWindow(RenderWindowCallback pCallback) const;
	void HighlightEnable(vec4 Color, bool DaughtersToo = false) const;
	void HighlightDisable() const;

private:
	void Render();
	static CWindowUI* GetActiveWindow()
	{
		if(!CWindowUI::ms_aWindows.empty())
			return CWindowUI::ms_aWindows.front();
		return nullptr;
	}
	CWindowUI* GetWindow(const char* pWindowName) const;
};


#endif
