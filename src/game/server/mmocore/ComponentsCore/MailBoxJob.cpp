/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#include <game/server/gamecontext.h>
#include "MailBoxJob.h"

using namespace sqlstr;

// mails
void MailBoxJob::ReceiveInbox(CPlayer *pPlayer, int InboxID)
{
	ResultPtr pRes = SJK.SD("ItemID, Count, Enchant", "tw_accounts_inbox", "WHERE ID = '%d'", InboxID);
	if(!pRes->next())
		return;
	
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
	SJK.DD("tw_accounts_inbox", "WHERE ID = '%d'", InboxID);

	const int Enchant = pRes->getInt("Enchant");
	pPlayer->GetItem(ItemID).Add(Count, 0, Enchant);
	GS()->Chat(pPlayer->GetCID(), "You received an attached item [{STR}].", GS()->GetItemInfo(ItemID).GetName(pPlayer));
}

// show a list of mails
void MailBoxJob::GetInformationInbox(CPlayer *pPlayer)
{
	int ShowLetterID = 0;
	bool EmptyMailBox = true;
	const int ClientID = pPlayer->GetCID();
	int HideID = (int)(NUM_TAB_MENU + InventoryJob::ms_aItemsInfo.size() + 200);
	ResultPtr pRes = SJK.SD("*", "tw_accounts_inbox", "WHERE OwnerID = '%d' LIMIT %d", pPlayer->Acc().m_AuthID, MAX_INBOX_LIST);
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
		GS()->AVH(ClientID, HideID, LIGHT_GOLDEN_COLOR, "✉ Letter({INT}) {STR}", &ShowLetterID, pRes->getString("MailName").c_str());
		GS()->AVM(ClientID, "null", NOPE, HideID, "{STR}", pRes->getString("MailDesc").c_str());
		if(ItemID <= 0 || Count <= 0)
			GS()->AVM(ClientID, "MAIL", MailID, HideID, "I read it (L{INT})", &ShowLetterID);
		else if(GS()->GetItemInfo(ItemID).IsEnchantable())
		{
			char aEnchantBuf[16];
			GS()->GetItemInfo(ItemID).FormatEnchantLevel(aEnchantBuf, sizeof(aEnchantBuf), Enchant);
			GS()->AVM(ClientID, "MAIL", MailID, HideID, "Receive {STR} {STR} (L{INT})",
				GS()->GetItemInfo(ItemID).GetName(pPlayer), (Enchant > 0 ? aEnchantBuf : "\0"), &ShowLetterID);
		}
		else
			GS()->AVM(ClientID, "MAIL", MailID, HideID, "Receive {STR}x{INT} (L{INT})", GS()->GetItemInfo(ItemID).GetName(pPlayer), &Count, &ShowLetterID);
	}

	if(EmptyMailBox)
		GS()->AVL(ClientID, "null", "Your mailbox is empty");
}

// check whether messages are available
int MailBoxJob::GetActiveInbox(CPlayer *pPlayer)
{
	ResultPtr pRes = SJK.SD("ID", "tw_accounts_inbox", "WHERE OwnerID = '%d'", pPlayer->Acc().m_AuthID);
	const int MailCount = pRes->rowsCount();
	return MailCount;
}

// sending a mail to a player
void MailBoxJob::SendInbox(int AuthID, const char* Name, const char* Desc, int ItemID, int Count, int Enchant)
{
	// clear str and connection
	const CSqlString<64> cName = CSqlString<64>(Name);
	const CSqlString<64> cDesc = CSqlString<64>(Desc);
	if (ItemID <= 0)
	{
		SJK.ID("tw_accounts_inbox", "(MailName, MailDesc, OwnerID) VALUES ('%s', '%s', '%d');", cName.cstr(), cDesc.cstr(), AuthID);
		return;
	}

	SJK.ID("tw_accounts_inbox", "(MailName, MailDesc, ItemID, Count, Enchant, OwnerID) VALUES ('%s', '%s', '%d', '%d', '%d', '%d');",
		 cName.cstr(), cDesc.cstr(), ItemID, Count, Enchant, AuthID);
	GS()->ChatAccountID(AuthID, "[Mailbox] New letter ({STR})!", cName.cstr());
}

bool MailBoxJob::OnParsingVoteCommands(CPlayer *pPlayer, const char *CMD, const int VoteID, const int VoteID2, int Get, const char *GetText)
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