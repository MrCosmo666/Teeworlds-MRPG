/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#include <engine/shared/config.h>
#include <game/server/gamecontext.h>
#include "account_main.h"

using namespace sqlstr;

std::map < int , AccountMainSql::StructData > AccountMainSql::Data;

void AccountMainSql::OnResetClientData(int ClientID)
{
	if (Data.find(ClientID) != Data.end())
		Data.erase(ClientID);
}

bool AccountMainSql::OnParseVotingMenu(CPlayer* pPlayer, const char* CMD, const int VoteID, const int VoteID2, int Get, const char* GetText)
{
	return false;
}

int AccountMainSql::SendAuthCode(int ClientID, int Code)
{
	if(!GS()->CheckClient(ClientID))
		return Code;

	CNetMsg_Sv_ClientProgressAuth ProgressMsg;
	ProgressMsg.m_Code = Code;
	GS()->Server()->SendPackMsg(&ProgressMsg, MSGFLAG_VITAL, ClientID);
	return Code;
}

int AccountMainSql::RegisterAccount(int ClientID, const char *Login, const char *Password)
{
	// если размер пароля логина мал или слишком большой
	if(str_length(Login) > 15 || str_length(Login) < 4 || str_length(Password) > 15 || str_length(Password) < 4)
	{
		GS()->ChatFollow(ClientID, "Username / Password must contain 4-15 characters");
		return SendAuthCode(ClientID, AUTH_ALL_MUSTCHAR);
	}

	// информация обо всем подключение
	CSqlString<32> clear_Login = CSqlString<32>(Login);
	CSqlString<32> clear_Pass = CSqlString<32>(Password);
	CSqlString<32> clear_Nick = CSqlString<32>(GS()->Server()->ClientName(ClientID));
	boost::scoped_ptr<ResultSet> RES2(SJK.SD("ID", "tw_accounts_data", "WHERE Nick = '%s'", clear_Nick.cstr()));
	if(RES2->next())
	{
		GS()->Chat(ClientID, "Your nick already used change and try again!");
		return SendAuthCode(ClientID, AUTH_REGISTER_ERROR_NICK);
	}

	// устанавливаем айди 
	boost::scoped_ptr<ResultSet> RES4(SJK.SD("ID", "tw_accounts", "ORDER BY ID DESC LIMIT 1"));
	int InitID = RES4->next() ? RES4->getInt("ID")+1 : 1; // thread save ? hm need for table all time auto increment = 1; NEED FIX IT

	// добавляем аккаунт в дб
	SJK.ID("tw_accounts", "(ID, Username, Password, RegisterDate) VALUES ('%d', '%s', '%s', UTC_TIMESTAMP())", InitID, clear_Login.cstr(), clear_Pass.cstr());
	SJK.ID("tw_accounts_data", "(ID, Nick) VALUES ('%d', '%s')", InitID, clear_Nick.cstr());

	// информация
	GS()->Chat(ClientID, "Discord group \"{STR}\"", g_Config.m_SvDiscordInviteGroup);
	GS()->Chat(ClientID, "You can log in: /login <login> <pass>!");
	return SendAuthCode(ClientID, AUTH_ALL_GOOD);
}

int AccountMainSql::LoginAccount(int ClientID, const char *Login, const char *Password)
{
	CPlayer *pPlayer = GS()->GetPlayer(ClientID, false);
	if(!pPlayer) 
		return SendAuthCode(ClientID, AUTH_ALL_UNKNOWN);
	
	// если размер пароля логина мал или слишком большой
	if(str_length(Login) > 15 || str_length(Login) < 4 || str_length(Password) > 15 || str_length(Password) < 4)
	{
		GS()->ChatFollow(ClientID, "Username / Password must contain 4-15 characters");
		return SendAuthCode(ClientID, AUTH_ALL_MUSTCHAR);
	}

	// проверяем ник на правильность		
	CSqlString<32> clear_Login = CSqlString<32>(Login);
	CSqlString<32> clear_Pass = CSqlString<32>(Password);
	CSqlString<32> clear_Nick = CSqlString<32>(GS()->Server()->ClientName(ClientID));
	boost::scoped_ptr<ResultSet> ACCOUNTDATA(SJK.SD("*", "tw_accounts_data", "WHERE Nick = '%s'", clear_Nick.cstr()));
	if(ACCOUNTDATA->next())
	{
		// проверяем по этому нику (ник может зарегестрирован только один), логин и пароль
		const int UserID = ACCOUNTDATA->getInt("ID");
		boost::scoped_ptr<ResultSet> CHECKACCESS(SJK.SD("ID", "tw_accounts", "WHERE Username = '%s' AND Password = '%s' AND ID = '%d'", clear_Login.cstr(), clear_Pass.cstr(), UserID));
		if (!CHECKACCESS->next())
		{
			GS()->Chat(ClientID, "Wrong login or password!");
			return SendAuthCode(ClientID, AUTH_LOGIN_WRONG);
		}

		// проверить онлайн ли игрок
		if (CheckOnlineAccount(UserID) >= 0)
		{
			GS()->Chat(ClientID, "The account is already in the game!");
			return SendAuthCode(ClientID, AUTH_LOGIN_ALREADY);
		}

		// если есть такое тогда погнали получать все данные игрока
		pPlayer->Acc().AuthID = UserID;
		pPlayer->Acc().Level = ACCOUNTDATA->getInt("Level");
		pPlayer->Acc().Exp = ACCOUNTDATA->getInt("Exp");
		pPlayer->Acc().GuildID = ACCOUNTDATA->getInt("GuildID");
		pPlayer->Acc().Upgrade = ACCOUNTDATA->getInt("Upgrade");
		pPlayer->Acc().GuildRank = ACCOUNTDATA->getInt("GuildRank");
		pPlayer->Acc().WorldID = ACCOUNTDATA->getInt("WorldID");
		str_copy(pPlayer->Acc().Login, clear_Login.cstr(), sizeof(pPlayer->Acc().Login));
		str_copy(pPlayer->Acc().Password, clear_Pass.cstr(), sizeof(pPlayer->Acc().Password));
		str_copy(pPlayer->Acc().LastLogin, ACCOUNTDATA->getString("LoginDate").c_str(), sizeof(pPlayer->Acc().LastLogin));

		// загрузка всех апгрейдов по списку что был установлен
		for (const auto& at : CGS::AttributInfo)
		{
			if (str_comp_nocase(at.second.FieldName, "unfield") == 0) continue;
			pPlayer->Acc().Stats[at.first] = ACCOUNTDATA->getInt(at.second.FieldName);
		}

		// сообщения функции меняем команду и все такое
		GS()->ChatFollow(ClientID, "Last Login: {STR} MSK", pPlayer->Acc().LastLogin);
		GS()->ChatFollow(ClientID, "You authed succesful!");
		GS()->ChatFollow(ClientID, "Player menu is available in votes!");
		GS()->m_pController->DoTeamChange(pPlayer, false);
		SJK.UD("tw_accounts_data", "LoginDate = CURRENT_TIMESTAMP WHERE ID = '%d'", UserID);
		return SendAuthCode(ClientID, AUTH_ALL_GOOD);
	}

	GS()->Chat(ClientID, "Your nickname was not found in the database!");
	return SendAuthCode(ClientID, AUTH_LOGIN_NICKNAME);
}

void AccountMainSql::LoadAccount(CPlayer *pPlayer, bool FirstInitilize)
{
	if(!pPlayer || !pPlayer->IsAuthed()) 
		return;

	int ClientID = pPlayer->GetCID();
	GS()->AddBroadcast(ClientID, GS()->Server()->GetWorldName(GS()->GetWorldID()), 200, 500);
	if(!FirstInitilize)
	{
		GS()->ResetVotes(ClientID, MAINMENU);

		// количество не прочитаных писем
		int CountMessageInbox = Job()->Inbox()->GetActiveInbox(ClientID);
		if(CountMessageInbox > 0) 
			GS()->Chat(ClientID, "You have unread [{INT} emails]. Check your Mailbox!", &CountMessageInbox);

		GS()->SendRangeEquipItem(ClientID, 0, MAX_CLIENTS);
		return;
	}

	Job()->OnInitAccount(ClientID);
	ShowDiscordCard(ClientID);

	if(!pPlayer->GetItem(itHammer).Count)
		pPlayer->GetItem(itHammer).Add(1, 0);

	// сменить мир если он не равен локальному
	if(pPlayer->Acc().WorldID != GS()->GetWorldID())
	{
		GS()->Server()->ChangeWorld(ClientID, pPlayer->Acc().WorldID);
		return;
	}

	GS()->SendRangeEquipItem(ClientID, 0, MAX_CLIENTS);
}

// Показать дискорд карту
void AccountMainSql::ShowDiscordCard(int ClientID)
{
	CPlayer *pPlayer = GS()->GetPlayer(ClientID, true);
	if(!pPlayer) return;

	const int Rank = GetRank(pPlayer->Acc().AuthID);
	GS()->Chat(-1, "{STR} joined to Mmo server Rank #{INT}", GS()->Server()->ClientName(ClientID), &Rank);

#ifdef CONF_DISCORD
	char pMsg[256], pLoggin[64];
	str_format(pLoggin, sizeof(pLoggin), "%s logged in Account ID %d", GS()->Server()->ClientName(ClientID), pPlayer->Acc().AuthID);
	str_format(pMsg, sizeof(pMsg), "?level=%d&player=%s&hungry=0&rank=%d&dicid=%d",
		pPlayer->Acc().Level, GS()->Server()->ClientName(ClientID), Rank, pPlayer->GetItemEquip(EQUIP_DISCORD));
	GS()->Server()->SendDiscordGenerateMessage("16757248", pLoggin, pMsg);
#endif
}

void AccountMainSql::DiscordConnect(int ClientID, const char *pDID)
{
#ifdef CONF_DISCORD
	CPlayer *pPlayer = GS()->GetPlayer(ClientID, true);
	if(!pPlayer) return;	

	// переменные
	CSqlString<64> cDID = CSqlString<64>(pDID);
	SJK.UD("tw_accounts_data", "DiscordID = '%s' WHERE ID = '%d'", cDID.cstr(), pPlayer->Acc().AuthID);

	GS()->Chat(ClientID, "Update Discord ID.");
	GS()->Chat(ClientID, "Check connect status in Discord \"!mconnect\".");
#endif
}

int AccountMainSql::GetRank(int AuthID)
{
	int Rank = 0;
	boost::scoped_ptr<ResultSet> RES(SJK.SD("ID", "tw_accounts_data", "ORDER BY Level DESC"));
	while(RES->next())
	{
		Rank++;
		int SelectedAuthID = RES->getInt("ID");
		if(AuthID == SelectedAuthID) 
			return Rank;
	}
	return -1;
}

int AccountMainSql::CheckOnlineAccount(int AuthID) const
{
	for(const auto& dt : AccountMainSql::Data)
	{
		if(dt.second.AuthID == AuthID && GS()->m_apPlayers[dt.first])
			return dt.first;
	}
	return -1;
}