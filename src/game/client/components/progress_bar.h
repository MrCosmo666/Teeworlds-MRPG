/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#ifndef GAME_CLIENT_COMPONENTS_PROGRESS_BAR_H
#define GAME_CLIENT_COMPONENTS_PROGRESS_BAR_H
#include <game/client/component.h>

class CProgressBar : public CComponent
{
	char m_ProgressText[128];
	float m_ProgressTime;
	int m_ProgressCount;
	int m_ProgressRequest;

public:
	bool IsActive() const;
	void Clear();

	virtual void OnRender();
	virtual void OnStateChange(int NewState, int OldState);
	virtual void OnMessage(int MsgType, void *pRawMsg);
	virtual bool OnInput(IInput::CEvent Event);

};

#endif
