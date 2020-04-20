/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#include <engine/shared/config.h>
#include <game/server/gamecontext.h>
#include "mailbox_job.h"

using namespace sqlstr;

// действие над письмом
void MailBoxJob::InteractiveInbox(CPlayer *pPlayer, int InboxID)
{
	boost::scoped_ptr<ResultSet> RES(SJK.SD("ItemID, Count, Enchant", "tw_inbox", "WHERE ID = '%d'", InboxID));
	if(RES->next())
	{
		// получаем информацию о письме
		const int ItemID = RES->getInt("ItemID"), Count = RES->getInt("Count");

		// даем предмет для игрока если даем то удаляем письмо
		if(ItemID > 0 && Count > 0)
		{
			// если уже имеется такой зачарованный предмет
			if(GS()->GetItemInfo(ItemID).BonusCount > 0 && pPlayer->GetItem(ItemID).Count > 0)
				return GS()->Chat(pPlayer->GetCID(), "Enchant item maximal count x1 in a backpack!");

			const int Enchant = RES->getInt("Enchant");
			pPlayer->GetItem(ItemID).Add(Count, 0, Enchant);
		}

		// удаляем письмо
		SJK.DD("tw_inbox", "WHERE ID = '%d'", InboxID);
	}
}

// показываем список писем
void MailBoxJob::GetInformationInbox(CPlayer *pPlayer)
{
	// инициализируем
	int ClientID = pPlayer->GetCID();
	int StartHideCount = (int)(NUMHIDEMENU + ItemSql::ItemsInfo.size() + 200);
	int HideID = StartHideCount;
	boost::scoped_ptr<ResultSet> RES(SJK.SD("*", "tw_inbox", "WHERE OwnerID = '%d' LIMIT %d", pPlayer->Acc().AuthID, MAX_MAILLIST));
	while(RES->next())
	{
		// получаем информацию для создания предмета
		const int MailID = RES->getInt("ID"), ItemID = RES->getInt("ItemID");
		const int Count = RES->getInt("Count"); HideID++;

		// добавляем меню голосования
		GS()->AVH(ClientID, HideID, GOLDEN_COLOR, "✉ Mail Name ID {INT}: {STR}", &MailID, RES->getString("MailName").c_str());
		GS()->AVM(ClientID, "null", NOPE, HideID, "Desc: {STR}", RES->getString("MailDesc").c_str());

		// проверяем мы читаем или получаем предмет
		if(ItemID <= 0 || Count <= 0)
		{
			GS()->AVM(ClientID, "MAIL", MailID, HideID, "I readed (delete email) MID {INT}", &MailID);
			continue;
		}
		
		// зачарованный предмет
		const int Enchant = RES->getInt("Enchant");
		if(Enchant > 0)
		{
			GS()->AVM(ClientID, "MAIL", MailID, HideID, "Receive and delete email [{STR}+{INT}x{INT}] MID {INT}", 
				GS()->GetItemInfo(ItemID).GetName(pPlayer), &Enchant, &Count, &MailID);
			continue;
		}
	
		// обычный предмет
		GS()->AVM(ClientID, "MAIL", MailID, HideID, "Receive and delete email [{STR}x{INT}] MID {INT}", 
			GS()->GetItemInfo(ItemID).GetName(pPlayer), &Count, &MailID);
	}

	// если пустой inbox
	if(HideID == StartHideCount)
		GS()->AVL(ClientID, "null", "Your mailbox is empty");
	return;
}
// проверить сообщения имеются ли
int MailBoxJob::GetActiveInbox(int ClientID)
{
	CPlayer *pPlayer = GS()->GetPlayer(ClientID);
	if(!pPlayer) return 0;

	boost::scoped_ptr<ResultSet> RES2(SJK.SD("ID", "tw_inbox", "WHERE OwnerID = '%d'", pPlayer->Acc().AuthID));
	const int MailCount = RES2->rowsCount();
	return MailCount;
}

// отправка письма игроку
void MailBoxJob::SendInbox(int AuthID, const char* Name, const char* Desc, int ItemID, int Count, int Enchant)
{
	// clear str and connection
	CSqlString<64> cName = CSqlString<64>(Name);
	CSqlString<64> cDesc = CSqlString<64>(Desc);

	// проверяем игрока онлайн
	GS()->ChatAccountID(AuthID, "You have a new [{STR}] mail!", cName.cstr());
	SJK.ID("tw_inbox", "(MailName, MailDesc, ItemID, Count, Enchant, OwnerID) VALUES ('%s', '%s', '%d', '%d', '%d', '%d');",
		 cName.cstr(), cDesc.cstr(), ItemID, Count, Enchant, AuthID);
}

bool MailBoxJob::OnParseVotingMenu(CPlayer *pPlayer, const char *CMD, const int VoteID, const int VoteID2, int Get, const char *GetText)
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
