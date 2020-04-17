/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#include <engine/textrender.h>
#include <engine/shared/config.h>
#include <generated/protocol.h>
#include <generated/client_data.h>

#include <game/client/gameclient.h>
#include <game/client/animstate.h>
#include "nameplates.h"
#include "controls.h"
#include "menus.h"

const char* CNamePlates::GetMoodName(int MoodType)
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
	else
		return "Player";
}

void CNamePlates::RenderNameplate(const CNetObj_Character *pPrevChar, const CNetObj_Character *pPlayerChar,
	const CNetObj_PlayerInfo *pPlayerInfo, int ClientID)
{
	float IntraTick = Client()->IntraGameTick();
	vec2 Position = mix(vec2(pPrevChar->m_X, pPrevChar->m_Y), vec2(pPlayerChar->m_X, pPlayerChar->m_Y), IntraTick);
	float FontSize = 18.0f + 20.0f * g_Config.m_ClNameplatesSize / 100.0f;

	// render name plate
	if (m_pClient->m_LocalClientID != ClientID)
	{
		CTextCursor Cursor;
		char aName[64];
		str_format(aName, sizeof(aName), "%s", g_Config.m_ClShowsocial ? m_pClient->m_aClients[ClientID].m_aName : "");

		float a = 0.95f;
		if (g_Config.m_ClNameplatesAlways == 0)
			a = clamp(0.95f - powf(distance(m_pClient->m_pControls->m_TargetPos, Position) / 200.0f, 16.0f), 0.0f, 0.95f);

		if (m_pClient->MmoServer() && m_pClient->m_aClients[ClientID].m_pLocalStats && a > 0.001f)
		{
			// переменные
			char aBuf[64], aIcon[32]; aIcon[0] = '\0';
			vec4 ColorNameplates = vec4(1.0f, 1.0f, 1.0f, a);
			vec4 OutlineNameplates = vec4(0.0f, 0.0f, 0.0f, 0.5f * a);

			const CNetObj_Mmo_ClientInfo* pClientStats = m_pClient->m_aClients[ClientID].m_pLocalStats;
			switch (pClientStats->m_MoodType)
			{
				case MOOD_ANGRY:
					str_format(aIcon, sizeof(aIcon), "angry");
					ColorNameplates = vec4(0.8f, 0.6f, 0.6f, a);
					OutlineNameplates = vec4(0.0f, 0.0f, 0.0f, 0.3f * a);
					break;
				case MOOD_AGRESSED_TANK:
					str_format(aIcon, sizeof(aIcon), "agressed_y");
					ColorNameplates = vec4(0.95f, 0.3f, 0.3f, a);
					OutlineNameplates = vec4(0.0f, 0.0f, 0.0f, 0.7f * a);
					break;
				case MOOD_AGRESSED_OTHER:
					str_format(aIcon, sizeof(aIcon), "agressed_o");
					ColorNameplates = vec4(0.6f, 0.4f, 0.8f, a);
					OutlineNameplates = vec4(0.0f, 0.0f, 0.0f, 0.6f * a);
					break;
				case MOOD_FRIENDLY:
					str_format(aIcon, sizeof(aIcon), "friendly");
					ColorNameplates = vec4(0.5f, 0.8f, 0.2f, a);
					OutlineNameplates = vec4(0.0f, 0.0f, 0.0f, 0.3f * a);
					break;
				case MOOD_QUESTING:
					str_format(aIcon, sizeof(aIcon), "paper");
					ColorNameplates = vec4(0.9f, 0.85f, 0.35f, a);
					OutlineNameplates = vec4(0.0f, 0.0f, 0.0f, 0.3f * a);
					break;
			}

			// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - //
			// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - //
			// прогресс бар
			str_format(aBuf, sizeof(aBuf), "L%d%s", pClientStats->m_Level, aName);
			float tw = TextRender()->TextWidth(0, FontSize, aBuf, -1, -1.0f);
			if (pClientStats->m_Health < pClientStats->m_HealthStart)
			{
				str_format(aBuf, sizeof(aBuf), "%d / %d", pClientStats->m_Health, pClientStats->m_HealthStart);

				CUIRect ExpBar = { Position.x - tw / 2.0f , Position.y - FontSize - 92.0f, tw, 25.0f };
				RenderTools()->DrawUIBar(TextRender(), ExpBar, ColorNameplates / 1.2f,
					pClientStats->m_Health, pClientStats->m_HealthStart, aBuf, 5, CUI::ALIGN_CENTER, 10.0f, 3.2f);
			}

			// ставим курсор для рисовки потом здоровья
			TextRender()->TextColor(ColorNameplates.r, ColorNameplates.g, ColorNameplates.b, ColorNameplates.a);
			TextRender()->TextOutlineColor(OutlineNameplates.r, OutlineNameplates.g, OutlineNameplates.b, OutlineNameplates.a);
			TextRender()->SetCursor(&Cursor, Position.x - tw / 2.0f, Position.y - FontSize - 70.0f, FontSize, TEXTFLAG_RENDER);

			// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - //
			// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - //
			{ // инфа по уровню
				str_format(aBuf, sizeof(aBuf), "%d", pClientStats->m_Level);
				RenderTools()->DrawUIText(TextRender(), &Cursor, aBuf,
					vec4(ColorNameplates.r, ColorNameplates.g, ColorNameplates.b, ColorNameplates.a / 3.0f) , vec4(1.0f, 1.0f, 1.0f, 1.0f), FontSize);
				TextRender()->TextEx(&Cursor, aName, -1);
			}

			// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - //
			// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - //
			if(aIcon[0] != '\0') // значок типа
			{ 
				CUIRect IconRect = { Cursor.m_X, Cursor.m_Y + FontSize / 5.0f, 16.0f, 16.0f };
				m_pClient->m_pMenus->DoItemIcon(aIcon, IconRect, FontSize);
			}
						
			// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - //
			// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - //
			if(pClientStats->m_ActiveQuest) // значок квестов
			{ 
				CUIRect IconRect = { Position.x - 64.0f / 2.0f, Cursor.m_Y - 55.0f, 16.0f, 16.0f };
				m_pClient->m_pMenus->DoItemIcon("quest_a", IconRect, 64.0f);
			}

			// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - //
			// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - //
			{ // рисуем информацию о агрессиии
				float FontSizeAgressed = FontSize - 8.0f;
				float twAgressed = TextRender()->TextWidth(0, FontSizeAgressed, GetMoodName(pClientStats->m_MoodType), -1, -1.0f);
				float AlphaMoon = clamp(a - 0.20f, 0.0f, a);
				TextRender()->TextColor(1.0f, 1.0f, 1.0f, AlphaMoon);
				TextRender()->TextOutlineColor(0.0f, 0.0f, 0.0f, 0.5f * AlphaMoon);
				TextRender()->SetCursor(&Cursor, Position.x - twAgressed / 2.0f, Position.y - FontSizeAgressed - 52.0f, FontSizeAgressed, TEXTFLAG_RENDER);
				TextRender()->TextEx(&Cursor, GetMoodName(pClientStats->m_MoodType), -1);
			}
		}
		else
		{
			float tw = TextRender()->TextWidth(0, FontSize, aName, -1, -1.0f) + RenderTools()->GetClientIdRectSize(FontSize);
			TextRender()->SetCursor(&Cursor, Position.x - tw / 2.0f, Position.y - FontSize - 38.0f, FontSize, TEXTFLAG_RENDER);

			TextRender()->TextOutlineColor(0.0f, 0.0f, 0.0f, 0.5f * a);
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

			if (a > 0.001f)
			{
				RenderTools()->DrawClientID(TextRender(), &Cursor, ClientID, BgIdColor, IdTextColor);
				TextRender()->TextEx(&Cursor, aName, -1);
			}
		}

		TextRender()->TextColor(1,1,1,1);
		TextRender()->TextOutlineColor(0.0f, 0.0f, 0.0f, 0.3f);

	}
}

void CNamePlates::OnRender()
{
	if (!g_Config.m_ClNameplates)
		return;

	for(int i = 0; i < MAX_CLIENTS; i++)
	{
		// only render active characters
		if(!m_pClient->m_Snap.m_aCharacters[i].m_Active)
			continue;

		const void *pInfo = Client()->SnapFindItem(IClient::SNAP_CURRENT, NETOBJTYPE_PLAYERINFO, i);

		if(pInfo)
		{
			RenderNameplate(
				&m_pClient->m_Snap.m_aCharacters[i].m_Prev,
				&m_pClient->m_Snap.m_aCharacters[i].m_Cur,
				(const CNetObj_PlayerInfo *)pInfo,
				i);
		}
	}
}
