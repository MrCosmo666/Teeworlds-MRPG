/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#include <generated/protocol.h>
#include <generated/client_data.h>

#include <base/color.h>

#include <engine/graphics.h>
#include <engine/textrender.h>
#include <engine/keys.h>

#include <game/client/animstate.h>
#include <game/client/gameclient.h>

#include "console.h"
#include "menus.h"
#include "sounds.h"

#include "talktext.h"

constexpr float CornersBackground = 18.0f;

bool CTalkText::IsActive() const
{
	return (bool)(m_ConversationClientID >= 0 && !m_pClient->m_pMenus->IsActive());
}

void CTalkText::UpdateDialogText()
{
	if(time_get() > m_UpdateDialogTextTime)
	{
		if(m_aDialogText[m_UpdateDialogCharPos] != '\0')
		{
			m_aUpdatedDialogText[m_UpdateDialogCharPos] = m_aDialogText[m_UpdateDialogCharPos];
			m_UpdateDialogCharPos++;
			m_UpdateDialogTextTime = time_get() + time_freq() / g_Config.m_ClDialogsSpeedNPC;
		}
		else
			m_UpdateDialogTextTime = 0;
	}
}

void CTalkText::Clear()
{
	m_Emote = 0;
	m_ConversationClientID = -1;
	m_UpdateDialogTextTime = 0;
	m_UpdateDialogCharPos = 0;
	mem_zero(m_aDialogText, sizeof(m_aDialogText));
	mem_zero(m_aUpdatedDialogText, sizeof(m_aUpdatedDialogText));
}

void CTalkText::OnInit()
{
	Clear();

	m_ScreenWidth = 400 * 3.0f * Graphics()->ScreenAspect();
	m_ScreenHeight = 400 * 3.0f;
	
	CUIRect BackgroundMainInit = { m_ScreenWidth / 2.0f, m_ScreenHeight / 1.6f, 0, 0 };
	m_pAnimBackground = m_pClient->AnimUI()->Get("TalkingBackground");
	m_pAnimBackgroundOther = m_pClient->AnimUI()->Get("TalkingBackgroundOther");
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

	// background
	m_pAnimBackground->Draw();
	m_pAnimBackgroundOther->Draw();
	if(!IsActive() || !m_pAnimBackground->GetPos()->m_AnimEnded)
		return;

	// dialog
	if((m_DialogFlag & TALKED_FLAG_PLAYER || m_DialogFlag & TALKED_FLAG_BOT))
	{
		constexpr float Height = 72.0f;
		CUIRect ShadowRect = m_pAnimBackground->GetPos()->GetRect();
		ShadowRect.y -= Height;
		ShadowRect.h = Height;
		RenderTools()->DrawUIRect4(&ShadowRect, vec4(0.0f, 0.0f, 0.0f, 0.0f), vec4(0.0f, 0.0f, 0.0f, 0.0f), vec4(0.025f, 0.025f, 0.025f, 0.5f), vec4(0.025f, 0.025f, 0.025f, 0.5f), CUI::CORNER_IB, CornersBackground);

		constexpr float Space = 62.0f;
		const float FontSize = 26.0f;
		const int TalkClientID = m_ConversationClientID;
		const int LocalClientID = m_pClient->m_LocalClientID;
		const vec2 PlayerPos(m_ScreenWidth / 4.0f, m_ScreenHeight / 1.85f);
		const vec2 TalkedPos(m_ScreenWidth / 1.35f, m_ScreenHeight / 1.85f);
		const vec4 PosEmoticon = m_DialogFlag & TALKED_FLAG_SAYS_PLAYER ? vec4(PlayerPos.x, PlayerPos.y - (Space * 2.0f), 140.0f, 140.0f) : (m_DialogFlag & TALKED_FLAG_BOT ? vec4(TalkedPos.x, TalkedPos.y - (Space * 2.0f), 140.0f, 140.0f) : vec4(TalkedPos.x - Space, TalkedPos.y - Space, 140.0f, 140.0f));
		Graphics()->TextureSet(g_pData->m_aImages[IMAGE_EMOTICONS].m_Id);
		Graphics()->QuadsBegin();
		RenderTools()->SelectSprite(m_EmoticionSpriteID);
		IGraphics::CQuadItem QuadItem(PosEmoticon.x, PosEmoticon.y, PosEmoticon.w, PosEmoticon.h);
		Graphics()->QuadsDraw(&QuadItem, 1);
		Graphics()->QuadsEnd();
		
		// player skins and nicknames
		const bool IsActiveLocalClient = m_pClient->m_aClients[LocalClientID].m_Active && m_DialogFlag & TALKED_FLAG_PLAYER;
		if((IsActiveLocalClient) || (!(m_DialogFlag & TALKED_FLAG_PLAYER) && m_DialogFlag & TALKED_FLAG_SAYS_PLAYER))
		{
			const char* pNickname = "Somebody....";
			if(IsActiveLocalClient)
			{
				pNickname = m_Stranger ? "Stranger" : m_pClient->m_aClients[LocalClientID].m_aName;
				CTeeRenderInfo RenderYou = m_pClient->m_aClients[LocalClientID].m_RenderInfo;
				RenderYou.m_Size = 128.0f;
				RenderTools()->RenderTee(CAnimState::GetIdle(), &RenderYou, m_DialogFlag & TALKED_FLAG_SAYS_PLAYER ? m_Emote : EMOTE_NORMAL, vec2(1.0f, 0.4f), PlayerPos);
			}
			
			m_TextCursor.Reset();
			m_TextCursor.m_FontSize = FontSize;
			m_TextCursor.m_Align = TEXTALIGN_LEFT;
			m_TextCursor.MoveTo(vec2(PlayerPos.x + (Space * 1.4f), PlayerPos.y - (Space / 3.0f)));
			TextRender()->TextOutlined(&m_TextCursor, pNickname, -1);
		}

		const bool IsActiveTargetClient = TalkClientID >= 0 && TalkClientID < MAX_CLIENTS && m_pClient->m_aClients[TalkClientID].m_Active&& m_DialogFlag & TALKED_FLAG_BOT;
		if((IsActiveTargetClient) || (!(m_DialogFlag & TALKED_FLAG_BOT) && m_DialogFlag & TALKED_FLAG_SAYS_BOT))
		{
			const char* pNickname = "Somebody....";
			if(IsActiveTargetClient)
			{
				pNickname = m_Stranger ? "Stranger" : m_pClient->m_aClients[TalkClientID].m_aName;
				CTeeRenderInfo RenderTalking = m_pClient->m_aClients[TalkClientID].m_RenderInfo;
				RenderTalking.m_Size = 128.0f;
				RenderTools()->RenderTee(CAnimState::GetIdle(), &RenderTalking, m_DialogFlag & TALKED_FLAG_SAYS_BOT ? m_Emote : EMOTE_NORMAL, vec2(-1.0f, 0.4f), TalkedPos);
			}
			
			m_TextCursor.Reset();
			m_TextCursor.m_FontSize = FontSize;
			m_TextCursor.m_Align = TEXTALIGN_RIGHT;
			m_TextCursor.MoveTo(vec2(TalkedPos.x - (Space * 1.4f), TalkedPos.y - (Space / 3.0f)));
			TextRender()->TextOutlined(&m_TextCursor, pNickname, -1);
		}
	}
		
	// text
	const float FontSize = 20.0f;
	CUIRect BackgroundOther = m_pAnimBackgroundOther->GetPos()->GetRect();
	BackgroundOther.VMargin(20.0f, &BackgroundOther);

	m_TextCursor.Reset();
	m_TextCursor.m_Align = TEXTALIGN_LEFT;
	m_TextCursor.m_Flags = TEXTFLAG_WORD_WRAP | TEXTFLAG_ELLIPSIS;
	m_TextCursor.m_FontSize = FontSize;
	m_TextCursor.m_MaxWidth = BackgroundOther.w;
	m_TextCursor.m_MaxLines = ceil(BackgroundOther.h / FontSize);
	m_TextCursor.MoveTo(BackgroundOther.x, BackgroundOther.y);
	TextRender()->TextColor(1.0f, 1.0f, 1.0f, 0.75f);
	TextRender()->TextOutlined(&m_TextCursor, m_aUpdatedDialogText, -1);

	// interactive text
	m_TextCursor.Reset();
	m_TextCursor.m_FontSize = 25.0f;
	m_TextCursor.MoveTo(m_ScreenWidth / 1.8f, m_ScreenHeight / 1.50f);
	TextRender()->TextOutlined(&m_TextCursor, Localize("Press (TAB) for continue!"), -1.0f);
	TextRender()->TextColor(CUI::ms_DefaultTextColor);
	
	// update dialog text
	UpdateDialogText();
}

static void GetDialogEmoticion(int Emote, vec4& pColor, vec4& pColorTo, int &pEmoticionSpriteID)
{
	pEmoticionSpriteID = SPRITE_DOTDOT;
	if(Emote == EMOTE_ANGRY)
	{
		const int EmoticionList[3] = { SPRITE_DEVILTEE, SPRITE_ZOMG, SPRITE_SUSHI };
		pEmoticionSpriteID = EmoticionList[random_int() % 3];
		pColor = vec4(0.5f, 0.15f, 0.1f, 0.90f);
		pColorTo = vec4(0.5f, 0.25f, 0.2f, 0.90f);
	}
	else if(Emote == EMOTE_HAPPY)
	{
		const int EmoticionList[2] = { SPRITE_EYES, SPRITE_HEARTS };
		pEmoticionSpriteID = EmoticionList[random_int() % 2];
		pColor = vec4(0.25f, 0.5f, 0.05f, 0.90f);
		pColorTo = vec4(0.4f, 0.5f, 0.2f, 0.90f);
	}
	else if(Emote == EMOTE_SURPRISE)
	{
		const int EmoticionList[3] = { SPRITE_GHOST, SPRITE_WTF, SPRITE_QUESTION };
		pEmoticionSpriteID = EmoticionList[random_int()%3];
		pColor = vec4(0.3f, 0.15f, 0.1f, 0.90f);
		pColorTo = vec4(0.3f, 0.25f, 0.2f, 0.90f);
	}
	else if(Emote == EMOTE_BLINK)
	{
		const int EmoticionList[3] = { SPRITE_DOTDOT, SPRITE_OOP, SPRITE_SORRY };
		pEmoticionSpriteID = EmoticionList[random_int() % 3];
		pColor = vec4(0.1f, 0.15f, 0.45f, 0.90f);
		pColorTo = vec4(0.15f, 0.3f, 0.45f, 0.90f);
	}
	else if(Emote == EMOTE_PAIN)
	{
		const int EmoticionList[2] = { SPRITE_DROP, SPRITE_OOP };
		pEmoticionSpriteID = EmoticionList[random_int() % 2];
		pColor = vec4(0.2f, 0.2f, 0.3f, 0.90f);
		pColorTo = vec4(0.2f, 0.2f, 0.35f, 0.90f);
	}
	else
	{
		pColor = vec4(0.15f, 0.2f, 0.5f, 0.90f);
		pColorTo = vec4(0.2f, 0.3f, 0.5f, 0.90f);
	}
}

void CTalkText::OnMessage(int MsgType, void *pRawMsg)
{
	if(Client()->State() == IClient::STATE_DEMOPLAYBACK)
		return;

	if(MsgType == NETMSGTYPE_SV_TALKTEXT)
	{
		// init background animations to
		CUIRect BackgroundMain = { m_ScreenWidth / 4.0f, m_ScreenHeight / 1.8f, m_ScreenWidth / 2.0f, m_ScreenHeight / 8.0f };
		m_pAnimBackground->Set(CornersBackground, CUI::CORNER_ALL, true);
		m_pAnimBackground->GetPos()->Anim(&BackgroundMain, 0.1f, ANIMATION_TYPE::AnimIN);

		CUIRect BackgroundOther;
		BackgroundMain.Margin(10.0f, &BackgroundOther);
		m_pAnimBackgroundOther->Set(CornersBackground, CUI::CORNER_ALL, true);
		m_pAnimBackgroundOther->GetPos()->Anim(&BackgroundOther, 0.2f, ANIMATION_TYPE::AnimIN);

		// color animation
		CNetMsg_Sv_TalkText* pMsg = (CNetMsg_Sv_TalkText*)pRawMsg;
		m_Emote = pMsg->m_Emote;
		m_DialogFlag = pMsg->m_Flag;
		m_ConversationClientID = pMsg->m_ConversationWithClientID;
		str_copy(m_aDialogText, pMsg->m_pText, sizeof(m_aDialogText));
		m_Stranger = (bool)(str_replace(m_aDialogText, "[Stranger]", "\0") > 0);

		vec4 ColorBackground, ColorBackgroundTo;
		const vec4 ColorBackgroundOther = vec4(0.1f, 0.1f, 0.1f, 0.50f);
		GetDialogEmoticion(m_Emote, ColorBackground, ColorBackgroundTo, m_EmoticionSpriteID);
		m_pAnimBackground->GetColor()->Init(ColorBackground);
		m_pAnimBackground->GetColor()->Anim(ColorBackgroundTo, 1.0f, ANIMATION_TYPE::LINEAR, true, true);
		m_pAnimBackgroundOther->GetColor()->Init(ColorBackgroundOther);

		// clear snake text
		m_UpdateDialogTextTime = 0;
		m_UpdateDialogCharPos = 0;
		mem_zero(m_aUpdatedDialogText, sizeof(m_aUpdatedDialogText));
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
