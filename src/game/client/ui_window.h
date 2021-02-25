/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#ifndef GAME_CLIENT_UI_WINDOW_H
#define GAME_CLIENT_UI_WINDOW_H

#include <vector>
#include <functional>
#include "ui.h"

class CWindowUI
{
	friend class CUI;
	typedef std::function<void(CUIRect&, CWindowUI&)> RenderWindowCallback;
	static std::vector<CWindowUI*> ms_aWindows;
	RenderWindowCallback m_pCallback;

	bool m_WindowMove;
	float m_WindowSkipX;
	float m_WindowSkipY;

	char m_aWindowName[128];
	bool m_Openned;
	CUIRect m_WindowRect;

public:
	CWindowUI() = default;
	CWindowUI(const CWindowUI& pWindow) = default;

	static CUI* m_pUI;
	static class CRenderTools* m_pRenderTools;
	static CWindowUI* GetActiveWindow()
	{
		if(!CWindowUI::ms_aWindows.empty())
			return CWindowUI::ms_aWindows.front();
		return nullptr;
	}

	void Init(const char* pWindowName, CUIRect WindowRect);
	CUIRect& GetRect();
	bool IsOpenned() const;
	bool IsActive() const;
	const char* GetWindowName() const { return m_aWindowName; }

	void Open() { m_Openned = true; }
	void Close() { m_Openned = false; }
	void OnRenderWindow(RenderWindowCallback pCallback);

private:
	void Render();

	// TODO optimize it / This is designed to make it very easy to create windows
	CWindowUI* GetWindow(const char* pWindowName) const;
};


#endif
