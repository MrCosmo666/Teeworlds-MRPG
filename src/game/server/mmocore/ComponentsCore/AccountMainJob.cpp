/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#include <engine/shared/config.h>
#include <game/server/gamecontext.h>
#include "AccountMainJob.h"

using namespace sqlstr;
std::map < int, AccountMainJob::StructData > AccountMainJob::Data;
std::map < int, AccountMainJob::StructTempPlayerData > AccountMainJob::PlayerTempData;

int AccountMainJob::SendAuthCode(int ClientID, int Code)
{
	if (GS()->CheckClient(ClientID))
	{
		CNetMsg_Sv_ClientProgressAuth ProgressMsg;
		ProgressMsg.m_Code = Code;
		GS()->Server()->SendPackMsg(&ProgressMsg, MSGFLAG_VITAL, ClientID);
	}
	return Code;
}

int AccountMainJob::CheckOnlineAccount(int AuthID) const
{
	for (const auto& dt : AccountMainJob::Data)
	{
		if (dt.second.AuthID == AuthID && GS()->m_apPlayers[dt.first])
			return dt.first;
	}
	return -1;
}

int AccountMainJob::RegisterAccount(int ClientID, const char *Login, const char *Password)
{
	if(str_length(Login) > 12 || str_length(Login) < 4 || str_length(Password) > 12 || str_length(Password) < 4)
	{
		GS()->ChatFollow(ClientID, "Username / Password must contain 4-12 characters");
		return SendAuthCode(ClientID, AUTH_ALL_MUSTCHAR);
	}

	CSqlString<32> clear_Nick = CSqlString<32>(GS()->Server()->ClientName(ClientID));
	boost::scoped_ptr<ResultSet> RES2(SJK.SD("ID", "tw_accounts_data", "WHERE Nick = '%s'", clear_Nick.cstr()));
	if(RES2->next())
	{
		GS()->Chat(ClientID, "Your nick already used change and try again!");
		return SendAuthCode(ClientID, AUTH_REGISTER_ERROR_NICK);
	}

	boost::scoped_ptr<ResultSet> RES4(SJK.SD("ID", "tw_accounts", "ORDER BY ID DESC LIMIT 1"));
	int InitID = RES4->next() ? RES4->getInt("ID")+1 : 1; // thread save ? hm need for table all time auto increment = 1; NEED FIX IT

	CSqlString<32> clear_Login = CSqlString<32>(Login);
	CSqlString<32> clear_Pass = CSqlString<32>(Password);
	SJK.ID("tw_accounts", "(ID, Username, Password, RegisterDate) VALUES ('%d', '%s', '%s', UTC_TIMESTAMP())", InitID, clear_Login.cstr(), clear_Pass.cstr());
	SJK.IDS(100, "tw_accounts_data", "(ID, Nick) VALUES ('%d', '%s')", InitID, clear_Nick.cstr());

	GS()->Chat(ClientID, "Discord group \"{STR}\"", g_Config.m_SvDiscordInviteGroup);
	GS()->Chat(ClientID, "You can log in: /login <login> <pass>!");
	return SendAuthCode(ClientID, AUTH_REGISTER_GOOD);
}

int AccountMainJob::LoginAccount(int ClientID, const char *Login, const char *Password)
{
	CPlayer *pPlayer = GS()->GetPlayer(ClientID, false);
	if(!pPlayer) 
		return SendAuthCode(ClientID, AUTH_ALL_UNKNOWN);
	
	if(str_length(Login) > 12 || str_length(Login) < 4 || str_length(Password) > 12 || str_length(Password) < 4)
	{
		GS()->ChatFollow(ClientID, "Username / Password must contain 4-12 characters");
		return SendAuthCode(ClientID, AUTH_ALL_MUSTCHAR);
	}

	CSqlString<32> clear_Login = CSqlString<32>(Login);
	CSqlString<32> clear_Pass = CSqlString<32>(Password);
	CSqlString<32> clear_Nick = CSqlString<32>(GS()->Server()->ClientName(ClientID));
	boost::scoped_ptr<ResultSet> ACCOUNTDATA(SJK.SD("*", "tw_accounts_data", "WHERE Nick = '%s'", clear_Nick.cstr()));
	if(ACCOUNTDATA->next())
	{
		const int UserID = ACCOUNTDATA->getInt("ID");
		boost::scoped_ptr<ResultSet> CHECKACCESS(SJK.SD("ID", "tw_accounts", "WHERE Username = '%s' AND Password = '%s' AND ID = '%d'", clear_Login.cstr(), clear_Pass.cstr(), UserID));
		if (!CHECKACCESS->next())
		{
			GS()->Chat(ClientID, "Wrong login or password!");
			return SendAuthCode(ClientID, AUTH_LOGIN_WRONG);
		}

		if (CheckOnlineAccount(UserID) >= 0)
		{
			GS()->Chat(ClientID, "The account is already in the game!");
			return SendAuthCode(ClientID, AUTH_LOGIN_ALREADY);
		}

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
		for (const auto& at : CGS::AttributInfo)
		{
			if (str_comp_nocase(at.second.FieldName, "unfield") == 0) continue;
			pPlayer->Acc().Stats[at.first] = ACCOUNTDATA->getInt(at.second.FieldName);
		}


		GS()->Chat(ClientID, "- - - - - - - [Successful login] - - - - - - -");
		GS()->Chat(ClientID, "Player menu is available in votes!");
		GS()->m_pController->DoTeamChange(pPlayer, false);
		SJK.UD("tw_accounts_data", "LoginDate = CURRENT_TIMESTAMP WHERE ID = '%d'", UserID);
		return SendAuthCode(ClientID, AUTH_LOGIN_GOOD);
	}

	GS()->Chat(ClientID, "Your nickname was not found in the database!");
	return SendAuthCode(ClientID, AUTH_LOGIN_NICKNAME);
}

void AccountMainJob::LoadAccount(CPlayer *pPlayer, bool FirstInitilize)
{
	if(!pPlayer || !pPlayer->IsAuthed() || !GS()->IsClientEqualWorldID(pPlayer->GetCID()))
		return;

	const int ClientID = pPlayer->GetCID();
	GS()->SBL(ClientID, BroadcastPriority::BROADCAST_MAIN_INFORMATION, 200, "You are located {STR}", GS()->Server()->GetWorldName(GS()->GetWorldID()));

	if(!FirstInitilize)
	{
		const int CountMessageInbox = Job()->Inbox()->GetActiveInbox(pPlayer);
		if (CountMessageInbox > 0)
			GS()->Chat(ClientID, "You have {INT} unread messages. Check your Mailbox!", &CountMessageInbox);

		GS()->ResetVotes(ClientID, MenuList::MAIN_MENU);
		GS()->SendRangeEquipItem(ClientID, 0, MAX_CLIENTS);
		return;
	}

	Job()->OnInitAccount(ClientID);
	const int Rank = GetRank(pPlayer->Acc().AuthID);
	GS()->Chat(-1, "{STR} joined to Mmo server Rank #{INT}", GS()->Server()->ClientName(ClientID), &Rank);
#ifdef CONF_DISCORD
	char pMsg[256], pLoggin[64];
	str_format(pLoggin, sizeof(pLoggin), "%s logged in Account ID %d", GS()->Server()->ClientName(ClientID), pPlayer->Acc().AuthID);
	str_format(pMsg, sizeof(pMsg), "?player=%s&rank=%d&dicid=%d",
		GS()->Server()->ClientName(ClientID), Rank, pPlayer->GetEquippedItem(EQUIP_DISCORD));
	GS()->Server()->SendDiscordGenerateMessage("16757248", pLoggin, pMsg);
#endif

	// настройки
	if(!pPlayer->GetItem(itModePVP).Count)
		pPlayer->GetItem(itModePVP).Add(1, 1);

	// включить спавн в безопасной зоне
	pPlayer->GetTempData().TempActiveSafeSpawn = true;
	if (!pPlayer->GetItem(itHammer).Count)
		pPlayer->GetItem(itHammer).Add(1);

	if(pPlayer->Acc().WorldID != GS()->GetWorldID())
	{
		pPlayer->ChangeWorld(pPlayer->Acc().WorldID);
		return;
	}
	GS()->SendRangeEquipItem(ClientID, 0, MAX_CLIENTS);
}

void AccountMainJob::DiscordConnect(int ClientID, const char *pDID)
{
#ifdef CONF_DISCORD
	CPlayer *pPlayer = GS()->GetPlayer(ClientID, true);
	if(!pPlayer) return;	

	// переменные
	CSqlString<64> cDID = CSqlString<64>(pDID);
	SJK.UD("tw_accounts_data", "DiscordID = '%s' WHERE ID = '%d'", cDID.cstr(), pPlayer->Acc().AuthID);

	GS()->Chat(ClientID, "Update DiscordID.");
	GS()->Chat(ClientID, "Check connect status in Discord \"!mconnect\".");
#endif
}

int AccountMainJob::GetRank(int AuthID)
{
	int Rank = 0;
	boost::scoped_ptr<ResultSet> RES(SJK.SD("ID", "tw_accounts_data", "ORDER BY Level DESC, Exp DESC"));
	while(RES->next())
	{
		Rank++;
		int SelectedAuthID = RES->getInt("ID");
		if(AuthID == SelectedAuthID) 
			return Rank;
	}
	return -1;
}

void AccountMainJob::OnResetClient(int ClientID)
{
	if(PlayerTempData.find(ClientID) != PlayerTempData.end())
		PlayerTempData.erase(ClientID);

	if (Data.find(ClientID) != Data.end())
		Data.erase(ClientID);
}