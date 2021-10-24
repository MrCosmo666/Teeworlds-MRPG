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

#include "console.h"
#include "menus.h"
#include "sounds.h"

#include "talktext.h"

// TODO: rework

bool CTalkText::IsActive() const
{
	return (bool)(m_TalkClientID >= 0 && !m_pClient->m_pMenus->IsActive());
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

void CTalkText::Clear()
{
	m_TalkClientID = -1;
	m_TalkedEmote = 0;
	m_RegrnizedTalkTime = 0;
	m_RegrnizedTalkPosition = 0;
	mem_zero(m_TalkText, sizeof(m_TalkText));
	mem_zero(m_RegrnizedTalkText, sizeof(m_RegrnizedTalkText));
}

void CTalkText::OnInit()
{
	Clear();

	m_ScreenWidth = 400 * 3.0f * Graphics()->ScreenAspect();
	m_ScreenHeight = 400 * 3.0f;
	m_pAnimBackground = m_pClient->AnimUI()->Get("TalkingBackground");
	m_pAnimBackgroundOther = m_pClient->AnimUI()->Get("TalkingBackgroundOther");

	CUIRect BackgroundMainInit = { m_ScreenWidth / 2.0f, m_ScreenHeight / 1.6f, 0, 0 };
	m_pAnimBackground->GetPos()->Init(&BackgroundMainInit);
	m_pAnimBackgroundOther->GetPos()->Init(&BackgroundMainInit);
}

void CTalkText::OnStateChange(int NewState, int OldState)
{
	if(OldState == IClient::STATE_ONLINE || OldState == IClient::STATE_OFFLINE)
		Clear();
}

void CTalkText::OnRender()
{
	if(!IsActive())
		return;

	Graphics()->MapScreen(0, 0, m_ScreenWidth, m_ScreenHeight);

	// --------------------- BACKGROUND -----------------------
	// --------------------------------------------------------
	m_pAnimBackground->Draw();
	m_pAnimBackgroundOther->Draw();
	if(!IsActive() || !m_pAnimBackground->GetPos()->m_AnimEnded)
		return;

	// ------------------ EMOTICION TALKED --------------------
	// --------------------------------------------------------
	vec4 PositionTalked = (m_PlayerTalked ? vec4(m_ScreenWidth / 3.5f, m_ScreenHeight / 2.0f, 128.0f, 128.0f) : vec4(m_ScreenWidth / 1.41f, m_ScreenHeight / 2.0f, -128.0f, 128.0f));
	Graphics()->TextureSet(g_pData->m_aImages[IMAGE_EMOTICONS].m_Id);
	Graphics()->QuadsBegin();

	RenderTools()->SelectSprite(m_TalkedEmoticionSpriteID);
	IGraphics::CQuadItem QuadItem(PositionTalked.x, PositionTalked.y, PositionTalked.w, PositionTalked.h);
	Graphics()->QuadsDraw(&QuadItem, 1);

	Graphics()->QuadsEnd();

	// ---------------- PLAYER SKINS NICKNAME -----------------
	// --------------------------------------------------------
	m_TextCursor.Reset();
	m_TextCursor.m_FontSize = 32.0f;

	const int TalkClientID = m_TalkClientID;
	const int LocalClientID = m_pClient->m_LocalClientID;
	if(m_pClient->m_aClients[LocalClientID].m_Active || (TalkClientID >= 0 && TalkClientID < MAX_CLIENTS && m_pClient->m_aClients[TalkClientID].m_Active))
	{
		// dialogue with someone
		if (LocalClientID != TalkClientID)
		{
			CTeeRenderInfo RenderTalking = m_pClient->m_aClients[TalkClientID].m_RenderInfo;
			RenderTalking.m_Size = 128.0f;
			RenderTools()->RenderTee(CAnimState::GetIdle(), &RenderTalking, m_PlayerTalked ? EMOTE_NORMAL : m_TalkedEmote, vec2(-1.0f, 0.4f), vec2(m_ScreenWidth / 1.35f, m_ScreenHeight / 1.85f));

			const char* pTalkedNick = m_pClient->m_aClients[TalkClientID].m_aName;
			if(m_Stranger)
			{
				pTalkedNick = "Stranger";
				TextRender()->TextColor(1.0f, 0.95f, 0.0f, 1);
			}

			float sizeNick = str_length(pTalkedNick);
			m_TextCursor.MoveTo((m_ScreenWidth / (1.45f + sizeNick / 64.0f)), m_ScreenHeight / 1.97f);
			TextRender()->TextOutlined(&m_TextCursor, pTalkedNick, -1);
			TextRender()->TextColor(1, 1, 1, 1);
		}

		// self dialogue
		CTeeRenderInfo RenderYou = m_pClient->m_aClients[LocalClientID].m_RenderInfo;
		RenderYou.m_Size = 128.0f;
		RenderTools()->RenderTee(CAnimState::GetIdle(), &RenderYou, m_PlayerTalked ? m_TalkedEmote : EMOTE_NORMAL, vec2(1.0f, 0.4f), vec2(m_ScreenWidth / 4.0f, m_ScreenHeight / 1.85f));

		m_TextCursor.MoveTo(m_ScreenWidth / 3.5f, m_ScreenHeight / 1.97f);
		TextRender()->TextOutlined(&m_TextCursor, m_pClient->m_aClients[LocalClientID].m_aName, -1);
	}

	// ------------------------ TEXT --------------------------
	// --------------------------------------------------------
	float FontSize = 22.0f;
	CUIRect BackgroundOther = m_pAnimBackgroundOther->GetPos()->GetRect();
	BackgroundOther.VMargin(20.0f, &BackgroundOther);

	m_TextCursor.Reset();
	m_TextCursor.m_Flags = TEXTFLAG_ALLOW_NEWLINE | TEXTFLAG_WORD_WRAP | TEXTFLAG_ELLIPSIS;
	m_TextCursor.m_FontSize = FontSize;
	m_TextCursor.m_MaxWidth = BackgroundOther.w;
	m_TextCursor.m_MaxLines = ceil(BackgroundOther.h / FontSize);
	m_TextCursor.MoveTo(BackgroundOther.x, BackgroundOther.y);
	TextRender()->TextColor(1.0f, 1.0f, 1.0f, 0.9f);
	TextRender()->TextOutlined(&m_TextCursor, m_RegrnizedTalkText, -1);

	// ------------------ INTERACTIVE TEXT -----------------
	// -----------------------------------------------------
	m_TextCursor.Reset();
	m_TextCursor.m_FontSize = 25.0f;
	m_TextCursor.MoveTo(m_ScreenWidth / 1.8f, m_ScreenHeight / 1.50f);
	TextRender()->TextOutlined(&m_TextCursor, Localize("Press (TAB) for continue!"), -1.0f);
	TextRender()->TextColor(1.0f, 1.0f, 1.0f, 1.0f);

	RegrnizedTalkingText();
}

void CTalkText::OnMessage(int MsgType, void *pRawMsg)
{
	if(Client()->State() == IClient::STATE_DEMOPLAYBACK)
		return;

	if(MsgType == NETMSGTYPE_SV_TALKTEXT)
	{
		// init background animations to
		CUIRect BackgroundMain = { m_ScreenWidth / 4.0f, m_ScreenHeight / 1.8f, m_ScreenWidth / 2.0f, m_ScreenHeight / 8.0f };
		m_pAnimBackground->Set(30.0f, CUI::CORNER_ALL);
		m_pAnimBackground->GetPos()->Anim(&BackgroundMain, 0.1f, ANIMATION_TYPE::AnimIN);

		CUIRect BackgroundOther;
		BackgroundMain.Margin(10.0f, &BackgroundOther);
		m_pAnimBackgroundOther->Set(25.0f, CUI::CORNER_ALL);
		m_pAnimBackgroundOther->GetPos()->Anim(&BackgroundOther, 0.2f, ANIMATION_TYPE::AnimIN);

		// color animation
		CNetMsg_Sv_TalkText* pMsg = (CNetMsg_Sv_TalkText*)pRawMsg;
		const int Style = pMsg->m_Style;
		vec4 ColorBackground = vec4(0.15f, 0.2f, 0.5f, 0.90f);
		vec4 ColorBackgroundAnimTo = vec4(0.2f, 0.35f, 0.5f, 0.90f);
		vec4 ColorBackgroundOther = vec4(0.1f, 0.1f, 0.1f, 0.50f);
		if(Style == TALK_STYLE_AGRESSIVE)
		{
			m_TalkedEmoticionSpriteID = SPRITE_DEVILTEE;
			ColorBackground = vec4(0.5f, 0.1f, 0.1f, 0.90f);
			ColorBackgroundAnimTo = vec4(0.5f, 0.4f, 0.2f, 0.90f);
		}
		else if(Style == TALK_STYLE_HAPPED)
		{
			m_TalkedEmoticionSpriteID = SPRITE_EYES;
			ColorBackground = vec4(0.25f, 0.5f, 0.05f, 0.90f);
			ColorBackgroundAnimTo = vec4(0.3f, 0.5f, 0.3f, 0.90f);
		}
		m_pAnimBackground->GetColor()->Init(ColorBackground);
		m_pAnimBackground->GetColor()->Anim(ColorBackgroundAnimTo, 1.0f, ANIMATION_TYPE::LINEAR, true, true);
		m_pAnimBackgroundOther->GetColor()->Init(ColorBackgroundOther);

		// start new dialogs
		m_TalkClientID = pMsg->m_pTalkClientID;
		m_TalkedEmote = pMsg->m_TalkedEmote;
		m_PlayerTalked = pMsg->m_PlayerTalked;
		str_copy(m_TalkText, pMsg->m_pText, sizeof(m_TalkText));
		m_Stranger = (bool)(str_replace(m_TalkText, "[Stranger]", "\0") > 0);
		m_RegrnizedTalkTime = time_get() + time_freq() / g_Config.m_ClDialogsSpeedNPC;

		// clear snake text
		m_RegrnizedTalkTime = 0;
		m_RegrnizedTalkPosition = 0;
		m_TalkedEmoticionSpriteID = SPRITE_DOTDOT;
		mem_zero(m_RegrnizedTalkText, sizeof(m_RegrnizedTalkText));
	}
	else if(MsgType == NETMSGTYPE_SV_CLEARTALKTEXT)
	{
		CUIRect BackgroundMainAnimEnd = { m_ScreenWidth / 2.0f, m_ScreenHeight / 1.6f, 0, 0 };
		m_pAnimBackground->GetPos()->Anim(&BackgroundMainAnimEnd, 0.1f, ANIMATION_TYPE::AnimOUT);
		m_pAnimBackground->GetPos()->SetAnimEndCallback([this]()
		{
			Clear();
		});
		m_pAnimBackgroundOther->GetPos()->Anim(&BackgroundMainAnimEnd, 0.1f, ANIMATION_TYPE::AnimOUT);
	}
}

bool CTalkText::OnInput(IInput::CEvent Event)
{
	if (m_pClient->m_pGameConsole->IsConsoleActive() || !IsActive())
		return false;

	if(IsActive() && Event.m_Flags&IInput::FLAG_PRESS && Event.m_Key == KEY_TAB)
	{
		CNetMsg_Cl_TalkInteractive Msg;
		Client()->SendPackMsg(&Msg, MSGFLAG_VITAL);
		m_pClient->m_pSounds->Play(CSounds::CHN_WORLD, SOUND_BUTTON_CLICK, 1.0f);
		return true;
	}
	return false;
}
