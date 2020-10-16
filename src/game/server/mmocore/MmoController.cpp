/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#include <thread>
#include <teeother/system/string.h>

#include <game/server/gamecontext.h>
#include "MmoController.h"

#include "ComponentsCore/AetherJob.h"

using namespace sqlstr;

MmoController::MmoController(CGS *pGameServer) : m_pGameServer(pGameServer)
{
	// order
	m_Components.add(m_pBotsInfo = new BotJob());
	m_Components.add(m_pItemWork = new InventoryJob());
	m_Components.add(m_pCraftJob = new CraftJob());
	m_Components.add(m_pStorageWork = new StorageJob());
	m_Components.add(m_pShopmail = new ShopJob());
	m_Components.add(m_pQuest = new QuestJob());
	m_Components.add(m_pDungeonJob = new DungeonJob());
	m_Components.add(new AetherJob());
	m_Components.add(m_pWorldSwapJob = new WorldSwapJob());
	m_Components.add(m_pHouseJob = new HouseJob());
	m_Components.add(m_pGuildJob = new GuildJob());
	m_Components.add(m_pSkillJob = new SkillsJob());
	m_Components.add(m_pAccMain = new AccountMainJob());
	m_Components.add(m_pAccMiner = new AccountMinerJob());
	m_Components.add(m_pAccPlant = new AccountPlantJob());
	m_Components.add(m_pMailBoxJob = new MailBoxJob());

	for(auto& component : m_Components.m_paComponents)
	{
		component->m_Job = this;
		component->m_GameServer = pGameServer;

		if(m_pGameServer->GetWorldID() == LAST_WORLD)
			component->OnInit();

		char aLocalSelect[64];
		str_format(aLocalSelect, sizeof(aLocalSelect), "WHERE WorldID = '%d'", m_pGameServer->GetWorldID());
		component->OnInitWorld(aLocalSelect);
	}
}

MmoController::~MmoController()
{
	SJK.DisconnectConnectionHeap();
	m_Components.clear();
}

void MmoController::OnTick()
{
	for(auto& component : m_Components.m_paComponents)
		component->OnTick();
}

void MmoController::OnInitAccount(int ClientID)
{
	CPlayer *pPlayer = GS()->GetPlayer(ClientID);
	if(!pPlayer || !pPlayer->IsAuthed()) 
		return;

	for(auto& component : m_Components.m_paComponents)
		component->OnInitAccount(pPlayer);
}

bool MmoController::OnPlayerHandleMainMenu(int ClientID, int Menulist, bool ReplaceMenu)
{
	CPlayer *pPlayer = GS()->GetPlayer(ClientID);
	if(!pPlayer || !pPlayer->IsAuthed()) 
		return true;

	for(auto& component : m_Components.m_paComponents)
	{
		if(component->OnHandleMenulist(pPlayer, Menulist, ReplaceMenu))
			return true;
	}
	return false;
}

bool MmoController::OnPlayerHandleTile(CCharacter *pChr, int IndexCollision)
{
	if(!pChr || !pChr->IsAlive()) 
		return true;

	for(auto & component : m_Components.m_paComponents)
	{
		if(component->OnHandleTile(pChr, IndexCollision))
			return true;
	}
	return false;
}

bool MmoController::OnParsingVoteCommands(CPlayer *pPlayer, const char *CMD, const int VoteID, const int VoteID2, int Get, const char *GetText)
{
	if(!pPlayer)
		return true;

	for(auto& component : m_Components.m_paComponents)
	{
		if(component->OnParsingVoteCommands(pPlayer, CMD, VoteID, VoteID2, Get, GetText))
			return true;
	}
	return false;
}

void MmoController::OnMessage(int MsgID, void *pRawMsg, int ClientID)
{
	if(!pRawMsg)
		return;

	for(auto& component : m_Components.m_paComponents)
		component->OnMessage(MsgID, pRawMsg, ClientID);
}

void MmoController::ResetClientData(int ClientID)
{
	for (auto& component : m_Components.m_paComponents)
		component->OnResetClient(ClientID);
}

// saving account
void MmoController::SaveAccount(CPlayer *pPlayer, int Table)
{
	if(!pPlayer->IsAuthed()) 
		return;
	
	if(Table == SaveType::SAVE_STATS)
	{
		const int EquipDiscord = pPlayer->GetEquippedItemID(EQUIP_DISCORD);
		SJK.UD("tw_accounts_data", "Level = '%d', Exp = '%d', DiscordEquip = '%d' WHERE ID = '%d'",
			pPlayer->Acc().m_Level, pPlayer->Acc().m_Exp, EquipDiscord, pPlayer->Acc().m_AuthID);
	}
	else if(Table == SaveType::SAVE_UPGRADES)
	{
		char aBuf[64];
		dynamic_string Buffer;
		for(const auto& at : CGS::ms_aAttributsInfo)
		{
			if(str_comp_nocase(at.second.m_aFieldName, "unfield") == 0) 
				continue;
			str_format(aBuf, sizeof(aBuf), ", %s = '%d' ", at.second.m_aFieldName, pPlayer->Acc().m_aStats[at.first]);
			Buffer.append_at(Buffer.length(), aBuf);
		}

		SJK.UD("tw_accounts_data", "Upgrade = '%d' %s WHERE ID = '%d'", pPlayer->Acc().m_Upgrade, Buffer.buffer(), pPlayer->Acc().m_AuthID);
		Buffer.clear();
	}
	else if(Table == SaveType::SAVE_PLANT_DATA)
	{
		char aBuf[64];
		dynamic_string Buffer;
		for(int i = 0; i < PLANT::NUM_PLANT; i++) {
			str_format(aBuf, sizeof(aBuf), "%s = '%d' %s", str_PLANT((PLANT) i), pPlayer->Acc().m_aPlant[i], (i == NUM_PLANT-1 ? "" : ", "));
			Buffer.append_at(Buffer.length(), aBuf);
		}
		SJK.UD("tw_accounts_plants", "%s WHERE AccountID = '%d'", Buffer.buffer(), pPlayer->Acc().m_AuthID);
		Buffer.clear();
	}
	else if(Table == SaveType::SAVE_MINER_DATA)
	{
		char aBuf[64];
		dynamic_string Buffer;
		for(int i = 0; i < MINER::NUM_MINER; i++) {
			str_format(aBuf, sizeof(aBuf), "%s = '%d' %s", str_MINER((MINER) i), pPlayer->Acc().m_aMiner[i], (i == NUM_MINER-1 ? "" : ", "));
			Buffer.append_at(Buffer.length(), aBuf);
		}
		SJK.UD("tw_accounts_miner", "%s WHERE AccountID = '%d'", Buffer.buffer(), pPlayer->Acc().m_AuthID);
		Buffer.clear();
	}
	else if(Table == SaveType::SAVE_GUILD_DATA)
	{
		SJK.UD("tw_accounts_data", "GuildID = '%d', GuildRank = '%d' WHERE ID = '%d'", pPlayer->Acc().m_GuildID, pPlayer->Acc().m_GuildRank, pPlayer->Acc().m_AuthID);	
	}
	else if(Table == SaveType::SAVE_POSITION)
	{
		int LatestCorrectWorldID = Account()->GetLastHistoryCorrectWorldID(pPlayer);
		SJK.UD("tw_accounts_data", "WorldID = '%d' WHERE ID = '%d'", LatestCorrectWorldID, pPlayer->Acc().m_AuthID);
	}
	else if(Table == SaveType::SAVE_LANGUAGE)
	{
		SJK.UD("tw_accounts", "Language = '%s' WHERE ID = '%d'", pPlayer->GetLanguage(), pPlayer->Acc().m_AuthID);
	}
	else
	{
		SJK.UD("tw_accounts", "Username = '%s' WHERE ID = '%d'", pPlayer->Acc().m_aLogin, pPlayer->Acc().m_AuthID);
	}
}

void MmoController::LoadLogicWorld()
{
	std::shared_ptr<ResultSet> RES(SJK.SD("*", "tw_logicworld", "WHERE WorldID = '%d'", GS()->GetWorldID()));
	while(RES->next())
	{
		const int Type = (int)RES->getInt("MobID"), Mode = (int)RES->getInt("Mode"), Health = (int)RES->getInt("ParseInt");
		const vec2 Position = vec2(RES->getInt("PosX"), RES->getInt("PosY"));
		GS()->m_pController->CreateLogic(Type, Mode, Position, Health);
	}
}

char SaveNick[32];
const char* MmoController::PlayerName(int AccountID)
{
	std::shared_ptr<ResultSet> RES(SJK.SD("Nick", "tw_accounts_data", "WHERE ID = '%d'", AccountID));
	if(RES->next())
	{
		str_copy(SaveNick, RES->getString("Nick").c_str(), sizeof(SaveNick));
		return SaveNick;
	}
	return "No found!";
}

void MmoController::ShowLoadingProgress(const char* pLoading, int Size)
{
	char aLoadingBuf[128];
	str_format(aLoadingBuf, sizeof(aLoadingBuf), "Loaded %d %s | CK WorldID %d.", Size, pLoading, GS()->GetWorldID());
	GS()->Console()->Print(IConsole::OUTPUT_LEVEL_ADDINFO, "LOAD DB", aLoadingBuf);
}

void MmoController::ShowTopList(CPlayer* pPlayer, int TypeID)
{
	const int ClientID = pPlayer->GetCID();
	pPlayer->m_Colored = SMALL_LIGHT_GRAY_COLOR;
	if(TypeID == ToplistTypes::GUILDS_LEVELING)
	{
		std::shared_ptr<ResultSet> RES(SJK.SD("*", "tw_guilds", "ORDER BY Level DESC, Experience DESC LIMIT 10"));
		while (RES->next())
		{
			char NameGuild[64];
			const int Rank = RES->getRow();
			const int Level = RES->getInt("Level");
			const int Experience = RES->getInt("Experience");
			str_copy(NameGuild, RES->getString("GuildName").c_str(), sizeof(NameGuild));
			GS()->AVL(ClientID, "null", "{INT}. {STR} :: Level {INT} : Exp {INT}", &Rank, NameGuild, &Level, &Experience);
		}
	}
	else if (TypeID == ToplistTypes::GUILDS_WEALTHY)
	{
		std::shared_ptr<ResultSet> RES(SJK.SD("*", "tw_guilds", "ORDER BY Bank DESC LIMIT 10"));
		while (RES->next())
		{
			char NameGuild[64];
			const int Rank = RES->getRow();
			const int Gold = RES->getInt("Bank");
			str_copy(NameGuild, RES->getString("GuildName").c_str(), sizeof(NameGuild));
			GS()->AVL(ClientID, "null", "{INT}. {STR} :: Gold {INT}", &Rank, NameGuild, &Gold);
		}
	}
	else if (TypeID == ToplistTypes::PLAYERS_LEVELING)
	{
		std::shared_ptr<ResultSet> RES(SJK.SD("*", "tw_accounts_data", "ORDER BY Level DESC, Exp DESC LIMIT 10"));
		while (RES->next())
		{
			char Nick[64];
			const int Rank = RES->getRow();
			const int Level = RES->getInt("Level");
			const int Experience = RES->getInt("Exp");
			str_copy(Nick, RES->getString("Nick").c_str(), sizeof(Nick));
			GS()->AVL(ClientID, "null", "{INT}. {STR} :: Level {INT} : Exp {INT}", &Rank, Nick, &Level, &Experience);
		}
	}
	else if (TypeID == ToplistTypes::PLAYERS_WEALTHY)
	{
		std::shared_ptr<ResultSet> RES(SJK.SD("*", "tw_accounts_items", "WHERE ItemID = '%d' ORDER BY Count DESC LIMIT 10", (int)itGold));
		while (RES->next())
		{
			char Nick[64];
			const int Rank = RES->getRow();
			const int Gold = RES->getInt("Count");
			const int OwnerID = RES->getInt("OwnerID");
			str_copy(Nick, PlayerName(OwnerID), sizeof(Nick));
			GS()->AVL(ClientID, "null", "{INT}. {STR} :: Gold {INT}", &Rank, Nick, &Gold);
		}
	}
}