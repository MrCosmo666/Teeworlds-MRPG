/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#include <engine/shared/config.h>
#include <game/server/gamecontext.h>
#include "member.h"

using namespace sqlstr;
std::map < int , MemberSql::MemberStruct > MemberSql::Member;
std::map < int , MemberSql::MemberStructHouse > MemberSql::MemberHouse;
std::map < int , MemberSql::MemberStructRank > MemberSql::MemberRank;

/* #########################################################################
	LOADING MEMBER 
######################################################################### */
// Инициализация класса
void MemberSql::OnInitLocal(const char *pLocal) 
{ 
	// загрузка домов
	boost::scoped_ptr<ResultSet> RES(SJK.SD("*", "tw_members_houses", pLocal));
	while(RES->next())
	{
		// позиции дома
		int MHID = RES->getInt("ID");
		MemberHouse[MHID].mDoorX = RES->getInt("DoorX");
		MemberHouse[MHID].mDoorY = RES->getInt("DoorY");	
		if(!MemberHouse[MHID].m_Door) { MemberHouse[MHID].m_Door = 0; }

		// информация дома
		MemberHouse[MHID].mOwnerMemberID = RES->getInt("OwnerMID");
		MemberHouse[MHID].mX = RES->getInt("mX");
		MemberHouse[MHID].mY = RES->getInt("mY");
		MemberHouse[MHID].mPrice = RES->getInt("Price");
		MemberHouse[MHID].mEveryDay = RES->getInt("EveryDay");
		MemberHouse[MHID].mWorldID = RES->getInt("WorldID");

		// текст позиции
		MemberHouse[MHID].mTextX = RES->getInt("mTextX");
		MemberHouse[MHID].mTextY = RES->getInt("mTextY");

		// создаем дверь если есть владельцы
		if(MemberHouse[MHID].mOwnerMemberID > 0 && !MemberHouse[MHID].m_Door)
			MemberHouse[MHID].m_Door = new MemberDoor(&GS()->m_World, vec2(MemberHouse[MHID].mDoorX, MemberHouse[MHID].mDoorY), MemberHouse[MHID].mOwnerMemberID);
	}
	Job()->ShowLoadingProgress("Guilds Houses", MemberHouse.size());
}

void MemberSql::OnInitGlobal()
{
	boost::scoped_ptr<ResultSet> RES(SJK.SD("*", "tw_members", "WHERE ID > '0'"));
	while(RES->next())
	{
		int MID = RES->getInt("ID");
		Member[MID].mOwnerID = RES->getInt("OwnerID");
		Member[MID].mLevel = RES->getInt("Level");
		Member[MID].mExperience = RES->getInt("Experience");
		Member[MID].mBank = RES->getInt("Bank");
		Member[MID].mScore = RES->getInt("Score");
		str_copy(Member[MID].mMemberName, RES->getString("MemberName").c_str(), sizeof(Member[MID].mMemberName));

		for(int i = 0; i < EMEMBERUPGRADE::NUM_EMEMBERUPGRADE; i++)
			Member[MID].mUpgrades[ i ] = RES->getInt(UpgradeNames(i, true).c_str());

		LoadMemberRank(MID);
	}
	Job()->ShowLoadingProgress("Guilds", Member.size());
}

// тик домов
void MemberSql::OnTick()
{
	// выводим информацию о клане
	if(GS()->Server()->Tick() % (GS()->Server()->TickSpeed()) == 0)
	{
		for(const auto& mh : MemberHouse)
		{
			if(mh.second.mWorldID != GS()->GetWorldID()) 
				continue;

			const int MemberID = mh.second.mOwnerMemberID;
			if(MemberID > 0)
			{
				GS()->CreateText(NULL, false, vec2(mh.second.mTextX, mh.second.mTextY), vec2 (0, 0), 49, MemberName(MemberID), mh.second.mWorldID);
				continue;
			}
			GS()->CreateText(NULL, false, vec2(mh.second.mTextX, mh.second.mTextY), vec2 (0, 0), 49, "Empty", mh.second.mWorldID);
		}
	}
}

// Проверка уплаты дома в случае нет казны освобождаем
void MemberSql::OnPaymentTime()
{
	boost::scoped_ptr<ResultSet> RES(SJK.SD("ID, Bank", "tw_members"));
	while(RES->next())
	{
		const int MID = RES->getInt("ID");
		const int HouseID = GetMemberHouseID(MID);
		if(HouseID > 0 && Member.find(MID) != Member.end() && MemberHouse.find(HouseID) != MemberHouse.end())
		{
			if( Member[MID].mBank < MemberHouse[ HouseID ].mEveryDay)
			{
				GS()->Chat(-1, "Guild {STR} lost house, nope payment!", Member[MID].mMemberName);
				SellGuildHouse(MID);
				continue;
			}
			
			Member[MID].mBank -= MemberHouse[ HouseID ].mEveryDay;
			SJK.UD("tw_members", "Bank = '%d' WHERE ID = '%d'", Member[MID].mBank, MID);
			GS()->ChatGuild(MID, "Payment {INT} gold was successful {INT}", &MemberHouse[HouseID].mEveryDay, &Member[MID].mBank);
		}
	}
}

// загрузка рангов организаций
void MemberSql::LoadMemberRank(int MemberID)
{
	boost::scoped_ptr<ResultSet> RES(SJK.SD("*", "tw_members_ranks", "WHERE ID > '0' AND MemberID = '%d'", MemberID));
	while(RES->next())
	{
		int ID = RES->getInt("ID");
		MemberRank[ ID ].MemberID = MemberID;
		MemberRank[ ID ].Access = RES->getInt("Access");
		str_copy(MemberRank[ ID ].Rank, RES->getString("Name").c_str(), sizeof(MemberRank[ ID ].Rank));
	}
}

/* #########################################################################
	GET CHECK MEMBER 
######################################################################### */
// имя организации
const char *MemberSql::MemberName(int MemberID) const
{	
	if(Member.find(MemberID) != Member.end())
		return Member[MemberID].mMemberName;	
	return "None"; 
}

// Лидер ли игрок
bool MemberSql::IsLeaderPlayer(CPlayer *pPlayer, int Access) const
{
	const int ClientID = pPlayer->GetCID();
	const int MemberID = pPlayer->Acc().MemberID;
	if(MemberID > 0 && Member.find(MemberID) != Member.end() && 
		(Member[MemberID].mOwnerID == pPlayer->Acc().AuthID || 
			MemberRank.find(pPlayer->Acc().MemberRank) != MemberRank.end() && 
				(MemberRank[pPlayer->Acc().MemberRank].Access == Access || MemberRank[pPlayer->Acc().MemberRank].Access == MACCESSFULL)))
		return true;
	return false;
}

// получить бонус от стульев организации
int MemberSql::GetMemberChairBonus(int MemberID, int Field) const
{
	if(MemberID > 0 && Member.find(MemberID) != Member.end())
		return Member[MemberID].mUpgrades[ Field ];
	return -1;
}

// получить имена апгрейдов
std::string MemberSql::UpgradeNames(int Field, bool DataTable)
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

// получить требуемое кол-во опыта для повышения уровня
int MemberSql::ExpForLevel(int Level)
{
	return (g_Config.m_SvGuildLeveling+Level*2)*(Level*Level);
} 

/* #########################################################################
	FUNCTIONS MEMBER MEMBER 
######################################################################### */
// создаем новую организацию
void MemberSql::CreateGuild(int ClientID, const char *MemberName)
{
	CPlayer *pPlayer = GS()->GetPlayer(ClientID, true);

	// проверяем находимся ли уже в организации
	if(pPlayer->Acc().MemberID > 0) return GS()->Chat(ClientID, "You already in guild group!");

	// проверяем доступность имени организации
	CSqlString<64> cMemberName = CSqlString<64>(MemberName);
	boost::scoped_ptr<ResultSet> RES(SJK.SD("ID", "tw_members", "WHERE MemberName = '%s'", cMemberName.cstr()));
	if(RES->next()) return GS()->Chat(ClientID, "This member name already useds!");
	
	// проверяем тикет забераем и создаем
	if(pPlayer && pPlayer->GetItem(itTicketGuild).Count > 0 && pPlayer->GetItem(itTicketGuild).Remove(1))
	{
		// получаем Айди для инициализации
		boost::scoped_ptr<ResultSet> RES2(SJK.SD("ID", "tw_members", "ORDER BY ID DESC LIMIT 1"));
		int InitID = RES2->next() ? RES2->getInt("ID")+1 : 1; // TODO: thread save ? hm need for table all time auto increment = 1; NEED FIX IT
		
		// инициализируем гильдию
		Member[InitID].mOwnerID = pPlayer->Acc().AuthID;
		Member[InitID].mLevel = 1;
		Member[InitID].mExperience = 0;
		Member[InitID].mBank = 0;
		Member[InitID].mScore = 0;
		str_copy(Member[InitID].mMemberName, cMemberName.cstr(), sizeof(Member[InitID].mMemberName));

		// улучшения инициализируем
		Member[InitID].mUpgrades[EMEMBERUPGRADE::AvailableNSTSlots] = 2;
		for(int i = 1/*skip slots upgrade*/; i < EMEMBERUPGRADE::NUM_EMEMBERUPGRADE; i++)
			Member[InitID].mUpgrades[ i ] = 1;

		// создаем в таблице гильдию
		SJK.ID("tw_members", "(ID, MemberName, OwnerID) VALUES ('%d', '%s', '%d')", InitID, cMemberName.cstr(), pPlayer->Acc().AuthID);

		// устанавливаем данные о владении и игроке
		pPlayer->Acc().MemberID = InitID;
		SJK.UD("tw_accounts_data", "MemberID = '%d' WHERE ID = '%d'", InitID, pPlayer->Acc().AuthID);

		// остальное
		GS()->Chat(-1, "New guilds [{STR}] have been created!", cMemberName.cstr());
	}
	else GS()->Chat(ClientID, "You need first buy guild ticket on shop!");
}

// подключение к организации
void MemberSql::JoinGuild(int AuthID, int MemberID)
{
	// проверяем клан есть или нет у этого и грока
	const char *PlayerName = Job()->PlayerName(AuthID);
	boost::scoped_ptr<ResultSet> RES(SJK.SD("ID", "tw_accounts_data", "WHERE ID = '%d' AND MemberID > '0'", AuthID));
	if(RES->next())
	{
		GS()->ChatAccountID(AuthID, "You already in guild group!");
		return GS()->ChatGuild(MemberID, "{STR} already joined your or another guilds", PlayerName);
	}

	// проверяем количество слотов доступных
	boost::scoped_ptr<ResultSet> RES2(SJK.SD("ID", "tw_accounts_data", "WHERE MemberID = '%d'", MemberID));
	if(RES2->rowsCount() >= Member[ MemberID ].mUpgrades[ EMEMBERUPGRADE::AvailableNSTSlots ])
	{
		GS()->ChatAccountID(AuthID, "You don't joined [No slots for join]");
		return GS()->ChatGuild(MemberID, "{STR} don't joined [No slots for join]", PlayerName);
	}

	// обновляем и получаем данные
	int ClientID = Job()->Account()->CheckOnlineAccount(AuthID);
	if(ClientID >= 0 && ClientID < MAX_PLAYERS)
	{
		AccountMainSql::Data[ClientID].MemberID = MemberID;
		AccountMainSql::Data[ClientID].MemberRank = 0;
		GS()->ResetVotes(ClientID, MAINMENU);
	}
	SJK.UD("tw_accounts_data", "MemberID = '%d', MemberRank = '0' WHERE ID = '%d'", MemberID, AuthID);
	GS()->ChatGuild(MemberID, "Player {STR} join in your guild!", PlayerName);
}

// выход из организации, AccountID чтобы можно было кикать игроков оффлайн
void MemberSql::ExitGuild(int AccountID)
{
	// проверяем если покидает лидер клан
	boost::scoped_ptr<ResultSet> RES(SJK.SD("ID", "tw_members", "WHERE OwnerID = '%d'", AccountID));
	if(RES->next()) return GS()->ChatAccountID(AccountID, "A leader cannot leave his or her guild group!");

	// проверяем аккаунт и его организацию и устанавливаем
	boost::scoped_ptr<ResultSet> RES2(SJK.SD("MemberID", "tw_accounts_data", "WHERE ID = '%d'", AccountID));
	if(!RES2->next()) return;

	// пишим гильдии что игрок покинул гильдию
	const int MemberID = RES2->getInt("MemberID");
	GS()->ChatGuild(MemberID, "{STR} left the Guild!", Job()->PlayerName(AccountID));
	AddHistoryGuild(MemberID, "'%s' exit or kicked.", Job()->PlayerName(AccountID));

	// обновляем информацию игроку
	int ClientID = Job()->Account()->CheckOnlineAccount(AccountID);
	if(ClientID >= 0 && ClientID < MAX_PLAYERS)
	{
		GS()->ResetVotes(ClientID, MAINMENU);
		AccountMainSql::Data[ClientID].MemberID = -1;
	}
	SJK.UD("tw_accounts_data", "MemberID = NULL, MemberRank = '-1', MemberDeposit = '0' WHERE ID = '%d'", AccountID);
}

// показываем меню организации
void MemberSql::ShowMenuGuild(CPlayer *pPlayer)
{
	// если не нашли гильдии такой то показывает 'Поиск гильдий'
	const int ClientID = pPlayer->GetCID();
	const int MemberID = pPlayer->Acc().MemberID;
	if(Member.find(MemberID) == Member.end()) return ShowFinderGuilds(ClientID);
	
	// показываем само меню
	int ExpNeed = ExpForLevel(Member[MemberID].mLevel);
	GS()->AVH(ClientID, HMEMBERSTATS, vec3(52,26,80), "Guild name: {STR}", Member[MemberID].mMemberName);
	GS()->AVM(ClientID, "null", NOPE, HMEMBERSTATS, "Level: {INT} Experience: {INT}/{INT}", &Member[MemberID].mLevel, &Member[MemberID].mExperience, &ExpNeed);
	GS()->AVM(ClientID, "null", NOPE, HMEMBERSTATS, "Maximal available player count: {INT}", &Member[MemberID].mUpgrades[EMEMBERUPGRADE::AvailableNSTSlots]);
	GS()->AVM(ClientID, "null", NOPE, HMEMBERSTATS, "Leader: {STR}", Job()->PlayerName(Member[MemberID].mOwnerID));
	GS()->AVM(ClientID, "null", NOPE, HMEMBERSTATS, "- - - - - - - - - -");
	GS()->AVM(ClientID, "null", NOPE, HMEMBERSTATS, "/ginvite <id> - to invite a player into members (for leader)");
	GS()->AVM(ClientID, "null", NOPE, HMEMBERSTATS, "/gexit - leave of guild group (for all members)");
	GS()->AVM(ClientID, "null", NOPE, HMEMBERSTATS, "Many options are unlocked with the purchase of a home");
	GS()->AVM(ClientID, "null", NOPE, HMEMBERSTATS, "- - - - - - - - - -");
	GS()->AVM(ClientID, "null", NOPE, HMEMBERSTATS, "Guild Bank: {INT}gold", &Member[MemberID].mBank);
	int MemberHouse = GetMemberHouseID(MemberID);
	if (MemberHouse > 0) { GS()->AVM(ClientID, "null", NOPE, HMEMBERSTATS, "Door Status: {STR}", GetMemberDoor(MemberID) ? "Closed" : "Opened"); }
	GS()->AV(ClientID, "null", "");

	pPlayer->m_Colored = { 10,10,10 };
	GS()->AVL(ClientID, "null", "# Your money: {INT}gold", &pPlayer->GetItem(itMoney).Count);
	GS()->AVL(ClientID, "MMONEY", "Add money guild bank. (Amount in a reason)", Member[MemberID].mMemberName);
	GS()->AVM(ClientID, "MENU", MEMBERRANK, NOPE, "Settings guild Rank's");
	GS()->AVM(ClientID, "MENU", MEMBERINVITES, NOPE, "Invites to your guild");
	GS()->AVM(ClientID, "MENU", MEMBERHISTORY, NOPE, "History of activity");

	// если имеется дом
	if(MemberHouse > 0)
	{
		GS()->AVL(ClientID, "MDOOR", "Change state [\"{STR} door\"]", GetMemberDoor(MemberID) ? "Open" : "Close");
		GS()->AVL(ClientID, "MSPAWN", "Teleport to guild house");
		GS()->AVL(ClientID, "MHOUSESELL", "Sell your guild house (in reason 777)");

		for(int i = EMEMBERUPGRADE::ChairNSTExperience ; i < EMEMBERUPGRADE::NUM_EMEMBERUPGRADE; i++)
		{
			int PriceUpgrade = Member[ MemberID ].mUpgrades[ i ] * g_Config.m_SvPriceUpgradeGuildAnother;
			GS()->AVM(ClientID, "MUPGRADE", i, NOPE, "Upgrade {STR} ({INT}) {INT}gold", UpgradeNames(i).c_str(), &Member[MemberID].mUpgrades[i], &PriceUpgrade);
		}
	}

	// улучения без дома
	int PriceUpgrade = Member[ MemberID ].mUpgrades[ EMEMBERUPGRADE::AvailableNSTSlots ] * g_Config.m_SvPriceUpgradeGuildSlot;
	GS()->AVM(ClientID, "MUPGRADE", EMEMBERUPGRADE::AvailableNSTSlots, NOPE, "Upgrade {STR} ({INT}) {INT}gold", 
		UpgradeNames(EMEMBERUPGRADE::AvailableNSTSlots).c_str(), &Member[MemberID].mUpgrades[ EMEMBERUPGRADE::AvailableNSTSlots ], &PriceUpgrade);
	GS()->AV(ClientID, "null", "");
	
	// список членов в гильдии
	int HideID = NUMHIDEMENU + ItemSql::ItemsInfo.size() + 1000;
	boost::scoped_ptr<ResultSet> RES(SJK.SD("ID, Nick, MemberRank, MemberDeposit", "tw_accounts_data", "WHERE MemberID = '%d'", MemberID));
	while(RES->next())
	{
		const int AuthID = RES->getInt("ID");
		const int RankID = RES->getInt("MemberRank");
		int Deposit = RES->getInt("MemberDeposit");
		GS()->AVH(ClientID, HideID, vec3(15,40,80), "Rank: {STR} {STR} Deposit: {INT}", GetMemberRank(MemberID, RankID), RES->getString("Nick").c_str(), &Deposit);

		// сбор всех рангов и вывод их
		for(auto mr: MemberRank) 
		{
			if(MemberID != mr.second.MemberID || RankID == mr.first) continue;
			GS()->AVD(ClientID, "MRANKCHANGE", AuthID, mr.first, HideID, "Change Rank to: {STR}{STR}", mr.second.Rank, mr.second.Access > 0 ? "*" : "");
		}
		GS()->AVM(ClientID, "MKICK", AuthID, HideID, "Kick");
		if(AuthID != pPlayer->Acc().AuthID) GS()->AVM(ClientID, "MLEADER", AuthID, HideID, "Give Leader (in reason 134)");
		HideID++;
	}
	GS()->AddBack(ClientID);
	return;
}

// добавляем опыт в организацию
void MemberSql::AddExperience(int MemberID)
{
	bool UpdateTable = false;
	Member[MemberID].mExperience += 1;
	for( ; Member[MemberID].mExperience >= ExpForLevel(Member[MemberID].mLevel); ) 
	{
		Member[MemberID].mExperience -= ExpForLevel(Member[MemberID].mLevel), Member[MemberID].mLevel++;
		GS()->Chat(-1, "Guild {STR} raised the level up to {INT}", Member[MemberID].mMemberName, &Member[MemberID].mLevel);
		GS()->ChatDiscord(false, DC_SERVER_INFO, "Information", "Guild {STR} raised the level up to {INT}", Member[MemberID].mMemberName, &Member[MemberID].mLevel);

		AddHistoryGuild(MemberID, "Guild raised level to '%d'.", Member[MemberID].mLevel);

		// если это последний уровень повышения
		if(Member[MemberID].mExperience < ExpForLevel(Member[MemberID].mLevel))
			UpdateTable = true;
	}
	if(rand()%10 == 2 || UpdateTable)
	{
		SJK.UD("tw_members", "Level = '%d', Experience = '%d' WHERE ID = '%d'", 
			Member[MemberID].mLevel, Member[MemberID].mExperience, MemberID);		
	}
}

// добавляем деньги в банк организаций
bool MemberSql::AddMoneyBank(int MemberID, int Money)
{
	boost::scoped_ptr<ResultSet> RES(SJK.SD("ID, Bank", "tw_members", "WHERE ID = '%d'", MemberID));
	if(!RES->next()) return false;
	
	// добавить деньги
	int MoneyLastBank = Member[MemberID].mBank;
	Member[MemberID].mBank = RES->getInt("Bank") + Money;
	SJK.UDS("tw_members", [&](){
		Member[MemberID].mBank = MoneyLastBank;
	}, "Bank = '%d' WHERE ID = '%d'", Member[MemberID].mBank, MemberID);
	return true;
}

// снимаем деньги с банка организации
bool MemberSql::RemoveMoneyBank(int MemberID, int Money)
{
	boost::scoped_ptr<ResultSet> RES(SJK.SD("ID, Bank", "tw_members", "WHERE ID = '%d'", MemberID));
	if(!RES->next()) return false;
	
	// проверяем хватает ли в банке на оплату
	Member[MemberID].mBank = RES->getInt("Bank");
	if(Money > Member[MemberID].mBank)
		return false;

	// оплата
	Member[MemberID].mBank -= Money;
	SJK.UD("tw_members", "Bank = '%d' WHERE ID = '%d'", Member[MemberID].mBank, MemberID);
	return true;
}

// покупка улучшения максимальное количество слотов
bool MemberSql::UpgradeGuild(int MemberID, int Field)
{
	boost::scoped_ptr<ResultSet> RES(SJK.SD("*", "tw_members", "WHERE ID = '%d'", MemberID));
	if(RES->next())
	{
		Member[ MemberID ].mBank = RES->getInt("Bank");
		Member[ MemberID ].mUpgrades[ Field ] = RES->getInt(UpgradeNames(Field, true).c_str());

		int UpgradePrice = g_Config.m_SvPriceUpgradeGuildAnother;
		if(Field == EMEMBERUPGRADE::AvailableNSTSlots) UpgradePrice = g_Config.m_SvPriceUpgradeGuildSlot;

		int PriceAvailable = Member[ MemberID ].mUpgrades[ Field ]*UpgradePrice;
		if(PriceAvailable > Member[ MemberID ].mBank)
			return false;

		Member[ MemberID ].mUpgrades[ Field ]++;
		Member[ MemberID ].mBank -= PriceAvailable;
		SJK.UD("tw_members", "Bank = '%d', %s = '%d' WHERE ID = '%d'", 
			Member[MemberID].mBank, UpgradeNames(Field, true).c_str(), Member[MemberID].mUpgrades[ Field ], MemberID);

		AddHistoryGuild(MemberID, "'%s' level up to '%d'.", UpgradeNames(Field).c_str(), Member[MemberID].mUpgrades[Field]);
		return true;
	}
	return false;
}

/* #########################################################################
	GET CHECK MEMBER RANK MEMBER 
######################################################################### */
// именна доступов
const char *MemberSql::AccessNames(int Access)
{
	switch(Access) 
	{
		default: return "No Access";
		case MACCESSINVITEKICK: return "Access Invite Kick";
		case MACCESSUPGHOUSE: return "Access Upgrades & House";
		case MACCESSFULL: return "Full Access";
	}
}

// получить имя ранга
const char *MemberSql::GetMemberRank(int MemberID, int RankID)
{
	if(MemberRank.find(RankID) != MemberRank.end() && MemberID == MemberRank[RankID].MemberID) 
		return MemberRank[RankID].Rank;
	return "Member";
}

// найти ранг по имени и организации
int MemberSql::FindMemberRank(int MemberID, const char *Rank) const
{
	for(auto mr: MemberRank) 
	{
		if(MemberID == mr.second.MemberID && str_comp(Rank, mr.second.Rank) == 0)
			return mr.first;
	}
	return -1;
}

/* #########################################################################
	FUNCTIONS MEMBER RANK MEMBER 
######################################################################### */
// добавить ранг
void MemberSql::AddRank(int MemberID, const char *Rank)
{
	int FindRank = FindMemberRank(MemberID, Rank);
	if(MemberRank.find(FindRank) != MemberRank.end())
		return GS()->ChatGuild(MemberID, "Found this rank in your table, change name");

	boost::scoped_ptr<ResultSet> RES(SJK.SD("ID", "tw_members_ranks", "WHERE MemberID = '%d'", MemberID));
	if(RES->rowsCount() >= 5) return;

	boost::scoped_ptr<ResultSet> RES2(SJK.SD("ID", "tw_members_ranks", "ORDER BY ID DESC LIMIT 1"));
	int InitID = RES2->next() ? RES2->getInt("ID")+1 : 1; 
	// thread save ? hm need for table all time auto increment = 1; NEED FIX IT

	CSqlString<64> cMemberRank = CSqlString<64>(Rank);
	SJK.ID("tw_members_ranks", "(ID, MemberID, Name) VALUES ('%d', '%d', '%s')", InitID, MemberID, cMemberRank.cstr());
	GS()->ChatGuild(MemberID, "Creates new rank [{STR}]!", Rank);
	AddHistoryGuild(MemberID, "Added new rank '%s'.", Rank);

	MemberRank[InitID].MemberID = MemberID;
	str_copy(MemberRank[InitID].Rank, Rank, sizeof(MemberRank[InitID].Rank));
}

// удалить ранг
void MemberSql::DeleteRank(int RankID, int MemberID)
{
	if(MemberRank.find(RankID) != MemberRank.end())
	{
		SJK.DD("tw_members_ranks", "WHERE ID = '%d' AND MemberID = '%d'", RankID, MemberID);
		GS()->ChatGuild(MemberID, "Rank [{STR}] succesful delete", MemberRank[RankID].Rank);
		AddHistoryGuild(MemberID, "Deleted rank '%s'.", MemberRank[RankID].Rank);
		MemberRank.erase(RankID);
	}
}

// изменить ранг
void MemberSql::ChangeRank(int RankID, int MemberID, const char *NewRank)
{
	int FindRank = FindMemberRank(MemberID, NewRank);
	if(MemberRank.find(FindRank) != MemberRank.end())
		return GS()->ChatGuild(MemberID, "Found this rank name in your table, change name");

	if(MemberRank.find(RankID) != MemberRank.end())
	{
		CSqlString<64> cMemberRank = CSqlString<64>(NewRank);
		SJK.UD("tw_members_ranks", "Name = '%s' WHERE ID = '%d' AND MemberID = '%d'", 
			cMemberRank.cstr(), RankID, MemberID);
		GS()->ChatGuild(MemberID, "Rank [{STR}] changes to [{STR}]", MemberRank[RankID].Rank, NewRank);
		AddHistoryGuild(MemberID, "Rank '%s' changes to '%s'.", MemberRank[RankID].Rank, NewRank);
		str_copy(MemberRank[RankID].Rank, NewRank, sizeof(MemberRank[RankID].Rank));
	}
}

// изменить доступ
void MemberSql::ChangeRankAccess(int RankID)
{
	if(MemberRank.find(RankID) != MemberRank.end())
	{
		MemberRank[ RankID ].Access++;
		if(MemberRank[ RankID ].Access > MACCESSFULL)
			MemberRank[ RankID ].Access = MACCESSNO;

		int MemberID = MemberRank[RankID].MemberID;
		SJK.UD("tw_members_ranks", "Access = '%d' WHERE ID = '%d' AND MemberID = '%d'", 
			MemberRank[ RankID ].Access, RankID, MemberID);
		GS()->ChatGuild(MemberID, "Rank [{STR}] changes [{STR}]!", MemberRank[RankID].Rank, AccessNames(MemberRank[RankID].Access));
	}	
}

// изменить игроку ранг
void MemberSql::ChangePlayerRank(int AuthID, int RankID)
{
	int ClientID = Job()->Account()->CheckOnlineAccount(AuthID);
	if(ClientID >= 0 && ClientID < MAX_PLAYERS)
	{
		AccountMainSql::Data[ClientID].MemberRank = RankID;
	}
	SJK.UD("tw_accounts_data", "MemberRank = '%d' WHERE ID = '%d'", RankID, AuthID);
}

// показывае меню рангов
void MemberSql::ShowMenuRank(CPlayer *pPlayer)
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
	
	int MemberID = pPlayer->Acc().MemberID;
	for(auto mr: MemberRank) 
	{
		if(MemberID != mr.second.MemberID) continue;
		
		HideID += mr.first;
		GS()->AVH(ClientID, HideID, vec3(40,20,15), "Rank [{STR}]", mr.second.Rank);
		GS()->AVM(ClientID, "MRANKSET", mr.first, HideID, "Change rank name to ({STR})", CGS::InteractiveSub[ClientID].RankName);
		GS()->AVM(ClientID, "MRANKACCESS", mr.first, HideID, "Access rank ({STR})", AccessNames(mr.second.Access));
		GS()->AVM(ClientID, "MRANKDELETE", mr.first, HideID, "Delete this rank");
	}
	GS()->AddBack(ClientID);
}

/* #########################################################################
	GET CHECK MEMBER INVITE MEMBER 
######################################################################### */
int MemberSql::GetGuildPlayerCount(int MemberID)
{
	int MemberPlayers = -1;
	boost::scoped_ptr<ResultSet> RES2(SJK.SD("ID", "tw_accounts_data", "WHERE MemberID = '%d'", MemberID));
		MemberPlayers = RES2->rowsCount();
	return MemberPlayers;
}

/* #########################################################################
	FUNCTIONS MEMBER INVITE MEMBER 
######################################################################### */
// добавить приглашение игрока в гильдию
bool MemberSql::AddInviteGuild(int MemberID, int OwnerID)
{
	boost::scoped_ptr<ResultSet> RES(SJK.SD("ID", "tw_members_invites", "WHERE MemberID = '%d' AND OwnerID = '%d'",  MemberID, OwnerID));
	if(RES->rowsCount() >= 1) return false;

	SJK.ID("tw_members_invites", "(MemberID, OwnerID) VALUES ('%d', '%d')", MemberID, OwnerID);
	GS()->ChatGuild(MemberID, "{STR} send invites to join our guilds", Job()->PlayerName(OwnerID));
	return true;
}

// показать лист приглашений в нашу гильдию
void MemberSql::ShowInvitesGuilds(int ClientID, int MemberID)
{
	int HideID = NUMHIDEMENU + ItemSql::ItemsInfo.size() + 1900;
	boost::scoped_ptr<ResultSet> RES(SJK.SD("*", "tw_members_invites", "WHERE MemberID = '%d'", MemberID));
	while(RES->next())
	{
		int OwnerID = RES->getInt("OwnerID");
		const char *PlayerName = Job()->PlayerName(OwnerID);
		GS()->AVH(ClientID, HideID, vec3(15,40,80), "Sender {STR} to join guilds", PlayerName);
		{
			GS()->AVD(ClientID, "MINVITEACCEPT", MemberID, OwnerID, HideID, "Accept {STR} to guild", PlayerName);
			GS()->AVD(ClientID, "MINVITEREJECT", MemberID, OwnerID, HideID, "Reject {STR} to guild", PlayerName);
		}
		HideID++;
	}
	GS()->AddBack(ClientID);
}

// показать топ гильдии и позваться к ним
void MemberSql::ShowFinderGuilds(int ClientID)
{
	GS()->AVL(ClientID, "null", "You are not in guild, or select member");
	GS()->AV(ClientID, "null", "Use reason how enter Value, Click fields!"); 	
	GS()->AV(ClientID, "null", "Example: Find guild: [], in reason name, and use this");
	GS()->AV(ClientID, "null", "");
	GS()->AVM(ClientID, "MINVITENAME", 1, NOPE, "Find guild: {STR}", CGS::InteractiveSub[ClientID].GuildName);

	int HideID = NUMHIDEMENU + ItemSql::ItemsInfo.size() + 1800;
	CSqlString<64> cGuildName = CSqlString<64>(CGS::InteractiveSub[ClientID].GuildName);
	boost::scoped_ptr<ResultSet> RES(SJK.SD("*", "tw_members", "WHERE MemberName LIKE '%%%s%%'", cGuildName.cstr()));
	while(RES->next())
	{
		int MemberID = RES->getInt("ID");
		int AvailableSlot = RES->getInt("AvailableSlots");
		const char *MemberName = RES->getString("MemberName").c_str();
		int PlayersCount = GetGuildPlayerCount(MemberID);

		GS()->AVH(ClientID, HideID, vec3(15,40,80), "Leader: {STR} Guild {STR} Players [{INT}/{INT}]", 
			Job()->PlayerName(Member[ MemberID ].mOwnerID), MemberName, &PlayersCount, &AvailableSlot);
		{
			GS()->AVM(ClientID, "null", NOPE, HideID, "House: {STR} | Bank: {INT} gold", (GetMemberHouseID(MemberID) <= 0 ? "No" : "Yes"), &Member[ MemberID ].mBank);
			GS()->AVM(ClientID, "MINVITESEND", MemberID, HideID, "Send request to join {STR}", MemberName);
		}		
		HideID++;
	}
	GS()->AddBack(ClientID);
}

/* #########################################################################
	FUNCTIONS MEMBER HISTORY MEMBER 
######################################################################### */
// показать список историй
void MemberSql::ShowHistoryGuild(int ClientID, int MemberID)
{
	// ищем в базе данных всю историю гильдии
	char aBuf[128];
	boost::scoped_ptr<ResultSet> RES(SJK.SD("*", "tw_members_history", "WHERE MemberID = '%d' ORDER BY ID DESC LIMIT 20", MemberID));
	while(RES->next()) 
	{
		str_format(aBuf, sizeof(aBuf), "[%s] %s", RES->getString("Time").c_str(), RES->getString("Text").c_str());
		GS()->AVM(ClientID, "null", NOPE, NOPE, "{STR}", aBuf);
	}
	GS()->AddBack(ClientID);	
}

// добавить в гильдию историю
void MemberSql::AddHistoryGuild(int MemberID, const char *Buffer, ...)
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
	SJK.ID("tw_members_history", "(MemberID, Text) VALUES ('%d', '%s')", MemberID, cBuf.cstr());
}

/* #########################################################################
	GET CHECK MEMBER HOUSING MEMBER 
######################################################################### */
// айди дома организации
int MemberSql::GetHouseMemberID(int HouseID) const	
{	
	if(MemberHouse.find(HouseID) != MemberHouse.end())
		return MemberHouse.at(HouseID).mOwnerMemberID;
	return -1;
}

// мир дома организации
int MemberSql::GetHouseWorldID(int HouseID) const
{
	if(MemberHouse.find(HouseID) != MemberHouse.end())
		return MemberHouse.at(HouseID).mWorldID;
	return -1;
}

// поиск владельца дома в позиции
int MemberSql::GetPosHouseID(vec2 Pos) const
{
	for(const auto& m: MemberHouse) 
	{
		vec2 PositionHouse = vec2(m.second.mX, m.second.mY);
		if(distance(Pos, PositionHouse) < 400)
			return m.first;
	}
	return -1;
}

// получаем информацию о двери дома организации
bool MemberSql::GetMemberDoor(int MemberID) const
{
	int HouseID = GetMemberHouseID(MemberID);
	if(HouseID > 0 && MemberHouse.find(HouseID) != MemberHouse.end())
	{
		bool DoorState = (MemberHouse[HouseID].m_Door);
		return DoorState;
	}
	return false;
}

// получаем позицию дома организации
vec2 MemberSql::GetPositionHouse(int MemberID) const
{
	int HouseID = GetMemberHouseID(MemberID);
	if(HouseID > 0 && MemberHouse.find(HouseID) != MemberHouse.end())
		return vec2(MemberHouse[HouseID].mX, MemberHouse[HouseID].mY);
	return vec2(0, 0);
}

// получаем ид дома организации
int MemberSql::GetMemberHouseID(int MemberID) const
{
	for(const auto& imh : MemberHouse)
	{
		if(MemberID > 0 && MemberID == imh.second.mOwnerMemberID)
			return imh.first;
	}
	return -1;
}

/* #########################################################################
	FUNCTIONS MEMBER HOUSING MEMBER 
######################################################################### */
// Покупка дома организации
void MemberSql::BuyGuildHouse(int MemberID, int HouseID)
{
	// проверяем есть ли дом, есть ли владелец у него
	if(GetMemberHouseID(MemberID) > 0 || HouseID <= 0 || MemberHouse[ HouseID ].mOwnerMemberID > 0) return;

	// ищем в базе данных дом
	boost::scoped_ptr<ResultSet> RES(SJK.SD("*", "tw_members_houses", "WHERE ID = '%d'", HouseID));
	if(!RES->next()) return;

	// проверяем деньги если не хватает пишет что не хватает
	int Price = RES->getInt("Price");
	if( Member[ MemberID ].mBank < Price)
	{
		GS()->ChatGuild(MemberID, "This Guild house requires {INT}gold!", &Price);
		return;
	}

	// устанавливаем владельца
	MemberHouse[ HouseID ].mOwnerMemberID = MemberID;
	SJK.UD("tw_members_houses", "OwnerMID = '%d' WHERE ID = '%d'", MemberID, HouseID);
	
	// снимаем деньги
	Member[ MemberID ].mBank -= Price;
	SJK.UD("tw_members", "Bank = '%d' WHERE ID = '%d'", Member[ MemberID ].mBank, MemberID);
	
	// остальное
	GS()->Chat(-1, "{STR} buyight guild house on {STR}!", Member[ MemberID ].mMemberName, GS()->Server()->GetWorldName(GS()->GetWorldID()));
	GS()->ChatDiscord(false, DC_SERVER_INFO, "Information", "{STR} buyight guild house on {STR}!", Member[ MemberID ].mMemberName, GS()->Server()->GetWorldName(GS()->GetWorldID()));
	AddHistoryGuild(MemberID, "Bought a house on '%s'.", GS()->Server()->GetWorldName(GS()->GetWorldID()));
}

// продажа дома организации
void MemberSql::SellGuildHouse(int MemberID)
{
	// проверяем дом
	int HouseID = GetMemberHouseID(MemberID);
	if(HouseID <= 0) return;	

	// ищем владельца дома
	boost::scoped_ptr<ResultSet> RES(SJK.SD("OwnerMID", "tw_members_houses", "WHERE ID = '%d'", HouseID));
	if(!RES->next()) return;

	// устанавливаем владельца на 0 и открываем дверь
	if(MemberHouse[HouseID].m_Door)
	{
		delete MemberHouse[HouseID].m_Door;
		MemberHouse[HouseID].m_Door = 0;
	}
	MemberHouse[ HouseID ].mOwnerMemberID = 0;
	SJK.UD("tw_members_houses", "OwnerMID = '0' WHERE ID = '%d'", HouseID);
	
	// возращаем деньги
	int SoldBack = MemberHouse[ HouseID ].mPrice;
	Member[ MemberID ].mBank += SoldBack;
	SJK.UD("tw_members", "Bank = '%d' WHERE ID = '%d'", Member[ MemberID ].mBank, MemberID);

	// остальное
	GS()->ChatGuild(MemberID, "House sold, {INT}gold returned in bank", &SoldBack);
	AddHistoryGuild(MemberID, "Lost a house on '%s'.", GS()->Server()->GetWorldName(GS()->GetWorldID()));
}

// меню продажи дома
void MemberSql::ShowBuyHouse(CPlayer *pPlayer, int MID)
{
	int ClientID = pPlayer->GetCID();
	int MemberID = pPlayer->Acc().MemberID;
	bool Leader = IsLeaderPlayer(pPlayer);

	GS()->AVH(ClientID, HMEMHOMEINFO, vec3(35,80,40), "Information Member Housing");
	GS()->AVM(ClientID, "null", NOPE, HMEMHOMEINFO, "Buying a house you will need to constantly the Treasury");
	GS()->AVM(ClientID, "null", NOPE, HMEMHOMEINFO, "In the intervals of time will be paid house");

	pPlayer->m_Colored = { 20, 20, 20 };
	
	if(MemberID == MID)
		GS()->AVM(ClientID, "null", NOPE, 0, "Guild Bank: {INT} Price: {INT}", &Member[ MemberID ].mBank, &MemberHouse[MID].mPrice);
	
	if(Leader && MemberID != MemberHouse[MID].mOwnerMemberID)
	{
		GS()->AVM(ClientID, "null", NOPE, 0, "Every day payment {INT} gold", &MemberHouse[MID].mEveryDay);
		GS()->AVM(ClientID, "BUYMEMBERHOUSE", MID, 0, "Buy this member house! Price: {INT}", &MemberHouse[MID].mPrice);
	}
}

// Действия над дверью
void MemberSql::ChangeStateDoor(int MemberID)
{
	if(Member.find(MemberID) == Member.end()) return;
	
	// парсим дом
	int HouseID = GetMemberHouseID(MemberID);
	if(MemberHouse.find(HouseID) == MemberHouse.end()) return;

	// если мир не равен данному
	if(MemberHouse[HouseID].mWorldID != GS()->GetWorldID())
	{
		GS()->ChatGuild(MemberID, "Change state door can only near your house.");	
		return;
	}

	// изменяем статистику двери
	if(MemberHouse[HouseID].m_Door) 
	{
		// дверь удаляем
		delete MemberHouse[HouseID].m_Door;
		MemberHouse[HouseID].m_Door = 0;
	}
	else
	{
		// создаем дверь
		MemberHouse[HouseID].m_Door = new MemberDoor(&GS()->m_World, vec2(MemberHouse[HouseID].mDoorX, MemberHouse[HouseID].mDoorY), MemberID);
	}

	// надпись если найдется игрок
	bool StateDoor = (MemberHouse[HouseID].m_Door);
	GS()->ChatGuild(MemberID, "{STR} the house for others.", (StateDoor ? "closed" : "opened"));
}

/* #########################################################################
	GLOBAL MEMBER  
######################################################################### */
// парсинг голосований для меню
bool MemberSql::OnParseVotingMenu(CPlayer *pPlayer, const char *CMD, const int VoteID, const int VoteID2, int Get, const char *GetText)
{
	int ClientID = pPlayer->GetCID();

	/* #########################################################################
		FUNCTIONS MEMBER 
	######################################################################### */
	if(PPSTR(CMD, "MLEADER") == 0)
	{
		// проверяем если не является лидером
		if(!IsLeaderPlayer(pPlayer) || Get != 134)
		{
			GS()->Chat(ClientID, "You have no access.");
			return true;
		}

		// устанавливаем нового лидера
		int MemberID = pPlayer->Acc().MemberID;
		SJK.UD("tw_members", "OwnerID = '%d' WHERE ID = '%d'", VoteID, MemberID);
		Member[MemberID].mOwnerID = VoteID;

		AddHistoryGuild(MemberID, "New guild leader '%s'.", Job()->PlayerName(VoteID));
		GS()->ChatGuild(MemberID, "Change leader {STR}->{STR}", GS()->Server()->ClientName(ClientID), Job()->PlayerName(VoteID));
		GS()->VResetVotes(ClientID, MEMBERMENU);
		return true;
	}

	// переместится домой
	if(PPSTR(CMD, "MSPAWN") == 0)
	{
		// если нет игрока в мире
		if(!pPlayer->GetCharacter()) return true;

		// проверяем если мир не является локальным
		const int MemberID = pPlayer->Acc().MemberID;
		const int HouseID = GetMemberHouseID(MemberID);
		const int WorldID = GetHouseWorldID(HouseID);
		if(WorldID != GS()->Server()->GetWorldID(ClientID))
		{
			// перемешаем в мир дома
			vec2 Position = GetPositionHouse(MemberID);
			pPlayer->Acc().TeleportX = Position.x;
			pPlayer->Acc().TeleportY = Position.y;
			GS()->Server()->ChangeWorld(ClientID, WorldID);
			return true;
		}
		else
		{
			// перемешаем в кординаты дома
			vec2 Position = GetPositionHouse(MemberID);
			pPlayer->GetCharacter()->ChangePosition(Position);
		}
		return true;
	}

	// кикнуть игрока из организации
	if(PPSTR(CMD, "MKICK") == 0)
	{
		// проверяем если не имеем прав на кик и приглашения
		if(!IsLeaderPlayer(pPlayer, MACCESSINVITEKICK))
		{
			GS()->Chat(ClientID, "You have no access.");
			return true;
		}

		// кикаем
		ExitGuild(VoteID);
		GS()->VResetVotes(ClientID, MEMBERMENU);
		return true;
	}

	// покупка улучшения стула
	if(PPSTR(CMD, "MUPGRADE") == 0)
	{
		// проверяем если нет доступа к дому и улучшениям
		if(!IsLeaderPlayer(pPlayer, MACCESSUPGHOUSE))
		{
			GS()->Chat(ClientID, "You have no access.");
			return true;
		}

		// улучшаем гильдию
		int MemberID = pPlayer->Acc().MemberID, UpgradeID = VoteID;
		if(UpgradeGuild(MemberID, UpgradeID))
		{
			int MemberCount = Member[MemberID].mUpgrades[ UpgradeID ]-1;
			GS()->Chat(ClientID, "Added ({INT}+1) {STR} to {STR}!", &MemberCount, UpgradeNames(UpgradeID).c_str(), Member[MemberID].mMemberName);
			GS()->VResetVotes(ClientID, MEMBERMENU);
		}
		else GS()->Chat(ClientID, "You don't have that much money in the Bank.");
		return true;
	}

	// добавка денег в банк организации
	if(PPSTR(CMD, "MMONEY") == 0)
	{
		// если добавляется менее 100
		if(Get < 100)
		{
			GS()->Chat(ClientID, "Minimal 100 gold.");
			return true;
		}

		// если у игрока нет денег
		if(pPlayer->CheckFailMoney(Get))
			return true;

		// добавляем деньги
		int MemberID = pPlayer->Acc().MemberID;
		if(AddMoneyBank(MemberID, Get))
		{
			SJK.UD("tw_accounts_data", "MemberDeposit = MemberDeposit + '%d' WHERE ID = '%d'", Get, pPlayer->Acc().AuthID);
			GS()->ChatGuild(MemberID, "{STR} deposit in treasury {INT}gold.", GS()->Server()->ClientName(ClientID), &Get);
			AddHistoryGuild(MemberID, "'%s' added to bank %dgold.", GS()->Server()->ClientName(ClientID), Get);
			GS()->VResetVotes(ClientID, MEMBERMENU);
		}
		return true;
	}

	/* #########################################################################
		FUNCTIONS MEMBER HOUSING MEMBER 
	######################################################################### */
	// покупка дома
	if(PPSTR(CMD, "BUYMEMBERHOUSE") == 0)
	{
		// проверяем если не является лидером
		if(!IsLeaderPlayer(pPlayer))
		{
			GS()->Chat(ClientID, "You have no access.");
			return true;
		}

		// покупаем дом
		int MemberID = pPlayer->Acc().MemberID;
		BuyGuildHouse(MemberID, VoteID);
		GS()->VResetVotes(ClientID, MAINMENU);
	}

	// продажа дома
	if(PPSTR(CMD, "MHOUSESELL") == 0)
	{
		// проверяем если не является лидером или число проверка не 777
		if(!IsLeaderPlayer(pPlayer) || Get != 777)
		{
			GS()->Chat(ClientID, "You have no access.");
			return true;
		}

		// продаем дом
		int MemberID = pPlayer->Acc().MemberID;
		SellGuildHouse(MemberID);
		GS()->VResetVotes(ClientID, MEMBERMENU);
		return true;
	}

	// дверь дома организации
	if(PPSTR(CMD, "MDOOR") == 0)
	{
		// проверяем если не имеет прав на управление домом
		if(!IsLeaderPlayer(pPlayer, MACCESSUPGHOUSE))
		{
			GS()->Chat(ClientID, "You have no access.");
			return true;
		}

		// открываем дом
		int MemberID = pPlayer->Acc().MemberID;
		ChangeStateDoor(MemberID);
		GS()->VResetVotes(ClientID, MEMBERMENU);
		return true;
	}

	/* #########################################################################
		FUNCTIONS MEMBER INVITES MEMBER 
	######################################################################### */
	// поиск гильдии по имени
	if(PPSTR(CMD, "MINVITENAME") == 0)
	{
		// если текст является нулем
		if(PPSTR(GetText, "NULL") == 0)
		{
			GS()->Chat(ClientID, "Use please another name.");
			return true;
		}

		// копируем имя гильдии в интерактив
		str_copy(CGS::InteractiveSub[ClientID].GuildName, GetText, sizeof(CGS::InteractiveSub[ClientID].GuildName));
		GS()->VResetVotes(ClientID, MEMBERMENU);
		return true;
	}

	// создать новое приглашение в гильдию
	if(PPSTR(CMD, "MINVITESEND") == 0)
	{
		// отправить приглашение если имеется отказать
		if(!AddInviteGuild(VoteID, pPlayer->Acc().AuthID))
		{
			GS()->Chat(ClientID, "You have already sent the invitation.");
			return true;
		}

		// если отправленно то пишет что отправили
		GS()->Chat(ClientID, "You sent the invitation to join.");		
		return true;
	}

	// принять приглошение
	if(PPSTR(CMD, "MINVITEACCEPT") == 0)
	{
		// проверяем если не имеет прав на приглашения и кик
		if(!IsLeaderPlayer(pPlayer, MACCESSINVITEKICK))
		{
			GS()->Chat(ClientID, "You have no access.");
			return true;
		}

		// принимаем приглашение
		SJK.DD("tw_members_invites", "WHERE MemberID = '%d' AND OwnerID = '%d'", VoteID, VoteID2);
		Job()->Inbox()->SendInbox(VoteID2, Member[ VoteID ].mMemberName, "You were accepted to join this guild");
		JoinGuild(VoteID2, VoteID);
		GS()->ResetVotes(ClientID, MEMBERMENU);
		return true;
	}

	// отклонить приглашение
	if(PPSTR(CMD, "MINVITEREJECT") == 0)
	{
		// проверяем если не имеет прав на приглашения и кик
		if(!IsLeaderPlayer(pPlayer, MACCESSINVITEKICK))
		{
			GS()->Chat(ClientID, "You have no access.");
			return true;
		}
		
		// отклоняем приглашение
		int MemberID = VoteID, OwnerID = VoteID2;
		GS()->Chat(ClientID, "You reject invite.");
		SJK.DD("tw_members_invites", "WHERE MemberID = '%d' AND OwnerID = '%d'", MemberID, OwnerID);
		Job()->Inbox()->SendInbox(OwnerID, Member[ MemberID ].mMemberName, "You were denied join this guild");
		GS()->ResetVotes(ClientID, MEMBERMENU);
		return true;
	}

	/* #########################################################################
		FUNCTIONS MEMBER RANK MEMBER 
	######################################################################### */
	// изменить поле ранга имени
	if(PPSTR(CMD, "MRANKNAME") == 0)
	{
		// проверяем является нулем
		if(PPSTR(GetText, "NULL") == 0)
		{
			GS()->Chat(ClientID, "Use please another name.");
			return true;
		}
		
		// устанавливаем текст полученый в ранг
		str_copy(CGS::InteractiveSub[ClientID].RankName, GetText, sizeof(CGS::InteractiveSub[ClientID].RankName));
		GS()->VResetVotes(ClientID, MEMBERRANK);
		return true;
	}
	
	// создать ранг
	if(PPSTR(CMD, "MRANKCREATE") == 0)
	{
		// проверяем если не лидер
		if(!IsLeaderPlayer(pPlayer))
		{
			GS()->Chat(ClientID, "You have no access.");
			return true;
		}

		// проверяем размер ранга
		if(str_length(CGS::InteractiveSub[ClientID].RankName) < 2)
		{
			GS()->Chat(ClientID, "Minimal symbols 2.");
			return true;
		}

		// создаем ранг
		int MemberID = pPlayer->Acc().MemberID;
		AddRank(MemberID, CGS::InteractiveSub[ClientID].RankName);
		GS()->ClearInteractiveSub(ClientID);
		GS()->VResetVotes(ClientID, MEMBERRANK);
		return true;
	}

	// удалить ранг
	if(PPSTR(CMD, "MRANKDELETE") == 0)
	{
		// проверяем если не лидер
		if(!IsLeaderPlayer(pPlayer))
		{
			GS()->Chat(ClientID, "You have no access.");
			return true;
		}

		// удаляем ранг
		int MemberID = pPlayer->Acc().MemberID;
		DeleteRank(VoteID, MemberID);
		GS()->VResetVotes(ClientID, MEMBERRANK);
		return true;
	}

	// изменить доступ рангу
	if(PPSTR(CMD, "MRANKACCESS") == 0)
	{
		// проверяем если не лидер
		if(!IsLeaderPlayer(pPlayer))
		{
			GS()->Chat(ClientID, "You have no access.");
			return true;
		}

		// меняем доступ рангу
		ChangeRankAccess(VoteID);
		GS()->VResetVotes(ClientID, MEMBERRANK);		
		return true;
	}

	// установить ранг
	if(PPSTR(CMD, "MRANKSET") == 0)
	{
		// проверяем если не лидер
		if(!IsLeaderPlayer(pPlayer))
		{
			GS()->Chat(ClientID, "You have no access.");
			return true;
		}

		// проверяем размер ранга
		if(str_length(CGS::InteractiveSub[ClientID].RankName) < 2)
		{
			GS()->Chat(ClientID, "Minimal symbols 2.");
			return true;
		}

		// устанавливаем рангу имя
		int MemberID = pPlayer->Acc().MemberID;
		ChangeRank(VoteID, MemberID, CGS::InteractiveSub[ClientID].RankName);
		GS()->ClearInteractiveSub(ClientID);
		GS()->VResetVotes(ClientID, MEMBERRANK);
		return true;
	}

	// изменить имя ранга
	if(PPSTR(CMD, "MRANKCHANGE") == 0)
	{
		// проверяем если не лидер
		if(!IsLeaderPlayer(pPlayer))
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
	return false;
}

/* #########################################################################
	HOUSE ENTITIES MEMBER  
######################################################################### */
MemberDoor::MemberDoor(CGameWorld *pGameWorld, vec2 Pos, int MemberID)
: CEntity(pGameWorld, CGameWorld::ENTTYPE_HOUSEDOOR, Pos)
{
	m_MemberID = MemberID;

	m_To = vec2(Pos.x, Pos.y-200);
	m_Pos.y += 30;

	GameWorld()->InsertEntity(this);
}

MemberDoor::~MemberDoor() {}

void MemberDoor::Tick()
{
	for(CCharacter *pChar = (CCharacter*) GameWorld()->FindFirst(CGameWorld::ENTTYPE_CHARACTER); pChar; pChar = (CCharacter *)pChar->TypeNext())
	{
		const int ClientID = pChar->GetPlayer()->GetCID();
		CPlayer *pPlayer = pChar->GetPlayer();

		if(m_MemberID == pPlayer->Acc().MemberID) continue;
	
		vec2 IntersectPos = closest_point_on_line(m_Pos, m_To, pChar->m_Core.m_Pos);
		float Distance = distance(IntersectPos, pChar->m_Core.m_Pos);
	
		// снижаем скокрость
		if(Distance < 64.0f && length(pChar->m_Core.m_Vel) >= 64.0)
			pChar->m_Core.m_Vel = vec2(0,0);

		// проверяем дистанцию
		if(Distance < 30.0f) 
		{
			vec2 Dir = normalize(pChar->m_Core.m_Pos - IntersectPos);
			float a = (30.0f*1.45f - Distance);
			float Velocity = 0.5f;
			if (length(pChar->m_Core.m_Vel) > 0.0001)
				Velocity = 1-(dot(normalize(pChar->m_Core.m_Vel), Dir)+1)/4;
		
			pChar->m_Core.m_Vel += (Dir*a*(Velocity*0.75f))*0.85f;
			pChar->m_Core.m_Pos = pChar->m_OldPos + Dir;
		}
	}
}

void MemberDoor::Snap(int SnappingClient)
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