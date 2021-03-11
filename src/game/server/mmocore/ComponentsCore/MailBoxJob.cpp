/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#include "MailBoxJob.h"

#include <base/threadpool.h>
#include <game/server/gamecontext.h>

using namespace sqlstr;

bool MailBoxJob::OnHandleVoteCommands(CPlayer* pPlayer, const char* CMD, const int VoteID, const int VoteID2, int Get, const char* GetText)
{
	const int ClientID = pPlayer->GetCID();
	if(PPSTR(CMD, "MAIL") == 0)
	{
		ReceiveInbox(pPlayer, VoteID);
		GS()->StrongUpdateVotes(ClientID, MenuList::MENU_INBOX);
		return true;
	}

	return false;
}

// check whether messages are available
int MailBoxJob::GetActiveInbox(CPlayer* pPlayer)
{
	ResultPtr pRes = SJK.SD("ID", "tw_accounts_inbox", "WHERE OwnerID = '%d'", pPlayer->Acc().m_AccountID);
	const int MailCount = pRes->rowsCount();
	return MailCount;
}

// show a list of mails
void MailBoxJob::GetInformationInbox(CPlayer *pPlayer)
{
	int ShowLetterID = 0;
	bool EmptyMailBox = true;
	const int ClientID = pPlayer->GetCID();
	int HideID = (int)(NUM_TAB_MENU + InventoryJob::ms_aItemsInfo.size() + 200);
	ResultPtr pRes = SJK.SD("*", "tw_accounts_inbox", "WHERE OwnerID = '%d' LIMIT %d", pPlayer->Acc().m_AccountID, MAX_INBOX_LIST);
	while(pRes->next())
	{
		// get the information to create an object
		const int MailID = pRes->getInt("ID");
		const int ItemID = pRes->getInt("ItemID");
		const int Count = pRes->getInt("Count");
		const int Enchant = pRes->getInt("Enchant");
		EmptyMailBox = false;
		ShowLetterID++;
		HideID++;

		// add vote menu
		GS()->AVH(ClientID, HideID, LIGHT_GOLDEN_COLOR, "✉ Letter({INT}) {STR}", ShowLetterID, pRes->getString("MailName").c_str());
		GS()->AVM(ClientID, "null", NOPE, HideID, "{STR}", pRes->getString("MailDesc").c_str());
		if(ItemID <= 0 || Count <= 0)
			GS()->AVM(ClientID, "MAIL", MailID, HideID, "I read it (L{INT})", ShowLetterID);
		else if(GS()->GetItemInfo(ItemID).IsEnchantable())
		{
			char aEnchantBuf[16];
			GS()->GetItemInfo(ItemID).FormatEnchantLevel(aEnchantBuf, sizeof(aEnchantBuf), Enchant);
			GS()->AVM(ClientID, "MAIL", MailID, HideID, "Receive {STR} {STR} (L{INT})",
				GS()->GetItemInfo(ItemID).GetName(pPlayer), (Enchant > 0 ? aEnchantBuf : "\0"), ShowLetterID);
		}
		else
			GS()->AVM(ClientID, "MAIL", MailID, HideID, "Receive {STR}x{INT} (L{INT})", GS()->GetItemInfo(ItemID).GetName(pPlayer), Count, ShowLetterID);
	}

	if(EmptyMailBox)
		GS()->AVL(ClientID, "null", "Your mailbox is empty");
}

// sending a mail to a player
void MailBoxJob::SendInbox(int AccountID, const char* Name, const char* Desc, int ItemID, int Count, int Enchant)
{
	// clear str and connection
	const CSqlString<64> cName = CSqlString<64>(Name);
	const CSqlString<64> cDesc = CSqlString<64>(Desc);

	// send information about new message
	CPlayer* pPlayer = GS()->GetPlayerFromAccountID(AccountID);
	if(pPlayer)
	{
		CNetMsg_Sv_SendGotNewMail Msg;
		Server()->SendPackMsg(&Msg, MSGFLAG_VITAL, pPlayer->GetCID());
	}
	GS()->ChatAccountID(AccountID, "[Mailbox] New letter ({STR})!", cName.cstr());

	// send new message
	if (ItemID <= 0)
	{
		SJK.ID("tw_accounts_inbox", "(MailName, MailDesc, OwnerID) VALUES ('%s', '%s', '%d');", cName.cstr(), cDesc.cstr(), AccountID);
		return;
	}
	SJK.ID("tw_accounts_inbox", "(MailName, MailDesc, ItemID, Count, Enchant, OwnerID) VALUES ('%s', '%s', '%d', '%d', '%d', '%d');",
		 cName.cstr(), cDesc.cstr(), ItemID, Count, Enchant, AccountID);

}

bool MailBoxJob::SendInbox(const char* pNickname, const char* pName, const char* pDesc, int ItemID, int Count, int Enchant)
{
	const CSqlString<64> cName = CSqlString<64>(pNickname);
	ResultPtr pRes = SJK.SD("ID, Nick", "tw_accounts_data", "WHERE Nick = '%s'", cName.cstr());
	if(pRes->next())
	{
		const int AccountID = pRes->getInt("ID");
		SendInbox(AccountID, pName, pDesc, ItemID, Count, Enchant);
		return true;
	}
	return false;
}

void MailBoxJob::ReceiveInbox(CPlayer* pPlayer, int InboxID)
{
	ResultPtr pRes = SJK.SD("ItemID, Count, Enchant", "tw_accounts_inbox", "WHERE ID = '%d'", InboxID);
	if(pRes->next())
	{
		// get informed about the mail
		const int ItemID = pRes->getInt("ItemID");
		const int Count = pRes->getInt("Count");
		if(ItemID <= 0 || Count <= 0)
		{
			SJK.DD("tw_accounts_inbox", "WHERE ID = '%d'", InboxID);
			return;
		}

		// recieve
		if(GS()->GetItemInfo(ItemID).IsEnchantable() && pPlayer->GetItem(ItemID).m_Count > 0)
		{
			GS()->Chat(pPlayer->GetCID(), "Enchant item maximal count x1 in a backpack!");
			return;
		}

		const int Enchant = pRes->getInt("Enchant");
		pPlayer->GetItem(ItemID).Add(Count, 0, Enchant);
		GS()->Chat(pPlayer->GetCID(), "You received an attached item [{STR}].", GS()->GetItemInfo(ItemID).GetName(pPlayer));
		SJK.DD("tw_accounts_inbox", "WHERE ID = '%d'", InboxID);
	}
}

// client server
void MailBoxJob::SendClientMailList(CPlayer* pPlayer)
{
	const int ClientID = pPlayer->GetCID();
	ResultPtr pRes = SJK.SD("*", "tw_accounts_inbox", "WHERE OwnerID = '%d'", pPlayer->Acc().m_AccountID);
	while(pRes->next())
	{
		const int MailID = pRes->getInt("ID");
		const int ItemID = pRes->getInt("ItemID");
		const int Count = pRes->getInt("Count");
		const int Enchant = pRes->getInt("Enchant");
		std::string Name = pRes->getString("MailName").c_str();
		std::string Desc = pRes->getString("MailDesc").c_str();

		CNetMsg_Sv_SendMailInfo Msg;
		Msg.m_MailID = MailID;
		Msg.m_pTitle = Name.c_str();
		Msg.m_pMsg = Desc.c_str();
		Msg.m_ItemID = ItemID;
		Msg.m_Count = Count;
		Msg.m_EnchantLevel = Enchant;
		Server()->SendPackMsg(&Msg, MSGFLAG_VITAL, ClientID);
	}
}

void MailBoxJob::OnMessage(int MsgID, void* pRawMsg, int ClientID)
{
	CPlayer* pPlayer = GS()->m_apPlayers[ClientID];
	if(MsgID == NETMSGTYPE_CL_SHOWMAILLISTREQUEST)
	{
		// We work with active async table inbox
		set_timer_detach([this, pPlayer]()
		{
			if(pPlayer)
				SendClientMailList(pPlayer);
		}, 100);
	}
	else if(MsgID == NETMSGTYPE_CL_RECEIVEMAIL)
	{
		CNetMsg_Cl_ReceiveMail* pMsg = (CNetMsg_Cl_ReceiveMail*)pRawMsg;
		ReceiveInbox(pPlayer, pMsg->m_MailID);
	}
	else if(MsgID == NETMSGTYPE_CL_SENDMAILTOPLAYER)
	{
		CNetMsg_Cl_SendMailToPlayer* pMsg = (CNetMsg_Cl_SendMailToPlayer*)pRawMsg;
		if(SendInbox(pMsg->m_pPlayer, pMsg->m_pTitle, pMsg->m_pMsg))
		{
			GS()->SendInformationBoxGUI(ClientID, "Your letter \"{STR}\" was sent to the player \"{STR}\"!", pMsg->m_pTitle, pMsg->m_pPlayer);
			return;
		}
		GS()->SendInformationBoxGUI(ClientID, "The player \"{STR}\" was not found!", pMsg->m_pPlayer);
	}
}
