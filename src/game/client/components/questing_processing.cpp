/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#include <engine/shared/config.h>
#include <engine/graphics.h>
#include <engine/textrender.h>
#include <engine/keys.h>

#include <generated/protocol.h>
#include <generated/client_data.h>

#include <game/client/animstate.h>
#include <game/client/gameclient.h>

#include "binds.h"
#include "menus.h"
#include "talktext.h"
#include "questing_processing.h"

void CQuestingProcessing::Clear()
{
	mem_zero((void *)QuestTable, sizeof(QuestTable));
}

bool CQuestingProcessing::IsActive()
{
	return QuestTable[0].TableActive() && m_pClient->m_pTalkText->IsActive();
}

void CQuestingProcessing::OnStateChange(int NewState, int OldState)
{
	if(OldState == IClient::STATE_ONLINE || OldState == IClient::STATE_OFFLINE)
		Clear();
}

void CQuestingProcessing::ProcessingRenderTable(int TableID, CUIRect &Box)
{
	CUIRect Table;
	Box.HMargin(120.0f, &Table);

	float FontSize = 24.0f;
	float Space = TableID * 60.0f;
	Table.y = Box.y + Space + 40.0f;
	RenderTools()->DrawRoundRect(&Table, vec4(0.5f, 0.41f, 0.0f, 0.5f), 15.0f);

	{ // RES
		vec4 ColorRes = vec4(0.4f, 0.05f, 0.0f, 0.4f);
		if(QuestTable[TableID].m_Have >= QuestTable[TableID].m_Requires)
			ColorRes = vec4(0.05f, 0.4f, 0.0f, 0.4f);

		CUIRect BackgroundResoult;
		Table.VSplitRight(100.0f, 0, &BackgroundResoult);
		BackgroundResoult.Margin(3.0f, &BackgroundResoult);
		RenderTools()->DrawRoundRect(&BackgroundResoult, ColorRes, 15.0f);

		char aQuestTable[32];
		str_format(aQuestTable, sizeof(aQuestTable), "%d / %d", QuestTable[TableID].m_Have, QuestTable[TableID].m_Requires);
		TextRender()->Text(0x0, BackgroundResoult.x, BackgroundResoult.y, 18.0f, aQuestTable, -1.0F);
	}

	//  TEXT 
	float SizeIcon = 42.0f;
	m_pClient->m_pMenus->DoItemIcon(QuestTable[TableID].m_aIcon, Table, SizeIcon);
	Table.x += SizeIcon;
	TextRender()->Text(0x0, Table.x, Table.y, FontSize, QuestTable[TableID].m_aText, -1.0F);
}

void CQuestingProcessing::OnRender()
{
	if(!IsActive())
		return;

	float Width = 400 * 3.0f * Graphics()->ScreenAspect();
	float Height = 400 * 3.0f;
	Graphics()->MapScreen(0, 0, Width, Height);

	// --------------------- BACKGROUND -----------------------
	// --------------------------------------------------------
	float tx = Width / 3.0f, ty = Height / 5.0f, tw = Width / 3.0f, th = Height / 4.0f;
	CUIRect BackgroundMain = { tx, ty, tw, th };
	RenderTools()->DrawRoundRect(&BackgroundMain, vec4(0.5f, 0.33f, 0.0f, 0.5f), 30.0f);

	CUIRect BackgroundOther;
	BackgroundMain.Margin(10.0f, &BackgroundOther);
	RenderTools()->DrawUIRect4(&BackgroundOther,
		vec4(0.0f, 0.0f, 0.0f, 0.1f), vec4(0.0f, 0.0f, 0.0f, 0.1f),
		vec4(0.05f, 0.05f, 0.05f, 0.35f), vec4(0.05f, 0.05f, 0.05f, 0.35f), CUI::CORNER_ALL, 30.0f);
	BackgroundOther.VMargin(20.0f, &BackgroundOther);

	// --------------------- DRAW TABLES ----------------------
	// --------------------------------------------------------
	for (int i = 0; i < MAX_TABLE; i++)
	{
		if(QuestTable[i].TableActive())
			ProcessingRenderTable(i, BackgroundOther);
	}

	// ---------------- TEXT (Quest Task List) ----------------
	// --------------------------------------------------------
	TextRender()->Text(0x0, tx + 110.0f, ty - 20.0f, 42.0f, Localize("Quest Task List"), -1.0f);
}

void CQuestingProcessing::OnMessage(int MsgType, void *pRawMsg)
{
	if(Client()->State() == IClient::STATE_DEMOPLAYBACK)
		return;

	if(MsgType == NETMSGTYPE_SV_ADDQUESTINGPROCESSING)
	{
		CNetMsg_Sv_AddQuestingProcessing* pMsg = (CNetMsg_Sv_AddQuestingProcessing*)pRawMsg;

		for (int i = 0; i < MAX_TABLE; i++)
		{
			if (QuestTable[i].TableActive())
				continue;

			QuestTable[i].m_Requires = pMsg->m_pRequiresNum;
			QuestTable[i].m_Have = pMsg->m_pHaveNum;
			str_copy(QuestTable[i].m_aText, pMsg->m_pText, sizeof(QuestTable[i].m_aText));
		
			char pIcon[16];
			IntsToStr(pMsg->m_pIcon, 4, pIcon);
			str_copy(QuestTable[i].m_aIcon, pIcon, sizeof(QuestTable[i].m_aIcon));
		}
	}
	else if (MsgType == NETMSGTYPE_SV_CLEARQUESTINGPROCESSING)
	{
		Clear();
	}
}

bool CQuestingProcessing::OnInput(IInput::CEvent Event)
{
	if(IsActive() && Event.m_Flags&IInput::FLAG_PRESS && Event.m_Key == KEY_TAB)
		return true;

	return false;
}
