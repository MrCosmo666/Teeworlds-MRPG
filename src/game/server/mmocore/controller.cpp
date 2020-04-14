/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#include <thread>
#include <engine/shared/config.h>

#include <game/server/gamecontext.h>
#include "controller.h"

using namespace sqlstr;

// список хранения всех компонентов Sql Work
SqlController::SqlController(CGS *pGameServer) : m_pGameServer(pGameServer)
{
	// добавляем компоненты в список
	m_Components.add(m_pAccMain = new AccountMainSql());
	m_Components.add(m_pBotsInfo = new ContextBots());
	m_Components.add(m_pAccMiner = new MinerAccSql());
	m_Components.add(m_pAccPlant = new PlantsAccSql());
	m_Components.add(m_pAccRelax = new SpaRelaxSql());
	m_Components.add(m_pCraftWork = new CraftSql());
	m_Components.add(m_pHouseWork = new HouseSql());
	m_Components.add(m_pInbox = new InboxSql());
	m_Components.add(m_pItemWork = new ItemSql());
	m_Components.add(m_pMemberWork = new MemberSql());
	m_Components.add(m_pQuest = new QuestBase());
	m_Components.add(m_pShopmail = new ShopMailSql());
	m_Components.add(m_pSkillsWork = new SkillsSql());
	m_Components.add(m_pStorageWork = new StorageSql());
	m_Components.add(m_pTeleportsWork = new TeleportsSql());
	m_Components.add(m_pWorldSwapWork = new WorldSwapSql());

	// инициализируем объекты
	for(auto& component : m_Components.m_paComponents)
	{
		component->m_Job = this;
		component->m_GameServer = pGameServer;

		// загрузить все статичные данные в конце инициализации
		if(GS()->GetWorldID() == LAST_WORLD)
			component->OnInitGlobal();

		// загрузить все статичные данные каждого из миров
		char aLocalSelect[128];
		str_format(aLocalSelect, sizeof(aLocalSelect), "WHERE WorldID = '%d'", GS()->GetWorldID());
		component->OnInitLocal(aLocalSelect);
	}
	LoadDungeons();
	m_pBotsInfo->LoadGlobalBots();
}

SqlController::~SqlController()
{
	SJK.DisconnectConnectionHeap();
	m_Components.clear();
}

// Загрузить все данные в локальном мире
void SqlController::LoadFullSystems()
{
	for(auto& component : m_Components.m_paComponents)
	{
		if(GS()->GetWorldID() == LAST_WORLD)
			component->OnInitGlobal();

		char aLocalSelect[128];
		str_format(aLocalSelect, sizeof(aLocalSelect), "WHERE WorldID = '%d'", GS()->GetWorldID());
		component->OnInitLocal(aLocalSelect);
	}
}

// Тик всех компонентов
void SqlController::OnTick()
{
	// весь тик компонентов
	for(auto& component : m_Components.m_paComponents)
	{
		component->OnTick();
		if(GS()->GetWorldID() == LOCALWORLD)
			component->OnTickLocalWorld();
	}

	// тик в локальном мире
	if(GS()->GetWorldID() == LOCALWORLD) 
	{
		// плата за дома и все владения
		if(GS()->Server()->CheckWorldTime(23, 30))
		{
			for(auto& component : m_Components.m_paComponents)
				component->OnPaymentTime();
		}
	}
}

void SqlController::OnInitAccount(int ClientID)
{
	CPlayer *pPlayer = GS()->GetPlayer(ClientID);
	if(!pPlayer || !pPlayer->IsAuthed()) return;

	for(auto& component : m_Components.m_paComponents)
		component->OnInitAccount(pPlayer);
}

void SqlController::OnPlayerHandleMainMenu(int ClientID, int Menulist)
{
	CPlayer *pPlayer = GS()->GetPlayer(ClientID);
	if(!pPlayer || !pPlayer->IsAuthed()) return;

	for(auto& component : m_Components.m_paComponents)
	{
		if(component->OnPlayerHandleMainMenu(pPlayer, Menulist))
			return;
	}
}

bool SqlController::OnPlayerHandleTile(CCharacter *pChr, int IndexCollision)
{
	if(!pChr) return true;
	for(auto & component : m_Components.m_paComponents)
	{
		if(component->OnPlayerHandleTile(pChr, IndexCollision))
			return true;
	}
	return false;
}

bool SqlController::OnParseFullVote(CPlayer *pPlayer, const char *CMD, const int VoteID, const int VoteID2, int Get, const char *GetText)
{
	for(auto& component : m_Components.m_paComponents)
	{
		if(component->OnParseVotingMenu(pPlayer, CMD, VoteID, VoteID2, Get, GetText))
			return true;
	}
	return false;
}

bool SqlController::OnMessage(int MsgID, void *pRawMsg, int ClientID)
{
	if(!pRawMsg)
		return true;

	for(auto& component : m_Components.m_paComponents)
	{
		if(component->OnMessage(MsgID, pRawMsg, ClientID))
			return true;
	}	
	return false;
}

// Показать продоваемые все дома и бизнесы
void SqlController::ShowBussinesHousesSell(CPlayer *pPlayer)
{
	// показываем лист продоваемых домов
	const int ClientID = pPlayer->GetCID();
	GS()->AVH(ClientID, HHOUSEAVAILABLE, vec3(52,26,80), _("List of available houses"), NULL);
	GS()->AVM(ClientID, "null", NOPE, HHOUSEAVAILABLE, _("Symbols: HL - House level"), NULL);
	boost::scoped_ptr<ResultSet> RES(SJK.SD("*", "tw_homes WHERE OwnerID < '1' ORDER BY Class DESC"));
	while(RES->next())
	{
		const int HouseID = RES->getInt("ID");
		const int WorldID = RES->getInt("WorldID");
		int Price = RES->getInt("Price");
		int Level = RES->getInt("FarmLevel");

		GS()->AVM(ClientID, "null", NOPE, HHOUSEAVAILABLE, "HL{INT} {STR} {INT}gold {STR}", &Level, House()->ClassName(HouseID), &Price, GS()->Server()->GetWorldName(WorldID));
	}
	GS()->AV(ClientID, "null", "");
	
	// показываем лист продоваемых бизнесов
	GS()->AVH(ClientID, HBUSINESSAVAILABLE, vec3(24,26,80), _("List of available business"), NULL);
	boost::scoped_ptr<ResultSet> RES2(SJK.SD("*", "tw_storages WHERE OwnerID < '1' AND MonsterSubType < '1' AND ID > '1' ORDER BY WorldID DESC"));
	while(RES2->next())
	{
		const int StorageID = RES2->getInt("ID");
		const int WorldID = RES2->getInt("WorldID");
		int Price = RES2->getInt("Price");

		GS()->AVM(ClientID, "null", NOPE, HBUSINESSAVAILABLE, "{STR} {INT}gold {STR}", Storage()->StorageName(StorageID), &Price, GS()->Server()->GetWorldName(WorldID));
	}
}

// Сохранение аккаунта
void SqlController::SaveAccount(CPlayer *pPlayer, int Table)
{
	const int ClientID = pPlayer->GetCID();
	if(!pPlayer->IsAuthed()) return;
	
	// сохранение статистики
	if(Table == SAVESTATS)
	{
		const int EquipDiscord = pPlayer->GetItemEquip(EQUIP_DISCORD);
		SJK.UD("tw_accounts_data", "Level = '%d', Exp = '%d', DiscordEquip = '%d' WHERE ID = '%d'",
			pPlayer->Acc().Level, pPlayer->Acc().Exp, EquipDiscord, pPlayer->Acc().AuthID);
		return;
	}

	// сохранение апгрейдов
	else if(Table == SAVEUPGRADES)
	{
		char aBuf[64];
		dynamic_string Buffer;
		
		// добавляем в буфер всю статистику что требуется обновить
		for(const auto& at : CGS::AttributInfo)
		{
			if(str_comp_nocase(at.second.FieldName, "unfield") == 0) continue;
			str_format(aBuf, sizeof(aBuf), ", %s = '%d' ", at.second.FieldName, pPlayer->Acc().Stats[at.first]);
			Buffer.append_at(Buffer.length(), aBuf);
		}

		// обновляем статистику и очищаем буфер
		SJK.UD("tw_accounts_data", "Upgrade = '%d' %s WHERE ID = '%d'", pPlayer->Acc().Upgrade, Buffer.buffer(), pPlayer->Acc().AuthID);
		Buffer.clear();
		return;
	}

	// сохранение спа аккаунта
	else if(Table == SAVESPAACCOUNT)
	{
		char aBuf[64];
		dynamic_string Buffer;
		for(int i = 0; i < RELAX::NUM_RELAX; i++) {
			str_format(aBuf, sizeof(aBuf), "%s = '%d' %s", str_RELAX((RELAX) i), pPlayer->Acc().Relax[i], (i == NUM_RELAX-1 ? "" : ", "));
			Buffer.append_at(Buffer.length(), aBuf);
		}
		SJK.UD("tw_accounts_relax", "%s WHERE AccountID = '%d'", Buffer.buffer(), pPlayer->Acc().AuthID);
		Buffer.clear();
		return;
	}

	// сохранение плант аккаунта
	else if(Table == SAVEPLANTACCOUNT)
	{
		char aBuf[64];
		dynamic_string Buffer;
		for(int i = 0; i < PLANT::NUM_PLANT; i++) {
			str_format(aBuf, sizeof(aBuf), "%s = '%d' %s", str_PLANT((PLANT) i), pPlayer->Acc().Plant[i], (i == NUM_PLANT-1 ? "" : ", "));
			Buffer.append_at(Buffer.length(), aBuf);
		}
		SJK.UD("tw_accounts_plants", "%s WHERE AccountID = '%d'", Buffer.buffer(), pPlayer->Acc().AuthID);
		Buffer.clear();
		return;
	}

	// сохранение минер аккаунта
	else if(Table == SAVEMINERACCOUNT)
	{
		char aBuf[64];
		dynamic_string Buffer;
		for(int i = 0; i < MINER::NUM_MINER; i++) {
			str_format(aBuf, sizeof(aBuf), "%s = '%d' %s", str_MINER((MINER) i), pPlayer->Acc().Miner[i], (i == NUM_MINER-1 ? "" : ", "));
			Buffer.append_at(Buffer.length(), aBuf);
		}
		SJK.UD("tw_accounts_miner", "%s WHERE AccountID = '%d'", Buffer.buffer(), pPlayer->Acc().AuthID);
		Buffer.clear();
		return;		
	}

	// сохранение гильдии даты
	else if(Table == SAVEMEMBERDATA)
	{
		SJK.UD("tw_accounts_data", "MemberID = '%d', MemberRank = '%d' WHERE ID = '%d'", pPlayer->Acc().MemberID, pPlayer->Acc().MemberRank, pPlayer->Acc().AuthID);
		return;			
	}

	// сохранение мира позиции
	else if(Table == SAVEPOSITION)
	{
		SJK.UD("tw_accounts", "WorldID = '%d' WHERE ID = '%d'", GS()->Server()->GetWorldID(ClientID), pPlayer->Acc().AuthID);
		return;
	}

	// сохранение аккаунта
	else
	{
		SJK.UD("tw_accounts", "Username = '%s', Password = '%s' WHERE ID = '%d'", pPlayer->Acc().Login, pPlayer->Acc().Password, pPlayer->Acc().AuthID);
		return;
	}
}

void SqlController::LoadLogicWorld()
{
	boost::scoped_ptr<ResultSet> RES(SJK.SD("*", "tw_logicworld", "WHERE WorldID = '%d'", GS()->GetWorldID()));
	while(RES->next())
	{
		int Type = (int)RES->getInt("MobID"), Mode = (int)RES->getInt("Mode"), Health = (int)RES->getInt("ParseInt");
		vec2 Position = vec2(RES->getInt("PosX"), RES->getInt("PosY"));
		GS()->m_pController->CreateLogic(Type, Mode, Position, Health);
	}
}

void SqlController::LoadDungeons()
{
	boost::scoped_ptr<ResultSet> RES(SJK.SD("*", "tw_dungeons"));
	while(RES->next())
	{
		int ID = RES->getInt("ID");
		str_copy(CGS::Dungeon[ID].Name, RES->getString("Name").c_str(), sizeof(CGS::Dungeon[ID].Name));

		// загружаем целечисленые типы
		CGS::Dungeon[ID].Level = RES->getInt("Level");
		CGS::Dungeon[ID].DoorX = RES->getInt("DoorX");
		CGS::Dungeon[ID].DoorY = RES->getInt("DoorY"); 
		CGS::Dungeon[ID].WorldID = RES->getInt("WorldID");
	}	
}

char SaveNick[64];
const char* SqlController::PlayerName(int AccountID)
{
	boost::scoped_ptr<ResultSet> RES(SJK.SD("Nick", "tw_accounts_data", "WHERE ID = '%d'", AccountID));
	if(RES->next())
	{
		str_copy(SaveNick, RES->getString("Nick").c_str(), sizeof(SaveNick));
		return SaveNick;
	}
	return "No found!";
}

void SqlController::ShowLoadingProgress(const char *Loading, int LoadCount)
{	
	GS()->Console()->Print(IConsole::OUTPUT_LEVEL_ADDINFO, "LOAD DB", "------------------------------------");
	// - - - - - - - - -
	char aLoadingBuf[128];
	str_format(aLoadingBuf, sizeof(aLoadingBuf), "Loaded %d %s | CK WorldID %d.", LoadCount, Loading, GS()->GetWorldID());
	GS()->Console()->Print(IConsole::OUTPUT_LEVEL_ADDINFO, "LOAD DB", aLoadingBuf);
	// - - - - - - - - -
	GS()->Console()->Print(IConsole::OUTPUT_LEVEL_ADDINFO, "LOAD DB", "------------------------------------");
}