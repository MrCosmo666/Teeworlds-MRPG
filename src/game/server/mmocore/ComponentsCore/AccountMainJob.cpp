/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#include <base/hash_ctxt.h>
#include <engine/shared/config.h>
#include <game/server/gamecontext.h>
#include <teeother/components/localization.h>
#include <teeother/tl/nlohmann_json.h>
#include "AccountMainJob.h"

using namespace sqlstr;
std::map < int, AccountMainJob::StructData > AccountMainJob::ms_aData;
std::map < int, AccountMainJob::StructTempPlayerData > AccountMainJob::ms_aPlayerTempData;

int AccountMainJob::GetHistoryLatestCorrectWorldID(CPlayer* pPlayer) const
{
	const auto pWorldIterator = std::find_if(pPlayer->Acc().m_aHistoryWorld.begin(), pPlayer->Acc().m_aHistoryWorld.end(), [=](int WorldID)
	{
		const int QuestToUnlock = Job()->WorldSwap()->GetNecessaryQuest(WorldID);
		const bool IsValidQuest = Job()->Quest()->IsValidQuest(QuestToUnlock);
		return !Job()->Dungeon()->IsDungeonWorld(WorldID) && ((IsValidQuest && pPlayer->GetQuest(QuestToUnlock).IsComplected()) || !IsValidQuest);
	});
	return pWorldIterator != pPlayer->Acc().m_aHistoryWorld.end() ? *pWorldIterator : MAIN_WORLD_ID;
}

int AccountMainJob::SendAuthCode(int ClientID, int Code)
{
	if(GS()->IsMmoClient(ClientID))
	{
		CNetMsg_Sv_ClientProgressAuth ProgressMsg;
		ProgressMsg.m_Code = Code;
		Server()->SendPackMsg(&ProgressMsg, MSGFLAG_VITAL, ClientID);
	}
	return Code;
}

int AccountMainJob::RegisterAccount(int ClientID, const char *Login, const char *Password)
{
	if(str_length(Login) > 12 || str_length(Login) < 4 || str_length(Password) > 12 || str_length(Password) < 4)
	{
		GS()->Chat(ClientID, "Username / Password must contain 4-12 characters");
		return SendAuthCode(ClientID, AUTH_ALL_MUSTCHAR);
	}
	CSqlString<32> clear_Nick = CSqlString<32>(Server()->ClientName(ClientID));
	ResultPtr pRes = SJK.SD("ID", "tw_accounts_data", "WHERE Nick = '%s'", clear_Nick.cstr());
	if(pRes->next())
	{
		GS()->Chat(ClientID, "- - - - [Your nickname is already registered] - - - -");
		GS()->Chat(ClientID, "Your nick is a unique identifier, and it has already been used!");
		GS()->Chat(ClientID, "You can restore access by contacting support, or change nick.");
		GS()->Chat(ClientID, "Discord group \"{STR}\".", g_Config.m_SvDiscordInviteLink);
		return SendAuthCode(ClientID, AUTH_REGISTER_ERROR_NICK);
	}

	ResultPtr pResID = SJK.SD("ID", "tw_accounts", "ORDER BY ID DESC LIMIT 1");
	const int InitID = pResID->next() ? pResID->getInt("ID")+1 : 1; // thread save ? hm need for table all time auto increment = 1; NEED FIX IT

	CSqlString<32> clear_Login = CSqlString<32>(Login);
	CSqlString<32> clear_Pass = CSqlString<32>(Password);

	char aAddrStr[64];
	Server()->GetClientAddr(ClientID, aAddrStr, sizeof(aAddrStr));

	char aSalt[32] = { 0 };
	secure_random_password(aSalt, sizeof(aSalt), 24);

	SJK.ID("tw_accounts", "(ID, Username, Password, PasswordSalt, RegisterDate, RegisteredIP) VALUES ('%d', '%s', '%s', '%s', UTC_TIMESTAMP(), '%s')", InitID, clear_Login.cstr(), HashPassword(clear_Pass.cstr(), aSalt).c_str(), aSalt, aAddrStr);
	SJK.IDS(100, "tw_accounts_data", "(ID, Nick) VALUES ('%d', '%s')", InitID, clear_Nick.cstr());

	GS()->Chat(ClientID, "- - - - - - - [Successful registered] - - - - - - -");
	GS()->Chat(ClientID, "Don't forget your data, have a nice game!");
	GS()->Chat(ClientID, "# Your nickname is a unique identifier!");
	GS()->Chat(ClientID, "# Log in: \"/login {STR} {STR}\"", clear_Login.cstr(), clear_Pass.cstr());
	return SendAuthCode(ClientID, AUTH_REGISTER_GOOD);
}

int AccountMainJob::LoginAccount(int ClientID, const char *Login, const char *Password)
{
	CPlayer *pPlayer = GS()->GetPlayer(ClientID, false);
	if(!pPlayer) 
		return SendAuthCode(ClientID, AUTH_ALL_UNKNOWN);
	
	const int LengthLogin = str_length(Login);
	const int LengthPassword = str_length(Password);
	if(LengthLogin > 12 || LengthLogin < 4 || LengthPassword > 12 || LengthPassword < 4)
	{
		GS()->ChatFollow(ClientID, "Username / Password must contain 4-12 characters");
		return SendAuthCode(ClientID, AUTH_ALL_MUSTCHAR);
	}

	CSqlString<32> clear_Login = CSqlString<32>(Login);
	CSqlString<32> clear_Pass = CSqlString<32>(Password);
	CSqlString<32> clear_Nick = CSqlString<32>(Server()->ClientName(ClientID));
	ResultPtr pResAccount = SJK.SD("*", "tw_accounts_data", "WHERE Nick = '%s'", clear_Nick.cstr());
	if(pResAccount->next())
	{
		const int UserID = pResAccount->getInt("ID");
		ResultPtr pResCheck = SJK.SD("ID, LoginDate, Language, Password, PasswordSalt", "tw_accounts", "WHERE Username = '%s' AND ID = '%d'", clear_Login.cstr(), UserID);

		bool LoginSuccess = false;
		if(pResCheck->next())
		{
			if(!str_comp(pResCheck->getString("Password").c_str(), HashPassword(clear_Pass.cstr(), pResCheck->getString("PasswordSalt").c_str()).c_str()))
				LoginSuccess = true;
		}

		if(!LoginSuccess)
		{
			GS()->Chat(ClientID, "Wrong login or password!");
			return SendAuthCode(ClientID, AUTH_LOGIN_WRONG);
		}

		if (GS()->GetPlayerFromAuthID(UserID) != nullptr)
		{
			GS()->Chat(ClientID, "The account is already in the game!");
			return SendAuthCode(ClientID, AUTH_LOGIN_ALREADY);
		}

		Server()->SetClientLanguage(ClientID, pResCheck->getString("Language").c_str());
		str_copy(pPlayer->Acc().m_aLogin, clear_Login.cstr(), sizeof(pPlayer->Acc().m_aLogin));
		str_copy(pPlayer->Acc().m_aLastLogin, pResCheck->getString("LoginDate").c_str(), sizeof(pPlayer->Acc().m_aLastLogin));

		pPlayer->Acc().m_AccountID = UserID;
		pPlayer->Acc().m_Level = pResAccount->getInt("Level");
		pPlayer->Acc().m_Exp = pResAccount->getInt("Exp");
		pPlayer->Acc().m_GuildID = pResAccount->getInt("GuildID");
		pPlayer->Acc().m_Upgrade = pResAccount->getInt("Upgrade");
		pPlayer->Acc().m_GuildRank = pResAccount->getInt("GuildRank");
		pPlayer->Acc().m_aHistoryWorld.push_front(pResAccount->getInt("WorldID"));

		for (const auto& at : CGS::ms_aAttributsInfo)
		{
			if (str_comp_nocase(at.second.m_aFieldName, "unfield") != 0)
				pPlayer->Acc().m_aStats[at.first] = pResAccount->getInt(at.second.m_aFieldName);
		}

		GS()->Chat(ClientID, "- - - - - - - [Successful login] - - - - - - -");
		GS()->Chat(ClientID, "Menu is available in call-votes!");
		GS()->m_pController->DoTeamChange(pPlayer, false);

		char aAddrStr[64];
		Server()->GetClientAddr(ClientID, aAddrStr, sizeof(aAddrStr));
		SJK.UD("tw_accounts", "LoginDate = CURRENT_TIMESTAMP, LoginIP = '%s' WHERE ID = '%d'", aAddrStr, UserID);
		return SendAuthCode(ClientID, AUTH_LOGIN_GOOD);
	}

	GS()->Chat(ClientID, "Your nickname was not found in the database!");
	return SendAuthCode(ClientID, AUTH_LOGIN_NICKNAME);
}

void AccountMainJob::LoadAccount(CPlayer *pPlayer, bool FirstInitilize)
{
	if(!pPlayer || !pPlayer->IsAuthed() || !GS()->IsPlayerEqualWorldID(pPlayer->GetCID()))
		return;

	const int ClientID = pPlayer->GetCID();
	GS()->Broadcast(ClientID, BroadcastPriority::BROADCAST_MAIN_INFORMATION, 200, "You are located {STR} ({STR})", 
		Server()->GetWorldName(GS()->GetWorldID()), (GS()->IsAllowedPVP() ? "Zone PVP" : "Safe zone"));

	GS()->SendWorldMusic(ClientID, (GS()->IsDungeon() ? -1 : 0));
	if(!FirstInitilize)
	{
		const int CountMessageInbox = Job()->Inbox()->GetActiveInbox(pPlayer);
		if (CountMessageInbox > 0)
			GS()->Chat(ClientID, "You have {INT} unread messages!", &CountMessageInbox);

		GS()->ResetVotes(ClientID, MenuList::MAIN_MENU);
		GS()->SendFullyEquipments(ClientID);
		return;
	}

	Job()->OnInitAccount(ClientID);
	const int Rank = GetRank(pPlayer->Acc().m_AccountID);
	GS()->Chat(-1, "{STR} logged to account. Rank #{INT}", Server()->ClientName(ClientID), &Rank);
#ifdef CONF_DISCORD
	char aLoginBuf[64];
	str_format(aLoginBuf, sizeof(aLoginBuf), "%s logged in Account ID %d", Server()->ClientName(ClientID), pPlayer->Acc().m_AccountID);
	Server()->SendDiscordGenerateMessage(aLoginBuf, pPlayer->Acc().m_AccountID);
#endif

	if (!pPlayer->GetItem(itHammer).m_Count)
	{
		pPlayer->GetItem(itHammer).Add(1);
		GS()->Chat(ClientID, "Quest NPCs are marked with an aura Heart and Shield.");
		GS()->Chat(ClientID, "Shield around you indicates location of active quest.");
	}
	
	// settings
	if(!pPlayer->GetItem(itModePVP).m_Count)
		pPlayer->GetItem(itModePVP).Add(1, 1);

	pPlayer->GetTempData().m_TempSafeSpawn = true;

	int LatestCorrectWorldID = GetHistoryLatestCorrectWorldID(pPlayer);
	if(LatestCorrectWorldID != GS()->GetWorldID())
	{
		pPlayer->ChangeWorld(LatestCorrectWorldID);
		return;
	}
	GS()->SendFullyEquipments(ClientID);
}

void AccountMainJob::DiscordConnect(int ClientID, const char *pDID)
{
#ifdef CONF_DISCORD
	CPlayer *pPlayer = GS()->GetPlayer(ClientID, true);
	if(!pPlayer) return;	

	CSqlString<64> DiscordID = CSqlString<64>(pDID);
	SJK.UD("tw_accounts_data", "DiscordID = 'null' WHERE DiscordID = '%s'", DiscordID.cstr());
	SJK.UD("tw_accounts_data", "DiscordID = '%s' WHERE ID = '%d'", DiscordID.cstr(), pPlayer->Acc().m_AccountID);

	GS()->Chat(ClientID, "Update DiscordID.");
	GS()->Chat(ClientID, "Check connect status in Discord \"!mconnect\".");
#endif
}

int AccountMainJob::GetRank(int AuthID)
{
	int Rank = 0;
	ResultPtr pRes = SJK.SD("ID", "tw_accounts_data", "ORDER BY Level DESC, Exp DESC");
	while(pRes->next())
	{
		Rank++;
		int SelectedAuthID = pRes->getInt("ID");
		if(AuthID == SelectedAuthID) 
			return Rank;
	}
	return -1;
}

bool AccountMainJob::OnHandleMenulist(CPlayer* pPlayer, int Menulist, bool ReplaceMenu)
{
	const int ClientID = pPlayer->GetCID();
	if (ReplaceMenu)
	{
		return false;
	}

	// settings
	if (Menulist == MenuList::MENU_SETTINGS)
	{
		pPlayer->m_LastVoteMenu = MenuList::MAIN_MENU;

		// settings
		GS()->AVH(ClientID, TAB_SETTINGS, RED_COLOR, "Some of the settings becomes valid after death");
		GS()->AVM(ClientID, "MENU", MenuList::MENU_SELECT_LANGUAGE, TAB_SETTINGS, "Settings language");
		for (const auto& it : InventoryJob::ms_aItems[ClientID])
		{
			const InventoryItem ItemData = it.second;
			if (ItemData.Info().m_Type == ItemType::TYPE_SETTINGS && ItemData.m_Count > 0)
				GS()->AVM(ClientID, "ISETTINGS", it.first, TAB_SETTINGS, "[{STR}] {STR}", (ItemData.m_Settings ? "Enable" : "Disable"), ItemData.Info().GetName(pPlayer));
		}

		// equipment modules
		bool IsFoundModules = false;
		GS()->AV(ClientID, "null");
		GS()->AVH(ClientID, TAB_SETTINGS_MODULES, GREEN_COLOR, "Modules settings");
		for (const auto& it : InventoryJob::ms_aItems[ClientID])
		{
			const InventoryItem ItemData = it.second;
			if (ItemData.Info().m_Type == ItemType::TYPE_MODULE && ItemData.m_Count > 0)
			{
				char aAttributes[128];
				ItemData.FormatAttributes(aAttributes, sizeof(aAttributes));
				GS()->AVMI(ClientID, ItemData.Info().GetIcon(), "ISETTINGS", it.first, TAB_SETTINGS_MODULES, "{STR} {STR}{STR}",
					ItemData.Info().GetName(pPlayer), aAttributes, (ItemData.m_Settings ? "✔" : "\0"));
				IsFoundModules = true;
			}
		}

		// if no modules are found
		if (!IsFoundModules)
			GS()->AVM(ClientID, "null", NOPE, TAB_SETTINGS_MODULES, "The list of modules equipment is empty.");
	
		GS()->AddVotesBackpage(ClientID);
		return true;
	}

	// language selection
	if (Menulist == MenuList::MENU_SELECT_LANGUAGE)
	{
		pPlayer->m_LastVoteMenu = MenuList::MENU_SETTINGS;
		GS()->AVH(ClientID, TAB_INFO_LANGUAGES, GREEN_COLOR, "Languages Information");
		GS()->AVM(ClientID, "null", NOPE, TAB_INFO_LANGUAGES, "Here you can choose the language.");
		GS()->AVM(ClientID, "null", NOPE, TAB_INFO_LANGUAGES, "Note: translation is not complete.");
		GS()->AV(ClientID, "null");

		const char* pPlayerLanguage = pPlayer->GetLanguage();
		GS()->AVH(ClientID, TAB_LANGUAGES, GRAY_COLOR, "Active language: [{STR}]", pPlayerLanguage);
		for(int i = 0; i < Server()->Localization()->m_pLanguages.size(); i++)
		{
			// do not show the language already selected by the player in the selection lists
			if(str_comp(pPlayerLanguage, Server()->Localization()->m_pLanguages[i]->GetFilename()) == 0)
				continue;

			// add language selection
			const char *pLanguageName = Server()->Localization()->m_pLanguages[i]->GetName();
			GS()->AVM(ClientID, "SELECTLANGUAGE", i, TAB_LANGUAGES, "Select language \"{STR}\"", pLanguageName);
		}
		GS()->AddVotesBackpage(ClientID);
		return true;
	}
	return false;
}

bool AccountMainJob::OnHandleVoteCommands(CPlayer* pPlayer, const char* CMD, const int VoteID, const int VoteID2, int Get, const char* GetText)
{
	const int ClientID = pPlayer->GetCID();
	if (PPSTR(CMD, "SELECTLANGUAGE") == 0)
	{
		const char *pSelectedLanguage = Server()->Localization()->m_pLanguages[VoteID]->GetFilename();
		Server()->SetClientLanguage(ClientID, pSelectedLanguage);
		GS()->Chat(ClientID, "You chosen a language \"{STR}\".", pSelectedLanguage);
		GS()->StrongUpdateVotes(ClientID, MenuList::MENU_SELECT_LANGUAGE);
		Job()->SaveAccount(pPlayer, SaveType::SAVE_LANGUAGE);
		return true;
	}
	return false;
}

void AccountMainJob::OnResetClient(int ClientID)
{
	ms_aPlayerTempData.erase(ClientID);
	ms_aData.erase(ClientID);
}

void AccountMainJob::OnMessage(int MsgID, void* pRawMsg, int ClientID)
{
	CPlayer *pPlayer = GS()->m_apPlayers[ClientID];
	if(!pPlayer)
		return;

	if(MsgID == NETMSGTYPE_CL_CLIENTAUTH)
	{
		CNetMsg_Cl_ClientAuth* pMsg = (CNetMsg_Cl_ClientAuth*)pRawMsg;

		// account registration
		if(pMsg->m_SelectRegister)
		{
			RegisterAccount(ClientID, pMsg->m_Login, pMsg->m_Password);
			return;
		}

		// account authorization
		if(LoginAccount(ClientID, pMsg->m_Login, pMsg->m_Password) == AUTH_LOGIN_GOOD)
			LoadAccount(pPlayer, true);
	}
}

std::string AccountMainJob::HashPassword(const char* pPassword, const char* pSalt)
{
	char aPlaintext[128] = { 0 };
	SHA256_CTX Sha256Ctx;
	sha256_init(&Sha256Ctx);
	str_format(aPlaintext, sizeof(aPlaintext), "%s%s%s", pSalt, pPassword, pSalt);
	sha256_update(&Sha256Ctx, aPlaintext, str_length(aPlaintext));
	SHA256_DIGEST Digest = sha256_finish(&Sha256Ctx);

	char aHash[SHA256_MAXSTRSIZE];
	sha256_str(Digest, aHash, sizeof(aHash));
	return std::string(aHash);
}

void AccountMainJob::UseVoucher(int ClientID, const char* pVoucher)
{
	CPlayer* pPlayer = GS()->m_apPlayers[ClientID];
	if (!pPlayer || !pPlayer->IsAuthed())
		return;

	char aSelect[256];
	CSqlString<32> VoucherCode = CSqlString<32>(pVoucher);
	str_format(aSelect, sizeof(aSelect), "v.*, IF((SELECT r.id FROM tw_voucher_redeemed r WHERE CASE v.multiple WHEN 1 THEN r.voucher_id = v.id AND r.user_id = %d ELSE r.voucher_id = v.id END) IS NULL, FALSE, TRUE) AS used", pPlayer->Acc().m_AccountID);

	ResultPtr pResVoucher = SJK.SD(aSelect, "tw_voucher v", "WHERE v.code = '%s'", VoucherCode.cstr());
	if (pResVoucher->next())
	{
		int VoucherID = pResVoucher->getInt("id");
		int ValidUntil = pResVoucher->getInt("valid_until");
		nlohmann::json JsonData = nlohmann::json::parse(pResVoucher->getString("data").c_str());

		if (ValidUntil > 0 && ValidUntil < time(0))
			GS()->Chat(ClientID, "The voucher code '{STR}' has expired.", pVoucher);
		else if (pResVoucher->getBoolean("used"))
			GS()->Chat(ClientID, "This voucher has already been redeemed.");
		else
		{
			int Exp = JsonData.value("exp", 0);
			int Money = JsonData.value("money", 0);
			int Upgrade = JsonData.value("upgrade", 0);

			if (Exp > 0)
				pPlayer->AddExp(Exp);
			if (Money > 0)
				pPlayer->AddMoney(Money);
			if(Upgrade > 0)
				pPlayer->Acc().m_Upgrade += Upgrade;

			if (JsonData.find("items") != JsonData.end() && JsonData["items"].is_array())
			{
				for (nlohmann::json Item : JsonData["items"])
				{
					int ItemID = Item.value("id", -1);
					int Value = Item.value("value", 0);

					if (ItemID > NOPE && Value > 0)
					{
						if (InventoryJob::ms_aItemsInfo.find(ItemID) != InventoryJob::ms_aItemsInfo.end())
							pPlayer->GetItem(ItemID).Add(Value);
					}
				}
			}

			GS()->Mmo()->SaveAccount(pPlayer, SaveType::SAVE_STATS);
			GS()->Mmo()->SaveAccount(pPlayer, SaveType::SAVE_UPGRADES);

			SJK.ID("tw_voucher_redeemed", "(voucher_id, user_id, time_created) VALUES (%d, %d, %d)", VoucherID, pPlayer->Acc().m_AccountID, (int)time(0));
			GS()->Chat(ClientID, "You have successfully redeemed the voucher '{STR}'.", pVoucher);
		}

		return;
	}

	GS()->Chat(ClientID, "The voucher code '{STR}' does not exist.", pVoucher);
}