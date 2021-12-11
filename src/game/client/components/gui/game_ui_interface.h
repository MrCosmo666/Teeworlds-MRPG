#ifndef GAME_CLIENT_COMPONENTS_INTERFACE_H
#define GAME_CLIENT_COMPONENTS_INTERFACE_H

#include <game/client/component.h>

#include <game/game_context.h>
#include <game/client/ui_window.h>

#include <map>


class CUIGameInterface : public CComponent
{
public:
	// structures
	// TODO: remove refactor it and change
	struct CItemDataClientInfo : public CItemDataInformation
	{
		int m_ItemID;
	};

	struct CClientItem
	{
		int m_ItemID;
		int m_Amount;
		int m_Enchant;

		bool Empty() const { return m_ItemID <= 0 || m_Amount <= 0; }
		CItemDataClientInfo* Info() const { return &m_aItemsDataInformation[m_ItemID]; }
	};
	static std::map< int /*itemid*/, CItemDataClientInfo > m_aItemsDataInformation;
	static std::map< int /*itemid*/, CClientItem > m_aClientItems;

	struct CMailboxLetter
	{
		int m_MailLetterID;
		char m_aName[32];
		char m_aDesc[128];
		char m_aFrom[32];
		bool m_IsRead;

		CClientItem m_AttachmentItem;
	};
	std::vector < CMailboxLetter > m_aLettersList;
	class CElementsGUI* m_ElemGUI;
	
private:
	vec2 m_MousePos;
	bool m_ActiveGUI;
	void RenderGuiIcons();

	// gui callbacks
	void CallbackPopupDeleteLetter(const CWindowUI* pPopupWindow, bool ButtonYes);
	
	// inbox
	enum MailBoxGUI
	{
		MAILBOX_GUI_LIST,
		MAILBOX_GUI_LETTER_SEND,
		MAILBOX_GUI_LETTER_INFO,
		MAILBOX_GUI_LETER_ACTION,
		NUM_MAILBOX_GUI
	};
	CMailboxLetter* m_pLetterSelected;
	CWindowUI* m_pWindowMailbox[NUM_MAILBOX_GUI];
	void CallbackRenderMailboxList(const CUIRect& pWindowRect, CWindowUI& pCurrentWindow);
	void CallbackRenderMailboxListButtonHelp(const CUIRect& pWindowRect, CWindowUI& pCurrentWindow);
	void CallbackRenderMailboxLetter(const CUIRect& pWindowRect, CWindowUI& pCurrentWindow);
	void CallbackRenderMailboxLetterSend(const CUIRect& pWindowRect, CWindowUI& pCurrentWindow);
	void CallbackRenderMailboxLetterActions(const CUIRect& pWindowRect, CWindowUI& pCurrentWindow);

	void SendLetterAction(CMailboxLetter* pLetter, int64 Flags);
	bool UnreadLetterMails() const;

	// questing
	enum QuestingGUI
	{
		QUESTING_GUI_LIST,
		NUM_QUESTING_GUI
	};
	CWindowUI* m_pWindowQuesting[NUM_QUESTING_GUI];
	void CallbackRenderQuests(const CUIRect& pWindowRect, CWindowUI& pCurrentWindow);

public:
	CUIGameInterface();
	~CUIGameInterface() override;
	bool IsActiveGUI() const { return m_ActiveGUI; }

	void OnInit() override;
	void OnRender() override;
	void OnReset() override;
	void OnConsoleInit() override;
	bool OnInput(IInput::CEvent Event) override;
	void OnStateChange(int NewState, int OldState) override;
	bool OnCursorMove(float x, float y, int CursorType) override;
	void OnMessage(int Msg, void* pRawMsg) override;
	void OnRelease() override;

	static void ConToggleGameHUDMRPG(IConsole::IResult* pResult, void* pUser);

	// ui logics
	void DrawUIRectIconItem(class CUIRect* pRect, float IconSize, const CClientItem& pItem);
	bool DoDrawItemIcon(CMenus::CButtonContainer* pBC, class CUIRect* pRect, float IconSize, const CClientItem& pItem);

	bool DoIconSelectionWindow(CMenus::CButtonContainer* pBC, class CUIRect* pRect, class CWindowUI* pWindow, int SpriteID, const char* pTagged = nullptr);
	void CreateMouseHoveredDescription(int Align, float Width, const char* pMessage);
};

#endif