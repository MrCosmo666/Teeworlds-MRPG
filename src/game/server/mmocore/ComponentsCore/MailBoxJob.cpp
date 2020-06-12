/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#include <engine/shared/config.h>
#include <game/server/gamecontext.h>
#include "MailBoxJob.h"

using namespace sqlstr;

// действие над письмом
void MailBoxJob::ReceiveInbox(CPlayer *pPlayer, int InboxID)
{
	boost::scoped_ptr<ResultSet> RES(SJK.SD("ItemID, Count, Enchant", "tw_accounts_inbox", "WHERE ID = '%d'", InboxID));
	if(!RES->next())
		return;
	
	// получаем информацию о письме
	const int ItemID = RES->getInt("ItemID");
	const int Count = RES->getInt("Count");
	
	// empty
	if(ItemID <= 0 || Count <= 0)
	{
		SJK.DD("tw_accounts_inbox", "WHERE ID = '%d'", InboxID);
		return;
	}

	// recivie
	if(GS()->GetItemInfo(ItemID).IsEnchantable() && pPlayer->GetItem(ItemID).Count > 0)
	{
		GS()->Chat(pPlayer->GetCID(), "Enchant item maximal count x1 in a backpack!");
		return;
	}
	SJK.DD("tw_accounts_inbox", "WHERE ID = '%d'", InboxID);

	const int Enchant = RES->getInt("Enchant");
	pPlayer->GetItem(ItemID).Add(Count, 0, Enchant);
	GS()->Chat(pPlayer->GetCID(), "You received an attached item [{STR}].", GS()->GetItemInfo(ItemID).GetName(pPlayer));
}

// показываем список писем
void MailBoxJob::GetInformationInbox(CPlayer *pPlayer)
{
	int ShowLetterID = 0;
	bool EmptyMailBox = true;
	const int ClientID = pPlayer->GetCID();
	int HideID = (int)(NUM_TAB_MENU + ItemJob::ItemsInfo.size() + 200);
	boost::scoped_ptr<ResultSet> RES(SJK.SD("*", "tw_accounts_inbox", "WHERE OwnerID = '%d' LIMIT %d", pPlayer->Acc().AuthID, MAX_INBOX_LIST));
	while(RES->next())
	{
		// получаем информацию для создания предмета
		const int MailID = RES->getInt("ID");
		const int ItemID = RES->getInt("ItemID");
		const int Count = RES->getInt("Count"); 
		const int Enchant = RES->getInt("Enchant");
		EmptyMailBox = false;
		ShowLetterID++;
		HideID++;

		// добавляем меню голосования
		GS()->AVH(ClientID, HideID, LIGHT_GOLDEN_COLOR, "✉ Letter({INT}) {STR}", &ShowLetterID, RES->getString("MailName").c_str());
		GS()->AVM(ClientID, "null", NOPE, HideID, "{STR}", RES->getString("MailDesc").c_str());
		if(ItemID <= 0 || Count <= 0)
			GS()->AVM(ClientID, "MAIL", MailID, HideID, "I read it (L{INT})", &ShowLetterID);
		else if(GS()->GetItemInfo(ItemID).IsEnchantable())
		{
			char aEnchantSize[16];
			str_format(aEnchantSize, sizeof(aEnchantSize), " [+%d]", Enchant);
			GS()->AVM(ClientID, "MAIL", MailID, HideID, "Receive {STR}{STR} (L{INT})",
				GS()->GetItemInfo(ItemID).GetName(pPlayer), (Enchant > 0 ? aEnchantSize : "\0"), &ShowLetterID);
		}
		else
		{
			GS()->AVM(ClientID, "MAIL", MailID, HideID, "Receive {STR}x{INT} (L{INT})", GS()->GetItemInfo(ItemID).GetName(pPlayer), &Count, &ShowLetterID);
		}
	}
	if(EmptyMailBox)
	{
		GS()->AVL(ClientID, "null", "Your mailbox is empty");
	}
}

// проверить сообщения имеются ли
int MailBoxJob::GetActiveInbox(CPlayer *pPlayer)
{
	boost::scoped_ptr<ResultSet> RES2(SJK.SD("ID", "tw_accounts_inbox", "WHERE OwnerID = '%d'", pPlayer->Acc().AuthID));
	const int MailCount = RES2->rowsCount();
	return MailCount;
}

// отправка письма игроку
void MailBoxJob::SendInbox(int AuthID, const char* Name, const char* Desc, int ItemID, int Count, int Enchant)
{
	// clear str and connection
	CSqlString<64> cName = CSqlString<64>(Name);
	CSqlString<64> cDesc = CSqlString<64>(Desc);
	if (ItemID <= 0)
	{
		SJK.ID("tw_accounts_inbox", "(MailName, MailDesc, OwnerID) VALUES ('%s', '%s', '%d');", cName.cstr(), cDesc.cstr(), AuthID);
		return;
	}
	SJK.ID("tw_accounts_inbox", "(MailName, MailDesc, ItemID, Count, Enchant, OwnerID) VALUES ('%s', '%s', '%d', '%d', '%d', '%d');",
		 cName.cstr(), cDesc.cstr(), ItemID, Count, Enchant, AuthID);
	GS()->ChatAccountID(AuthID, "[Mailbox] New letter ({STR})!", cName.cstr());
}

bool MailBoxJob::OnVotingMenu(CPlayer *pPlayer, const char *CMD, const int VoteID, const int VoteID2, int Get, const char *GetText)
{
	const int ClientID = pPlayer->GetCID();
	if(PPSTR(CMD, "MAIL") == 0)
	{
		ReceiveInbox(pPlayer, VoteID);
		GS()->VResetVotes(ClientID, MenuList::MENU_INBOX);
		return true;
	}

	return false;
}
