#include <engine/keys.h>
#include <engine/console.h>
#include <engine/shared/config.h>
#include <generated/client_data.h>

#include <game/client/components/sounds.h>
#include <game/client/components/menus.h>
#include <game/client/components/console.h>
#include <game/client/ui_window.h>

#include "game_ui_interface.h"

void CUIGameInterface::OnRender()
{
	if(!m_ActiveHUD)
		return;

	const CUIRect* pScreen = UI()->Screen();
	Graphics()->MapScreen(pScreen->x, pScreen->y, pScreen->w, pScreen->h);

	// inbox/mail gui icon
	CUIRect IconView;
	pScreen->VSplitRight(60.0f, 0, &IconView);
	IconView.HSplitBottom(400.0f, 0, &IconView);
	static CMenus::CButtonContainer s_MailListButton;
	CWindowUI WindowMails("Mail list", { 150,150,250,400 }, CWindowFlags::WINDOW_ALL);
	DoIconSelectionWindow(&s_MailListButton, &IconView, &WindowMails, SPRITE_HUD_ICON_MAIL);
	{
		// inbox/mail window
		WindowMails.OnRenderWindow([&](const CUIRect& pWindowRect, CWindowUI& pCurrentWindow)
		{
			CUIRect List = pWindowRect;
			int OldSelectedFont = m_MailboxSelectedOption;
			static CMenus::CListBox s_ListBox;
			s_ListBox.DoHeader(&List, Localize("Option"), 20.0f, 2.0f);
			s_ListBox.DoStart(20.0f, 200, 1, 1, m_MailboxSelectedOption);

			for(int i = 0; i < 200; i++)
			{
				static int s_DefaultEntitiyId;
				CMenus::CListboxItem Item = s_ListBox.DoNextItem(i > 0 ? (void*)i : (void*)&s_DefaultEntitiyId, m_MailboxSelectedOption == i);
				if(Item.m_Visible)
				{
					float IconSize = 21.0f;
					IconSize = 18.0f;
					Item.m_Rect.y += 1.25f;
					Item.m_Rect.x += 2.0f;

					bool Icon = m_pClient->m_pMenus->DoItemIcon("ignot_r", { Item.m_Rect.x + 2.0f, Item.m_Rect.y, Item.m_Rect.w, Item.m_Rect.h, }, IconSize);
					Item.m_Rect.VMargin((Icon ? 25.0f : 5.0f), &Item.m_Rect);
					Item.m_Rect.y += 2.0f;

					UI()->DoLabel(&Item.m_Rect, "Suka bleat", 16.0f, CUI::ALIGN_LEFT);
				}
			}
			m_MailboxSelectedOption = s_ListBox.DoEnd();
		});
	}

	// questing gui icon
	IconView.HMargin(52.0f, &IconView);
	static CMenus::CButtonContainer s_QuestingListButton;
	CWindowUI WindowQuestBook("Quest book", { 150, 150, 300, 250 }, CWindowFlags::WINDOW_ALL);
	DoIconSelectionWindow(&s_QuestingListButton, &IconView, &WindowQuestBook, SPRITE_HUD_ICON_QUEST);
	{
		// questing window
		WindowQuestBook.OnRenderWindow([&](const CUIRect& pWindowRect, CWindowUI& pCurrentWindow)
		{

		});
	}

	m_pTestWindow.Init("test", { 150, 150, 300, 250 }, CWindowFlags::WINDOW_ALL);
	m_pTestWindow.OnRenderWindow([&](const CUIRect& pWindowRect, CWindowUI& pCurrentWindow)
	{

	});

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

	if(Event.m_Flags & IInput::FLAG_PRESS)
	{
		if(Event.m_Key == KEY_ESCAPE)
		{
			m_ActiveHUD = false;
			return true;
		}
	}

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

void CUIGameInterface::DoIconSelectionWindow(CMenus::CButtonContainer* pBC, CUIRect* pRect, CWindowUI *pWindow, int SpriteID)
{
	if(!pBC || !pWindow)
		return;

	if(UI()->HotItem() == pBC->GetID())
	{
		vec4 ColorHighlight = vec4(1.0f, 0.55f, 0.0f, 0.4f);
		pWindow->HighlightEnable(ColorHighlight);
	}
	else
	{
		pWindow->HighlightDisable();
	}

	bool WindowOpenned = pWindow->IsOpenned();
	const float FadeVal = WindowOpenned ? max(pBC->GetFade() + 0.5f, 1.0f) : pBC->GetFade();
	float Size = 42.0f + (FadeVal * 6.0f);
	CUIRect Icon = { pRect->x, pRect->y, Size, Size };
	RenderTools()->DrawUIRect(&Icon, vec4(0.0f + FadeVal, 0.0f + FadeVal, 0.0f + FadeVal, 0.3f), CUI::CORNER_ALL, 5.0f);

	Graphics()->TextureSet(g_pData->m_aImages[IMAGE_MMOGAMEHUD].m_Id);
	Graphics()->WrapClamp();
	Graphics()->QuadsBegin();
	RenderTools()->SelectSprite(SpriteID);
	IGraphics::CQuadItem MainIcon(pRect->x, pRect->y, Size, Size);
	Graphics()->QuadsDrawTL(&MainIcon, 1);
	Graphics()->QuadsEnd();
	Graphics()->WrapNormal();

	const void* pLastActiveItem = UI()->GetActiveItem();
	if(UI()->DoButtonLogic(pBC->GetID(), &Icon))
		pWindow->CloseOpen();

	// UI sounds
	if(g_Config.m_SndEnableUI)
	{
		if(g_Config.m_SndEnableUIHover && UI()->NextHotItem() == pBC->GetID() && UI()->NextHotItem() != UI()->HotItem())
			m_pClient->m_pSounds->Play(CSounds::CHN_GUI, SOUND_BUTTON_HOVER, 1);
		if(UI()->GetActiveItem() == pBC->GetID() && pLastActiveItem != pBC->GetID())
			m_pClient->m_pSounds->Play(CSounds::CHN_GUI, SOUND_BUTTON_CLICK, 0);
	}
}