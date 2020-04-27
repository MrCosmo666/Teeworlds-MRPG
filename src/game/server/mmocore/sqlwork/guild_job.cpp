/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#include <engine/shared/config.h>
#include <game/server/gamecontext.h>
#include "guild_job.h"

using namespace sqlstr;
std::map < int , GuildJob::GuildStruct > GuildJob::Guild;
std::map < int , GuildJob::GuildStructHouse > GuildJob::HouseGuild;
std::map < int , GuildJob::GuildStructRank > GuildJob::RankGuild;

void GuildJob::LoadGuildRank(int GuildID)
{
	boost::scoped_ptr<ResultSet> RES(SJK.SD("*", "tw_guilds_ranks", "WHERE ID > '0' AND GuildID = '%d'", GuildID));
	while (RES->next())
	{
		int ID = RES->getInt("ID");
		RankGuild[ID].GuildID = GuildID;
		RankGuild[ID].Access = RES->getInt("Access");
		str_copy(RankGuild[ID].Rank, RES->getString("Name").c_str(), sizeof(RankGuild[ID].Rank));
	}
}

void GuildJob::OnInitGlobal()
{
	boost::scoped_ptr<ResultSet> RES(SJK.SD("*", "tw_guilds"));
	while (RES->next())
	{
		int GuildID = RES->getInt("ID");
		Guild[GuildID].m_OwnerID = RES->getInt("OwnerID");
		Guild[GuildID].m_Level = RES->getInt("Level");
		Guild[GuildID].m_Exp = RES->getInt("Experience");
		Guild[GuildID].m_Bank = RES->getInt("Bank");
		Guild[GuildID].m_Score = RES->getInt("Score");
		str_copy(Guild[GuildID].m_Name, RES->getString("GuildName").c_str(), sizeof(Guild[GuildID].m_Name));

		for (int i = 0; i < EMEMBERUPGRADE::NUM_EMEMBERUPGRADE; i++)
			Guild[GuildID].m_Upgrades[i] = RES->getInt(UpgradeNames(i, true).c_str());

		LoadGuildRank(GuildID);
	}
	Job()->ShowLoadingProgress("Guilds", Guild.size());
}

void GuildJob::OnInitLocal(const char *pLocal) 
{ 
	// загрузка домов
	boost::scoped_ptr<ResultSet> RES(SJK.SD("*", "tw_guilds_houses", pLocal));
	while(RES->next())
	{
		int HouseID = RES->getInt("ID");
		HouseGuild[HouseID].m_DoorX = RES->getInt("DoorX");
		HouseGuild[HouseID].m_DoorY = RES->getInt("DoorY");	
		HouseGuild[HouseID].m_GuildID = RES->getInt("OwnerMID");
		HouseGuild[HouseID].m_PosX = RES->getInt("PosX");
		HouseGuild[HouseID].m_PosY = RES->getInt("PosY");
		HouseGuild[HouseID].m_Price = RES->getInt("Price");
		HouseGuild[HouseID].m_Payment = RES->getInt("Payment");
		HouseGuild[HouseID].m_WorldID = RES->getInt("WorldID");
		HouseGuild[HouseID].m_TextX = RES->getInt("TextX");
		HouseGuild[HouseID].m_TextY = RES->getInt("TextY");
		if (HouseGuild[HouseID].m_GuildID > 0 && !HouseGuild[HouseID].m_Door)
		{
			HouseGuild[HouseID].m_Door = 0;
			HouseGuild[HouseID].m_Door = new GuildDoor(&GS()->m_World, vec2(HouseGuild[HouseID].m_DoorX, HouseGuild[HouseID].m_DoorY), HouseGuild[HouseID].m_GuildID);
		}
	}

	// загрузка декораций
	boost::scoped_ptr<ResultSet> DecoLoadingRES(SJK.SD("*", "tw_guilds_decorations", pLocal));
	while (DecoLoadingRES->next())
	{
		const int DecoID = DecoLoadingRES->getInt("ID");
		m_DecorationHouse[DecoID] = new DecoHouse(&GS()->m_World, vec2(DecoLoadingRES->getInt("X"),
			DecoLoadingRES->getInt("Y")), DecoLoadingRES->getInt("HouseID"), DecoLoadingRES->getInt("DecoID"));
	}
	Job()->ShowLoadingProgress("Guilds Houses", HouseGuild.size());
	Job()->ShowLoadingProgress("Guilds Houses Decorations", m_DecorationHouse.size());
}

bool GuildJob::OnPlayerHandleTile(CCharacter* pChr, int IndexCollision)
{
	CPlayer* pPlayer = pChr->GetPlayer();
	const int ClientID = pPlayer->GetCID();

	if (pChr->GetHelper()->TileEnter(IndexCollision, TILE_GUILD_HOUSE))
	{
		const int HouseID = GetPosHouseID(pChr->m_Core.m_Pos);
		if (HouseID > 0)
		{
			GS()->ResetVotes(ClientID, MAINMENU);
			GS()->Chat(ClientID, "Information load in Vote!");
		}
		pChr->m_Core.m_ProtectHooked = pChr->m_NoAllowDamage = true;
		return true;
	}
	else if (pChr->GetHelper()->TileExit(IndexCollision, TILE_GUILD_HOUSE))
	{
		GS()->ResetVotes(ClientID, MAINMENU);
		pChr->m_Core.m_ProtectHooked = pChr->m_NoAllowDamage = false;
		return true;
	}


	if (pChr->GetHelper()->TileEnter(IndexCollision, TILE_GUILD_CHAIRS))
	{
		pChr->m_Core.m_ProtectHooked = pChr->m_NoAllowDamage = true; 
		return true;
	}
	else if (pChr->GetHelper()->TileExit(IndexCollision, TILE_GUILD_CHAIRS))
	{
		pChr->m_Core.m_ProtectHooked = pChr->m_NoAllowDamage = false;
		return true;
	}
	if (pChr->GetHelper()->BoolIndex(TILE_GUILD_CHAIRS))
	{
		if (GS()->Server()->Tick() % GS()->Server()->TickSpeed() == 0)
		{
			const int HouseID = GetPosHouseID(pChr->m_Core.m_Pos);
			const int GuildID = GetHouseGuildID(HouseID);
			if (HouseID <= 0 || GuildID <= 0) 
				return true;

			const int Exp = GetMemberChairBonus(GuildID, EMEMBERUPGRADE::ChairNSTExperience);
			const int Money = GetMemberChairBonus(GuildID, EMEMBERUPGRADE::ChairNSTMoney);
			pPlayer->AddExp(Exp);
			pPlayer->AddMoney(Money);
		}
		return true;
	}

	return false;
}

/* #########################################################################
	GLOBAL MEMBER
######################################################################### */
// парсинг голосований для меню
bool GuildJob::OnParseVotingMenu(CPlayer* pPlayer, const char* CMD, const int VoteID, const int VoteID2, int Get, const char* GetText)
{
	const int ClientID = pPlayer->GetCID();
	if (PPSTR(CMD, "MLEADER") == 0)
	{
		const int GuildID = pPlayer->Acc().GuildID;
		if (GuildID <= 0 || !IsLeaderPlayer(pPlayer) || Get != 134)
		{
			GS()->Chat(ClientID, "You have no access.");
			return true;
		}

		const int SelectedAccountID = VoteID;
		SJK.UD("tw_guilds", "OwnerID = '%d' WHERE ID = '%d'", SelectedAccountID, GuildID);
		Guild[GuildID].m_OwnerID = SelectedAccountID;

		AddHistoryGuild(GuildID, "New guild leader '%s'.", Job()->PlayerName(SelectedAccountID));
		GS()->ChatGuild(GuildID, "Change leader {STR}->{STR}", GS()->Server()->ClientName(ClientID), Job()->PlayerName(SelectedAccountID));
		GS()->VResetVotes(ClientID, MEMBERMENU);
		return true;
	}

	// переместится домой
	if (PPSTR(CMD, "MSPAWN") == 0)
	{
		const int GuildID = pPlayer->Acc().GuildID;
		const int HouseID = GetGuildHouseID(GuildID);
		if (GuildID <= 0 || HouseID <= 0)
			return true;

		const int WorldID = HouseGuild[HouseID].m_WorldID;
		if (!GS()->IsClientEqualWorldID(ClientID, WorldID))
		{
			vec2 Position = GetPositionHouse(GuildID);
			pPlayer->Acc().TeleportX = Position.x;
			pPlayer->Acc().TeleportY = Position.y;
			GS()->Server()->ChangeWorld(ClientID, WorldID);
			return true;
		}

		vec2 Position = GetPositionHouse(GuildID);
		pPlayer->GetCharacter()->ChangePosition(Position);
		return true;
	}

	// кикнуть игрока из организации
	if (PPSTR(CMD, "MKICK") == 0)
	{
		const int GuildID = pPlayer->Acc().GuildID;
		if (GuildID <= 0 || !IsLeaderPlayer(pPlayer, GuildAccess::ACCESS_INVITE_KICK))
		{
			GS()->Chat(ClientID, "You have no access.");
			return true;
		}

		// todo: close kick for accesses peoples
		ExitGuild(VoteID);
		GS()->VResetVotes(ClientID, MEMBERMENU);
		return true;
	}

	// покупка улучшения стула
	if (PPSTR(CMD, "MUPGRADE") == 0)
	{
		const int GuildID = pPlayer->Acc().GuildID;
		if (GuildID <= 0 || !IsLeaderPlayer(pPlayer, GuildAccess::ACCESS_UPGRADE_HOUSE))
		{
			GS()->Chat(ClientID, "You have no access.");
			return true;
		}

		const int UpgradeID = VoteID;
		if (UpgradeGuild(GuildID, UpgradeID))
		{
			int GuildCount = Guild[GuildID].m_Upgrades[UpgradeID] - 1;
			GS()->Chat(ClientID, "Added ({INT}+1) {STR} to {STR}!", &GuildCount, UpgradeNames(UpgradeID).c_str(), Guild[GuildID].m_Name);
			GS()->VResetVotes(ClientID, MEMBERMENU);
		}
		else 
			GS()->Chat(ClientID, "You don't have that much money in the Bank.");
		return true;
	}

	// добавка денег в банк организации
	if (PPSTR(CMD, "MMONEY") == 0)
	{
		const int GuildID = pPlayer->Acc().GuildID;
		if (GuildID <= 0)
			return true;

		if (Get < 100)
		{
			GS()->Chat(ClientID, "Minimal 100 gold.");
			return true;
		}

		if (pPlayer->CheckFailMoney(Get))
			return true;

		if (AddMoneyBank(GuildID, Get))
		{
			SJK.UD("tw_accounts_data", "GuildDeposit = GuildDeposit + '%d' WHERE ID = '%d'", Get, pPlayer->Acc().AuthID);
			GS()->ChatGuild(GuildID, "{STR} deposit in treasury {INT}gold.", GS()->Server()->ClientName(ClientID), &Get);
			AddHistoryGuild(GuildID, "'%s' added to bank %dgold.", GS()->Server()->ClientName(ClientID), Get);
			GS()->VResetVotes(ClientID, MEMBERMENU);
		}
		return true;
	}

	// покупка дома
	if (PPSTR(CMD, "BUYMEMBERHOUSE") == 0)
	{
		const int GuildID = pPlayer->Acc().GuildID;
		if (GuildID <= 0 || !IsLeaderPlayer(pPlayer))
		{
			GS()->Chat(ClientID, "You have no access.");
			return true;
		}

		BuyGuildHouse(GuildID, VoteID);
		GS()->VResetVotes(ClientID, MAINMENU);
	}

	// продажа дома
	if (PPSTR(CMD, "MHOUSESELL") == 0)
	{
		const int GuildID = pPlayer->Acc().GuildID;
		if (GuildID <= 0 || !IsLeaderPlayer(pPlayer) || Get != 777)
		{
			GS()->Chat(ClientID, "You have no access.");
			return true;
		}

		SellGuildHouse(GuildID);
		GS()->VResetVotes(ClientID, MEMBERMENU);
		return true;
	}

	// дверь дома организации
	if (PPSTR(CMD, "MDOOR") == 0)
	{
		const int GuildID = pPlayer->Acc().GuildID;
		if (GuildID <= 0 || !IsLeaderPlayer(pPlayer, GuildAccess::ACCESS_UPGRADE_HOUSE))
		{
			GS()->Chat(ClientID, "You have no access.");
			return true;
		}

		ChangeStateDoor(GuildID);
		GS()->VResetVotes(ClientID, MEMBERMENU);
		return true;
	}

	// принять приглошение
	if (PPSTR(CMD, "MINVITEACCEPT") == 0)
	{
		const int GuildID = pPlayer->Acc().GuildID;
		if (GuildID <= 0 || !IsLeaderPlayer(pPlayer, GuildAccess::ACCESS_INVITE_KICK))
		{
			GS()->Chat(ClientID, "You have no access.");
			return true;
		}

		const int SenderID = VoteID;
		SJK.DD("tw_guilds_invites", "WHERE GuildID = '%d' AND OwnerID = '%d'", GuildID, SenderID);
		Job()->Inbox()->SendInbox(SenderID, Guild[GuildID].m_Name, "You were accepted to join guild");
		JoinGuild(SenderID, GuildID);
		GS()->ResetVotes(ClientID, MEMBERMENU);
		return true;
	}

	// отклонить приглашение
	if (PPSTR(CMD, "MINVITEREJECT") == 0)
	{
		const int GuildID = pPlayer->Acc().GuildID;
		if (GuildID <= 0 || !IsLeaderPlayer(pPlayer, GuildAccess::ACCESS_INVITE_KICK))
		{
			GS()->Chat(ClientID, "You have no access.");
			return true;
		}

		const int SenderID = VoteID;
		GS()->Chat(ClientID, "You reject invite.");
		SJK.DD("tw_guilds_invites", "WHERE GuildID = '%d' AND OwnerID = '%d'", GuildID, SenderID);
		Job()->Inbox()->SendInbox(SenderID, Guild[GuildID].m_Name, "You were denied join guild");
		GS()->ResetVotes(ClientID, MEMBERMENU);
		return true;
	}

	// - - - - - - -
	// Поиск гильдии
	if (PPSTR(CMD, "MINVITENAME") == 0)
	{
		if (PPSTR(GetText, "NULL") == 0)
		{
			GS()->Chat(ClientID, "Use please another name.");
			return true;
		}

		str_copy(CGS::InteractiveSub[ClientID].GuildName, GetText, sizeof(CGS::InteractiveSub[ClientID].GuildName));
		GS()->VResetVotes(ClientID, MEMBERMENU);
		return true;
	}

	// создать новое приглашение в гильдию
	if (PPSTR(CMD, "MINVITESEND") == 0)
	{
		if (!AddInviteGuild(VoteID, pPlayer->Acc().AuthID))
		{
			GS()->Chat(ClientID, "You have already sent the invitation.");
			return true;
		}

		GS()->Chat(ClientID, "You sent the invitation to join.");
		return true;
	}

	// - - - - - - РАНГИ - - - - 
	// изменить поле ранга имени
	if (PPSTR(CMD, "MRANKNAME") == 0)
	{
		if (PPSTR(GetText, "NULL") == 0)
		{
			GS()->Chat(ClientID, "Use please another name.");
			return true;
		}

		str_copy(CGS::InteractiveSub[ClientID].RankName, GetText, sizeof(CGS::InteractiveSub[ClientID].RankName));
		GS()->VResetVotes(ClientID, GUILDRANK);
		return true;
	}

	// создать ранг
	if (PPSTR(CMD, "MRANKCREATE") == 0)
	{
		const int GuildID = pPlayer->Acc().GuildID;
		if (GuildID <= 0 || !IsLeaderPlayer(pPlayer))
		{
			GS()->Chat(ClientID, "You have no access.");
			return true;
		}

		if (str_length(CGS::InteractiveSub[ClientID].RankName) < 2)
		{
			GS()->Chat(ClientID, "Minimal symbols 2.");
			return true;
		}

		AddRank(GuildID, CGS::InteractiveSub[ClientID].RankName);
		GS()->ClearInteractiveSub(ClientID);
		GS()->VResetVotes(ClientID, GUILDRANK);
		return true;
	}

	// удалить ранг
	if (PPSTR(CMD, "MRANKDELETE") == 0)
	{
		const int GuildID = pPlayer->Acc().GuildID;
		if (GuildID <= 0 || !IsLeaderPlayer(pPlayer))
		{
			GS()->Chat(ClientID, "You have no access.");
			return true;
		}

		const int RankID = VoteID;
		DeleteRank(RankID, GuildID);
		GS()->VResetVotes(ClientID, GUILDRANK);
		return true;
	}

	// изменить доступ рангу
	if (PPSTR(CMD, "MRANKACCESS") == 0)
	{
		const int GuildID = pPlayer->Acc().GuildID;
		if (GuildID <= 0 || !IsLeaderPlayer(pPlayer))
		{
			GS()->Chat(ClientID, "You have no access.");
			return true;
		}

		const int RankID = VoteID;
		ChangeRankAccess(RankID);
		GS()->VResetVotes(ClientID, GUILDRANK);
		return true;
	}

	// установить ранг
	if (PPSTR(CMD, "MRANKSET") == 0)
	{
		const int GuildID = pPlayer->Acc().GuildID;
		if (GuildID <= 0 || !IsLeaderPlayer(pPlayer))
		{
			GS()->Chat(ClientID, "You have no access.");
			return true;
		}

		if (str_length(CGS::InteractiveSub[ClientID].RankName) < 2)
		{
			GS()->Chat(ClientID, "Minimal symbols 2.");
			return true;
		}

		const int RankID = VoteID;
		ChangeRank(RankID, GuildID, CGS::InteractiveSub[ClientID].RankName);
		GS()->ClearInteractiveSub(ClientID);
		GS()->VResetVotes(ClientID, GUILDRANK);
		return true;
	}

	// изменить имя ранга
	if (PPSTR(CMD, "MRANKCHANGE") == 0)
	{
		const int GuildID = pPlayer->Acc().GuildID;
		if (GuildID <= 0 || !IsLeaderPlayer(pPlayer))
		{
			GS()->Chat(ClientID, "You have no access.");
			return true;
		}

		// меняем ранг и очишаем меню интерактивов
		ChangePlayerRank(VoteID, VoteID2);
		GS()->ClearInteractiveSub(ClientID);
		GS()->VResetVotes(ClientID, MEMBERMENU);
		return true;
	}


	// начала расстановки декорации
	if (PPSTR(CMD, "DECOGUILDSTART") == 0)
	{
		const int GuildID = pPlayer->Acc().GuildID;
		const int HouseID = GetGuildHouseID(GuildID);
		if (GuildID <= 0 || HouseID <= 0 || !IsLeaderPlayer(pPlayer, GuildAccess::ACCESS_UPGRADE_HOUSE))
		{
			GS()->Chat(ClientID, "You have no access.");
			return true;
		}

		vec2 PositionHouse = GetPositionHouse(GuildID);
		if (distance(PositionHouse, pPlayer->GetCharacter()->m_Core.m_Pos) > 600)
		{
			GS()->Chat(ClientID, "Maximum distance between your home 600!");
			return true;
		}

		GS()->ClearVotes(ClientID);
		GS()->AV(ClientID, "null", "Please close vote and press Left Mouse,");
		GS()->AV(ClientID, "null", "on position where add decoration!");
		GS()->AddBack(ClientID);

		const int DecoItemID = VoteID;
		CGS::InteractiveSub[ClientID].TempID = DecoItemID;
		CGS::InteractiveSub[ClientID].TempID2 = DECOTYPE_GUILD_HOUSE;
		pPlayer->m_LastVoteMenu = INVENTORY;
		return true;
	}

	if (PPSTR(CMD, "DECOGUILDDELETE") == 0)
	{
		const int GuildID = pPlayer->Acc().GuildID;
		const int HouseID = GetGuildHouseID(GuildID);
		if (GuildID <= 0 || HouseID <= 0 || !IsLeaderPlayer(pPlayer, GuildAccess::ACCESS_UPGRADE_HOUSE))
		{
			GS()->Chat(ClientID, "You have no access.");
			return true;
		}

		const int DecoID = VoteID;
		const int DecoItemID = VoteID2;
		if (DeleteDecorationHouse(DecoID))
		{
			ItemSql::ItemPlayer& PlDecoItem = pPlayer->GetItem(DecoItemID);
			GS()->Chat(ClientID, "You back to the backpack {STR}!", PlDecoItem.Info().GetName(pPlayer));
			PlDecoItem.Add(1);
		}
		GS()->VResetVotes(ClientID, HOUSEGUILDDECORATION);
		return true;
	}
	return false;
}

bool GuildJob::OnPlayerHandleMainMenu(CPlayer* pPlayer, int Menulist, bool ReplaceMenu)
{
	int ClientID = pPlayer->GetCID();
	if (ReplaceMenu)
	{
		return false;
	}

	if (Menulist == MEMBERMENU)
	{
		pPlayer->m_LastVoteMenu = MAINMENU;
		ShowMenuGuild(pPlayer);
		return true;
	}

	if (Menulist == MEMBERHISTORY)
	{
		pPlayer->m_LastVoteMenu = MEMBERMENU;
		ShowHistoryGuild(ClientID, pPlayer->Acc().GuildID);
		return true;
	}

	if (Menulist == GUILDRANK)
	{
		pPlayer->m_LastVoteMenu = MEMBERMENU;
		ShowMenuRank(pPlayer);
		return true;
	}

	if (Menulist == MEMBERINVITES)
	{
		pPlayer->m_LastVoteMenu = MEMBERMENU;
		ShowInvitesGuilds(ClientID, pPlayer->Acc().GuildID);
		return true;
	}

	if (Menulist == HOUSEGUILDDECORATION)
	{
		pPlayer->m_LastVoteMenu = HOUSEMENU;
		GS()->AVH(ClientID, HDECORATION, GREEN_COLOR, "Decorations Information");
		GS()->AVM(ClientID, "null", NOPE, HDECORATION, "Add: Select your item in list. Select (Add to house),");
		GS()->AVM(ClientID, "null", NOPE, HDECORATION, "later press (ESC) and mouse select position");
		GS()->AVM(ClientID, "null", NOPE, HDECORATION, "Return in inventory: Select down your decorations");
		GS()->AVM(ClientID, "null", NOPE, HDECORATION, "and press (Back to inventory).");

		Job()->Item()->ListInventory(pPlayer, ItemType::TYPE_DECORATION);
		GS()->AV(ClientID, "null", "");
		ShowDecorationList(pPlayer);
		GS()->AddBack(ClientID);
		return true;
	}
	return false;
}

/* #########################################################################
	BASED GUILDS
######################################################################### */
void GuildJob::OnTick()
{
	TickHousingText();
}

void GuildJob::TickHousingText()
{
	if (GS()->Server()->Tick() % (GS()->Server()->TickSpeed()) != 0)
		return;

	for (const auto& mh : HouseGuild)
	{
		if (mh.second.m_WorldID != GS()->GetWorldID())
			continue;

		const int LifeTime = GS()->Server()->TickSpeed() - 5;
		const int GuildID = mh.second.m_GuildID;
		if (GuildID > 0)
		{
			GS()->CreateText(NULL, false, vec2(mh.second.m_TextX, mh.second.m_TextY), vec2(0, 0), LifeTime, Guild[GuildID].m_Name, mh.second.m_WorldID);
			continue;
		}
		GS()->CreateText(NULL, false, vec2(mh.second.m_TextX, mh.second.m_TextY), vec2(0, 0), LifeTime, "FREE", mh.second.m_WorldID);
	}
}

void GuildJob::OnPaymentTime()
{
	boost::scoped_ptr<ResultSet> RES(SJK.SD("ID, Bank", "tw_guilds"));
	while(RES->next())
	{
		const int MID = RES->getInt("ID");
		const int HouseID = GetGuildHouseID(MID);
		if(HouseID > 0 && Guild.find(MID) != Guild.end() && HouseGuild.find(HouseID) != HouseGuild.end())
		{
			if(Guild[MID].m_Bank < HouseGuild[ HouseID ].m_Payment)
			{
				GS()->Chat(-1, "Guild {STR} lost house, nope payment!", Guild[MID].m_Name);
				SellGuildHouse(MID);
				continue;
			}
			
			Guild[MID].m_Bank -= HouseGuild[ HouseID ].m_Payment;
			SJK.UD("tw_guilds", "Bank = '%d' WHERE ID = '%d'", Guild[MID].m_Bank, MID);
			GS()->ChatGuild(MID, "Payment {INT} gold was successful {INT}", &HouseGuild[HouseID].m_Payment, &Guild[MID].m_Bank);
		}
	}
}

/* #########################################################################
	GET CHECK MEMBER 
######################################################################### */
const char *GuildJob::GuildName(int GuildID) const
{	
	if(Guild.find(GuildID) != Guild.end())
		return Guild[GuildID].m_Name;
	return "None"; 
}

bool GuildJob::IsLeaderPlayer(CPlayer *pPlayer, int Access) const
{
	const int GuildID = pPlayer->Acc().GuildID;
	if(GuildID > 0 && Guild.find(GuildID) != Guild.end() &&
		(Guild[GuildID].m_OwnerID == pPlayer->Acc().AuthID ||
			(RankGuild.find(pPlayer->Acc().GuildRank) != RankGuild.end() &&
				(RankGuild[pPlayer->Acc().GuildRank].Access == Access || RankGuild[pPlayer->Acc().GuildRank].Access == GuildAccess::ACCESS_FULL))))
		return true;
	return false;
}

int GuildJob::GetMemberChairBonus(int GuildID, int Field) const
{
	if(GuildID > 0 && Guild.find(GuildID) != Guild.end())
		return Guild[GuildID].m_Upgrades[Field];
	return -1;
}

std::string GuildJob::UpgradeNames(int Field, bool DataTable)
{
	if(Field >= 0 && Field < EMEMBERUPGRADE::NUM_EMEMBERUPGRADE)  
	{
		std::string stratt;
		stratt = str_EMEMBERUPGRADE((EMEMBERUPGRADE) Field);
		if(DataTable) stratt.erase(stratt.find("NST"), 3);
		else stratt.replace(stratt.find("NST"), 3, " ");  
		return stratt;
	}
	return "Error";
}

int GuildJob::ExpForLevel(int Level) { return (g_Config.m_SvGuildLeveling+Level*2)*(Level*Level); } 


/* #########################################################################
	FUNCTIONS HOUSES DECORATION
######################################################################### */
bool GuildJob::AddDecorationHouse(int DecoID, int GuildID, vec2 Position)
{
	if (Guild.find(GuildID) == Guild.end())
		return false;

	vec2 PositionHouse = GetPositionHouse(GuildID);
	if (distance(PositionHouse, Position) > 600)
		return false;

	int HouseID = GetGuildHouseID(GuildID);
	boost::scoped_ptr<ResultSet> RES(SJK.SD("ID", "tw_guilds_decorations", "WHERE HouseID = '%d'", HouseID));
	if ((int)RES->rowsCount() >= g_Config.m_SvLimitDecoration) 
		return false;

	boost::scoped_ptr<ResultSet> RES2(SJK.SD("ID", "tw_guilds_decorations", "ORDER BY ID DESC LIMIT 1"));
	int InitID = (RES2->next() ? RES2->getInt("ID") + 1 : 1);
	SJK.ID("tw_guilds_decorations", "(ID, DecoID, HouseID, X, Y, WorldID) VALUES ('%d', '%d', '%d', '%d', '%d', '%d')",
		InitID, DecoID, HouseID, (int)Position.x, (int)Position.y, GS()->GetWorldID());
	m_DecorationHouse[InitID] = new DecoHouse(&GS()->m_World, Position, HouseID, DecoID);
	return true;
}

bool GuildJob::DeleteDecorationHouse(int ID)
{
	if (m_DecorationHouse.find(ID) == m_DecorationHouse.end())
		return false;

	if (m_DecorationHouse.at(ID))
	{
		delete m_DecorationHouse.at(ID);
		m_DecorationHouse.at(ID) = 0;
	}
	m_DecorationHouse.erase(ID);
	SJK.DD("tw_guilds_decorations", "WHERE ID = '%d'", ID);
	return true;
}

void GuildJob::ShowDecorationList(CPlayer* pPlayer)
{
	int ClientID = pPlayer->GetCID();
	int GuildID = pPlayer->Acc().GuildID;
	int HouseID = GetGuildHouseID(GuildID);
	for (auto deco = m_DecorationHouse.begin(); deco != m_DecorationHouse.end(); deco++)
	{
		if (deco->second && deco->second->m_HouseID == HouseID)
		{
			GS()->AVD(ClientID, "DECOGUILDDELETE", deco->first, deco->second->m_DecoID, 1, "{STR}:{INT} back to the inventory",
				GS()->GetItemInfo(deco->second->m_DecoID).GetName(pPlayer), &deco->first);
		}
	}
}

/* #########################################################################
	FUNCTIONS MEMBER MEMBER 
######################################################################### */
void GuildJob::CreateGuild(int ClientID, const char *GuildName)
{
	CPlayer *pPlayer = GS()->GetPlayer(ClientID, true);
	if (!pPlayer)
		return;

	// проверяем находимся ли уже в организации
	if(pPlayer->Acc().GuildID > 0) 
		return GS()->Chat(ClientID, "You already in guild group!");

	// проверяем доступность имени организации
	CSqlString<64> cGuildName = CSqlString<64>(GuildName);
	boost::scoped_ptr<ResultSet> RES(SJK.SD("ID", "tw_guilds", "WHERE GuildName = '%s'", cGuildName.cstr()));
	if(RES->next()) 
		return GS()->Chat(ClientID, "This guild name already useds!");
	
	// проверяем тикет забераем и создаем
	if(pPlayer->GetItem(itTicketGuild).Count > 0 && pPlayer->GetItem(itTicketGuild).Remove(1))
	{
		// получаем Айди для инициализации
		boost::scoped_ptr<ResultSet> RES2(SJK.SD("ID", "tw_guilds", "ORDER BY ID DESC LIMIT 1"));
		int InitID = RES2->next() ? RES2->getInt("ID")+1 : 1; // TODO: thread save ? hm need for table all time auto increment = 1; NEED FIX IT
		
		// инициализируем гильдию
		Guild[InitID].m_OwnerID = pPlayer->Acc().AuthID;
		Guild[InitID].m_Level = 1;
		Guild[InitID].m_Exp = 0;
		Guild[InitID].m_Bank = 0;
		Guild[InitID].m_Score = 0;
		str_copy(Guild[InitID].m_Name, cGuildName.cstr(), sizeof(Guild[InitID].m_Name));
		Guild[InitID].m_Upgrades[EMEMBERUPGRADE::AvailableNSTSlots] = 2;
		Guild[InitID].m_Upgrades[EMEMBERUPGRADE::ChairNSTExperience] = 0;
		Guild[InitID].m_Upgrades[EMEMBERUPGRADE::ChairNSTMoney] = 0;
		pPlayer->Acc().GuildID = InitID;

		// создаем в таблице гильдию
		SJK.ID("tw_guilds", "(ID, GuildName, OwnerID) VALUES ('%d', '%s', '%d')", InitID, cGuildName.cstr(), pPlayer->Acc().AuthID);
		SJK.UD("tw_accounts_data", "GuildID = '%d' WHERE ID = '%d'", InitID, pPlayer->Acc().AuthID);
		GS()->Chat(-1, "New guilds [{STR}] have been created!", cGuildName.cstr());
	}
	else 
		GS()->Chat(ClientID, "You need first buy guild ticket on shop!");
}

void GuildJob::JoinGuild(int AuthID, int GuildID)
{
	// проверяем клан есть или нет у этого и грока
	const char *PlayerName = Job()->PlayerName(AuthID);
	boost::scoped_ptr<ResultSet> CheckJoinRES(SJK.SD("ID", "tw_accounts_data", "WHERE ID = '%d' AND GuildID > '0'", AuthID));
	if(CheckJoinRES->next())
	{
		GS()->ChatAccountID(AuthID, "You already in guild group!");
		return GS()->ChatGuild(GuildID, "{STR} already joined your or another guilds", PlayerName);
	}

	// проверяем количество слотов доступных
	boost::scoped_ptr<ResultSet> CheckSlotsRES(SJK.SD("ID", "tw_accounts_data", "WHERE GuildID = '%d'", GuildID));
	if((int)CheckSlotsRES->rowsCount() >= Guild[GuildID].m_Upgrades[EMEMBERUPGRADE::AvailableNSTSlots])
	{
		GS()->ChatAccountID(AuthID, "You don't joined [No slots for join]");
		return GS()->ChatGuild(GuildID, "{STR} don't joined [No slots for join]", PlayerName);
	}

	// обновляем и получаем данные
	int ClientID = Job()->Account()->CheckOnlineAccount(AuthID);
	if(ClientID >= 0 && ClientID < MAX_PLAYERS)
	{
		AccountMainSql::Data[ClientID].GuildID = GuildID;
		AccountMainSql::Data[ClientID].GuildRank = 0;
		GS()->ResetVotes(ClientID, MAINMENU);
	}
	SJK.UD("tw_accounts_data", "GuildID = '%d', GuildRank = NULL WHERE ID = '%d'", GuildID, AuthID);
	GS()->ChatGuild(GuildID, "Player {STR} join in your guild!", PlayerName);
}

void GuildJob::ExitGuild(int AuthID)
{
	// проверяем если покидает лидер клан
	boost::scoped_ptr<ResultSet> RES(SJK.SD("ID", "tw_guilds", "WHERE OwnerID = '%d'", AuthID));
	if (RES->next())
		return GS()->ChatAccountID(AuthID, "A leader cannot leave his guild group!");

	// проверяем аккаунт и его организацию и устанавливаем
	boost::scoped_ptr<ResultSet> RES2(SJK.SD("GuildID", "tw_accounts_data", "WHERE ID = '%d'", AuthID));
	if (RES2->next())
	{
		// пишим гильдии что игрок покинул гильдию
		const int GuildID = RES2->getInt("GuildID");
		GS()->ChatGuild(GuildID, "{STR} left the Guild!", Job()->PlayerName(AuthID));
		AddHistoryGuild(GuildID, "'%s' exit or kicked.", Job()->PlayerName(AuthID));

		// обновляем информацию игроку
		int ClientID = Job()->Account()->CheckOnlineAccount(AuthID);
		if(ClientID >= 0 && ClientID < MAX_PLAYERS)
		{
			GS()->ResetVotes(ClientID, MAINMENU);
			AccountMainSql::Data[ClientID].GuildID = 0;
		}
		SJK.UD("tw_accounts_data", "GuildID = NULL, GuildRank = NULL, GuildDeposit = '0' WHERE ID = '%d'", AuthID);
	}
}

void GuildJob::ShowMenuGuild(CPlayer *pPlayer)
{
	// если не нашли гильдии такой то показывает 'Поиск гильдий'
	const int ClientID = pPlayer->GetCID();
	const int GuildID = pPlayer->Acc().GuildID;
	if(Guild.find(GuildID) == Guild.end()) 
		return ShowFinderGuilds(ClientID);
	
	// показываем само меню
	int MemberHouse = GetGuildHouseID(GuildID);
	int ExpNeed = ExpForLevel(Guild[GuildID].m_Level);
	GS()->AVH(ClientID, HMEMBERSTATS, BLUE_COLOR, "Guild name: {STR}", Guild[GuildID].m_Name);
	GS()->AVM(ClientID, "null", NOPE, HMEMBERSTATS, "Level: {INT} Experience: {INT}/{INT}", &Guild[GuildID].m_Level, &Guild[GuildID].m_Exp, &ExpNeed);
	GS()->AVM(ClientID, "null", NOPE, HMEMBERSTATS, "Maximal available player count: {INT}", &Guild[GuildID].m_Upgrades[EMEMBERUPGRADE::AvailableNSTSlots]);
	GS()->AVM(ClientID, "null", NOPE, HMEMBERSTATS, "Leader: {STR}", Job()->PlayerName(Guild[GuildID].m_OwnerID));
	GS()->AVM(ClientID, "null", NOPE, HMEMBERSTATS, "- - - - - - - - - -");
	GS()->AVM(ClientID, "null", NOPE, HMEMBERSTATS, "/ginvite <id> - to invite a player into members (for leader)");
	GS()->AVM(ClientID, "null", NOPE, HMEMBERSTATS, "/gexit - leave of guild group (for all members)");
	GS()->AVM(ClientID, "null", NOPE, HMEMBERSTATS, "- - - - - - - - - -");
	GS()->AVM(ClientID, "null", NOPE, HMEMBERSTATS, "Guild Bank: {INT}gold", &Guild[GuildID].m_Bank);

	GS()->AV(ClientID, "null", "");
	pPlayer->m_Colored = GOLDEN_COLOR;
	GS()->AVL(ClientID, "null", "₪ Players list on guild", &pPlayer->GetItem(itMoney).Count);
	ShowGuildPlayers(pPlayer);
	GS()->AV(ClientID, "null", "");

	pPlayer->m_Colored = LIGHT_GRAY_COLOR;
	GS()->AVL(ClientID, "null", "◍ Your money: {INT}gold", &pPlayer->GetItem(itMoney).Count);
	pPlayer->m_Colored = SMALL_LIGHT_GRAY_COLOR;
	GS()->AVL(ClientID, "MMONEY", "Add money guild bank. (Amount in a reason)", Guild[GuildID].m_Name);

	GS()->AV(ClientID, "null", "");
	pPlayer->m_Colored = LIGHT_GRAY_COLOR;
	GS()->AVL(ClientID, "null", "▤ Guild system", &pPlayer->GetItem(itMoney).Count);
	pPlayer->m_Colored = SMALL_LIGHT_GRAY_COLOR;
	GS()->AVM(ClientID, "MENU", GUILDRANK, NOPE, "Settings guild Rank(s)");
	GS()->AVM(ClientID, "MENU", MEMBERINVITES, NOPE, "Invites to your guild");
	GS()->AVM(ClientID, "MENU", MEMBERHISTORY, NOPE, "History of activity");

	if (MemberHouse > 0)
	{
		GS()->AV(ClientID, "null", "");
		pPlayer->m_Colored = LIGHT_GRAY_COLOR;
		GS()->AVL(ClientID, "null", "⌂ Housing system", &pPlayer->GetItem(itMoney).Count);
		pPlayer->m_Colored = SMALL_LIGHT_GRAY_COLOR;
		GS()->AVM(ClientID, "MENU", HOUSEGUILDDECORATION, NOPE, "Settings Decoration(s)");
		GS()->AVL(ClientID, "MDOOR", "Change state (\"{STR} door\")", GetGuildDoor(GuildID) ? "Open" : "Close");
		GS()->AVL(ClientID, "MSPAWN", "Teleport to guild house");
		GS()->AVL(ClientID, "MHOUSESELL", "Sell your guild house (in reason 777)");
	}


	GS()->AV(ClientID, "null", "");
	pPlayer->m_Colored = LIGHT_GRAY_COLOR;
	GS()->AVL(ClientID, "null", "☆ Guild upgrades", &pPlayer->GetItem(itMoney).Count);
	pPlayer->m_Colored = SMALL_LIGHT_GRAY_COLOR;
	if (MemberHouse > 0)
	{
		for(int i = EMEMBERUPGRADE::ChairNSTExperience ; i < EMEMBERUPGRADE::NUM_EMEMBERUPGRADE; i++)
		{
			int PriceUpgrade = Guild[ GuildID ].m_Upgrades[ i ] * g_Config.m_SvPriceUpgradeGuildAnother;
			GS()->AVM(ClientID, "MUPGRADE", i, NOPE, "Upgrade {STR} ({INT}) {INT}gold", UpgradeNames(i).c_str(), &Guild[GuildID].m_Upgrades[i], &PriceUpgrade);
		}
	}
	int PriceUpgrade = Guild[ GuildID ].m_Upgrades[ EMEMBERUPGRADE::AvailableNSTSlots ] * g_Config.m_SvPriceUpgradeGuildSlot;
	GS()->AVM(ClientID, "MUPGRADE", EMEMBERUPGRADE::AvailableNSTSlots, NOPE, "Upgrade {STR} ({INT}) {INT}gold", 
		UpgradeNames(EMEMBERUPGRADE::AvailableNSTSlots).c_str(), &Guild[GuildID].m_Upgrades[ EMEMBERUPGRADE::AvailableNSTSlots ], &PriceUpgrade);
	GS()->AddBack(ClientID);
	return;
}

void GuildJob::ShowGuildPlayers(CPlayer* pPlayer)
{
	const int ClientID = pPlayer->GetCID();
	const int GuildID = pPlayer->Acc().GuildID;
	int HideID = NUMHIDEMENU + ItemSql::ItemsInfo.size() + 1000;
	boost::scoped_ptr<ResultSet> RES(SJK.SD("ID, Nick, GuildRank, GuildDeposit", "tw_accounts_data", "WHERE GuildID = '%d'", GuildID));
	while (RES->next())
	{
		const int AuthID = RES->getInt("ID");
		const int RankID = RES->getInt("GuildRank");
		const int Deposit = RES->getInt("GuildDeposit");
		GS()->AVH(ClientID, HideID, LIGHT_GOLDEN_COLOR, "{STR} {STR} Deposit: {INT}", GetGuildRank(GuildID, RankID), RES->getString("Nick").c_str(), &Deposit);

		for (auto mr : RankGuild)
		{
			if (GuildID != mr.second.GuildID || RankID == mr.first)
				continue;

			GS()->AVD(ClientID, "MRANKCHANGE", AuthID, mr.first, HideID, "Change Rank to: {STR}{STR}", mr.second.Rank, mr.second.Access > 0 ? "*" : "");
		}
		GS()->AVM(ClientID, "MKICK", AuthID, HideID, "Kick");
		if (AuthID != pPlayer->Acc().AuthID) 
			GS()->AVM(ClientID, "MLEADER", AuthID, HideID, "Give Leader (in reason 134)");
		HideID++;
	}

}

void GuildJob::AddExperience(int GuildID)
{
	bool UpdateTable = false;
	Guild[GuildID].m_Exp += 1;
	for( ; Guild[GuildID].m_Exp >= ExpForLevel(Guild[GuildID].m_Level); )
	{
		Guild[GuildID].m_Exp -= ExpForLevel(Guild[GuildID].m_Level), Guild[GuildID].m_Level++;
		GS()->Chat(-1, "Guild {STR} raised the level up to {INT}", Guild[GuildID].m_Name, &Guild[GuildID].m_Level);
		GS()->ChatDiscord(false, DC_SERVER_INFO, "Information", "Guild {STR} raised the level up to {INT}", Guild[GuildID].m_Name, &Guild[GuildID].m_Level);
		AddHistoryGuild(GuildID, "Guild raised level to '%d'.", Guild[GuildID].m_Level);

		// если это последний уровень повышения
		if(Guild[GuildID].m_Exp < ExpForLevel(Guild[GuildID].m_Level))
			UpdateTable = true;
	}
	if(rand()%10 == 2 || UpdateTable)
	{
		SJK.UD("tw_guilds", "Level = '%d', Experience = '%d' WHERE ID = '%d'", Guild[GuildID].m_Level, Guild[GuildID].m_Exp, GuildID);
	}
}

bool GuildJob::AddMoneyBank(int GuildID, int Money)
{
	boost::scoped_ptr<ResultSet> RES(SJK.SD("ID, Bank", "tw_guilds", "WHERE ID = '%d'", GuildID));
	if(!RES->next()) 
		return false;
	
	// добавить деньги
	Guild[GuildID].m_Bank = RES->getInt("Bank") + Money;
	SJK.UD("tw_guilds", "Bank = '%d' WHERE ID = '%d'", Guild[GuildID].m_Bank, GuildID);
	return true;
}

bool GuildJob::RemoveMoneyBank(int GuildID, int Money)
{
	boost::scoped_ptr<ResultSet> RES(SJK.SD("ID, Bank", "tw_guilds", "WHERE ID = '%d'", GuildID));
	if(!RES->next()) 
		return false;
	
	// проверяем хватает ли в банке на оплату
	Guild[GuildID].m_Bank = RES->getInt("Bank");
	if(Money > Guild[GuildID].m_Bank)
		return false;

	// оплата
	Guild[GuildID].m_Bank -= Money;
	SJK.UD("tw_guilds", "Bank = '%d' WHERE ID = '%d'", Guild[GuildID].m_Bank, GuildID);
	return true;
}

// покупка улучшения максимальное количество слотов
bool GuildJob::UpgradeGuild(int GuildID, int Field)
{
	boost::scoped_ptr<ResultSet> RES(SJK.SD("*", "tw_guilds", "WHERE ID = '%d'", GuildID));
	if(RES->next())
	{
		Guild[ GuildID ].m_Bank = RES->getInt("Bank");
		Guild[ GuildID ].m_Upgrades[ Field ] = RES->getInt(UpgradeNames(Field, true).c_str());

		int UpgradePrice = g_Config.m_SvPriceUpgradeGuildAnother;
		if(Field == EMEMBERUPGRADE::AvailableNSTSlots) UpgradePrice = g_Config.m_SvPriceUpgradeGuildSlot;

		int PriceAvailable = Guild[ GuildID ].m_Upgrades[ Field ]*UpgradePrice;
		if(PriceAvailable > Guild[ GuildID ].m_Bank)
			return false;

		Guild[ GuildID ].m_Upgrades[ Field ]++;
		Guild[ GuildID ].m_Bank -= PriceAvailable;
		SJK.UD("tw_guilds", "Bank = '%d', %s = '%d' WHERE ID = '%d'", 
			Guild[GuildID].m_Bank, UpgradeNames(Field, true).c_str(), Guild[GuildID].m_Upgrades[ Field ], GuildID);

		AddHistoryGuild(GuildID, "'%s' level up to '%d'.", UpgradeNames(Field).c_str(), Guild[GuildID].m_Upgrades[Field]);
		return true;
	}
	return false;
}

/* #########################################################################
	GET CHECK MEMBER RANK MEMBER 
######################################################################### */
// именна доступов
const char *GuildJob::AccessNames(int Access)
{
	switch(Access) 
	{
		default: return "No Access";
		case GuildAccess::ACCESS_INVITE_KICK: return "Access Invite Kick";
		case GuildAccess::ACCESS_UPGRADE_HOUSE: return "Access Upgrades & House";
		case GuildAccess::ACCESS_FULL: return "Full Access";
	}
}

// получить имя ранга
const char *GuildJob::GetGuildRank(int GuildID, int RankID)
{
	if(RankGuild.find(RankID) != RankGuild.end() && GuildID == RankGuild[RankID].GuildID)
		return RankGuild[RankID].Rank;
	return "Member";
}

// найти ранг по имени и организации
int GuildJob::FindGuildRank(int GuildID, const char *Rank) const
{
	for(auto mr: RankGuild)
	{
		if(GuildID == mr.second.GuildID && str_comp(Rank, mr.second.Rank) == 0)
			return mr.first;
	}
	return -1;
}

/* #########################################################################
	FUNCTIONS MEMBER RANK MEMBER 
######################################################################### */
// добавить ранг
void GuildJob::AddRank(int GuildID, const char *Rank)
{
	int FindRank = FindGuildRank(GuildID, Rank);
	if(RankGuild.find(FindRank) != RankGuild.end())
		return GS()->ChatGuild(GuildID, "Found this rank in your table, change name");

	boost::scoped_ptr<ResultSet> RES(SJK.SD("ID", "tw_guilds_ranks", "WHERE GuildID = '%d'", GuildID));
	if(RES->rowsCount() >= 5) return;

	boost::scoped_ptr<ResultSet> RES2(SJK.SD("ID", "tw_guilds_ranks", "ORDER BY ID DESC LIMIT 1"));
	int InitID = RES2->next() ? RES2->getInt("ID")+1 : 1; 
	// thread save ? hm need for table all time auto increment = 1; NEED FIX IT

	CSqlString<64> cGuildRank = CSqlString<64>(Rank);
	SJK.ID("tw_guilds_ranks", "(ID, GuildID, Name) VALUES ('%d', '%d', '%s')", InitID, GuildID, cGuildRank.cstr());
	GS()->ChatGuild(GuildID, "Creates new rank [{STR}]!", Rank);
	AddHistoryGuild(GuildID, "Added new rank '%s'.", Rank);

	RankGuild[InitID].GuildID = GuildID;
	str_copy(RankGuild[InitID].Rank, Rank, sizeof(RankGuild[InitID].Rank));
}

// удалить ранг
void GuildJob::DeleteRank(int RankID, int GuildID)
{
	if(RankGuild.find(RankID) != RankGuild.end())
	{
		SJK.UD("tw_accounts_data", "GuildRank = NULL WHERE GuildRank = '%d' AND GuildID = '%d'", RankID, GuildID);
		SJK.DD("tw_guilds_ranks", "WHERE ID = '%d' AND GuildID = '%d'", RankID, GuildID);
		GS()->ChatGuild(GuildID, "Rank [{STR}] succesful delete", RankGuild[RankID].Rank);
		AddHistoryGuild(GuildID, "Deleted rank '%s'.", RankGuild[RankID].Rank);
		RankGuild.erase(RankID);
	}
}

// изменить ранг
void GuildJob::ChangeRank(int RankID, int GuildID, const char *NewRank)
{
	int FindRank = FindGuildRank(GuildID, NewRank);
	if(RankGuild.find(FindRank) != RankGuild.end())
		return GS()->ChatGuild(GuildID, "Found this rank name in your table, change name");

	if(RankGuild.find(RankID) != RankGuild.end())
	{
		CSqlString<64> cGuildRank = CSqlString<64>(NewRank);
		SJK.UD("tw_guilds_ranks", "Name = '%s' WHERE ID = '%d' AND GuildID = '%d'", 
			cGuildRank.cstr(), RankID, GuildID);
		GS()->ChatGuild(GuildID, "Rank [{STR}] changes to [{STR}]", RankGuild[RankID].Rank, NewRank);
		AddHistoryGuild(GuildID, "Rank '%s' changes to '%s'.", RankGuild[RankID].Rank, NewRank);
		str_copy(RankGuild[RankID].Rank, NewRank, sizeof(RankGuild[RankID].Rank));
	}
}

// изменить доступ
void GuildJob::ChangeRankAccess(int RankID)
{
	if(RankGuild.find(RankID) != RankGuild.end())
	{
		RankGuild[ RankID ].Access++;
		if(RankGuild[ RankID ].Access > GuildAccess::ACCESS_FULL)
			RankGuild[ RankID ].Access = GuildAccess::ACCESS_NO;

		int GuildID = RankGuild[RankID].GuildID;
		SJK.UD("tw_guilds_ranks", "Access = '%d' WHERE ID = '%d' AND GuildID = '%d'", 
			RankGuild[ RankID ].Access, RankID, GuildID);
		GS()->ChatGuild(GuildID, "Rank [{STR}] changes [{STR}]!", RankGuild[RankID].Rank, AccessNames(RankGuild[RankID].Access));
	}	
}

// изменить игроку ранг
void GuildJob::ChangePlayerRank(int AuthID, int RankID)
{
	int ClientID = Job()->Account()->CheckOnlineAccount(AuthID);
	if(ClientID >= 0 && ClientID < MAX_PLAYERS)
	{
		AccountMainSql::Data[ClientID].GuildRank = RankID;
	}
	SJK.UD("tw_accounts_data", "GuildRank = '%d' WHERE ID = '%d'", RankID, AuthID);
}

// показывае меню рангов
void GuildJob::ShowMenuRank(CPlayer *pPlayer)
{
	int ClientID = pPlayer->GetCID(), HideID = NUMHIDEMENU + ItemSql::ItemsInfo.size() + 1300;
	pPlayer->m_LastVoteMenu = MEMBERMENU;

	GS()->AV(ClientID, "null", "Use reason how enter Value, Click fields!");
	GS()->AV(ClientID, "null", "Example: Name rank: [], in reason name, and use this");
	GS()->AV(ClientID, "null", "For leader access full, ignored ranks");
	GS()->AV(ClientID, "null", "- - - - - - - - - -");
	GS()->AV(ClientID, "null", "- Maximal 5 ranks for one guild");
	GS()->AVM(ClientID, "MRANKNAME", 1, NOPE, "Name rank: {STR}", CGS::InteractiveSub[ClientID].RankName);
	GS()->AVM(ClientID, "MRANKCREATE", 1, NOPE, "Create new rank");
	GS()->AV(ClientID, "null", "");
	
	int GuildID = pPlayer->Acc().GuildID;
	for(auto mr: RankGuild)
	{
		if(GuildID != mr.second.GuildID) continue;
		
		HideID += mr.first;
		GS()->AVH(ClientID, HideID, LIGHT_GOLDEN_COLOR, "Rank [{STR}]", mr.second.Rank);
		GS()->AVM(ClientID, "MRANKSET", mr.first, HideID, "Change rank name to ({STR})", CGS::InteractiveSub[ClientID].RankName);
		GS()->AVM(ClientID, "MRANKACCESS", mr.first, HideID, "Access rank ({STR})", AccessNames(mr.second.Access));
		GS()->AVM(ClientID, "MRANKDELETE", mr.first, HideID, "Delete this rank");
	}
	GS()->AddBack(ClientID);
}

/* #########################################################################
	GET CHECK MEMBER INVITE MEMBER 
######################################################################### */
int GuildJob::GetGuildPlayerCount(int GuildID)
{
	int MemberPlayers = -1;
	boost::scoped_ptr<ResultSet> RES2(SJK.SD("ID", "tw_accounts_data", "WHERE GuildID = '%d'", GuildID));
		MemberPlayers = RES2->rowsCount();
	return MemberPlayers;
}

/* #########################################################################
	FUNCTIONS MEMBER INVITE MEMBER 
######################################################################### */
// добавить приглашение игрока в гильдию
bool GuildJob::AddInviteGuild(int GuildID, int OwnerID)
{
	boost::scoped_ptr<ResultSet> RES(SJK.SD("ID", "tw_guilds_invites", "WHERE GuildID = '%d' AND OwnerID = '%d'",  GuildID, OwnerID));
	if(RES->rowsCount() >= 1) return false;

	SJK.ID("tw_guilds_invites", "(GuildID, OwnerID) VALUES ('%d', '%d')", GuildID, OwnerID);
	GS()->ChatGuild(GuildID, "{STR} send invites to join our guilds", Job()->PlayerName(OwnerID));
	return true;
}

// показать лист приглашений в нашу гильдию
void GuildJob::ShowInvitesGuilds(int ClientID, int GuildID)
{
	int HideID = NUMHIDEMENU + ItemSql::ItemsInfo.size() + 1900;
	boost::scoped_ptr<ResultSet> RES(SJK.SD("*", "tw_guilds_invites", "WHERE GuildID = '%d'", GuildID));
	while(RES->next())
	{
		int SenderID = RES->getInt("OwnerID");
		const char *PlayerName = Job()->PlayerName(SenderID);
		GS()->AVH(ClientID, HideID, LIGHT_BLUE_COLOR, "Sender {STR} to join guilds", PlayerName);
		{
			GS()->AVM(ClientID, "MINVITEACCEPT", SenderID, HideID, "Accept {STR} to guild", PlayerName);
			GS()->AVM(ClientID, "MINVITEREJECT", SenderID, HideID, "Reject {STR} to guild", PlayerName);
		}
		HideID++;
	}
	GS()->AddBack(ClientID);
}

// показать топ гильдии и позваться к ним
void GuildJob::ShowFinderGuilds(int ClientID)
{
	GS()->AVL(ClientID, "null", "You are not in guild, or select member");
	GS()->AV(ClientID, "null", "Use reason how enter Value, Click fields!"); 	
	GS()->AV(ClientID, "null", "Example: Find guild: [], in reason name, and use this");
	GS()->AV(ClientID, "null", "");
	GS()->AVM(ClientID, "MINVITENAME", 1, NOPE, "Find guild: {STR}", CGS::InteractiveSub[ClientID].GuildName);

	int HideID = NUMHIDEMENU + ItemSql::ItemsInfo.size() + 1800;
	CSqlString<64> cGuildName = CSqlString<64>(CGS::InteractiveSub[ClientID].GuildName);
	boost::scoped_ptr<ResultSet> RES(SJK.SD("*", "tw_guilds", "WHERE GuildName LIKE '%%%s%%'", cGuildName.cstr()));
	while(RES->next())
	{
		int GuildID = RES->getInt("ID");
		int AvailableSlot = RES->getInt("AvailableSlots");
		int PlayersCount = GetGuildPlayerCount(GuildID);
		cGuildName = RES->getString("GuildName").c_str();

		GS()->AVH(ClientID, HideID, RED_COLOR, "{STR} : Leader {STR} : Players [{INT}/{INT}]", 
			cGuildName.cstr(), Job()->PlayerName(Guild[GuildID].m_OwnerID), &PlayersCount, &AvailableSlot);
		GS()->AVM(ClientID, "null", NOPE, HideID, "House: {STR} | Bank: {INT} gold", (GetGuildHouseID(GuildID) <= 0 ? "No" : "Yes"), &Guild[ GuildID ].m_Bank);
		GS()->AVM(ClientID, "MINVITESEND", GuildID, HideID, "Send request to join {STR}", cGuildName.cstr());
		HideID++;
	}
	GS()->AddBack(ClientID);
}

/* #########################################################################
	FUNCTIONS MEMBER HISTORY MEMBER 
######################################################################### */
// показать список историй
void GuildJob::ShowHistoryGuild(int ClientID, int GuildID)
{
	// ищем в базе данных всю историю гильдии
	char aBuf[128];
	boost::scoped_ptr<ResultSet> RES(SJK.SD("*", "tw_guilds_history", "WHERE GuildID = '%d' ORDER BY ID DESC LIMIT 20", GuildID));
	while(RES->next()) 
	{
		str_format(aBuf, sizeof(aBuf), "[%s] %s", RES->getString("Time").c_str(), RES->getString("Text").c_str());
		GS()->AVM(ClientID, "null", NOPE, NOPE, "{STR}", aBuf);
	}
	GS()->AddBack(ClientID);	
}

// добавить в гильдию историю
void GuildJob::AddHistoryGuild(int GuildID, const char *Buffer, ...)
{
	char aBuf[512];
	va_list VarArgs; 
	va_start(VarArgs, Buffer);
	#if defined(CONF_FAMILY_WINDOWS)
		_vsnprintf(aBuf, sizeof(aBuf), Buffer, VarArgs);
	#else
		vsnprintf(aBuf, sizeof(aBuf), Buffer, VarArgs);
	#endif
	va_end(VarArgs);

	CSqlString<64> cBuf = CSqlString<64>(aBuf);
	SJK.ID("tw_guilds_history", "(GuildID, Text) VALUES ('%d', '%s')", GuildID, cBuf.cstr());
}

/* #########################################################################
	GET CHECK MEMBER HOUSING MEMBER 
######################################################################### */
// айди дома организации
int GuildJob::GetHouseGuildID(int HouseID) const	
{	
	if(HouseGuild.find(HouseID) != HouseGuild.end())
		return HouseGuild.at(HouseID).m_GuildID;
	return -1;
}

int GuildJob::GetHouseWorldID(int HouseID) const
{
	if (HouseGuild.find(HouseID) != HouseGuild.end())
		return HouseGuild.at(HouseID).m_GuildID;
	return -1;
}

// поиск владельца дома в позиции
int GuildJob::GetPosHouseID(vec2 Pos) const
{
	for(const auto& m: HouseGuild)
	{
		if (m.second.m_WorldID != GS()->GetWorldID())
			continue;

		vec2 PositionHouse = vec2(m.second.m_PosX, m.second.m_PosY);
		if(distance(Pos, PositionHouse) < 1000)
			return m.first;
	}
	return -1;
}

bool GuildJob::GetGuildDoor(int GuildID) const
{
	int HouseID = GetGuildHouseID(GuildID);
	if(HouseID > 0 && HouseGuild.find(HouseID) != HouseGuild.end())
		return (bool)HouseGuild[HouseID].m_Door;
	return false;
}

// получаем позицию дома организации
vec2 GuildJob::GetPositionHouse(int GuildID) const
{
	int HouseID = GetGuildHouseID(GuildID);
	if(HouseID > 0 && HouseGuild.find(HouseID) != HouseGuild.end())
		return vec2(HouseGuild[HouseID].m_PosX, HouseGuild[HouseID].m_PosY);
	return vec2(0, 0);
}

// получаем ид дома организации
int GuildJob::GetGuildHouseID(int GuildID) const
{
	for(const auto& imh : HouseGuild)
	{
		if(GuildID > 0 && GuildID == imh.second.m_GuildID)
			return imh.first;
	}
	return -1;
}

/* #########################################################################
	FUNCTIONS MEMBER HOUSING MEMBER 
######################################################################### */
// Покупка дома организации
void GuildJob::BuyGuildHouse(int GuildID, int HouseID)
{
	// проверяем есть ли дом, есть ли владелец у него
	if(GetGuildHouseID(GuildID) > 0 || HouseID <= 0 || HouseGuild[ HouseID ].m_GuildID > 0) return;

	// ищем в базе данных дом
	boost::scoped_ptr<ResultSet> RES(SJK.SD("*", "tw_guilds_houses", "WHERE ID = '%d'", HouseID));
	if(!RES->next()) return;

	// проверяем деньги если не хватает пишет что не хватает
	int Price = RES->getInt("Price");
	if( Guild[ GuildID ].m_Bank < Price)
	{
		GS()->ChatGuild(GuildID, "This Guild house requires {INT}gold!", &Price);
		return;
	}

	// устанавливаем владельца
	HouseGuild[ HouseID ].m_GuildID = GuildID;
	SJK.UD("tw_guilds_houses", "OwnerMID = '%d' WHERE ID = '%d'", GuildID, HouseID);
	
	// снимаем деньги
	Guild[ GuildID ].m_Bank -= Price;
	SJK.UD("tw_guilds", "Bank = '%d' WHERE ID = '%d'", Guild[ GuildID ].m_Bank, GuildID);
	
	// остальное
	GS()->Chat(-1, "{STR} buyight guild house on {STR}!", Guild[ GuildID ].m_Name, GS()->Server()->GetWorldName(GS()->GetWorldID()));
	GS()->ChatDiscord(false, DC_SERVER_INFO, "Information", "{STR} buyight guild house on {STR}!", Guild[ GuildID ].m_Name, GS()->Server()->GetWorldName(GS()->GetWorldID()));
	AddHistoryGuild(GuildID, "Bought a house on '%s'.", GS()->Server()->GetWorldName(GS()->GetWorldID()));
}

// продажа дома организации
void GuildJob::SellGuildHouse(int GuildID)
{
	// проверяем дом
	int HouseID = GetGuildHouseID(GuildID);
	if(HouseID <= 0) return;	

	// ищем владельца дома
	boost::scoped_ptr<ResultSet> RES(SJK.SD("OwnerMID", "tw_guilds_houses", "WHERE ID = '%d'", HouseID));
	if(!RES->next()) return;

	// устанавливаем владельца на 0 и открываем дверь
	if(HouseGuild[HouseID].m_Door)
	{
		delete HouseGuild[HouseID].m_Door;
		HouseGuild[HouseID].m_Door = 0;
	}
	HouseGuild[ HouseID ].m_GuildID = 0;
	SJK.UD("tw_guilds_houses", "OwnerMID = NULL WHERE ID = '%d'", HouseID);
	
	// возращаем деньги
	int SoldBack = HouseGuild[ HouseID ].m_Price;
	Guild[ GuildID ].m_Bank += SoldBack;
	SJK.UD("tw_guilds", "Bank = '%d' WHERE ID = '%d'", Guild[ GuildID ].m_Bank, GuildID);

	// остальное
	GS()->ChatGuild(GuildID, "House sold, {INT}gold returned in bank", &SoldBack);
	AddHistoryGuild(GuildID, "Lost a house on '%s'.", GS()->Server()->GetWorldName(GS()->GetWorldID()));
}

// меню продажи дома
void GuildJob::ShowBuyHouse(CPlayer *pPlayer, int MID)
{
	int ClientID = pPlayer->GetCID();
	int GuildID = pPlayer->Acc().GuildID;
	bool Leader = IsLeaderPlayer(pPlayer);

	GS()->AVH(ClientID, HMEMHOMEINFO, GREEN_COLOR, "Information Member Housing");
	GS()->AVM(ClientID, "null", NOPE, HMEMHOMEINFO, "Buying a house you will need to constantly the Treasury");
	GS()->AVM(ClientID, "null", NOPE, HMEMHOMEINFO, "In the intervals of time will be paid house");

	pPlayer->m_Colored = { 20, 20, 20 };
	
	if(GuildID == MID)
		GS()->AVM(ClientID, "null", NOPE, NOPE, "Guild Bank: {INT} Price: {INT}", &Guild[ GuildID ].m_Bank, &HouseGuild[MID].m_Price);
	
	if(Leader && GuildID != HouseGuild[MID].m_GuildID)
	{
		GS()->AVM(ClientID, "null", NOPE, NOPE, "Every day payment {INT} gold", &HouseGuild[MID].m_Payment);
		GS()->AVM(ClientID, "BUYMEMBERHOUSE", MID, NOPE, "Buy this guild house! Price: {INT}", &HouseGuild[MID].m_Price);
	}
}

// Действия над дверью
void GuildJob::ChangeStateDoor(int GuildID)
{
	if(Guild.find(GuildID) == Guild.end()) 
		return;
	
	// парсим дом
	int HouseID = GetGuildHouseID(GuildID);
	if(HouseGuild.find(HouseID) == HouseGuild.end()) 
		return;

	// если мир не равен данному
	if(HouseGuild[HouseID].m_WorldID != GS()->GetWorldID())
	{
		GS()->ChatGuild(GuildID, "Change state door can only near your house.");	
		return;
	}

	// изменяем статистику двери
	if(HouseGuild[HouseID].m_Door) 
	{
		// дверь удаляем
		delete HouseGuild[HouseID].m_Door;
		HouseGuild[HouseID].m_Door = 0;
	}
	else
	{
		// создаем дверь
		HouseGuild[HouseID].m_Door = new GuildDoor(&GS()->m_World, vec2(HouseGuild[HouseID].m_DoorX, HouseGuild[HouseID].m_DoorY), GuildID);
	}

	// надпись если найдется игрок
	bool StateDoor = (HouseGuild[HouseID].m_Door);
	GS()->ChatGuild(GuildID, "{STR} the house for others.", (StateDoor ? "closed" : "opened"));
}


/* #########################################################################
	HOUSE ENTITIES MEMBER  
######################################################################### */
GuildDoor::GuildDoor(CGameWorld *pGameWorld, vec2 Pos, int GuildID)
: CEntity(pGameWorld, CGameWorld::ENTTYPE_HOUSEDOOR, Pos)
{
	m_GuildID = GuildID;

	m_To = vec2(Pos.x, Pos.y-200);
	m_Pos.y += 30;

	GameWorld()->InsertEntity(this);
}

GuildDoor::~GuildDoor() {}

void GuildDoor::Tick()
{
	for (CCharacter* pChar = (CCharacter*)GameWorld()->FindFirst(CGameWorld::ENTTYPE_CHARACTER); pChar; pChar = (CCharacter*)pChar->TypeNext())
	{
		CPlayer* pPlayer = pChar->GetPlayer();
		if (m_GuildID == pPlayer->Acc().GuildID)
			continue;

		vec2 IntersectPos = closest_point_on_line(m_Pos, m_To, pChar->m_Core.m_Pos);
		float Distance = distance(IntersectPos, pChar->m_Core.m_Pos);
		if (Distance <= g_Config.m_SvDoorRadiusHit)
			pChar->m_DoorHit = true;
	}
}

void GuildDoor::Snap(int SnappingClient)
{
	if (NetworkClipped(SnappingClient))
		return;

	CNetObj_Laser *pObj = static_cast<CNetObj_Laser *>(Server()->SnapNewItem(NETOBJTYPE_LASER, GetID(), sizeof(CNetObj_Laser)));
	if (!pObj)
		return;

	pObj->m_X = int(m_Pos.x);
	pObj->m_Y = int(m_Pos.y);
	pObj->m_FromX = int(m_To.x);
	pObj->m_FromY = int(m_To.y);
	pObj->m_StartTick = Server()->Tick()-2;
}