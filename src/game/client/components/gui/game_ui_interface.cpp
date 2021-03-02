#include <engine/keys.h>
#include <engine/console.h>
#include <engine/shared/config.h>
#include <generated/client_data.h>

#include <game/client/components/console.h>
#include <game/client/ui_window.h>

#include "game_ui_interface.h"

void CUIGameInterface::OnRender()
{
	if(!m_ActiveHUD)
		return;

	const CUIRect* pScreen = UI()->Screen();
	Graphics()->MapScreen(pScreen->x, pScreen->y, pScreen->w, pScreen->h);

	// maillist window
	CWindowUI m_WindowMails;
	m_WindowMails.Init("Mail list", { 150, 150, 400, 300 });
	m_WindowMails.OnRenderWindow([&](const CUIRect& pWindowRect, CWindowUI& pCurrentWindow)
	{

	});

	// quest book window
	CWindowUI m_WindowQuestBook;
	m_WindowQuestBook.Init("Quest book", { 150, 150, 400, 300 });
	m_WindowQuestBook.OnRenderWindow([&](const CUIRect& pWindowRect, CWindowUI& pCurrentWindow)
	{

	});

	// icons
	CUIRect IconView;
	pScreen->VSplitRight(60.0f, 0, &IconView);
	IconView.HSplitBottom(400.0f, 0, &IconView);
	RenderIconLogic(&IconView, &m_WindowMails, SPRITE_HUD_ICON_MAIL);
	
	IconView.HMargin(50.0f, &IconView);
	RenderIconLogic(&IconView, &m_WindowQuestBook, SPRITE_HUD_ICON_QUEST);

	// mouse
	UI()->Update(m_MousePos.x, m_MousePos.y, m_MousePos.x * 3.0f, m_MousePos.y * 3.0f);
	Graphics()->TextureSet(g_pData->m_aImages[IMAGE_CURSOR].m_Id);
	Graphics()->WrapClamp();
	Graphics()->QuadsBegin();
	IGraphics::CQuadItem QuadItem(UI()->MouseX(), UI()->MouseY(), 24, 24);
	Graphics()->QuadsDrawTL(&QuadItem, 1);
	Graphics()->QuadsEnd();
	Graphics()->WrapNormal();
}

void CUIGameInterface::OnConsoleInit()
{
	Console()->Register("toggle_game_hud_mrpg", "", CFGFLAG_CLIENT, ConToggleGameHUDMRPG, this, "Toggle game hud mrpg");
}

bool CUIGameInterface::OnInput(IInput::CEvent Event)
{
	if(m_pClient->m_pGameConsole->IsConsoleActive() || !m_ActiveHUD)
		return false;

	if(Event.m_Key == KEY_MOUSE_1)
	{
		return true;
	}
	else if(Event.m_Key == KEY_MOUSE_2)
	{
		return true;
	}
	else if(Event.m_Key == KEY_MOUSE_WHEEL_UP)
	{
		return true;
	}
	else if(Event.m_Key == KEY_MOUSE_WHEEL_DOWN)
	{
		return true;
	}
	return false;
}

void CUIGameInterface::OnStateChange(int NewState, int OldState)
{
	if(NewState != IClient::STATE_ONLINE)
		m_ActiveHUD = false;
}

bool CUIGameInterface::OnCursorMove(float x, float y, int CursorType)
{
	if(!m_ActiveHUD)
		return false;

	const CUIRect* pScreen = UI()->Screen();
	UI()->ConvertCursorMove(&x, &y, CursorType);
	m_MousePos += vec2(x, y);

	if(m_MousePos.x < 0) m_MousePos.x = 0;
	if(m_MousePos.y < 0) m_MousePos.y = 0;
	if(m_MousePos.x > pScreen->w) m_MousePos.x = pScreen->w;
	if(m_MousePos.y > pScreen->h) m_MousePos.y = pScreen->h;
	return true;
}

void CUIGameInterface::ConToggleGameHUDMRPG(IConsole::IResult* pResult, void* pUser)
{
	CUIGameInterface* pGameHUD = static_cast<CUIGameInterface*>(pUser);
	if(pGameHUD->Client()->State() != IClient::STATE_ONLINE)
		return;

	pGameHUD->m_ActiveHUD ^= true;
}

void CUIGameInterface::RenderIconLogic(CUIRect* pRect, CWindowUI *pWindow, int SpriteID)
{
	CUIRect Icon = { pRect->x, pRect->y, 48.0f, 48.0f };
	int MoveLogic = UI()->DoMouseEventLogic(&Icon, KEY_MOUSE_1);
	if(MoveLogic & CUI::CButtonLogicEvent::EVENT_PRESS)
		pWindow->CloseOpen();

	float Size = MoveLogic & CUI::CButtonLogicEvent::EVENT_HOVERED ? 56.0f : 48.0f;
	Graphics()->TextureSet(g_pData->m_aImages[IMAGE_MMOGAMEHUD].m_Id);
	Graphics()->WrapClamp();
	Graphics()->QuadsBegin();
	RenderTools()->SelectSprite(SpriteID);
	IGraphics::CQuadItem MainIcon(pRect->x, pRect->y, Size, Size);
	Graphics()->QuadsDrawTL(&MainIcon, 1);
	Graphics()->QuadsEnd();
	Graphics()->WrapNormal();

}