/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#include <engine/graphics.h>
#include <engine/textrender.h>
#include <engine/keys.h>

#include <generated/protocol.h>

#include "console.h"
#include "menus.h"
#include "talktext.h"
#include "questing_processing.h"

#define COLOR_BACKGROUND vec4(0.27f, 0.07f, 0.01f, 1.00f)

void CQuestingProcessing::Clear()
{
	mem_zero((void *)QuestTable, sizeof(QuestTable));
}

bool CQuestingProcessing::IsActive() const
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
	CUIRect Table = Box;
	const float Space = TableID * 60.0f;
	Table.h = 40.0f;
	Table.y = Box.y + Space + 30.0f;

	{ // RES
		char aQuestTable[128];
		vec4 ColorBarUI = (QuestTable[TableID].m_Have >= QuestTable[TableID].m_Requires ? vec4(0.20f, 0.50f, 0.1f, 0.50f) : vec4(0.50f, 0.20f, 0.1f, 0.50f));
		if (QuestTable[TableID].m_GivingTable)
		{
			ColorBarUI = vec4(0.50f, 0.30f, 0.0f, 0.50f);
			str_format(aQuestTable, sizeof(aQuestTable), "%sx%d", QuestTable[TableID].m_aText, QuestTable[TableID].m_Requires);
			RenderTools()->DrawUIBar(TextRender(), Table, ColorBarUI, 10, 10, aQuestTable, 3, 10.0f, 8.0f);
		}
		else
		{
			str_format(aQuestTable, sizeof(aQuestTable), "%s %d / %d", QuestTable[TableID].m_aText, QuestTable[TableID].m_Have, QuestTable[TableID].m_Requires);
			RenderTools()->DrawUIBar(TextRender(), Table, ColorBarUI, QuestTable[TableID].m_Have, QuestTable[TableID].m_Requires, aQuestTable, 3, 10.0f, 8.0f);
		}
	}

	//  ICON
	const float SizeIcon = 70.0f;
	Table.VSplitRight(50.0f, 0, &Table);
	Table.HSplitBottom(55.0f, 0, &Table);
	m_pClient->m_pMenus->DoItemIcon(QuestTable[TableID].m_aIcon, Table, SizeIcon);
}

int CQuestingProcessing::TableSize() const
{
	int sizetab = 0;
	for (int i = 0; i < MAX_TABLE; i++)
	{
		if (QuestTable[i].TableActive())
			sizetab++;
	}
	return sizetab;
}

void CQuestingProcessing::OnRender()
{
	if(!IsActive())
		return;

	const int tabsize = TableSize();
	const float Width = 400 * 3.0f * Graphics()->ScreenAspect();
	const float Height = 400 * 3.0f;
	Graphics()->MapScreen(0, 0, Width, Height);

	// --------------------- BACKGROUND -----------------------
	// --------------------------------------------------------
	const float tx = Width / 3.0f, ty = Height / 2.5f, tw = Width / 3.0f, th = 60.0f;
	CUIRect BackgroundMain = { tx, ty - tabsize * (60.0f), tw, (45.0f + th * tabsize) };
	BackgroundMain.Margin(5.0f, &BackgroundMain);
	RenderTools()->DrawUIRectMonochromeGradient(&BackgroundMain, COLOR_BACKGROUND, CUI::CORNER_ALL, 30.0f);
	BackgroundMain.Margin(5.0f, &BackgroundMain);
	RenderTools()->DrawUIRectMonochromeGradient(&BackgroundMain, vec4(0.f, 0.f, 0.f, 0.1f), CUI::CORNER_ALL, 30.0f);
	BackgroundMain.VMargin(20.0f, &BackgroundMain);

	// --------------------- DRAW TABLES ----------------------
	// --------------------------------------------------------
	for (int i = 0; i < MAX_TABLE; i++)
	{
		if (QuestTable[i].TableActive())
			ProcessingRenderTable(i, BackgroundMain);
	}

	// ---------------- TEXT (Quest Task List) ----------------
	// --------------------------------------------------------
	CTextCursor Cursor(42.0f, BackgroundMain.x, BackgroundMain.y - 24.f);
	TextRender()->TextShadowed(&Cursor, Localize("Quest Task List"), -1.0f, vec2(0.5f, 0.8f));
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
			QuestTable[i].m_GivingTable = pMsg->m_pGivingTable;
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
