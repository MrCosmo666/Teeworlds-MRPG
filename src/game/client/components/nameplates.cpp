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
	else if (MoodType == MOOD_PLAYER_TANK)
		return "Tank Player";	
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
		char aName[64];
		str_format(aName, sizeof(aName), "%s", g_Config.m_ClShowsocial ? m_pClient->m_aClients[ClientID].m_aName : "");

		float a = 1.0f;
		if (g_Config.m_ClNameplatesAlways == 0)
			a = clamp(1.0f - powf(distance(m_pClient->m_pControls->m_TargetPos, Position) / 200.0f, 16.0f), 0.0f, 1.0f);
		
		CTextCursor Cursor;
		if (m_pClient->MmoServer() && m_pClient->m_aClients[ClientID].m_pLocalStats && a > 0.001f)
		{
			// ïåðåìåííûå
			char aBuf[64], aIconPlayerType[32]; aIconPlayerType[0] = '\0';
			vec4 ColorNameplates = vec4(1.0f, 1.0f, 1.0f, a);
			vec4 OutlineNameplates = vec4(0.0f, 0.0f, 0.0f, 0.5f * a);

			const CNetObj_Mmo_ClientInfo* pClientStats = m_pClient->m_aClients[ClientID].m_pLocalStats;
			switch (pClientStats->m_MoodType)
			{
				case MOOD_ANGRY:
					str_format(aIconPlayerType, sizeof(aIconPlayerType), "angry");
					ColorNameplates = vec4(0.9f, 0.65f, 0.65f, a);
					break;
				case MOOD_AGRESSED_TANK:
					str_format(aIconPlayerType, sizeof(aIconPlayerType), "agressed_y");
					ColorNameplates = vec4(0.9f, 0.4f, 0.4f, a);
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

			// - - - - - - - - - - - - - - - -ÏÐÎÃÐÅÑÑ ÁÀÐ - - - - - - - - - - - - - - //
			// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - //
			bool ShowedProgressBar = false;
			str_format(aBuf, sizeof(aBuf), "L%d%s", pClientStats->m_Level, aName);
			float tw = TextRender()->TextWidth(0, FontSize, aBuf, -1, -1.0f);
			if (pClientStats->m_Health < pClientStats->m_HealthStart)
			{
				str_format(aBuf, sizeof(aBuf), "%d / %d", pClientStats->m_Health, pClientStats->m_HealthStart);

				CUIRect ExpBar = { Position.x - tw / 2.0f , Position.y - FontSize - 92.0f, tw, 22.0f };
				RenderTools()->DrawUIBar(TextRender(), ExpBar, ColorNameplates / 1.2f,
					pClientStats->m_Health, pClientStats->m_HealthStart, aBuf, 5, 6.0f, 3.0f);
				ShowedProgressBar = true;
			}

			// - - - - - - - - - - - - - - -ÃÈËÜÄÈß ÈÃÐÎÊÀ - - - - - - - - - - - - - - //
			// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - //
			{
				IntsToStr(pClientStats->m_StateName, 6, aBuf);
				if (str_length(aBuf) > 3)
				{
					const float FontGuildname = FontSize - 10.0f;
					const float twGuildname = TextRender()->TextWidth(0, FontGuildname, aBuf, -1, -1.0f);
					const float AlphaMoon = clamp(a - 0.20f, 0.0f, a);
					const float GuildnameY = Position.y - FontGuildname - (ShowedProgressBar ? 120.0f : 95.0f);
					TextRender()->TextColor(1.0f, 0.95f, 0.0f, AlphaMoon);
					TextRender()->TextOutlineColor(0.0f, 0.0f, 0.0f, 0.5f * AlphaMoon);
					TextRender()->SetCursor(&Cursor, Position.x - twGuildname / 2.0f, GuildnameY, FontGuildname, TEXTFLAG_RENDER);
					TextRender()->TextEx(&Cursor, aBuf, -1);
				}
			}

			// - - - - - - - - - - -  - - - ÓÐÎÂÅÍÜ ÈÃÐÎÊÀ - - - - - - - - - - - - - - //
			// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - //
			{
				TextRender()->TextColor(ColorNameplates.r, ColorNameplates.g, ColorNameplates.b, ColorNameplates.a);
				TextRender()->TextOutlineColor(OutlineNameplates.r, OutlineNameplates.g, OutlineNameplates.b, OutlineNameplates.a);
				TextRender()->SetCursor(&Cursor, Position.x - tw / 2.0f, Position.y - FontSize - 70.0f, FontSize, TEXTFLAG_RENDER);

				str_format(aBuf, sizeof(aBuf), "%d", pClientStats->m_Level);
				RenderTools()->DrawUIText(TextRender(), &Cursor, aBuf,
					vec4(ColorNameplates.r, ColorNameplates.g, ColorNameplates.b, ColorNameplates.a / 3.0f), vec4(1.0f, 1.0f, 1.0f, 1.0f), FontSize);
				TextRender()->TextEx(&Cursor, aName, -1);
			}
			// - - - - - - - - - - - - -  ÇÍÀ×ÎÊ ÒÈÏÀ ÈÃÐÎÊÀ - - - - - - - - - - - - - //
			// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - //
			if(aIconPlayerType[0] != '\0') // çíà÷îê òèïà
			{ 
				CUIRect IconRect = { Cursor.m_X, Cursor.m_Y + FontSize / 5.0f, 16.0f, 16.0f };
				m_pClient->m_pMenus->DoItemIcon(aIconPlayerType, IconRect, FontSize);
			}
						
			// - - - - - - - - - - - - - - ÇÍÀ×ÎÊ ÊÂÅÑÒÎÂ  - - - - - - - - - - - - - - //
			// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - //
			if(pClientStats->m_ActiveQuest) // çíà÷îê êâåñòîâ
			{ 
				CUIRect IconRect = { Position.x - 64.0f / 2.0f, Cursor.m_Y - 65.0f, 16.0f, 16.0f };
				m_pClient->m_pMenus->DoItemIcon("quest_a", IconRect, 64.0f);
			}

			// - - - - - - - - - - - - - - -  ÒÈÏ ÀÃÐÅÑÑÈß - - - - - - - - - - - - - - //
			// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - //
			{
				const float FontSizeAgressed = FontSize - 8.0f;
				const float twAgressed = TextRender()->TextWidth(0, FontSizeAgressed, GetMoodName(pClientStats->m_MoodType), -1, -1.0f);
				const float AlphaMoon = clamp(a - 0.20f, 0.0f, a);
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

		TextRender()->TextColor(CUI::ms_DefaultTextColor);
		TextRender()->TextOutlineColor(CUI::ms_DefaultTextOutlineColor);
	}
}

void CNamePlates::OnRender()
{
	if(!g_Config.m_ClNameplates || Client()->State() < IClient::STATE_ONLINE)
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
