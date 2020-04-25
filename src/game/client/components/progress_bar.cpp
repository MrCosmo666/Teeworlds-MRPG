/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#include <engine/shared/config.h>
#include <engine/graphics.h>
#include <engine/textrender.h>
#include <engine/keys.h>

#include <generated/protocol.h>
#include <generated/client_data.h>

#include "console.h"
#include "menus.h"
#include "talktext.h"
#include "progress_bar.h"

#define COLOR_BACKGROUND vec4(0.2f, 0.2f, 0.2f, 0.4f)
#define COLOR_BACKBACKGROUND vec4(0.2f, 0.2f, 0.2f, 0.2f)

void CProgressBar::Clear()
{
	m_ProgressTime = 0;
	mem_zero(m_ProgressText, sizeof(m_ProgressText));
}

bool CProgressBar::IsActive()
{
	return (bool)(m_ProgressTime > 0.0f);
}

void CProgressBar::OnStateChange(int NewState, int OldState)
{
	if(OldState == IClient::STATE_ONLINE || OldState == IClient::STATE_OFFLINE)
		Clear();
}

void CProgressBar::OnRender()
{
	if(!IsActive())
		return;

	float Width = 400 * 3.0f * Graphics()->ScreenAspect();
	float Height = 400 * 3.0f;
	Graphics()->MapScreen(0, 0, Width, Height);
	if (Client()->LocalTime() < m_ProgressTime)
	{
		// --------------------- BACKGROUND -----------------------
		// --------------------------------------------------------
		CUIRect BackgroundMain = { Width / 3.0f, Height - 120.0f, Width / 3.0f, Height / 17.0f };	
		RenderTools()->DrawRoundRect(&BackgroundMain, COLOR_BACKBACKGROUND, 30.0f);

		CUIRect BackgroundOther;
		BackgroundMain.VMargin(10.0f, &BackgroundOther);
		RenderTools()->DrawRoundRect(&BackgroundOther, COLOR_BACKGROUND, 30.0f);
		BackgroundOther.VMargin(20.0f, &BackgroundOther);

		// ---------------------- EXP BAR -------------------------
		// --------------------------------------------------------
		char aBuf[128];
		CUIRect ExpBar;
		BackgroundOther.HMargin(15.0f, &ExpBar);
		ExpBar.h = 40.0f;
		str_format(aBuf, sizeof(aBuf), "%d / %d", m_ProgressCount, m_ProgressRequest);
		RenderTools()->DrawUIBar(TextRender(), ExpBar, vec4(0.10f, 0.70f, 0.06f, 0.5f),
			m_ProgressCount, m_ProgressRequest, aBuf, 5, 10.0f, 6.0f);

		// ----------------------- TEXT ---------------------------
		// --------------------------------------------------------
		CTextCursor Cursor;
		float FontSize = 32.0f;
		float CenterText = BackgroundOther.x + (BackgroundOther.w / 2.0f);
		float tw = TextRender()->TextWidth(0, FontSize, m_ProgressText, -1, -1.0f);
		BackgroundOther.HSplitBottom(95.0f, 0, &BackgroundOther);
		TextRender()->SetCursor(&Cursor, CenterText - tw / 2.0f, BackgroundOther.y, FontSize, TEXTFLAG_RENDER);
		Cursor.m_LineWidth = BackgroundOther.w;
		Cursor.m_MaxLines = ceil(BackgroundOther.h / FontSize);
		TextRender()->TextEx(&Cursor, m_ProgressText, -1);
	}
}

void CProgressBar::OnMessage(int MsgType, void *pRawMsg)
{
	if(Client()->State() == IClient::STATE_DEMOPLAYBACK)
		return;

	if(MsgType == NETMSGTYPE_SV_PROGRESSBAR)
	{
		CNetMsg_Sv_ProgressBar* pMsg = (CNetMsg_Sv_ProgressBar*)pRawMsg;
		m_ProgressCount = pMsg->m_pCount;
		m_ProgressRequest = pMsg->m_pRequires;
		m_ProgressTime = Client()->LocalTime() + 2.0f;
		str_copy(m_ProgressText, pMsg->m_pText, sizeof(m_ProgressText));
	}
}

bool CProgressBar::OnInput(IInput::CEvent Event)
{
	// fix console Press TAB
	if (m_pClient->m_pGameConsole->IsConsoleActive())
		return false;

	if(IsActive() && Event.m_Flags&IInput::FLAG_PRESS && Event.m_Key == KEY_TAB)
		return true;

	return false;
}