/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#ifndef GAME_CLIENT_COMPONENTS_TALKTEXT_H
#define GAME_CLIENT_COMPONENTS_TALKTEXT_H
#include <game/client/component.h>

class CTalkText : public CComponent
{
	// mmotee render talk text
	int m_TalkClientID;
	int m_Style;
	int m_TalkedEmote;
	bool m_PlayerTalked;
	char m_TalkText[512];

	void ClientPressed();

public:
	void Clear();
	bool IsActive();
	const char *GetTalkText() const { return m_TalkText; }

	virtual void OnRender();
	virtual void OnStateChange(int NewState, int OldState);
	virtual void OnMessage(int MsgType, void *pRawMsg);
	virtual bool OnInput(IInput::CEvent Event);

};

#endif
