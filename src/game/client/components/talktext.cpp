/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#include <generated/protocol.h>
#include <generated/client_data.h>

#include <base/color.h>

#include <engine/shared/config.h>
#include <engine/graphics.h>
#include <engine/textrender.h>
#include <engine/keys.h>

#include <game/client/animstate.h>
#include <game/client/gameclient.h>

#include "binds.h"
#include "console.h"
#include "menus.h"
#include "sounds.h"

#include "talktext.h"

void CTalkText::Clear()
{
	m_TalkClientID = 0;
	m_TalkedEmote = 0;
	m_RegrnizedTalkTime = 0;
	m_RegrnizedTalkPosition = 0;
	mem_zero(m_TalkText, sizeof(m_TalkText));
	mem_zero(m_RegrnizedTalkText, sizeof(m_RegrnizedTalkText));
}

bool CTalkText::IsActive()
{
	return (bool)(m_TalkClientID > 0 && m_pClient->m_pMenus->IsActive() <= 0);
}

void CTalkText::OnStateChange(int NewState, int OldState)
{
	if(OldState == IClient::STATE_ONLINE || OldState == IClient::STATE_OFFLINE)
		Clear();
}

void CTalkText::RegrnizedTalkingText()
{
	if(time_get() > m_RegrnizedTalkTime)
	{
		if(m_TalkText[m_RegrnizedTalkPosition] != '\0')
		{
			m_RegrnizedTalkText[m_RegrnizedTalkPosition] = m_TalkText[m_RegrnizedTalkPosition];
			m_RegrnizedTalkPosition++;
			m_RegrnizedTalkTime = time_get() + time_freq() / g_Config.m_ClDialogsSpeedNPC;
		}
		else
			m_RegrnizedTalkTime = 0;
	}
}

void CTalkText::OnRender()
{
	if(!IsActive() || m_pClient->m_pMenus->IsActive())
		return;

	int TalkingEmoticion = SPRITE_DOTDOT;
	const float Width = 400 * 3.0f * Graphics()->ScreenAspect();
	const float Height = 400 * 3.0f;
	Graphics()->MapScreen(0, 0, Width, Height);

	// --------------------- BACKGROUND -----------------------
	// --------------------------------------------------------
	CUIRect BackgroundOther;
	{
		vec4 ColorBackground = vec4(0.06f, 0.13f, 0.37f, 0.65f);
		vec4 ColorBackgroundOther = vec4(0.05f, 0.05f, 0.05f, 0.35f);
		if (m_Style == TALK_STYLE_AGRESSIVE)
		{
			TalkingEmoticion = SPRITE_DEVILTEE;
			ColorBackground = vec4(0.37f, 0.06f, 0.11f, 0.65f);
			ColorBackgroundOther = vec4(0.05f, 0.05f, 0.05f, 0.35f);
		}
		else if (m_Style == TALK_STYLE_HAPPED)
		{

			TalkingEmoticion = SPRITE_EYES;
			ColorBackground = vec4(0.08f, 0.34f, 0.10f, 0.65f);
			ColorBackgroundOther = vec4(0.05f, 0.05f, 0.05f, 0.35f);
		}

		CUIRect BackgroundMain = { Width / 4.0f, Height / 1.8f, Width / 2.0f, Height / 8.0f };
		RenderTools()->DrawUIRect4(&BackgroundMain, ColorBackground, ColorBackground, ColorBackground / 1.8f, ColorBackground / 1.8f, CUI::CORNER_ALL, 30.0f);

		BackgroundMain.Margin(10.0f, &BackgroundOther);
		RenderTools()->DrawRoundRect(&BackgroundOther, ColorBackgroundOther, 30.0f);
		BackgroundOther.VMargin(20.0f, &BackgroundOther);
	}

	// ---------------------- EMOTICION TALKED ----------------
	// --------------------------------------------------------
	vec4 PositionTalked = (m_PlayerTalked ? vec4(Width / 3.5f, Height / 2.0f, 128.0f, 128.0f) : vec4(Width / 1.41f, Height / 2.0f, -128.0f, 128.0f));
	Graphics()->TextureSet(g_pData->m_aImages[IMAGE_EMOTICONS].m_Id);
	Graphics()->QuadsBegin();

	RenderTools()->SelectSprite(TalkingEmoticion);
	IGraphics::CQuadItem QuadItem(PositionTalked.x, PositionTalked.y, PositionTalked.z, PositionTalked.w);
	Graphics()->QuadsDraw(&QuadItem, 1);

	Graphics()->QuadsEnd();

	// -------------------- PLAYER SKINS ----------------------
	// --------------------------------------------------------
	const int TalkClientID = m_TalkClientID;
	const int LocalClientID = m_pClient->m_LocalClientID;
	if(m_pClient->m_aClients[LocalClientID].m_Active ||
		(TalkClientID >= 0 && TalkClientID < MAX_CLIENTS && m_pClient->m_aClients[TalkClientID].m_Active))
	{
		if (LocalClientID != TalkClientID) // NPC 
		{
			// skin
			CTeeRenderInfo RenderTalking = m_pClient->m_aClients[TalkClientID].m_RenderInfo;
			RenderTalking.m_Size = 128.0f;
			RenderTools()->RenderTee(CAnimState::GetIdle(), &RenderTalking, m_PlayerTalked ? EMOTE_NORMAL : m_TalkedEmote, vec2(-1.0f, 0.4f), vec2(Width / 1.35f, Height / 1.85f));

			if(m_Stranger)
				TextRender()->TextColor(1.0f, 0.95f, 0.0f, 1);

			const char* pTalkedNick = m_Stranger ? "Stranger" : m_pClient->m_aClients[TalkClientID].m_aName;
			float sizeLize = str_length(pTalkedNick);
			TextRender()->Text(nullptr, (Width / (1.45f + sizeLize / 64.0f)), Height / 1.97f, 32.0f, pTalkedNick, -1.0f);
			TextRender()->TextColor(1, 1, 1, 1);
		}

		// skin
		CTeeRenderInfo RenderYou = m_pClient->m_aClients[LocalClientID].m_RenderInfo;
		RenderYou.m_Size = 128.0f;
		RenderTools()->RenderTee(CAnimState::GetIdle(), &RenderYou,
			m_PlayerTalked ? m_TalkedEmote : EMOTE_NORMAL, vec2(1.0f, 0.4f), vec2(Width / 4.0f, Height / 1.85f));

		TextRender()->Text(nullptr, Width / 3.5f, Height / 1.97f, 32.0f, m_pClient->m_aClients[LocalClientID].m_aName, -1.0f);
	}

	// ------------------------ TEXT --------------------------
	// --------------------------------------------------------
	CTextCursor Cursor; float FontSize = 22.0f;
	TextRender()->SetCursor(&Cursor, BackgroundOther.x, BackgroundOther.y, FontSize, TEXTFLAG_RENDER);
	Cursor.m_LineWidth = BackgroundOther.w;
	Cursor.m_MaxLines = ceil(BackgroundOther.h / FontSize);
	TextRender()->TextColor(1.0f, 1.0f, 1.0f, 0.9f);
	TextRender()->TextEx(&Cursor, m_RegrnizedTalkText, -1);


	// ------------------ INTERACTIVE TEXT -----------------
	// -----------------------------------------------------
	TextRender()->Text(nullptr, Width / 1.8f, Height / 1.50f, 25.0f, Localize("Press (TAB) for continue!"), -1.0f);
	TextRender()->TextColor(1.0f, 1.0f, 1.0f, 1.0f);
	RegrnizedTalkingText();
}

void CTalkText::OnMessage(int MsgType, void *pRawMsg)
{
	if(Client()->State() == IClient::STATE_DEMOPLAYBACK)
		return;

	if(MsgType == NETMSGTYPE_SV_TALKTEXT)
	{
		// clear snake text
		m_RegrnizedTalkTime = 0;
		m_RegrnizedTalkPosition = 0;
		mem_zero(m_RegrnizedTalkText, sizeof(m_RegrnizedTalkText));

		// start new dialogs
		CNetMsg_Sv_TalkText* pMsg = (CNetMsg_Sv_TalkText*)pRawMsg;
		str_copy(m_TalkText, pMsg->m_pText, sizeof(m_TalkText));
		m_TalkClientID = pMsg->m_pTalkClientID;
		m_TalkedEmote = pMsg->m_TalkedEmote;
		m_PlayerTalked = pMsg->m_PlayerTalked;
		m_Style = pMsg->m_Style;
		m_Stranger = (bool)(str_replace(m_TalkText, "[Stranger]", "\0") > 0);
		m_RegrnizedTalkTime = time_get() + time_freq() / g_Config.m_ClDialogsSpeedNPC;
	}
	else if(MsgType == NETMSGTYPE_SV_CLEARTALKTEXT)
	{
		Clear();
	}
}

bool CTalkText::OnInput(IInput::CEvent Event)
{
	// fix console Press TAB
	if (m_pClient->m_pGameConsole->IsConsoleActive() || !IsActive())
		return false;

	if(IsActive() && Event.m_Flags&IInput::FLAG_PRESS && Event.m_Key == KEY_TAB)
	{
		//m_pClient->m_pSounds->Play(CSounds::CHN_WORLD, SOUND_UI_SELECTED_CLICK, 100.00f);
		ClientPressed();
		return true;
	}
	return false;
}

void CTalkText::ClientPressed()
{
	CNetMsg_Cl_TalkInteractive Msg;
	Client()->SendPackMsg(&Msg, MSGFLAG_VITAL);
}