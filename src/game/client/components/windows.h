#ifndef GAME_CLIENT_COMPONENTS_WINDOWS_H
#define GAME_CLIENT_COMPONENTS_WINDOWS_H

#include <game/client/component.h>

/*
	It is important to use this to keep the order of the Renderer.
	So that the windows are above the menu but below everything else
*/

class CWindowsRender : public CComponent
{
public:
	void OnRender() override;

	void CallbackWindowHelper(const CUIRect& pWindowRect, CWindowUI& pCurrentWindow);
};

#endif