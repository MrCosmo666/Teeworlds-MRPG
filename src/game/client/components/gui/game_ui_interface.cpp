#include <engine/console.h>
#include <engine/keys.h>
#include <engine/textrender.h>
#include <engine/shared/config.h>
#include <generated/client_data.h>

#include <game/game_context.h>
#include <game/client/components/console.h>
#include <game/client/components/menus.h>
#include <game/client/components/sounds.h>

#include "game_ui_interface.h"

#include <teeother/tl/nlohmann_json.h>

std::map< int /*itemid*/, CUIGameInterface::CItemDataClientInfo > CUIGameInterface::m_aItemsDataInformation;
std::map< int /*itemid*/, CUIGameInterface::CClientItem > CUIGameInterface::m_aClientItems;

/*
 * Refactoring later. I'm lazy.
 */
void CUIGameInterface::OnInit()
{
	// gui elements
	m_pWindowPopupBox = UI()->CreateWindow("Are you sure?", vec2(0, 0), nullptr, &m_ActiveGUI);
	m_pWindowInformationBox = UI()->CreateWindow("Information", vec2(0, 0), nullptr, &m_ActiveGUI);
	m_pWindowPopupBox->Register(WINREGISTER(&CUIGameInterface::CallbackRenderGuiPopupBox, this));
	m_pWindowInformationBox->Register(WINREGISTER(&CUIGameInterface::CallbackRenderInfoWindow, this));

	// inbox system
	m_pWindowMailboxList = UI()->CreateWindow("Mailbox list", vec2(320, 250), nullptr, &m_ActiveGUI);
	m_pWindowMailboxLetter = UI()->CreateWindow("Letter", vec2(250, 140), m_pWindowMailboxList, &m_ActiveGUI);
	m_pWindowMailboxLetterActions = UI()->CreateWindow("Letter actions", vec2(0, 0), m_pWindowMailboxList, &m_ActiveGUI, CUI::WINDOW_WITHOUT_BORDURE | CUI::WINDOW_CLOSE_CLICKING_OUTSIDE);
	m_pWindowMailboxLetterSend = UI()->CreateWindow("Sending a letter", vec2(220, 190), m_pWindowMailboxList, &m_ActiveGUI);
	m_pWindowMailboxList->Register(WINREGISTER(&CUIGameInterface::CallbackRenderMailboxList, this));
	m_pWindowMailboxLetter->Register(WINREGISTER(&CUIGameInterface::CallbackRenderMailboxLetter, this));
	m_pWindowMailboxLetterActions->Register(WINREGISTER(&CUIGameInterface::CallbackRenderMailboxLetterActions, this));
	m_pWindowMailboxLetterSend->Register(WINREGISTER(&CUIGameInterface::CallbackRenderMailboxLetterSend, this));

	m_pWindowMailboxList->RegisterHelp(WINREGISTER(&CUIGameInterface::CallbackRenderMailboxListButtonHelp, this));

	// questing system
	m_pWindowQuestsList = UI()->CreateWindow("Quest book", vec2(300, 80));
	m_pWindowQuestsList->Register(WINREGISTER(&CUIGameInterface::CallbackRenderQuests, this));
}

void CUIGameInterface::OnReset()
{
	m_ActiveGUI = false;
	m_WindowPopupCallback = nullptr;

	m_pWindowPopupBox->Close();
	m_pWindowInformationBox->Close();
	m_pWindowMailboxList->Close();
	m_pWindowMailboxLetter->Close();
	m_pWindowMailboxLetterSend->Close();
	m_pWindowQuestsList->Close();
}

bool CUIGameInterface::OnInput(IInput::CEvent Event)
{
	if(m_pClient->m_pGameConsole->IsConsoleActive() || !IsActiveGUI())
		return false;

	if(Event.m_Flags & IInput::FLAG_PRESS && Event.m_Key == KEY_ESCAPE)
	{
		m_ActiveGUI = false;
		return true;
	}
	if(Event.m_Key == KEY_MOUSE_1)
	{
		return true;
	}
	if(Event.m_Key == KEY_MOUSE_2)
	{
		return true;
	}
	if(Event.m_Key == KEY_MOUSE_WHEEL_UP)
	{
		return true;
	}
	if(Event.m_Key == KEY_MOUSE_WHEEL_DOWN)
	{
		return true;
	}

	return false;
}

void CUIGameInterface::OnRender()
{
	if(!IsActiveGUI())
		return;

	const CUIRect* pScreen = UI()->Screen();
	Graphics()->MapScreen(pScreen->x, pScreen->y, pScreen->w, pScreen->h);

	RenderGuiIcons();

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

void CUIGameInterface::RenderGuiIcons()
{
	// inbox/mail gui icon
	CUIRect IconView;
	const CUIRect* pScreen = UI()->Screen();
	{
		pScreen->VSplitRight(60.0f, 0, &IconView);
		IconView.HSplitBottom(400.0f, 0, &IconView);
		static CMenus::CButtonContainer s_MailListButton;
		const bool UnreadLetters = UnreadLetterMails();
		if(DoIconSelectionWindow(&s_MailListButton, &IconView, m_pWindowMailboxList, SPRITE_HUD_ICON_MAIL, UnreadLetters ? "New" : nullptr) && m_pWindowMailboxList->IsOpenned())
			SendLetterAction(nullptr, MAILLETTERFLAG_REFRESH);
	}
	// questing gui icon
	{
		IconView.HMargin(52.0f, &IconView);
		static CMenus::CButtonContainer s_QuestingListButton;
		if(DoIconSelectionWindow(&s_QuestingListButton, &IconView, m_pWindowQuestsList, SPRITE_HUD_ICON_QUEST))
		{
		}
	}
}


void CUIGameInterface::OnConsoleInit()
{
	Console()->Register("toggle_game_hud_mrpg", "", CFGFLAG_CLIENT, ConToggleGameHUDMRPG, this, "Toggle game hud mrpg");
}

void CUIGameInterface::OnStateChange(int NewState, int OldState)
{
	if(NewState != IClient::STATE_ONLINE)
		m_ActiveGUI = false;
}

bool CUIGameInterface::OnCursorMove(float x, float y, int CursorType)
{
	if(!IsActiveGUI())
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

	if(Msg == NETMSGTYPE_SV_SENDMAILLETTERINFO)
	{
		CNetMsg_Sv_SendMailLetterInfo* pMsg = (CNetMsg_Sv_SendMailLetterInfo*)pRawMsg;

		// basic information
		CMailboxLetter Letter;
		Letter.m_MailLetterID = pMsg->m_MailLetterID;
		str_copy(Letter.m_aName, pMsg->m_pTitle, sizeof(Letter.m_aName));
		str_copy(Letter.m_aDesc, pMsg->m_pMsg, sizeof(Letter.m_aDesc));
		str_copy(Letter.m_aFrom, pMsg->m_pFrom, sizeof(Letter.m_aFrom));
		Letter.m_IsRead = pMsg->m_IsRead;

		// attachement items
		nlohmann::json JsonAttachmentItem = nlohmann::json::parse(pMsg->m_pJsonAttachementItem, nullptr, false);
		if(!JsonAttachmentItem.is_discarded())
		{
			Letter.m_AttachmentItem.m_ItemID = JsonAttachmentItem.value("item", 0);
			Letter.m_AttachmentItem.m_Amount = JsonAttachmentItem.value("value", 0);
			Letter.m_AttachmentItem.m_Enchant = JsonAttachmentItem.value("enchant", 0);
		}
		m_aLettersList.push_back(Letter);
	}
	else if(Msg == NETMSGTYPE_SV_SENDGUIINFORMATIONBOX)
	{
		CNetMsg_Sv_SendGuiInformationBox* pMsg = (CNetMsg_Sv_SendGuiInformationBox*)pRawMsg;
		CreateInformationBox(nullptr, 300, pMsg->m_pMsg);
	}
}

void CUIGameInterface::ConToggleGameHUDMRPG(IConsole::IResult* pResult, void* pUser)
{
	CUIGameInterface* pGameGUI = static_cast<CUIGameInterface*>(pUser);
	if(pGameGUI->m_pClient->m_pMenus->IsActiveAuthMRPG() || pGameGUI->Client()->State() != IClient::STATE_ONLINE)
		return;

	pGameGUI->m_ActiveGUI ^= true;
}

void CUIGameInterface::OnRelease()
{
	OnReset();
}

// inbox
void CUIGameInterface::CallbackRenderMailboxList(const CUIRect& pWindowRect, CWindowUI& pCurrentWindow)
{
	CUIRect ListBoxRect, DownButtonsRect, ButtonRefreshRect, ButtonSendRect;
	pWindowRect.HSplitBottom(40.0f, &ListBoxRect, &DownButtonsRect);
	DownButtonsRect.Margin(8.0f, &DownButtonsRect);
	DownButtonsRect.VSplitLeft(40.0f, &ButtonSendRect, &ButtonRefreshRect);

	static CMenus::CButtonContainer s_ButtonSend;
	if(m_pClient->m_pMenus->DoButton_Menu(&s_ButtonSend, "\xe2\x9c\x89", 0, &ButtonSendRect, 0, CUI::CORNER_L, 4.0f))
		m_pWindowMailboxLetterSend->Reverse();

	static CMenus::CButtonContainer s_ButtonRefresh;
	if(m_pClient->m_pMenus->DoButton_Menu(&s_ButtonRefresh, "Refresh", 0, &ButtonRefreshRect, 0, CUI::CORNER_R, 4.0f))
		SendLetterAction(nullptr, MAILLETTERFLAG_REFRESH);

	if(m_aLettersList.empty())
	{
		CUIRect LabelEmpty;
		pWindowRect.HSplitTop(pWindowRect.h / 2.5f, 0, &LabelEmpty);
		UI()->DoLabel(&LabelEmpty, "Is empty.\nTry refresh the list.", 12.0f, CUI::ALIGN_CENTER);
		return;
	}

	char aBuf[128];
	static int s_LetterSelected = -1;
	static CMenus::CListBox s_ListBox;
	str_format(aBuf, sizeof(aBuf), "Mailbox capacity %d/%d", (int)m_aLettersList.size(), MAILLETTER_MAX_CAPACITY);
	s_ListBox.DoHeader(&ListBoxRect, aBuf, 20.0f, 0.0f);
	s_ListBox.DoStart(18.0f, m_aLettersList.size(), 1, 3, -1, 0, true, 0, true);
	for(int i = 0; i < (int)m_aLettersList.size(); i++)
	{
		const CMailboxLetter* pLetter = &m_aLettersList[i];
		CMenus::CListboxItem Item = s_ListBox.DoNextItem(pLetter, s_LetterSelected == i);
		if(Item.m_Visible)
		{
			static float LetterIconSize = 16.0f;
			const bool HasItem = !pLetter->m_AttachmentItem.Empty();
			const vec4 TextColor = pLetter->m_IsRead ? vec4(0.6f, 0.6f, 0.6f, 1.0f) : vec4(1.0f, 1.0f, 1.0f, 1.0f);
			const char* pIcon = HasItem ? "letter_i" : "letter";

			CUIRect Label, LetterIconRect;
			Item.m_Rect.VMargin(4.0f, &Item.m_Rect);
			Item.m_Rect.VSplitLeft(LetterIconSize, &LetterIconRect, &Label);
			Label.Margin(2.0f, &Label);
			m_pClient->m_pMenus->DoItemIcon(pIcon, LetterIconRect, LetterIconSize);
			UI()->DoLabelColored(&Label, pLetter->m_aName, 10.0f, CUI::ALIGN_LEFT, TextColor, Label.w, false);

			const float TextWidth = TextRender()->TextWidth(10.0f, pLetter->m_aFrom, -1);
			Label.VSplitRight(TextWidth + 16.0f, 0, &Label);
			UI()->DoLabelColored(&Label, "by", 10.0f, CUI::ALIGN_LEFT, vec4(0.5f, 0.5f, 0.5f, 1.0f), Label.w, false);
			Label.VSplitLeft(15.0f, 0, &Label);
			UI()->DoLabelColored(&Label, pLetter->m_aFrom, 10.0f, CUI::ALIGN_LEFT, vec4(0.85f, 0.85f, 0.85f, 1.0f), Label.w, false);
		}
	}

	s_LetterSelected = s_ListBox.DoEnd();
	if(s_LetterSelected >= 0 && s_LetterSelected < (int)m_aLettersList.size())
	{
		if(s_ListBox.WasItemActivated())
		{
			m_pLetterSelected = &m_aLettersList[s_LetterSelected];
			m_pWindowMailboxLetter->Open();
		}
		if(s_ListBox.WasRightMouseClick())
		{
			m_pLetterSelected = &m_aLettersList[s_LetterSelected];
			m_pWindowMailboxLetter->Close();
			m_pWindowMailboxLetterActions->Open();
		}
	}
}

void CUIGameInterface::CallbackRenderMailboxListButtonHelp(const CUIRect& pWindowRect, CWindowUI& pCurrentWindow)
{
	CUIRect Label = pWindowRect;
	const float FontSize = 10.0f;
	const char* pLineHelp = "This is where you can control your inbox.View.And interact with it.";

	float TextWidth = TextRender()->TextWidth(FontSize, pLineHelp, -1);
	UI()->DoLabel(&Label, pLineHelp, FontSize, CUI::EAlignment::ALIGN_LEFT);

	pCurrentWindow.SetWorkspaceSize({ TextWidth, 30 });
}

void CUIGameInterface::CallbackRenderMailboxLetter(const CUIRect& pWindowRect, CWindowUI& pCurrentWindow)
{
	if(!m_pLetterSelected)
	{
		pCurrentWindow.Close();
		return;
	}

	// update state read
	SendLetterAction(m_pLetterSelected, MAILLETTERFLAG_READ);

	// labels
	const bool HasItem = !m_pLetterSelected->m_AttachmentItem.Empty();
	CUIRect Label = pWindowRect, ItemSlot, AcceptButton, DeleteButton;
	Label.VMargin(10.0f, &Label);
	Label.HSplitBottom(28.0f, &Label, &AcceptButton);
	UI()->DoLabelColored(&Label, " Title:", 12.0f, CUI::ALIGN_LEFT, vec4(1.0f, 0.9f, 0.8f, 1.0f));
	UI()->DoLabel(&Label, m_pLetterSelected->m_aName, 12.0f, CUI::ALIGN_CENTER);
	Label.HSplitTop(22.0f, 0, &Label);
	RenderTools()->DrawUIRectLine(&Label, vec4(0.3f, 0.3f, 0.3f, 0.3f), LineDirectionFlag::LINE_LEFT);
	if(HasItem)
		Label.VSplitRight(64.0f, &Label, &ItemSlot);
	UI()->DoLabel(&Label, m_pLetterSelected->m_aDesc, 10.0f, CUI::ALIGN_LEFT, Label.w);

	AcceptButton.HMargin(5.0f, &AcceptButton);
	AcceptButton.VSplitLeft(80.0f, &DeleteButton, &AcceptButton);

	// button accept
	AcceptButton.VMargin(3.0f, &AcceptButton);
	static CMenus::CButtonContainer s_aButtonAccept;
	if(m_pClient->m_pMenus->DoButton_Menu(&s_aButtonAccept, "Accept", 0, &AcceptButton, 0, CUI::CORNER_ALL))
	{
		SendLetterAction(m_pLetterSelected, MAILLETTERFLAG_ACCEPT | MAILLETTERFLAG_REFRESH);
		pCurrentWindow.Close();
	}

	// button delete
	DeleteButton.VMargin(3.0f, &DeleteButton);
	static CMenus::CButtonContainer s_aButtonDelete;
	if(m_pClient->m_pMenus->DoButton_Menu(&s_aButtonDelete, "Delete", 0, &DeleteButton, 0, CUI::CORNER_ALL))
		CreatePopupBox(&pCurrentWindow, 200.0f, "Do you really want to delete letter?", POPUP_REGISTER(&CUIGameInterface::CallbackPopupDeleteLetter, this));

	// icon item
	if(HasItem)
	{
		const float IconSize = 36.0f;
		static CMenus::CButtonContainer s_IconButton;
		ItemSlot.Margin(6.0f, &ItemSlot);
		ItemSlot.VMargin(8.0f, &ItemSlot);
		CUIRect LabelAmount = ItemSlot;
		LabelAmount.y += 3.0f + IconSize;

		char aBufAttachmentLabel[256];
		str_format(aBufAttachmentLabel, sizeof(aBufAttachmentLabel), "Amount\n%d", m_pLetterSelected->m_AttachmentItem.m_Amount);
		UI()->DoLabelColored(&LabelAmount, aBufAttachmentLabel, 10.0f, CUI::ALIGN_CENTER, vec4(1.0f, 0.9f, 0.8f, 1.0f));
		DoDrawItemIcon(&s_IconButton, &ItemSlot, IconSize, m_pLetterSelected->m_AttachmentItem);
	}
}

void CUIGameInterface::CallbackRenderMailboxLetterSend(const CUIRect& pWindowRect, CWindowUI& pCurrentWindow)
{
	static char s_aBufTitle[64];
	static char s_aBufPlayer[32];
	static char s_aBufMessage[64];

	// player
	CUIRect Label, EditPlayerBox;
	pWindowRect.Margin(10.0f, &Label);
	Label.VSplitLeft(60.0f, 0, &EditPlayerBox);
	UI()->DoLabel(&Label, Localize("Player:"), 10.0f, CUI::ALIGN_LEFT);
	{
		static int s_BoxPlayerMsg = 0;
		static float s_OffsetPlayerLetter = 0.0f;
		EditPlayerBox.HSplitTop(22.0f, &EditPlayerBox, 0);
		m_pClient->m_pMenus->DoEditBox(&s_BoxPlayerMsg, &EditPlayerBox, s_aBufPlayer, sizeof(s_aBufPlayer), 10.0f, &s_OffsetPlayerLetter);
	}
	Label.HSplitTop(24.0f, 0, &Label);

	// title
	CUIRect EditTitleBox;
	Label.VSplitLeft(60.0f, 0, &EditTitleBox);
	UI()->DoLabel(&Label, Localize("Title:"), 10.0f, CUI::ALIGN_LEFT);
	{
		static int s_BoxTitleMsg = 0;
		static float s_OffsetTitleLetter = 0.0f;
		EditTitleBox.HSplitTop(22.0f, &EditTitleBox, 0);
		m_pClient->m_pMenus->DoEditBox(&s_BoxTitleMsg, &EditTitleBox, s_aBufTitle, sizeof(s_aBufTitle), 10.0f, &s_OffsetTitleLetter);
	}
	Label.HSplitTop(32.0f, 0, &Label);
	RenderTools()->DrawUIRectLine(&Label, vec4(0.3f, 0.3f, 0.3f, 0.3f), LineDirectionFlag::LINE_LEFT);

	// message
	CUIRect EditMessageBox;
	Label.HSplitTop(6.0f, 0, &Label);
	UI()->DoLabel(&Label, Localize("Message:"), 10.0f, CUI::ALIGN_LEFT);
	Label.HSplitTop(20.0f, 0, &EditMessageBox);
	{
		static int s_BoxMessageMsg = 0;
		static float s_OffsetMessageLetter = 0.0f;
		EditMessageBox.HSplitTop(22.0f, &EditMessageBox, 0);
		m_pClient->m_pMenus->DoEditBox(&s_BoxMessageMsg, &EditMessageBox, s_aBufMessage, sizeof(s_aBufMessage), 10.0f, &s_OffsetMessageLetter);
	}
	Label.HSplitTop(50.0f, 0, &Label);
	RenderTools()->DrawUIRectLine(&Label, vec4(0.3f, 0.3f, 0.3f, 0.3f), LineDirectionFlag::LINE_LEFT);

	CUIRect ButtonSend;
	Label.HSplitBottom(22.0f, 0, &ButtonSend);

	static CMenus::CButtonContainer s_ButtonRefresh;
	if(m_pClient->m_pMenus->DoButton_Menu(&s_ButtonRefresh, "Send", 0, &ButtonSend, 0, CUI::CORNER_ALL, 10.0f))
	{
		if(str_length(s_aBufTitle) < 3 || str_length(s_aBufTitle) > 12 || str_length(s_aBufPlayer) < 1 || str_length(s_aBufPlayer) > 24
			|| str_length(s_aBufMessage) < 1 || str_length(s_aBufMessage) > 48)
		{
			CreateInformationBox(&pCurrentWindow, 260.0f, "The minimum number of characters entered can.\n- Title (3 - 12)\n- Player (1 - 24)\n- Message (1 - 48)");
		}
		else
		{
			CNetMsg_Cl_SendMailLetterTo Msg;
			Msg.m_pTitle = s_aBufTitle;
			Msg.m_pMsg = s_aBufMessage;
			Msg.m_pPlayer = s_aBufPlayer;
			Msg.m_FromClientID = m_pClient->m_LocalClientID;
			Client()->SendPackMsg(&Msg, MSGFLAG_VITAL);

			mem_zero(s_aBufPlayer, sizeof(s_aBufPlayer));
			mem_zero(s_aBufTitle, sizeof(s_aBufTitle));
			mem_zero(s_aBufMessage, sizeof(s_aBufMessage));
			pCurrentWindow.Close();
		}
	}
}

void CUIGameInterface::CallbackRenderMailboxLetterActions(const CUIRect& pWindowRect, CWindowUI& pCurrentWindow)
{
	float ButtonAmount = 2;
	static float ButtonHeight = 16.0f;

	CUIRect MainView = pWindowRect, Button;
	MainView.HSplitTop(ButtonHeight, &Button, &MainView);
	static CMenus::CButtonContainer s_aButtonOpen;
	if(m_pClient->m_pMenus->DoButton_Menu(&s_aButtonOpen, "Open letter", 0, &Button, 0, CUI::CORNER_ALL, 2.0f))
	{
		m_pWindowMailboxLetter->Open();
		pCurrentWindow.Close();
	}

	if(!m_pLetterSelected->m_IsRead)
	{
		ButtonAmount++;
		MainView.HSplitTop(ButtonHeight, &Button, &MainView);
		static CMenus::CButtonContainer s_aButtonMarkRead;
		if(m_pClient->m_pMenus->DoButton_Menu(&s_aButtonMarkRead, "Mark as read", 0, &Button, 0, CUI::CORNER_ALL, 2.0f))
		{
			SendLetterAction(m_pLetterSelected, MAILLETTERFLAG_READ);
			pCurrentWindow.Close();
		}
	}

	MainView.HSplitTop(ButtonHeight, &Button, &MainView);
	static CMenus::CButtonContainer s_aButtonDelete;
	if(m_pClient->m_pMenus->DoButton_Menu(&s_aButtonDelete, "Delete letter", 0, &Button, 0, CUI::CORNER_ALL, 2.0f))
	{
		CreatePopupBox(m_pWindowMailboxLetter, 200.0f, "Do you really want to delete letter?", POPUP_REGISTER(&CUIGameInterface::CallbackPopupDeleteLetter, this));
		pCurrentWindow.Close();
	}

	pCurrentWindow.SetWorkspaceSize(vec2(100, 4.0f + ButtonAmount * ButtonHeight));
}

void CUIGameInterface::SendLetterAction(CMailboxLetter* pLetter, int64 Flags)
{
	if(Client()->State() != IClient::STATE_ONLINE)
		return;

	if(pLetter && Flags & MAILLETTERFLAG_READ)
		pLetter->m_IsRead = true;

	if(Flags & MAILLETTERFLAG_REFRESH)
		m_aLettersList.clear();

	CNetMsg_Cl_MailLetterActions Msg;
	Msg.m_MailLetterID = pLetter ? pLetter->m_MailLetterID : -1;
	Msg.m_MailLetterFlags = Flags;
	Client()->SendPackMsg(&Msg, MSGFLAG_VITAL);
}

bool CUIGameInterface::UnreadLetterMails() const
{
	for(int i = 0; i < (int)m_aLettersList.size(); i++)
	{
		if(!m_aLettersList[i].m_IsRead)
			return true;
	}
	return false;
}


void CUIGameInterface::CallbackRenderQuests(const CUIRect& pWindowRect, CWindowUI& pCurrentWindow)
{
	CUIRect Label;
	pWindowRect.Margin(12.0f, &Label);
	UI()->DoLabelColored(&Label, "Coming soon..", 18.0f, CUI::EAlignment::ALIGN_LEFT, vec4(1.0f, 0.5f, 0.5f, 1.0f), Label.w - 5.0f, true);
}

// ui logics
bool CUIGameInterface::DoIconSelectionWindow(CMenus::CButtonContainer* pBC, CUIRect* pRect, CWindowUI* pWindow, int SpriteID, const char* pTagged)
{
	if(!pBC || !pWindow)
		return false;

	const float FadeVal = pWindow->IsOpenned() ? max(pBC->GetFade() + 0.5f, 1.0f) : pBC->GetFade();
	const float Size = 38.0f + (FadeVal * 6.0f);
	bool WasUseds = false;

	CUIRect IconRect = { pRect->x, pRect->y, Size, Size };
	RenderTools()->DrawUIRect(&IconRect, vec4(0.0f + FadeVal, 0.0f + FadeVal, 0.0f + FadeVal, 0.3f), CUI::CORNER_ALL, 5.0f);

	Graphics()->TextureSet(g_pData->m_aImages[IMAGE_MMOGAMEHUD].m_Id);
	Graphics()->WrapClamp();
	Graphics()->QuadsBegin();
	RenderTools()->SelectSprite(SpriteID);
	IGraphics::CQuadItem MainIcon(IconRect.x, IconRect.y, IconRect.w, IconRect.h);
	Graphics()->QuadsDrawTL(&MainIcon, 1);
	Graphics()->QuadsEnd();
	Graphics()->WrapNormal();

	if(pTagged != nullptr)
		RenderTools()->DrawUIText(TextRender(), vec2(IconRect.x, IconRect.y), pTagged, vec4(1.0f, 0.0f, 0.0f, 0.5f), vec4(1.0f,1.0f,1.0f,1.0f), 10.0f + FadeVal);

	if(UI()->HotItem() == pBC->GetID() && UI()->NextHotItem() != pBC->GetID())
	{
		CreateMouseHoveredDescription(TEXTALIGN_CENTER, 120.0f, pWindow->GetWindowName());

		const vec4 ColorHighlight = vec4(1.0f, 0.75f, 0.0f, 1.0f);
		pWindow->HighlightEnable(ColorHighlight, true);
	}
	else if(UI()->HotItem() != pBC->GetID())
	{
		pWindow->HighlightDisable();
	}

	const void* pLastActiveItem = UI()->GetActiveItem();
	if(UI()->DoButtonLogic(pBC->GetID(), &IconRect))
	{
		pWindow->Reverse();
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

bool CUIGameInterface::DoDrawItemIcon(CMenus::CButtonContainer* pBC, CUIRect* pRect, float IconSize, const CClientItem& pItem)
{
	if(!pBC)
		return false;

	pRect->w = pRect->h = IconSize;
	DrawUIRectIconItem(pRect, IconSize, pItem);

	if(UI()->HotItem() == pBC->GetID())
	{
		CUIRect InformationRect;
		InformationRect.x = m_MousePos.x;
		InformationRect.y = m_MousePos.y;
		InformationRect.w = 240.0f;
		InformationRect.h = 80.0f;
		UI()->MouseRectLimitMapScreen(&InformationRect, 1.0f, CUI::RECTLIMITSCREEN_DOWN);

		// background
		RenderTools()->DrawUIRectMonochromeGradient(&InformationRect, vec4(0.15f, 0.15f, 0.15f, 0.8f), CUI::CORNER_ALL, 12.0f);
		InformationRect.Margin(3.0f, &InformationRect);
		RenderTools()->DrawUIRectMonochromeGradient(&InformationRect, vec4(0.1f, 0.1f, 0.1f, 0.8f), CUI::CORNER_ALL, 12.0f);

		// informations
		char aBuf[256];
		CUIRect Label = InformationRect;
		Label.Margin(5.0f, &Label);
		str_format(aBuf, sizeof(aBuf), "%s", pItem.Info()->m_aName);
		UI()->DoLabel(&Label, aBuf, 12.0f, CUI::EAlignment::ALIGN_LEFT, InformationRect.w - IconSize, false);

		CUIRect IconRight;
		static float IconRightSize = 24.0f;
		Label.VSplitRight(IconRightSize, 0, &IconRight);
		IconRight.HSplitTop(IconRightSize, &IconRight, 0);
		DrawUIRectIconItem(&IconRight, IconRight.h, pItem);

		Label.HSplitTop(IconRightSize + 4.0f, 0, &Label);
		RenderTools()->DrawUIRectLine(&Label, vec4(0.3f, 0.3f, 0.3f, 0.3f), LineDirectionFlag::LINE_LEFT);
		str_format(aBuf, sizeof(aBuf), "%s", pItem.Info()->m_aDesc);
		UI()->DoLabel(&Label, aBuf, 10.0f, CUI::EAlignment::ALIGN_LEFT, InformationRect.w, false);
	}

	const void* pLastActiveItem = UI()->GetActiveItem();
	const bool Logic = UI()->DoButtonLogic(pBC->GetID(), pRect);

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

void CUIGameInterface::DrawUIRectIconItem(CUIRect* pRect, float IconSize, const CClientItem& pItem)
{
	const char* pIcon = pItem.Info()->m_aIcon;

	// draw information about item
	pRect->w = pRect->h = IconSize;
	RenderTools()->DrawUIRectMonochromeGradient(pRect, vec4(0.2f, 0.2f, 0.2f, 0.8f), CUI::CORNER_ALL, 5.0f);

	// icon
	CUIRect IconRect;
	pRect->Margin(3.0f, &IconRect);
	m_pClient->m_pMenus->DoItemIcon(pIcon, IconRect, IconRect.h);

	char aBuf[16];
	str_format(aBuf, sizeof(aBuf), "%d%s", min(pItem.m_Amount, 999), pItem.m_Amount > 999 ? "+" : "\0");
	RenderTools()->DrawUIText(TextRender(), vec2(IconRect.x - 7.0f, IconRect.y - 6.0f), aBuf, vec4(0.0f, 0.4f, 1.0f, 0.5f), vec4(1.0f, 1.0f, 1.0f, 1.0f), pRect->w / 3.0f);
}

void CUIGameInterface::CreateMouseHoveredDescription(int Align, float Width, const char* pMessage)
{
	const CUIRect* pScreen = UI()->Screen();
	Graphics()->MapScreen(pScreen->x, pScreen->y, pScreen->w, pScreen->h);
	static CTextCursor s_CursorHoveredDescription;

	// plain text for information box
	const float FontSize = 10.0f;
	s_CursorHoveredDescription.Reset();
	s_CursorHoveredDescription.m_Flags = TEXTFLAG_ALLOW_NEWLINE;
	s_CursorHoveredDescription.m_FontSize = FontSize;
	s_CursorHoveredDescription.m_Align = Align;
	s_CursorHoveredDescription.m_MaxWidth = Width;
	s_CursorHoveredDescription.m_MaxLines = -1;

	const vec4 TextColor = vec4(0.8f, 0.8f, 0.8f, 1.0f);
	const vec4 OldColor = TextRender()->GetColor();
	TextRender()->TextColor(TextColor);
	TextRender()->TextDeferred(&s_CursorHoveredDescription, pMessage, -1);
	TextRender()->TextColor(OldColor);

	CUIRect BackgroundBox = { UI()->MouseX(), UI()->MouseY(), Width, 12.0f + ((float)s_CursorHoveredDescription.LineCount() * FontSize) };
	UI()->MouseRectLimitMapScreen(&BackgroundBox, 2.0f, CUI::RECTLIMITSCREEN_UP);

	// background
	RenderTools()->DrawUIRectMonochromeGradient(&BackgroundBox, vec4(0.15f, 0.15f, 0.15f, 0.8f), CUI::CORNER_ALL, 7.0f);
	BackgroundBox.Margin(1.0f, &BackgroundBox);
	RenderTools()->DrawUIRectMonochromeGradient(&BackgroundBox, vec4(0.1f, 0.1f, 0.1f, 0.85f), CUI::CORNER_ALL, 7.0f);

	// text plain
	if(Align == TEXTALIGN_CENTER)
		BackgroundBox.x += BackgroundBox.w / 2.0f;
	s_CursorHoveredDescription.MoveTo(BackgroundBox.x, BackgroundBox.y);
	TextRender()->DrawTextPlain(&s_CursorHoveredDescription);
}

/* *
 * Information	Box		|	 GUI
 * */
static float s_InformationBoxLabelSpace = 8.0f;
void CUIGameInterface::CreateInformationBox(CWindowUI* pDependentWindow, float Width, const char* pMessage)
{
	const CUIRect* pScreen = UI()->Screen();
	Graphics()->MapScreen(pScreen->x, pScreen->y, pScreen->w, pScreen->h);

	// plain text for information box
	const float FontSize = 10.0f;
	m_CursorInformationBox.Reset();
	m_CursorInformationBox.m_Flags = TEXTFLAG_ALLOW_NEWLINE;
	m_CursorInformationBox.m_FontSize = FontSize;
	m_CursorInformationBox.m_Align = TEXTALIGN_LEFT;
	m_CursorInformationBox.m_MaxWidth = Width - (s_InformationBoxLabelSpace * 2.0f) - 1.0f;
	m_CursorInformationBox.m_MaxLines = -1;

	const vec4 TextColor = vec4(0.8f, 0.8f, 0.8f, 1.0f);
	const vec4 OldColor = TextRender()->GetColor();
	TextRender()->TextColor(TextColor);
	TextRender()->TextDeferred(&m_CursorInformationBox, pMessage, -1);
	TextRender()->TextColor(OldColor);

	// re-init and open new window
	m_pWindowInformationBox->Init(vec2(Width, 80.0f + ((float)m_CursorInformationBox.LineCount() * FontSize)), pDependentWindow, &m_ActiveGUI);
	m_pWindowInformationBox->Open();
}

void CUIGameInterface::CallbackRenderInfoWindow(const CUIRect& pWindowRect, CWindowUI& pCurrentWindow)
{
	CUIRect Label, ButtonAccept;
	pWindowRect.Margin(s_InformationBoxLabelSpace, &Label);
	Label.HSplitBottom(24.0f, &Label, &ButtonAccept);

	m_CursorInformationBox.MoveTo(Label.x, Label.y);
	TextRender()->DrawTextPlain(&m_CursorInformationBox);

	static CMenus::CButtonContainer s_ButtonAccept;
	if(m_pClient->m_pMenus->DoButton_Menu(&s_ButtonAccept, "Ok", 0, &ButtonAccept, 0, CUI::CORNER_ALL, 8.0f))
		pCurrentWindow.Close();
}

/* *
 * Popup	Box		|	 GUI
 * */
void CUIGameInterface::CreatePopupBox(CWindowUI* pDependentWindow, float Width, const char* pMessage, PopupWindowCallback Callback)
{
	const CUIRect* pScreen = UI()->Screen();
	Graphics()->MapScreen(pScreen->x, pScreen->y, pScreen->w, pScreen->h);

	// plain text for information box
	const float FontSize = 10.0f;
	m_CursorPopupBox.Reset();
	m_CursorPopupBox.m_Flags = TEXTFLAG_ALLOW_NEWLINE;
	m_CursorPopupBox.m_FontSize = FontSize;
	m_CursorPopupBox.m_Align = TEXTALIGN_LEFT;
	m_CursorPopupBox.m_MaxWidth = Width - (s_InformationBoxLabelSpace * 2.0f) - 1.0f;
	m_CursorPopupBox.m_MaxLines = -1;
	m_WindowPopupCallback = Callback;

	const vec4 TextColor = vec4(0.8f, 0.8f, 0.8f, 1.0f);
	const vec4 OldColor = TextRender()->GetColor();
	TextRender()->TextColor(TextColor);
	TextRender()->TextDeferred(&m_CursorPopupBox, pMessage, -1);
	TextRender()->TextColor(OldColor);

	// re-init and open new window
	m_pWindowPopupBox->Init(vec2(Width, 80.0f + ((float)m_CursorPopupBox.LineCount() * FontSize)), pDependentWindow, &m_ActiveGUI);
	m_pWindowPopupBox->Open();
}


void CUIGameInterface::CallbackRenderGuiPopupBox(const CUIRect& pWindowRect, CWindowUI& pCurrentWindow)
{
	CUIRect Label, ButtonAccept, ButtonDeny;
	pWindowRect.Margin(s_InformationBoxLabelSpace, &Label);
	Label.HSplitBottom(20.0f, &Label, &ButtonAccept);
	ButtonAccept.VSplitLeft(Label.w / 2.0f, &ButtonDeny, &ButtonAccept);

	m_CursorPopupBox.MoveTo(Label.x, Label.y);
	TextRender()->DrawTextPlain(&m_CursorPopupBox);

	static CMenus::CButtonContainer s_ButtonAccept;
	if(m_pClient->m_pMenus->DoButton_Menu(&s_ButtonAccept, "Yes", 0, &ButtonAccept, 0, CUI::CORNER_ALL, 8.0f))
	{
		if(m_WindowPopupCallback)
			m_WindowPopupCallback(&pCurrentWindow, true);
		pCurrentWindow.Close();
	}

	static CMenus::CButtonContainer s_ButtonDeny;
	if(m_pClient->m_pMenus->DoButton_Menu(&s_ButtonDeny, "No", 0, &ButtonDeny, 0, CUI::CORNER_ALL, 8.0f))
	{
		if(m_WindowPopupCallback)
			m_WindowPopupCallback(&pCurrentWindow, false);
		pCurrentWindow.Close();
	}
}

void CUIGameInterface::CallbackPopupDeleteLetter(const CWindowUI* pPopupWindow, bool ButtonYes)
{
	if(ButtonYes)
	{
		SendLetterAction(m_pLetterSelected, MAILLETTERFLAG_DELETE | MAILLETTERFLAG_REFRESH);
		m_pWindowMailboxLetter->Close();
	}
}
