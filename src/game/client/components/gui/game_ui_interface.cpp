#include <engine/console.h>
#include <engine/keys.h>
#include <engine/textrender.h>
#include <engine/shared/config.h>
#include <generated/client_data.h>

#include <game/enum_context.h>
#include <game/client/components/console.h>
#include <game/client/components/menus.h>
#include <game/client/components/sounds.h>

#include "game_ui_interface.h"

std::map< int /*itemid*/, CUIGameInterface::CItemDataClientInfo > CUIGameInterface::m_aItemsDataInformation;
std::map< int /*itemid*/, CUIGameInterface::CClientItem > CUIGameInterface::m_aClientItems;

void CUIGameInterface::OnInit()
{
	// gui
	m_WindowInformationBox.Init("Information", { 250, 250, 400, 150 }, true);

	// inbox system
	m_GotNewLetter = false;
	m_LetterActiveSelected = -1;
	m_WindowMailboxLetterSelected.Init("Letter", { 250, 250, 250, 125 }, true);
	m_WindowMailboxLetterSend.Init("Sending a letter", { 150,150,250,200 }, true);
	m_WindowMailboxList.Init("Mailbox list", { 150,150,250,300 }, true, { m_WindowMailboxLetterSelected, m_WindowMailboxLetterSend });

	// questing system
	m_WindowQuestsList.Init("Quest book", { 150, 150, 300, 80 }, true);
}

void CUIGameInterface::OnReset()
{
	m_WindowInformationBox.Close();
	m_WindowMailboxLetterSelected.Close();
	m_WindowMailboxLetterSend.Close();
	m_WindowMailboxList.Close();
	m_WindowQuestsList.Close();
}

void CUIGameInterface::OnRender()
{
	// todo: fix it
	RenderGuiElements();

	if(!m_ActiveHUD)
		return;

	const CUIRect* pScreen = UI()->Screen();
	Graphics()->MapScreen(pScreen->x, pScreen->y, pScreen->w, pScreen->h);

	// inbox/mail gui icon
	CUIRect IconView;
	{
		pScreen->VSplitRight(60.0f, 0, &IconView);
		IconView.HSplitBottom(400.0f, 0, &IconView);
		static CMenus::CButtonContainer s_MailListButton;
		if(DoIconSelectionWindow(&s_MailListButton, &IconView, &m_WindowMailboxList, SPRITE_HUD_ICON_MAIL) && m_WindowMailboxList.IsOpenned())
			RefreshLetters();

		// icon notification about new message
		if(m_GotNewLetter)
		{
			static float NewLetterSize = 16.0f;
			CUIRect NewLetter = { IconView.x, IconView.y, NewLetterSize, NewLetterSize };
			RenderTools()->DrawRoundRect(&NewLetter, vec4(0.5f, 0.5f, 0.7f, 0.5f), 3.0f);
			UI()->DoLabel(&NewLetter, "+", 12.0f, CUI::ALIGN_CENTER);
		}

		RenderInbox();
	}
	// questing gui icon
	{
		IconView.HMargin(52.0f, &IconView);
		static CMenus::CButtonContainer s_QuestingListButton;
		if(DoIconSelectionWindow(&s_QuestingListButton, &IconView, &m_WindowQuestsList, SPRITE_HUD_ICON_QUEST))
		{
		}

		RenderQuests();
	}


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

void CUIGameInterface::OnMessage(int Msg, void* pRawMsg)
{
	if(Client()->State() == IClient::STATE_DEMOPLAYBACK)
		return;

	if(Msg == NETMSGTYPE_SV_SENDMAILINFO)
	{
		CNetMsg_Sv_SendMailInfo* pMsg = (CNetMsg_Sv_SendMailInfo*)pRawMsg;

		CMailboxLetter Letter;
		Letter.m_MailID = pMsg->m_MailID;
		str_copy(Letter.m_aName, pMsg->m_pTitle, sizeof(Letter.m_aName));
		str_copy(Letter.m_aDesc, pMsg->m_pMsg, sizeof(Letter.m_aDesc));
		Letter.m_ItemID = pMsg->m_ItemID;
		Letter.m_Count = pMsg->m_Count;
		Letter.m_Enchant = pMsg->m_EnchantLevel;
		m_aLettersList.push_back(Letter);
	}
	else if(Msg == NETMSGTYPE_SV_SENDGOTNEWMAIL)
	{
		m_GotNewLetter = true;
	}
	else if(Msg == NETMSGTYPE_SV_SENDGUIINFORMATIONBOX)
	{
		CNetMsg_Sv_SendGuiInformationBox* pMsg = (CNetMsg_Sv_SendGuiInformationBox*)pRawMsg;
		CreateInformationBox(pMsg->m_pMsg);
	}
}

void CUIGameInterface::ConToggleGameHUDMRPG(IConsole::IResult* pResult, void* pUser)
{
	CUIGameInterface* pGameHUD = static_cast<CUIGameInterface*>(pUser);
	if(pGameHUD->Client()->State() != IClient::STATE_ONLINE)
		return;
	pGameHUD->m_ActiveHUD ^= true;
}

void CUIGameInterface::RenderGuiElements()
{
	if(m_aInformationBoxBuf[0] != '\0')
	{
		m_WindowInformationBox.OnRenderWindow([&](const CUIRect& pWindowRect, CWindowUI& pCurrentWindow)
		{
			CUIRect Label, ButtonAccept;
			pWindowRect.Margin(12.0f, &Label);
			Label.HSplitBottom(24.0f, &Label, &ButtonAccept);
			UI()->DoLabel(&Label, m_aInformationBoxBuf, 12.0f, CUI::EAlignment::ALIGN_LEFT, Label.w - 5.0f, true);

			static CMenus::CButtonContainer s_ButtonAccept;
			if(m_pClient->m_pMenus->DoButton_Menu(&s_ButtonAccept, "Accept", 0, &ButtonAccept, 0, CUI::CORNER_ALL, 12.0f))
			{
				mem_zero(m_aInformationBoxBuf, sizeof(m_aInformationBoxBuf));
				pCurrentWindow.Close();
			}
		});
	}
}

// inbox
void CUIGameInterface::RefreshLetters()
{
	if(Client()->State() == IClient::STATE_ONLINE)
	{
		m_aLettersList.clear();
		CNetMsg_Cl_ShowMailListRequest Msg;
		Client()->SendPackMsg(&Msg, MSGFLAG_VITAL);
		m_GotNewLetter = false;
	}
}

void CUIGameInterface::RenderInbox()
{
	// render inbox list
	if(m_WindowMailboxList.IsOpenned())
	{
		m_WindowMailboxList.OnRenderWindow([&](const CUIRect& pWindowRect, CWindowUI& pCurrentWindow)
		{
			CUIRect ListBoxRect, DownButtonsRect, ButtonRefreshRect, ButtonSendRect;
			pWindowRect.HSplitBottom(30.0f, &ListBoxRect, &DownButtonsRect);
			DownButtonsRect.VSplitLeft(40.0f, &ButtonSendRect, &ButtonRefreshRect);

			static CMenus::CButtonContainer s_ButtonSend;
			if(m_pClient->m_pMenus->DoButton_Menu(&s_ButtonSend, "\xe2\x9c\x89", 0, &ButtonSendRect, 0, CUI::CORNER_BL, 12.0f))
				m_WindowMailboxLetterSend.CloseOpen();

			static CMenus::CButtonContainer s_ButtonRefresh;
			if(m_pClient->m_pMenus->DoButton_Menu(&s_ButtonRefresh, "Refresh", 0, &ButtonRefreshRect, 0, CUI::CORNER_BR, 12.0f))
				RefreshLetters();

			if(m_aLettersList.empty())
			{
				CUIRect LabelEmpty;
				pWindowRect.HSplitTop(pWindowRect.h / 3.0f, 0, &LabelEmpty);
				UI()->DoLabel(&LabelEmpty, "The mail list is empty.", 16.0f, CUI::ALIGN_CENTER);
				LabelEmpty.HSplitTop(20.0f, 0, &LabelEmpty);
				UI()->DoLabel(&LabelEmpty, "Try refresh it.", 12.0f, CUI::ALIGN_CENTER);
				return;
			}

			static int s_LetterSelected = -1;
			static CMenus::CListBox s_ListBox;
			s_ListBox.DoHeader(&ListBoxRect, "", 0.0f, 8.0f);
			s_ListBox.DoStart(40.0f, m_aLettersList.size(), 1, 3, -1, 0, false);
			for(int i = 0; i < m_aLettersList.size(); i++)
			{
				const CMailboxLetter* pLetter = &m_aLettersList[i];
				CMenus::CListboxItem Item = s_ListBox.DoNextItem(pLetter, s_LetterSelected == i);
				if(Item.m_Visible)
				{
					static float SkipLabelWeidth = 5.0f;
					static float LetterIconSize = 38.0f;
					bool HasItem = pLetter->m_ItemID > 0;

					CUIRect LetterIconRect;
					Item.m_Rect.Margin(5.0f, &LetterIconRect);
					m_pClient->m_pMenus->DoItemIcon("ticket", LetterIconRect, LetterIconRect.h);

					CUIRect Label, Attachment;
					Item.m_Rect.VMargin((LetterIconSize + 5.0f), &Label);
					Label.VSplitRight(HasItem ? 0.0f : 20.0f, &Label, &Attachment);
					Label.HSplitTop(3.0f, 0, &Label);
					UI()->DoLabel(&Label, pLetter->m_aName, 15.0f, CUI::ALIGN_LEFT, Label.w - SkipLabelWeidth, false);

					Label.HSplitTop(20.0f, 0, &Label);
					UI()->DoLabel(&Label, pLetter->m_aDesc, 10.0f, CUI::ALIGN_LEFT, Label.w - SkipLabelWeidth, false);

					if(HasItem)
					{
						Attachment.HSplitTop(4.0f, 0, &Attachment);
						DrawUIRectIconItem(&Attachment, 32.0f, pLetter->m_ItemID);
					}
					else
					{
						Attachment.HSplitTop(12.0f, 0, &Attachment);
						UI()->DoLabelColored(&Attachment, "Only letter", 10.0f, CUI::ALIGN_LEFT, vec4(1.0f, 0.5f, 0.5f, 1.0f));
					}
				}
			}

			s_LetterSelected = s_ListBox.DoEnd();
			if(s_ListBox.WasItemActivated() && (s_LetterSelected != m_LetterActiveSelected || !m_WindowMailboxLetterSelected.IsOpenned()))
			{
				m_LetterActiveSelected = s_LetterSelected;
				m_WindowMailboxLetterSelected.Open();
			}
		});
	}

	// render mailbox letter selected
	if(m_WindowMailboxLetterSelected.IsOpenned())
	{
		m_WindowMailboxLetterSelected.OnRenderWindow([&](const CUIRect& pWindowRect, CWindowUI& pCurrentWindow)
		{
			if((m_LetterActiveSelected < 0 || m_LetterActiveSelected >= m_aLettersList.size()) && pCurrentWindow.IsOpenned())
			{
				pCurrentWindow.Close();
				return;
			}

			const CMailboxLetter* pLetter = &m_aLettersList[m_LetterActiveSelected];
			CUIRect Label = pWindowRect, ItemSlot, AcceptButton;
			Label.VMargin(10.0f, &Label);
			UI()->DoLabelColored(&Label, " Title:", 12.0f, CUI::ALIGN_LEFT, vec4(1.0f, 0.9f, 0.8f, 1.0f));
			UI()->DoLabel(&Label, pLetter->m_aName, 12.0f, CUI::ALIGN_CENTER);
			Label.HSplitTop(20.0f, 0, &Label);
			RenderTools()->DrawUIRectLine(&Label, vec4(0.3f, 0.3f, 0.3f, 0.3f), LineDirectionFlag::LINE_LEFT);
			Label.VSplitRight(64.0f, &Label, &ItemSlot);
			UI()->DoLabel(&Label, pLetter->m_aDesc, 10.0f, CUI::ALIGN_LEFT, Label.w);

			// icon item
			ItemSlot.HSplitBottom(22.0f, &ItemSlot, &AcceptButton);
			ItemSlot.Margin(10.0f, &ItemSlot);
			ItemSlot.VMargin(2.0f, &ItemSlot);
			CUIRect Item = ItemSlot;

			// button accept
			static CMenus::CButtonContainer s_aButtonAccept;
			AcceptButton.HMargin(3.0f, &AcceptButton);
			if(m_pClient->m_pMenus->DoButton_Menu(&s_aButtonAccept, "Accept", 0, &AcceptButton, 0, CUI::CORNER_ALL))
			{
				CNetMsg_Cl_ReceiveMail Msg;
				Msg.m_MailID = pLetter->m_MailID;
				Client()->SendPackMsg(&Msg, MSGFLAG_VITAL);

				RefreshLetters();
				pCurrentWindow.Close();
			}

			// icon
			static CMenus::CButtonContainer s_IconButton;
			DoDrawItemIcon(&s_IconButton, &Item, Item.h, pLetter->m_ItemID);
		});
	}

	// render mailbox letter sending
	if(m_WindowMailboxLetterSend.IsOpenned())
	{
		m_WindowMailboxLetterSend.OnRenderWindow([&](const CUIRect& pWindowRect, CWindowUI& pCurrentWindow)
		{
			static char s_aBufTitle[64];
			static char s_aBufPlayer[32];
			static char s_aBufMessage[64];

			// player
			CUIRect Label, EditPlayerBox;
			pWindowRect.Margin(10.0f, &Label);
			Label.VSplitLeft(60.0f, 0, &EditPlayerBox);
			UI()->DoLabel(&Label, Localize("Player:"), 14.0f, CUI::ALIGN_LEFT);
			{
				static int s_BoxPlayerMsg = 0;
				static float s_OffsetPlayerLetter = 0.0f;
				EditPlayerBox.HSplitTop(22.0f, &EditPlayerBox, 0);
				m_pClient->m_pMenus->DoEditBox(&s_BoxPlayerMsg, &EditPlayerBox, s_aBufPlayer, sizeof(s_aBufPlayer), 12.0f, &s_OffsetPlayerLetter);
			}
			Label.HSplitTop(24.0f, 0, &Label);

			// title
			CUIRect EditTitleBox;
			Label.VSplitLeft(60.0f, 0, &EditTitleBox);
			UI()->DoLabel(&Label, Localize("Title:"), 14.0f, CUI::ALIGN_LEFT);
			{
				static int s_BoxTitleMsg = 0;
				static float s_OffsetTitleLetter = 0.0f;
				EditTitleBox.HSplitTop(22.0f, &EditTitleBox, 0);
				m_pClient->m_pMenus->DoEditBox(&s_BoxTitleMsg, &EditTitleBox, s_aBufTitle, sizeof(s_aBufTitle), 12.0f, &s_OffsetTitleLetter);
			}
			Label.HSplitTop(32.0f, 0, &Label);
			RenderTools()->DrawUIRectLine(&Label, vec4(0.3f, 0.3f, 0.3f, 0.3f), LineDirectionFlag::LINE_LEFT);

			// message
			CUIRect EditMessageBox;
			UI()->DoLabel(&Label, Localize("Message:"), 14.0f, CUI::ALIGN_LEFT);
			Label.HSplitTop(20.0f, 0, &EditMessageBox);
			{
				static int s_BoxMessageMsg = 0;
				static float s_OffsetMessageLetter = 0.0f;
				EditMessageBox.HSplitTop(22.0f, &EditMessageBox, 0);
				m_pClient->m_pMenus->DoEditBox(&s_BoxMessageMsg, &EditMessageBox, s_aBufMessage, sizeof(s_aBufMessage), 12.0f, &s_OffsetMessageLetter);
			}
			Label.HSplitTop(50.0f, 0, &Label);
			RenderTools()->DrawUIRectLine(&Label, vec4(0.3f, 0.3f, 0.3f, 0.3f), LineDirectionFlag::LINE_LEFT);

			CUIRect ButtonSend;
			Label.HSplitBottom(30.0f, 0, &ButtonSend);

			static CMenus::CButtonContainer s_ButtonRefresh;
			if(m_pClient->m_pMenus->DoButton_Menu(&s_ButtonRefresh, "Send", 0, &ButtonSend, 0, CUI::CORNER_ALL, 12.0f))
			{
				if(str_length(s_aBufTitle) < 3 || str_length(s_aBufPlayer) < 1 || str_length(s_aBufMessage) < 1)
					CreateInformationBox("The minimum number of characters entered can be: Title - 3, Player - 1, Message - 1");
				else
				{
					CNetMsg_Cl_SendMailToPlayer Msg;
					Msg.m_pTitle = s_aBufTitle;
					Msg.m_pMsg = s_aBufMessage;
					Msg.m_pPlayer = s_aBufPlayer;
					Client()->SendPackMsg(&Msg, MSGFLAG_VITAL);

					mem_zero(s_aBufPlayer, sizeof(s_aBufPlayer));
					mem_zero(s_aBufTitle, sizeof(s_aBufTitle));
					mem_zero(s_aBufMessage, sizeof(s_aBufMessage));
					pCurrentWindow.Close();
				}
			}
		});
	}
}

void CUIGameInterface::RenderQuests()
{
	// render inbox list
	if(m_WindowQuestsList.IsOpenned())
	{
		m_WindowQuestsList.OnRenderWindow([&](const CUIRect& pWindowRect, CWindowUI& pCurrentWindow)
		{
			CUIRect Label;
			pWindowRect.Margin(12.0f, &Label);
			UI()->DoLabelColored(&Label, "Coming soon..", 18.0f, CUI::EAlignment::ALIGN_LEFT, vec4(1.0f, 0.5f, 0.5f, 1.0f), Label.w - 5.0f, true);
		});
	}
}

// ui logics
bool CUIGameInterface::DoIconSelectionWindow(CMenus::CButtonContainer* pBC, CUIRect* pRect, CWindowUI* pWindow, int SpriteID)
{
	if(!pBC || !pWindow)
		return false;

	bool WasUseds = false;
	bool WindowOpenned = pWindow->IsOpenned();
	const float FadeVal = WindowOpenned ? max(pBC->GetFade() + 0.5f, 1.0f) : pBC->GetFade();
	float Size = 42.0f + (FadeVal * 6.0f);
	CUIRect Icon = { pRect->x, pRect->y, Size, Size };
	RenderTools()->DrawUIRect(&Icon, vec4(0.0f + FadeVal, 0.0f + FadeVal, 0.0f + FadeVal, 0.3f), CUI::CORNER_ALL, 5.0f);

	if(UI()->HotItem() == pBC->GetID() && UI()->NextHotItem() != pBC->GetID())
	{
		vec4 ColorHighlight = vec4(1.0f, 0.55f, 0.0f, 0.4f);
		pWindow->HighlightEnable(ColorHighlight, true);
	}
	else if(UI()->HotItem() != pBC->GetID())
	{
		pWindow->HighlightDisable();
	}

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
	{
		pWindow->CloseOpen();
		WasUseds = true;
	}

	// UI sounds
	if(g_Config.m_SndEnableUI)
	{
		if(g_Config.m_SndEnableUIHover && UI()->NextHotItem() == pBC->GetID() && UI()->NextHotItem() != UI()->HotItem())
			m_pClient->m_pSounds->Play(CSounds::CHN_GUI, SOUND_BUTTON_HOVER, 1);
		if(UI()->GetActiveItem() == pBC->GetID() && pLastActiveItem != pBC->GetID())
			m_pClient->m_pSounds->Play(CSounds::CHN_GUI, SOUND_BUTTON_CLICK, 0);
	}
	return WasUseds;
}

bool CUIGameInterface::DoDrawItemIcon(CMenus::CButtonContainer* pBC, CUIRect* pRect, float IconSize, int ItemID)
{
	CItemDataClientInfo* pItem = &m_aItemsDataInformation[ItemID];
	if(!pBC || !pItem)
		return false;

	DrawUIRectIconItem(pRect, IconSize, ItemID);

	if(UI()->HotItem() == pBC->GetID())
	{
		const CUIRect* pScreen = UI()->Screen();

		CUIRect InformationRect;
		InformationRect.x = m_MousePos.x;
		InformationRect.y = m_MousePos.y;
		InformationRect.w = 200.0f;
		InformationRect.h = 150.0f;
		if((InformationRect.x + InformationRect.w) > pScreen->w)
			InformationRect.x = m_MousePos.x - InformationRect.w;
		if((InformationRect.y + InformationRect.h) > pScreen->h)
			InformationRect.y = pScreen->h - InformationRect.h;

		// background
		RenderTools()->DrawUIRectMonochromeGradient(&InformationRect, vec4(0.15f, 0.15f, 0.15f, 0.8f), CUI::CORNER_ALL, 12.0f);
		InformationRect.Margin(3.0f, &InformationRect);
		RenderTools()->DrawUIRectMonochromeGradient(&InformationRect, vec4(0.1f, 0.1f, 0.1f, 0.8f), CUI::CORNER_ALL, 12.0f);

		// informations
		char aBuf[256];
		CUIRect Label = InformationRect;
		Label.Margin(5.0f, &Label);
		str_format(aBuf, sizeof(aBuf), "%s", pItem->m_aName);
		UI()->DoLabel(&Label, aBuf, 16.0f, CUI::EAlignment::ALIGN_LEFT, InformationRect.w, false);

		CUIRect IconRight;
		static float IconRightSize = 24.0f;
		Label.VSplitRight(IconRightSize, 0, &IconRight);
		IconRight.HSplitTop(IconRightSize, &IconRight, 0);
		DrawUIRectIconItem(&IconRight, IconRight.h, ItemID);

		Label.HSplitTop(IconRightSize + 4.0f, 0, &Label);
		RenderTools()->DrawUIRectLine(&Label, vec4(0.3f, 0.3f, 0.3f, 0.3f), LineDirectionFlag::LINE_LEFT);
		str_format(aBuf, sizeof(aBuf), "Description: %s", pItem->m_aDesc);
		UI()->DoLabel(&Label, aBuf, 10.0f, CUI::EAlignment::ALIGN_LEFT, InformationRect.w, false);
	}

	const void* pLastActiveItem = UI()->GetActiveItem();
	bool Logic = UI()->DoButtonLogic(pBC->GetID(), pRect);

	// UI sounds
	if(g_Config.m_SndEnableUI)
	{
		if(g_Config.m_SndEnableUIHover && UI()->NextHotItem() == pBC->GetID() && UI()->NextHotItem() != UI()->HotItem())
			m_pClient->m_pSounds->Play(CSounds::CHN_GUI, SOUND_BUTTON_HOVER, 1);
		if(UI()->GetActiveItem() == pBC->GetID() && pLastActiveItem != pBC->GetID())
			m_pClient->m_pSounds->Play(CSounds::CHN_GUI, SOUND_BUTTON_CLICK, 0);
	}
	return Logic;
}

void CUIGameInterface::DrawUIRectIconItem(CUIRect* pRect, float IconSize, int ItemID)
{
	CItemDataClientInfo* pItem = &m_aItemsDataInformation[ItemID];
	const char* pIcon = pItem ? pItem->m_aIcon : "empty";

	// draw information about item
	CUIRect IconRect = { pRect->x, pRect->y, IconSize, IconSize };
	RenderTools()->DrawUIRectMonochromeGradient(&IconRect, vec4(0.2f, 0.2f, 0.2f, 0.8f), CUI::CORNER_ALL, 5.0f);

	// icon
	IconRect.Margin(3.0f, &IconRect);
	m_pClient->m_pMenus->DoItemIcon(pIcon, IconRect, IconRect.h);
}

void CUIGameInterface::CreateInformationBox(const char* pMessage)
{
	str_copy(m_aInformationBoxBuf, pMessage, sizeof(m_aInformationBoxBuf));
	m_WindowInformationBox.Open();
}
