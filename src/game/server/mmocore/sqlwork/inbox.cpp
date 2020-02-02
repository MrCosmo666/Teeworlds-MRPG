/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#include <engine/shared/config.h>
#include <game/server/gamecontext.h>
#include "inbox.h"

using namespace sqlstr;

// действие над письмом
void InboxSql::InteractiveInbox(CPlayer *pPlayer, int InboxID)
{
	boost::scoped_ptr<ResultSet> RES(SJK.SD("ItemID, Count, Enchant", "tw_inbox", "WHERE ID = '%d'", InboxID));
	if(RES->next())
	{
		// получаем информацию о письме
		const int ItemID = RES->getInt("ItemID"), Count = RES->getInt("Count");

		// даем предмет для игрока если даем то удаляем письмо
		if(ItemID > 0 && Count > 0)
		{
			const int Enchant = RES->getInt("Enchant");

			// если уже имеется такой зачарованный предмет
			if(GS()->GetItemInfo(ItemID).BonusID >= 0 && pPlayer->GetItem(ItemID).Count >= 1)
				return GS()->Chat(pPlayer->GetCID(), "Enchant item maximal count x1!");	

			pPlayer->GetItem(ItemID).Add(Count, 0, Enchant);
		}

		// удаляем письмо
		SJK.DD("tw_inbox", "WHERE ID = '%d'", InboxID);
	}
}

// показываем список писем
void InboxSql::GetInformationInbox(CPlayer *pPlayer)
{
	// инициализируем
	int ClientID = pPlayer->GetCID();
	dbg_msg("test", "here %d", pPlayer->Acc().AuthID);


	int HideID = NUMHIDEMENU + ItemSql::ItemsInfo.size() + 200;
	boost::scoped_ptr<ResultSet> RES(SJK.SD("*", "tw_inbox", "WHERE OwnerID = '%d' LIMIT %d", pPlayer->Acc().AuthID, MAX_MAILLIST));
	while(RES->next())
	{
		// получаем информацию для создания предмета
		const int MailID = RES->getInt("ID"), ItemID = RES->getInt("ItemID");
		const int Count = RES->getInt("Count"); HideID++;

		// добавляем меню голосования
		GS()->AVH(ClientID, HideID, vec3(15,40,80), _("✉ Mail Name ID {i:id}: {s:name}"), "id", &MailID, "name", RES->getString("MailName").c_str(), NULL);
		GS()->AVM(ClientID, "null", NOPE, HideID, _("Desc: {s:desc}"), "desc", RES->getString("MailDesc").c_str(), NULL);

		// проверяем мы читаем или получаем предмет
		if(ItemID <= 0 || Count <= 0)
		{
			GS()->AVM(ClientID, "MAIL", MailID, HideID, _("I readed (delete email) MID {i:id}"), "id", &MailID, NULL);
			continue;
		}
		
		// зачарованный предмет
		const int Enchant = RES->getInt("Enchant");
		if(Enchant > 0)
		{
			GS()->AVM(ClientID, "MAIL", MailID, HideID, _("Receive and delete email [{s:name}+{i:enchant}x{i:count}] MID {i:id}"), 
				"name", GS()->GetItemInfo(ItemID).GetName(pPlayer), "enchant", &Enchant, "count", &Count, "id", &MailID, NULL);
			continue;
		}
	
		// обычный предмет
		GS()->AVM(ClientID, "MAIL", MailID, HideID, _("Receive and delete email [{s:name}x{i:count}] MID {i:id}"), 
			"name", GS()->GetItemInfo(ItemID).GetName(pPlayer), "count", &Count, "id", &MailID, NULL);
	}

	// если пустой inbox
	bool EmptyInbox = (HideID == NUMHIDEMENU + ItemSql::ItemsInfo.size() + 200);
	if(EmptyInbox) GS()->AVL(ClientID, "null", _("Your mailbox is empty"), NULL);
	return;
}
// проверить сообщения имеются ли
int InboxSql::GetActiveInbox(int ClientID)
{
	CPlayer *pPlayer = GS()->GetPlayer(ClientID);
	if(!pPlayer) return 0;

	boost::scoped_ptr<ResultSet> RES2(SJK.SD("ID", "tw_inbox", "WHERE OwnerID = '%d'", pPlayer->Acc().AuthID));
	const int MailCount = RES2->rowsCount();
	return MailCount;
}

// отправка письма игроку
void InboxSql::SendInbox(int AuthID, const char* Name, const char* Desc, int ItemID, int Count, int Enchant)
{
	// clear str and connection
	CSqlString<64> cName = CSqlString<64>(Name);
	CSqlString<64> cDesc = CSqlString<64>(Desc);

	// проверяем игрока онлайн
	GS()->ChatAccountID(AuthID, "You have a new [{STR}] mail!", cName.cstr());
	SJK.ID("tw_inbox", "(MailName, MailDesc, ItemID, Count, Enchant, OwnerID) VALUES ('%s', '%s', '%d', '%d', '%d', '%d');",
		 cName.cstr(), cDesc.cstr(), ItemID, Count, Enchant, AuthID);
}

bool InboxSql::OnParseVotingMenu(CPlayer *pPlayer, const char *CMD, const int VoteID, const int VoteID2, int Get, const char *GetText)
{
	const int ClientID = pPlayer->GetCID();
	if(PPSTR(CMD, "MAIL") == 0)
	{
		InteractiveInbox(pPlayer, VoteID);
		GS()->VResetVotes(ClientID, INBOXLIST);
		return true;
	} 
	return false;
}
