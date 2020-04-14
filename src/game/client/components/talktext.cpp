/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#include <generated/protocol.h>
#include <generated/client_data.h>

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

// очистка текста
void CTalkText::Clear()
{
	m_TalkClientID = 0;
	m_TalkedEmote = 0;
	mem_zero(m_TalkText, sizeof(m_TalkText));
}

// если активный
bool CTalkText::IsActive()
{
	// dont render talktext if the menu is active
	return m_TalkClientID > 0;
}

// изменения статуса
void CTalkText::OnStateChange(int NewState, int OldState)
{
	if(OldState == IClient::STATE_ONLINE || OldState == IClient::STATE_OFFLINE)
		Clear();
}

// прорисовка
void CTalkText::OnRender()
{
	if(!IsActive())
		return;

	int TalkingEmoticion = SPRITE_DOTDOT;
	float Width = 400 * 3.0f * Graphics()->ScreenAspect();
	float Height = 400 * 3.0f;
	Graphics()->MapScreen(0, 0, Width, Height);

	// --------------------- BACKGROUND -----------------------
	// --------------------------------------------------------
	CUIRect BackgroundOther;
	{
		vec4 ColorBackground = vec4(0.3f, 0.23f, 0.15f, 0.5f);
		vec4 ColorBackgroundOther = vec4(0.05f, 0.05f, 0.05f, 0.35f);
		if (m_Style == TALK_STYLE_AGRESSIVE)
		{
			TalkingEmoticion = SPRITE_DEVILTEE;
			ColorBackground = vec4(0.6f, 0.15f, 0.22f, 0.4f);
			ColorBackgroundOther = vec4(0.05f, 0.05f, 0.05f, 0.35f);
		}
		else if (m_Style == TALK_STYLE_HAPPED)
		{
			TalkingEmoticion = SPRITE_EYES;
			ColorBackground = vec4(0.15f, 0.6f, 0.22f, 0.4f);
			ColorBackgroundOther = vec4(0.05f, 0.05f, 0.05f, 0.35f);
		}

		CUIRect BackgroundMain = { Width / 4.0f, Height / 1.8f, Width / 2.0f, Height / 8.0f };
		RenderTools()->DrawRoundRect(&BackgroundMain, ColorBackground, 30.0f);

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

	// ------------------------ TEXT --------------------------
	// --------------------------------------------------------
	CTextCursor Cursor;
	int LocalClientID = m_pClient->m_LocalClientID; float FontSize = 22.0f;
	TextRender()->SetCursor(&Cursor, BackgroundOther.x, BackgroundOther.y, FontSize, TEXTFLAG_RENDER);
	Cursor.m_LineWidth = BackgroundOther.w;
	Cursor.m_MaxLines = ceil(BackgroundOther.h / FontSize);
	TextRender()->TextEx(&Cursor, m_TalkText, -1);

	// -------------------- PLAYER SKINS ----------------------
	// --------------------------------------------------------
	int TalkClientID = m_TalkClientID;
	if(m_pClient->m_aClients[LocalClientID].m_Active ||
		(TalkClientID >= 0 && TalkClientID < MAX_CLIENTS && m_pClient->m_aClients[TalkClientID].m_Active))
	{
		if (LocalClientID != TalkClientID) // NPC 
		{
			// skin
			CTeeRenderInfo RenderTalking = m_pClient->m_aClients[TalkClientID].m_RenderInfo;
			RenderTalking.m_Size = 128.0f;
			RenderTools()->RenderTee(CAnimState::GetIdle(), &RenderTalking, m_TalkedEmote, vec2(-1.0f, 0.4f), vec2(Width / 1.35f, Height / 1.85f));

			float sizeLize = str_length(m_pClient->m_aClients[TalkClientID].m_aName);
			TextRender()->Text(0x0, (Width / (1.45f + sizeLize / 64.0f)), Height / 1.97f, 32.0f, m_pClient->m_aClients[TalkClientID].m_aName, -1.0f);
		}

		// skin
		CTeeRenderInfo RenderYou = m_pClient->m_aClients[LocalClientID].m_RenderInfo;
		RenderYou.m_Size = 128.0f;
		RenderTools()->RenderTee(CAnimState::GetIdle(), &RenderYou,
			m_pClient->m_aClients[LocalClientID].m_Emoticon, vec2(1.0f, 0.4f), vec2(Width / 4.0f, Height / 1.85f));

		TextRender()->Text(0x0, Width / 3.5f, Height / 1.97f, 32.0f, m_pClient->m_aClients[LocalClientID].m_aName, -1.0f);
	}

	// ------------------ INTERACTIVE TEXT -----------------
	// -----------------------------------------------------
	TextRender()->Text(0x0, Width / 1.8f, Height / 1.50f, 25.0f, Localize("Press (TAB) for continue!"), -1.0f);
}

// пакеты между сервером клиентом
void CTalkText::OnMessage(int MsgType, void *pRawMsg)
{
	if(Client()->State() == IClient::STATE_DEMOPLAYBACK)
		return;

	if(MsgType == NETMSGTYPE_SV_TALKTEXT)
	{
		CNetMsg_Sv_TalkText* pMsg = (CNetMsg_Sv_TalkText*)pRawMsg;
		str_copy(m_TalkText, pMsg->m_pText, sizeof(m_TalkText));
		m_TalkClientID = pMsg->m_pTalkClientID;
		m_TalkedEmote = pMsg->m_TalkedEmote;
		m_PlayerTalked = pMsg->m_PlayerTalked;
		m_Style = pMsg->m_Style;
	}
	else if (MsgType == NETMSGTYPE_SV_CLEARTALKTEXT)
	{
		Clear();
	}
}

// ожидание прожатия введение действий
bool CTalkText::OnInput(IInput::CEvent Event)
{
	// fix console Press TAB
	if (m_pClient->m_pGameConsole->IsConsoleActive())
		return false;

	if(IsActive() && Event.m_Flags&IInput::FLAG_PRESS && Event.m_Key == KEY_TAB)
	{
		m_pClient->m_pSounds->Play(CSounds::CHN_WORLD, SOUND_UI_SELECTED_CLICK, 100.00f);
		ClientPressed();
		return true;
	}
	return false;
}

// нажатие клиента продолжения
void CTalkText::ClientPressed()
{
	if(!IsActive())
		return;

	CNetMsg_Cl_TalkInteractive Msg;
	Client()->SendPackMsg(&Msg, MSGFLAG_VITAL);
}