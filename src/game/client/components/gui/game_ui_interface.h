#ifndef GAME_CLIENT_COMPONENTS_INTERFACE_H
#define GAME_CLIENT_COMPONENTS_INTERFACE_H

#include <game/client/component.h>

#include <game/game_context.h>
#include <game/client/ui_window.h>

#include <map>
#include <functional>

#define POPUP_REGISTER(f, o)  std::bind(f, o, std::placeholders::_1, std::placeholders::_2)

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

private:
	vec2 m_MousePos;
	bool m_ActiveGUI;
	void RenderGuiIcons();

	// test window

	// gui information box
	CTextCursor m_CursorInformationBox;
	CWindowUI* m_pWindowInformationBox;
	void CallbackRenderInfoWindow(const CUIRect& pWindowRect, CWindowUI& pCurrentWindow);

	// gui popup box
	typedef std::function<void(CWindowUI*, bool)> PopupWindowCallback;
	PopupWindowCallback m_WindowPopupCallback;
	CTextCursor m_CursorPopupBox;
	CWindowUI* m_pWindowPopupBox;
	void CallbackRenderGuiPopupBox(const CUIRect& pWindowRect, CWindowUI& pCurrentWindow);

	// popup callbacks
	void CallbackPopupDeleteLetter(const CWindowUI* pPopupWindow, bool ButtonYes);

	// inbox
	CMailboxLetter* m_pLetterSelected;
	CWindowUI* m_pWindowMailboxList;
	void CallbackRenderMailboxList(const CUIRect& pWindowRect, CWindowUI& pCurrentWindow);
	void CallbackRenderMailboxListButtonHelp(const CUIRect& pWindowRect, CWindowUI& pCurrentWindow);

	CWindowUI* m_pWindowMailboxLetterSend;
	void CallbackRenderMailboxLetter(const CUIRect& pWindowRect, CWindowUI& pCurrentWindow);

	CWindowUI* m_pWindowMailboxLetter;
	void CallbackRenderMailboxLetterSend(const CUIRect& pWindowRect, CWindowUI& pCurrentWindow);

	CWindowUI* m_pWindowMailboxLetterActions;
	void CallbackRenderMailboxLetterActions(const CUIRect& pWindowRect, CWindowUI& pCurrentWindow);

	void SendLetterAction(CMailboxLetter* pLetter, int64 Flags);
	bool UnreadLetterMails() const;

	// questing
	CWindowUI* m_pWindowQuestsList;
	void CallbackRenderQuests(const CUIRect& pWindowRect, CWindowUI& pCurrentWindow);

public:
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
	void CreateInformationBox(CWindowUI* pDependentWindow, float Width, const char* pMessage);
	void CreatePopupBox(CWindowUI* pDependentWindow, float Width, const char* pMessage, PopupWindowCallback Callback);
	void CreateMouseHoveredDescription(int Align, float Width, const char* pMessage);
};

#endif