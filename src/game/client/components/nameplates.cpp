/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#include <engine/textrender.h>
#include <engine/shared/config.h>
#include <generated/protocol.h>

#include <game/client/gameclient.h>
#include "nameplates.h"
#include "controls.h"
#include "menus.h"

const char* CNamePlates::GetMoodName(int MoodType) const
{
	if (MoodType == MOOD_AGRESSED_OTHER)
		return "Agressed to other";
	else if (MoodType == MOOD_AGRESSED_TANK)
		return "Agressed to you";
	else if (MoodType == MOOD_ANGRY)
		return "Angry";
	else if (MoodType == MOOD_FRIENDLY)
		return "Friendly";
	else if (MoodType == MOOD_QUESTING)
		return "Questing NPC";
	else if (MoodType == MOOD_PLAYER_TANK)
		return "Tank Player";
	else
		return "Player";
}

void CNamePlates::RenderNameplate(const CNetObj_Character *pPrevChar, const CNetObj_Character *pPlayerChar, int ClientID) const
{
	bool Predicted = m_pClient->ShouldUsePredicted() && m_pClient->ShouldUsePredictedChar(ClientID);
	vec2 Position = m_pClient->GetCharPos(ClientID, Predicted);

	float FontSize = 18.0f + 20.0f * g_Config.m_ClNameplatesSize / 100.0f;

	// render name plate
	char aName[64];
	str_format(aName, sizeof(aName), "%s", g_Config.m_ClShowsocial ? m_pClient->m_aClients[ClientID].m_aName : "");

	float a = 1.0f;
	if (g_Config.m_ClNameplatesAlways == 0)
		a = clamp(1.0f - powf(distance(m_pClient->m_pControls->m_TargetPos, Position) / 200.0f, 16.0f), 0.0f, 1.0f);

	static CTextCursor s_Cursor(FontSize);
	s_Cursor.Reset();
	if (m_pClient->MmoServer() && m_pClient->m_aClients[ClientID].m_pLocalStats && a > 0.001f)
	{
		// TODO: optimzie and rework it
		char aBuf[64], aIconPlayerType[32];
		vec4 ColorNameplates = vec4(1.0f, 1.0f, 1.0f, a);
		vec4 OutlineNameplates = vec4(0.0f, 0.0f, 0.0f, 0.5f * a);
		aIconPlayerType[0] = '\0';

		const CNetObj_Mmo_ClientInfo* pClientStats = m_pClient->m_aClients[ClientID].m_pLocalStats;
		switch (pClientStats->m_MoodType)
		{
			case MOOD_ANGRY:
				str_format(aIconPlayerType, sizeof(aIconPlayerType), "angry");
				ColorNameplates = vec4(0.9f, 0.5f, 0.5f, a);
				break;
			case MOOD_AGRESSED_TANK:
				str_format(aIconPlayerType, sizeof(aIconPlayerType), "agressed_y");
				ColorNameplates = vec4(0.9f, 0.3f, 0.3f, a);
				break;
			case MOOD_AGRESSED_OTHER:
				str_format(aIconPlayerType, sizeof(aIconPlayerType), "agressed_o");
				ColorNameplates = vec4(0.5f, 0.3f, 0.7f, a);
				break;
			case MOOD_FRIENDLY:
				str_format(aIconPlayerType, sizeof(aIconPlayerType), "friendly");
				ColorNameplates = vec4(0.4f, 0.8f, 0.2f, a);
				break;
			case MOOD_QUESTING:
				str_format(aIconPlayerType, sizeof(aIconPlayerType), "paper");
				ColorNameplates = vec4(0.9f, 0.85f, 0.35f, a);
				break;
			case MOOD_PLAYER_TANK:
				str_format(aIconPlayerType, sizeof(aIconPlayerType), "rose");
				ColorNameplates = vec4(0.15f, 0.60f, 1.00f, a);
				break;
		}

		// Healthbar
		bool ShowHealthBar = false;
		str_format(aBuf, sizeof(aBuf), "LVL%d%s", pClientStats->m_Level, aName);
		float TextWeidthTemp = TextRender()->TextWidth(FontSize, aBuf, -1);
		if (pClientStats->m_Health < pClientStats->m_HealthStart)
		{
			CUIRect HealthBar = { Position.x - (TextWeidthTemp / 2.0f), Position.y - FontSize - 92.0f, TextWeidthTemp, 21.f };
			str_format(aBuf, sizeof(aBuf), "%d / %d", pClientStats->m_Health, pClientStats->m_HealthStart);
			RenderTools()->DrawUIBar(TextRender(), HealthBar, ColorNameplates / 1.2f, pClientStats->m_Health, pClientStats->m_HealthStart, aBuf, 5, 6.0f, 3.0f);
			ShowHealthBar = true;
		}

		// Guild / state name
		{
			IntsToStr(pClientStats->m_StateName, 6, aBuf);
			if (str_length(aBuf) > 3)
			{
				const float FontStateSize = FontSize - 10.0f;
				const float twState = TextRender()->TextWidth(FontStateSize, aBuf, -1);
				s_Cursor.Reset();
				s_Cursor.m_FontSize = FontStateSize;

				const float AlphaMoon = clamp(a - 0.20f, 0.0f, a);
				const float GuildnameY = Position.y - FontStateSize - (ShowHealthBar ? 120.0f : 95.0f);
				TextRender()->TextColor(1.0f, 0.95f, 0.0f, AlphaMoon);
				TextRender()->TextSecondaryColor(0.0f, 0.0f, 0.0f, 0.5f * AlphaMoon);
				s_Cursor.MoveTo(Position.x - (twState / 2.0f), GuildnameY);
				TextRender()->TextOutlined(&s_Cursor, aBuf, -1);
			}
		}

		// Nick name
		{
			TextRender()->TextColor(ColorNameplates.r, ColorNameplates.g, ColorNameplates.b, ColorNameplates.a);
			TextRender()->TextSecondaryColor(OutlineNameplates.r, OutlineNameplates.g, OutlineNameplates.b, OutlineNameplates.a);

			s_Cursor.Reset();
			s_Cursor.m_FontSize = FontSize;
			s_Cursor.m_Align = TEXTALIGN_LEFT;

			str_format(aBuf, sizeof(aBuf), "%d", pClientStats->m_Level);
			float Skipped = RenderTools()->DrawUIText(TextRender(), vec2((Position.x - (TextWeidthTemp / 2.0f)), Position.y - FontSize - 70.0f), aBuf,
				ColorNameplates, vec4(1.0f, 1.0f, 1.0f, 1.0f), FontSize);
			s_Cursor.MoveTo((Position.x + Skipped) - (TextWeidthTemp / 2.0f), Position.y - FontSize - 70.0f);
			TextRender()->TextOutlined(&s_Cursor, aName, -1);

			// Icon
			if(aIconPlayerType[0] != '\0')
			{
				float tw = TextRender()->TextWidth(FontSize, aName, -1);
				CUIRect IconRect = { s_Cursor.CursorPosition().x + (tw + 3.0f), s_Cursor.CursorPosition().y + FontSize / 5.0f, 16.0f, 16.0f };
				m_pClient->m_pMenus->DoItemIcon(aIconPlayerType, IconRect, FontSize);
			}

			// Quest npc
			if(pClientStats->m_ActiveQuest)
			{
				CUIRect IconRect = { Position.x - 64.0f / 2.0f, s_Cursor.CursorPosition().y - 82.0f, 16.0f, 16.0f };
				m_pClient->m_pMenus->DoItemIcon("quest_a", IconRect, 64.0f);
			}
		}

		// Mood type
		{
			const float FontSizeAgressed = FontSize - 8.0f;
			const float twAgressed = TextRender()->TextWidth(FontSizeAgressed, GetMoodName(pClientStats->m_MoodType), -1);
			const float AlphaMoon = clamp(a - 0.20f, 0.0f, a);
			TextRender()->TextColor(1.0f, 1.0f, 1.0f, AlphaMoon);
			TextRender()->TextSecondaryColor(0.0f, 0.0f, 0.0f, 0.5f * AlphaMoon);

			s_Cursor.Reset();
			s_Cursor.MoveTo(Position.x - twAgressed / 2.0f, Position.y - FontSizeAgressed - 52.0f);
			s_Cursor.m_FontSize = FontSizeAgressed;
			TextRender()->TextOutlined(&s_Cursor, GetMoodName(pClientStats->m_MoodType), -1);
		}
	}
	else
	{
		float tw = TextRender()->TextWidth(FontSize, aName, -1) + RenderTools()->GetClientIdRectSize(FontSize);
		TextRender()->TextSecondaryColor(0.0f, 0.0f, 0.0f, 0.5f * a);
		TextRender()->TextColor(1.0f, 1.0f, 1.0f, a);

		if(g_Config.m_ClNameplatesTeamcolors && m_pClient->m_GameInfo.m_GameFlags&GAMEFLAG_TEAMS)
		{
			if(m_pClient->m_aClients[ClientID].m_Team == TEAM_RED)
				TextRender()->TextColor(1.0f, 0.5f, 0.5f, a);
			else if(m_pClient->m_aClients[ClientID].m_Team == TEAM_BLUE)
				TextRender()->TextColor(0.7f, 0.7f, 1.0f, a);
		}

		const vec4 IdTextColor(0.1f, 0.1f, 0.1f, a);
		vec4 BgIdColor(1.0f, 1.0f, 1.0f, a * 0.5f);
		if(g_Config.m_ClNameplatesTeamcolors && m_pClient->m_GameInfo.m_GameFlags&GAMEFLAG_TEAMS)
		{
			if(m_pClient->m_aClients[ClientID].m_Team == TEAM_RED)
				BgIdColor = vec4(1.0f, 0.5f, 0.5f, a * 0.5f);
			else if(m_pClient->m_aClients[ClientID].m_Team == TEAM_BLUE)
				BgIdColor = vec4(0.7f, 0.7f, 1.0f, a * 0.5f);
		}

		s_Cursor.Reset();
		s_Cursor.MoveTo(Position.x - tw / 2.0f, Position.y - FontSize - 38.0f);
		s_Cursor.m_FontSize = FontSize;
		TextRender()->TextDeferred(&s_Cursor, aName, -1);

		if (a > 0.001f)
		{
			vec2 CursorPosition = vec2(Position.x - tw / 2.0f, Position.y - FontSize - 38.0f);
			CursorPosition.x += RenderTools()->DrawClientID(TextRender(), s_Cursor.m_FontSize, CursorPosition, ClientID, BgIdColor, IdTextColor);
			s_Cursor.MoveTo(CursorPosition.x, CursorPosition.y);
			TextRender()->DrawTextOutlined(&s_Cursor, a);
		}
	}

	TextRender()->TextColor(CUI::ms_DefaultTextColor);
	TextRender()->TextSecondaryColor(CUI::ms_DefaultTextOutlineColor);
}

void CNamePlates::OnRender()
{
	if(!g_Config.m_ClNameplates || Client()->State() < IClient::STATE_ONLINE)
		return;

	for(int i = 0; i < MAX_CLIENTS; i++)
	{
		// only render active characters
		if(m_pClient->m_aClients[i].m_Active && m_pClient->m_Snap.m_aCharacters[i].m_Active && m_pClient->m_LocalClientID != i)
		{
			RenderNameplate(
				&m_pClient->m_Snap.m_aCharacters[i].m_Prev,
				&m_pClient->m_Snap.m_aCharacters[i].m_Cur,
				i);
		}
	}
}
