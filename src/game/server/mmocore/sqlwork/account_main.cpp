/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#include <engine/shared/config.h>
#include <game/server/gamecontext.h>
#include "account_main.h"

using namespace sqlstr;

std::map < int , AccountMainSql::StructData > AccountMainSql::Data;

// Тик компонента
void AccountMainSql::OnTickLocalWorld()
{
	// показать табло лидеров игроков по уровню
	if(GS()->Server()->Tick() % (GS()->Server()->TickSpeed() * (29 * 60)) == 0)
		ShowLeaderboardPlayers(-1, "Level", 5);
}

// Отправить данные о авторизации клиенту
int AccountMainSql::SendAuthCode(int ClientID, int Code)
{
	// если ванильный клиент просто возращаем код
	if(!GS()->CheckClient(ClientID))
		return Code;

	// отправляем пакет с кодом клиенту на авторизацию
	CNetMsg_Sv_ClientProgressAuth ProgressMsg;
	ProgressMsg.m_Code = Code;
	GS()->Server()->SendPackMsg(&ProgressMsg, MSGFLAG_VITAL, ClientID);
	return Code;
}

// Регистрация аккаунта
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

	// проверяем есть ли ник в бд
	boost::scoped_ptr<ResultSet> RES2(SJK.SD("ID", "tw_accounts_data", "WHERE Nick = '%s'", clear_Nick.cstr()));
	if(RES2->next())
	{
		GS()->Chat(ClientID, "Your nick already used change and try again!");
		return SendAuthCode(ClientID, AUTH_REGISTER_ERROR_NICK);
	}

	// устанавливаем айди 
	boost::scoped_ptr<ResultSet> RES3(SJK.SD("ID", "tw_accounts", "ORDER BY ID DESC LIMIT 1"));
	int InitID = RES3->next() ? RES3->getInt("ID")+1 : 1; // thread save ? hm need for table all time auto increment = 1; NEED FIX IT

	// добавляем аккаунт в дб
	SJK.ID("tw_accounts", "(ID, Username, Password, RegisterDate) VALUES ('%d', '%s', '%s', UTC_TIMESTAMP())", InitID, clear_Login.cstr(), clear_Pass.cstr());
	SJK.ID("tw_accounts_data", "(ID, Nick) VALUES ('%d', '%s')", InitID, clear_Nick.cstr());

	// информация
	GS()->Chat(ClientID, "Discord group \"{STR}\"", g_Config.m_SvDiscordInviteGroup);
	GS()->Chat(ClientID, "You can log in: /login <login> <pass>!");
	return SendAuthCode(ClientID, AUTH_ALL_GOOD);
}

// Авторизация игрока
int AccountMainSql::LoginAccount(int ClientID, const char *Login, const char *Password)
{
	CPlayer *pPlayer = GS()->GetPlayer(ClientID, false);
	if(!pPlayer) return SendAuthCode(ClientID, AUTH_ALL_UNKNOWN);
	
	// если размер пароля логина мал или слишком большой
	if(str_length(Login) > 15 || str_length(Login) < 4 || str_length(Password) > 15 || str_length(Password) < 4)
	{
		GS()->ChatFollow(ClientID, "Username / Password must contain 4-15 characters");
		return SendAuthCode(ClientID, AUTH_ALL_MUSTCHAR);
	}

	// проверяем логин и пароль и проверяем онлайн игрока
	CSqlString<32> clear_Login = CSqlString<32>(Login);
	CSqlString<32> clear_Pass = CSqlString<32>(Password);
	boost::scoped_ptr<ResultSet> RES(SJK.SD("ID, WorldID", "tw_accounts", "WHERE Username = '%s' AND Password = '%s'", clear_Login.cstr(), clear_Pass.cstr()));
	if(!RES->next())
	{
		GS()->Chat(ClientID, "Wrong login or password!");
		return SendAuthCode(ClientID, AUTH_LOGIN_WRONG);
	}

	const int UserID = RES->getInt("ID");
	const int WorldID = RES->getInt("WorldID");

	// если игрок уже в игре
	if(CheckOnlineAccount(UserID) >= 0)
		return SendAuthCode(ClientID, AUTH_LOGIN_ALREADY);

	// проверяем ник на правильность
	CSqlString<32> clear_Nick = CSqlString<32>(GS()->Server()->ClientName(ClientID));
	boost::scoped_ptr<ResultSet> RES2(SJK.SD("*", "tw_accounts_data", "WHERE Nick = '%s' AND ID = '%d'", clear_Nick.cstr(), UserID));
	if(!RES2->next())
	{
		// если ник не верен из базы данных
		GS()->Chat(ClientID, "Wrong nickname, use how at registration!");
		return SendAuthCode(ClientID, AUTH_LOGIN_NICKNAME);
	}

	// если есть такое тогда погнали получать все данные игрока
	pPlayer->Acc().AuthID = UserID;
	pPlayer->Acc().Level = RES2->getInt("Level");
	pPlayer->Acc().Exp = RES2->getInt("Exp");
	pPlayer->Acc().MemberID = RES2->getInt("MemberID");
	pPlayer->Acc().Hungry = RES2->getInt("Eat");
	pPlayer->Acc().Upgrade = RES2->getInt("Upgrade");
	pPlayer->Acc().MemberRank = RES2->getInt("MemberRank");
	pPlayer->Acc().WorldID = WorldID;
	str_copy(pPlayer->Acc().Login, clear_Login.cstr(), sizeof(pPlayer->Acc().Login));
	str_copy(pPlayer->Acc().Password, clear_Pass.cstr(), sizeof(pPlayer->Acc().Password));
	str_copy(pPlayer->Acc().LastLogin, RES2->getString("LoginDate").c_str(), sizeof(pPlayer->Acc().LastLogin));

	// загрузка всех апгрейдов по списку что был установлен
	for(const auto& at : CGS::AttributInfo)
	{
		if(str_comp_nocase(at.second.FieldName, "unfield") == 0) continue;
		pPlayer->Acc().Stats[at.first] = RES2->getInt(at.second.FieldName);
	}

	// сообщения функции меняем команду и все такое
	GS()->ChatFollow(ClientID, "Last Login: {STR} MSK", pPlayer->Acc().LastLogin);
	GS()->ChatFollow(ClientID, "You authed succesful!");
	GS()->ChatFollow(ClientID, "Player menu is available in votes!");
	GS()->m_pController->DoTeamChange(pPlayer, false);

	// обновляем дату логина
	SJK.UD("tw_accounts_data", "LoginDate = CURRENT_TIMESTAMP WHERE ID = '%d'", UserID);
	return SendAuthCode(ClientID, AUTH_ALL_GOOD);
}

// Загрузка данных при успешной авторизации
void AccountMainSql::LoadAccount(CPlayer *pPlayer, bool FirstInitilize)
{
	if(!pPlayer || !pPlayer->IsAuthed()) return;

	// если обычная прогрузка между мирами
	int ClientID = pPlayer->GetCID();
	GS()->AddBroadcast(ClientID, GS()->Server()->GetWorldName(GS()->GetWorldID()), 200, 500);

	// первая прогрузка
	if(!FirstInitilize)
	{
		// проверяем квесты и отправляем одетые предметы всем
		Job()->Quest()->CheckQuest(pPlayer);
		GS()->ResetVotes(ClientID, MAINMENU);

		// количество не прочитаных писем
		int CountMessageInbox = Job()->Inbox()->GetActiveInbox(ClientID);
		if(CountMessageInbox > 0) 
			GS()->Chat(ClientID, "You have unread [{INT} emails]. Check your Mailbox!", &CountMessageInbox);

		SendAuthCode(ClientID, AUTH_ALL_GOOD);
		return;
	}

	// загружаем данные аккаунта
	Job()->OnInitAccount(ClientID);
	Job()->Quest()->CheckQuest(pPlayer);
	ShowDiscordCard(ClientID);

	// выдаем молот если нет его
	if(!pPlayer->GetItem(itHammer).Count)
		pPlayer->GetItem(itHammer).Add(1, 0);

	// сменить мир если он не равен локальному
	if(pPlayer->Acc().WorldID != GS()->GetWorldID())
	{
		GS()->Server()->ChangeWorld(ClientID, pPlayer->Acc().WorldID);
		return;
	}

	// если клиент проверен отправим свой скин
	if(GS()->CheckClient(ClientID))
		GS()->SendEquipItem(ClientID, ClientID);
}

// Парсинг голосований
bool AccountMainSql::OnParseVotingMenu(CPlayer *pPlayer, const char *CMD, const int VoteID, const int VoteID2, int Get, const char *GetText)
{
	return false;
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
	str_format(pMsg, sizeof(pMsg), "?level=%d&player=%s&hungry=%d&rank=%d&dicid=%d",
		pPlayer->Acc().Level, GS()->Server()->ClientName(ClientID), pPlayer->Acc().Hungry, Rank, pPlayer->GetItemEquip(EQUIP_DISCORD));
	GS()->Server()->SendDiscordGenerateMessage("16757248", pLoggin, pMsg);
#endif
}

// смена дискорда подключение к дискорду
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

// Показать топ лист игроков и всего чего можно
void AccountMainSql::ShowLeaderboardPlayers(int ClientID, const char *IntegerField, int ShowMaximal)
{
	GS()->Chat(ClientID, "----- Player Leaderboard {STR}", IntegerField);

	int Position = 0;
	boost::scoped_ptr<ResultSet> RES(SJK.SD("*", "tw_accounts_data", "ORDER BY %s DESC", IntegerField));
	while(RES->next())
	{
		if(Position == ShowMaximal) break;
		Position++;

		const int Count = RES->getInt(IntegerField);
		const char *Nick = RES->getString("Nick").c_str();
		GS()->Chat(ClientID, "{INT}. {STR} | {STR}: {INT}", &Position, Nick, IntegerField, &Count);
	}
}

// Получить ранг игрока
int AccountMainSql::GetRank(int AuthID)
{
	int Rank = 0;
	boost::scoped_ptr<ResultSet> RES(SJK.SD("ID", "tw_accounts_data", "ORDER BY Level DESC"));
	while(RES->next())
	{
		Rank++;
		int SelectedAuthID = RES->getInt("ID");
		if(AuthID != SelectedAuthID) 
			continue;

		return Rank;
	}
	return -1;
}

// Проверяем игрок по AccountID онлайн если да то возращаем его ClientID
int AccountMainSql::CheckOnlineAccount(int AuthID) const
{
	for(const auto& dt : AccountMainSql::Data)
	{
		if(dt.second.AuthID == AuthID && GS()->m_apPlayers[dt.first])
			return dt.first;
	}
	return -1;
}
