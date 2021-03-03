#ifndef GAME_CLIENT_COMPONENTS_INTERFACE_H
#define GAME_CLIENT_COMPONENTS_INTERFACE_H

#include <game/client/component.h>

class CUIGameInterface : public CComponent
{
	bool m_ActiveHUD;
	vec2 m_MousePos;
	int m_MailboxSelectedOption;

public:
	virtual void OnRender();
	virtual void OnConsoleInit();
	virtual bool OnInput(IInput::CEvent Event);
	virtual void OnStateChange(int NewState, int OldState);
	virtual bool OnCursorMove(float x, float y, int CursorType);

	static void ConToggleGameHUDMRPG(IConsole::IResult* pResult, void* pUser);

	bool IsActiveHUD() const { return m_ActiveHUD; }

	void DoIconSelectionWindow(CMenus::CButtonContainer* pBC, class CUIRect* pRect, class CWindowUI* pWindow, int SpriteID);
};

#endif