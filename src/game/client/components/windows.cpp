#include "windows.h"

#include <game/client/ui_window.h>

/*
	It is important to use this to keep the order of the Renderer.
	So that the windows are above the menu but below everything else
*/

void CWindowsRender::OnRender()
{
	UI()->WindowRender();
	UI()->FinishCheck();
}