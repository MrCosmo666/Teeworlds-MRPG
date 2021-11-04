/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#ifndef GAME_CLIENT_COMPONENTS_TALKTEXT_H
#define GAME_CLIENT_COMPONENTS_TALKTEXT_H
#include <game/client/component.h>

#define TALKING_SIZE 512

class CTalkText : public CComponent
{
	CAnimElementsUI* m_pAnimBackground;
	CAnimElementsUI* m_pAnimBackgroundOther;

	// mmotee talk text
	int m_ConversationClientID;
	int m_Emote;
	int m_EmoticionSpriteID;
	int m_DialogFlag;
	bool m_Stranger;
	CTextCursor m_TextCursor;

	int m_UpdateDialogCharPos;
	int64 m_UpdateDialogTextTime;
	char m_aDialogText[TALKING_SIZE];
	char m_aUpdatedDialogText[TALKING_SIZE];

	float m_ScreenWidth;
	float m_ScreenHeight;

	void UpdateDialogText();
	void Clear();
	const char* GetTalkText() const { return m_aDialogText; }

public:
	bool IsActive() const;

	void OnInit() override;
	void OnStateChange(int NewState, int OldState) override;
	void OnRender() override;
	void OnMessage(int MsgType, void *pRawMsg) override;
	bool OnInput(IInput::CEvent Event) override;

};

#endif
