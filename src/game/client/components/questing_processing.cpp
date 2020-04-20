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
#include "questing_processing.h"

#define COLOR_TABLE vec4(0.40f, 0.22f, 0.08f, 0.3f)
#define COLOR_BACKGROUND vec4(0.35f, 0.20f, 0.15f, 0.4f)
#define COLOR_BACKBACKGROUND vec4(0.30f, 0.19f, 0.15f, 0.30f)
#define COLOR_UIBAR vec4(0.35f, 0.55f, 0.1f, 0.45f)

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

	float Space = TableID * 60.0f;
	Table.y = Box.y + Space + 40.0f;
	RenderTools()->DrawRoundRect(&Table, COLOR_TABLE, 15.0f);

	{ // RES
		vec4 ColorBarUI = (QuestTable[TableID].m_Have >= QuestTable[TableID].m_Requires ? vec4(0.40f, 0.80f, 0.1f, 0.50f) : vec4(0.80f, 0.30f, 0.1f, 0.50f));

		char aQuestTable[32];
		str_format(aQuestTable, sizeof(aQuestTable), "%s %d / %d", QuestTable[TableID].m_aText, QuestTable[TableID].m_Have, QuestTable[TableID].m_Requires);
		RenderTools()->DrawUIBar(TextRender(), Table, ColorBarUI,
			QuestTable[TableID].m_Have, QuestTable[TableID].m_Requires, aQuestTable, 3, 10.0f, 8.0f);
	}

	//  ICON
	float SizeIcon = 80.0f;
	Table.VSplitRight(50.0f, 0, &Table);
	Table.HSplitBottom(70.0f, 0, &Table);
	m_pClient->m_pMenus->DoItemIcon(QuestTable[TableID].m_aIcon, Table, SizeIcon);
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
	RenderTools()->DrawRoundRect(&BackgroundMain, COLOR_BACKBACKGROUND, 30.0f);

	CUIRect BackgroundOther;
	BackgroundMain.Margin(10.0f, &BackgroundOther);
	RenderTools()->DrawRoundRect(&BackgroundOther, COLOR_BACKGROUND, 30.0f);
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
	TextRender()->Text(0x0, tx + 30.0f, ty - 20.0f, 42.0f, Localize("Quest Task List"), -1.0f);
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
			return;
		}
	}
	else if (MsgType == NETMSGTYPE_SV_CLEARQUESTINGPROCESSING)
	{
		Clear();
	}
}

bool CQuestingProcessing::OnInput(IInput::CEvent Event)
{
	// fix console Press TAB
	if (m_pClient->m_pGameConsole->IsConsoleActive())
		return false;

	if(IsActive() && Event.m_Flags&IInput::FLAG_PRESS && Event.m_Key == KEY_TAB)
		return true;

	return false;
}
