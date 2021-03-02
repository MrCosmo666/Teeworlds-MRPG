/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#ifndef GAME_CLIENT_UI_WINDOW_H
#define GAME_CLIENT_UI_WINDOW_H

#include <vector>
#include <functional>
#include "ui.h"

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

	bool m_Openned;
	char m_aWindowName[128];
	CUIRect m_WindowRect;
	CUIRect m_WindowRectOld;

	int m_WindowFlags;
	bool m_WindowHidden;
	bool m_WindowMoved;
	float m_WindowSkipX;
	float m_WindowSkipY;

public:
	CWindowUI() = default;
	CWindowUI(const CWindowUI& pWindow) = default;

	bool IsOpenned() const;
	bool IsActive() const;
	const CUIRect& GetRect();
	const char* GetWindowName() const { return m_aWindowName; }

	void Open();
	void Close();
	void CloseOpen();
	void Init(const char* pWindowName, CUIRect WindowRect, int WindowFlags = CWindowFlags::WINDOW_ALL);
	void OnRenderWindow(RenderWindowCallback pCallback);

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
