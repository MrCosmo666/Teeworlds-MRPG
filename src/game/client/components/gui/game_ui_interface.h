#ifndef GAME_CLIENT_COMPONENTS_INTERFACE_H
#define GAME_CLIENT_COMPONENTS_INTERFACE_H

#include <game/client/component.h>

#include <game/game_context.h>
#include <game/client/ui_window.h>

class CUIGameInterface : public CComponent
{
	bool m_ActiveHUD;
	vec2 m_MousePos;
	int m_LetterActiveSelected;

	// gui
	char m_aInformationBoxBuf[256];
	CWindowUI m_WindowInformationBox;
	void RenderGuiElements();

	// inbox
	bool m_GotNewLetter;
	CWindowUI m_WindowMailboxList;
	CWindowUI m_WindowMailboxLetterSelected;
	CWindowUI m_WindowMailboxLetterSend;
	void RefreshLetters();
	void RenderInbox();

	// questing
	CWindowUI m_WindowQuestsList;
	void RenderQuests();

public:
	virtual void OnInit();
	virtual void OnRender();
	virtual void OnReset();
	virtual void OnConsoleInit();
	virtual bool OnInput(IInput::CEvent Event);
	virtual void OnStateChange(int NewState, int OldState);
	virtual bool OnCursorMove(float x, float y, int CursorType);
	virtual void OnMessage(int Msg, void* pRawMsg);

	static void ConToggleGameHUDMRPG(IConsole::IResult* pResult, void* pUser);

	bool IsActiveHUD() const { return m_ActiveHUD; }

	// ui logics
	bool DoIconSelectionWindow(CMenus::CButtonContainer* pBC, class CUIRect* pRect, class CWindowUI* pWindow, int SpriteID);
	bool DoDrawItemIcon(CMenus::CButtonContainer* pBC, class CUIRect* pRect, float IconSize, int ItemID);
	void DrawUIRectIconItem(class CUIRect* pRect, float IconSize, int ItemID);
	void CreateInformationBox(const char *pMessage);

	// structures
	struct CMailboxLetter
	{
		int m_MailID;
		char m_aName[32];
		char m_aDesc[128];
		int m_ItemID;
		int m_Count;
		int m_Enchant;
	};
	std::vector < CMailboxLetter > m_aLettersList;

	// TODO: remove refactor it and change
	struct CItemDataClientInfo : public CItemDataInformation
	{
		int m_ItemID;
	};

	struct CClientItem
	{
		CClientItem()
		{
			m_Count = 0;
			m_Enchant = 0;
		}

		int m_ItemID;
		int m_Count;
		int m_Enchant;
		CItemDataClientInfo* Info() { return &m_aItemsDataInformation[m_ItemID]; }
	};
	static std::map< int /*itemid*/, CItemDataClientInfo > m_aItemsDataInformation;
	static std::map< int /*itemid*/, CClientItem > m_aClientItems;

};

#endif