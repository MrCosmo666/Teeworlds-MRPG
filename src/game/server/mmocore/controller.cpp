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
	m_Components.add(m_pAccMain = new AccountMainSql());
	m_Components.add(m_pBotsInfo = new ContextBots());
	m_Components.add(m_pAccMiner = new MinerAccSql());
	m_Components.add(m_pAccPlant = new PlantsAccSql());
	m_Components.add(m_pCraftJob = new CraftJob());
	m_Components.add(m_pDungeonJob = new DungeonJob());
	m_Components.add(m_pHouseJob = new HouseJob());
	m_Components.add(m_pMailBoxJob = new MailBoxJob());
	m_Components.add(m_pItemWork = new ItemSql());
	m_Components.add(m_pGuildJob = new GuildJob());
	m_Components.add(m_pQuest = new QuestBase());
	m_Components.add(m_pShopmail = new ShopMailSql());
	m_Components.add(m_pSkillJob = new SkillJob());
	m_Components.add(m_pStorageWork = new StorageSql());
	m_Components.add(m_pTeleportsWork = new TeleportsSql());
	m_Components.add(m_pWorldSwapJob = new WorldSwapJob());

	for(auto& component : m_Components.m_paComponents)
	{
		component->m_Job = this;
		component->m_GameServer = pGameServer;

		if(m_pGameServer->GetWorldID() == LAST_WORLD)
			component->OnInitGlobal();

		char aLocalSelect[128];
		str_format(aLocalSelect, sizeof(aLocalSelect), "WHERE WorldID = '%d'", m_pGameServer->GetWorldID());
		component->OnInitLocal(aLocalSelect);
	}
	m_pBotsInfo->LoadGlobalBots();
}

SqlController::~SqlController()
{
	SJK.DisconnectConnectionHeap();
	m_Components.clear();
}

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

bool SqlController::OnPlayerHandleMainMenu(int ClientID, int Menulist, bool ReplaceMenu)
{
	CPlayer *pPlayer = GS()->GetPlayer(ClientID);
	if(!pPlayer || !pPlayer->IsAuthed()) return false;

	for(auto& component : m_Components.m_paComponents)
	{
		if(component->OnPlayerHandleMainMenu(pPlayer, Menulist, ReplaceMenu))
			return true;
	}
	return false;
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

void SqlController::ResetClientData(int ClientID)
{
	for (auto& component : m_Components.m_paComponents)
		component->OnResetClientData(ClientID);
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
		SJK.UD("tw_accounts_data", "GuildID = '%d', GuildRank = '%d' WHERE ID = '%d'", pPlayer->Acc().GuildID, pPlayer->Acc().GuildRank, pPlayer->Acc().AuthID);
		return;			
	}

	// сохранение мира позиции
	else if(Table == SAVEPOSITION)
	{
		// запрет в данже сохранять позицию
		if (GS()->DungeonID() > 0) return;

		SJK.UD("tw_accounts_data", "WorldID = '%d' WHERE ID = '%d'", GS()->Server()->GetWorldID(ClientID), pPlayer->Acc().AuthID);
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
	char aLoadingBuf[128];
	str_format(aLoadingBuf, sizeof(aLoadingBuf), "Loaded %d %s | CK WorldID %d.", LoadCount, Loading, GS()->GetWorldID());
	GS()->Console()->Print(IConsole::OUTPUT_LEVEL_ADDINFO, "LOAD DB", aLoadingBuf);
}

void SqlController::ShowTopList(CPlayer* pPlayer, int TypeID)
{
	int ClientID = pPlayer->GetCID();
	pPlayer->m_Colored = { 10, 10, 10 };
	if(TypeID == ToplistTypes::GUILDS_LEVELING)
	{
		boost::scoped_ptr<ResultSet> RES(SJK.SD("*", "tw_guilds", "ORDER BY Level DESC LIMIT 10"));
		while (RES->next())
		{
			char NameGuild[64];
			int Rank = RES->getRow();
			int Level = RES->getInt("Level");
			str_copy(NameGuild, RES->getString("GuildName").c_str(), sizeof(NameGuild));
			GS()->AVL(ClientID, "null", "{INT}. {STR} : Level {INT}", &Rank, NameGuild, &Level);
		}
	}
	else if (TypeID == ToplistTypes::GUILDS_WEALTHY)
	{
		boost::scoped_ptr<ResultSet> RES(SJK.SD("*", "tw_guilds", "ORDER BY Bank DESC LIMIT 10"));
		while (RES->next())
		{
			char NameGuild[64];
			int Rank = RES->getRow();
			int Gold = RES->getInt("Bank");
			str_copy(NameGuild, RES->getString("GuildName").c_str(), sizeof(NameGuild));
			GS()->AVL(ClientID, "null", "{INT}. {STR} : Gold {INT}", &Rank, NameGuild, &Gold);
		}
	}
	else if (TypeID == ToplistTypes::PLAYERS_LEVELING)
	{
		boost::scoped_ptr<ResultSet> RES(SJK.SD("*", "tw_accounts_data", "ORDER BY Level DESC LIMIT 10"));
		while (RES->next())
		{
			char Nick[64];
			int Rank = RES->getRow();
			int Level = RES->getInt("Level");
			str_copy(Nick, RES->getString("Nick").c_str(), sizeof(Nick));
			GS()->AVL(ClientID, "null", "{INT}. {STR} : Level {INT}", &Rank, Nick, &Level);
		}
	}
}