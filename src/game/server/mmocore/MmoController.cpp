/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#include "MmoController.h"

#include <engine/storage.h>
#include <engine/shared/datafile.h>
#include <game/server/gamecontext.h>
#include <teeother/system/string.h>

#include "Components/Accounts/AccountCore.h"
#include "Components/Accounts/AccountMinerCore.h"
#include "Components/Accounts/AccountPlantCore.h"
#include "Components/Aethers/AetherCore.h"
#include "Components/Bots/BotCore.h"
#include "Components/Crafts/CraftCore.h"
#include "Components/Dungeons/DungeonJob.h"
#include "Components/Guilds/GuildJob.h"
#include "Components/Houses/HouseCore.h"
#include "Components/Inventory/InventoryCore.h"
#include "Components/Mails/MailBoxCore.h"
#include "Components/Quests/QuestCore.h"
#include "Components/Shops/ShopCore.h"
#include "Components/Skills/SkillsCore.h"
#include "Components/Storages/StorageCore.h"
#include "Components/Worlds/WorldSwapCore.h"

using namespace sqlstr;

MmoController::MmoController(CGS *pGameServer) : m_pGameServer(pGameServer)
{
	// order
	m_Components.add(m_pBotsInfo = new CBotCore());
	m_Components.add(m_pItemWork = new CInventoryCore());
	m_Components.add(m_pCraftJob = new CCraftCore());
	m_Components.add(m_pStorageWork = new CStorageCore());
	m_Components.add(m_pShopmail = new CShopCore());
	m_Components.add(m_pQuest = new QuestCore());
	m_Components.add(m_pDungeonJob = new DungeonJob());
	m_Components.add(new CAetherCore());
	m_Components.add(m_pWorldSwapJob = new CWorldSwapCore());
	m_Components.add(m_pHouseJob = new CHouseCore());
	m_Components.add(m_pGuildJob = new GuildJob());
	m_Components.add(m_pSkillJob = new CSkillsCore());
	m_Components.add(m_pAccMain = new CAccountCore());
	m_Components.add(m_pAccMiner = new CAccountMinerCore());
	m_Components.add(m_pAccPlant = new CAccountPlantCore());
	m_Components.add(m_pMailBoxJob = new CMailBoxCore());

	for(auto& pComponent : m_Components.m_paComponents)
	{
		pComponent->m_Job = this;
		pComponent->m_GameServer = pGameServer;
		pComponent->m_pServer = pGameServer->Server();

		if(m_pGameServer->GetWorldID() == MAIN_WORLD_ID)
			pComponent->OnInit();

		char aLocalSelect[64];
		str_format(aLocalSelect, sizeof(aLocalSelect), "WHERE WorldID = '%d'", m_pGameServer->GetWorldID());
		pComponent->OnInitWorld(aLocalSelect);
	}
}

MmoController::~MmoController()
{
	m_Components.free();
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

	for(auto& pComponent : m_Components.m_paComponents)
	{
		if(pComponent->OnHandleMenulist(pPlayer, Menulist, ReplaceMenu))
			return true;
	}
	return false;
}

bool MmoController::OnPlayerHandleTile(CCharacter *pChr, int IndexCollision)
{
	if(!pChr || !pChr->IsAlive())
		return true;

	for(auto & pComponent : m_Components.m_paComponents)
	{
		if(pComponent->OnHandleTile(pChr, IndexCollision))
			return true;
	}
	return false;
}

bool MmoController::OnParsingVoteCommands(CPlayer *pPlayer, const char *CMD, const int VoteID, const int VoteID2, int Get, const char *GetText)
{
	if(!pPlayer)
		return true;

	for(auto& pComponent : m_Components.m_paComponents)
	{
		if(pComponent->OnHandleVoteCommands(pPlayer, CMD, VoteID, VoteID2, Get, GetText))
			return true;
	}
	return false;
}

void MmoController::PrepareInformation(IStorageEngine *pStorage)
{
	// write mmo data to file
	CDataFileWriter DataInfoWriter;
	if(!DataInfoWriter.Open(pStorage, MMO_DATA_FILE))
		return;

	for(auto& pComponent : m_Components.m_paComponents)
		pComponent->OnPrepareInformation(pStorage, &DataInfoWriter);

	DataInfoWriter.Finish();

	CDataFileReader DataInfoReader;
	if(!DataInfoReader.Open(pStorage, MMO_DATA_FILE, IStorageEngine::TYPE_ALL))
		return;

	const char* pItem = (const char*)DataInfoReader.FindItem(MMO_DATA_INVENTORY_INFORMATION, 0);
	if(pItem)
	{
		dbg_msg("test", "%s", pItem);
	}

	char aSha256[SHA256_MAXSTRSIZE];
	sha256_str(DataInfoReader.Sha256(), aSha256, sizeof(aSha256));
	dbg_msg("test", "mmo data file sha256 is %s", aSha256);
	dbg_msg("test", "mmo data file Crc is %08x", DataInfoReader.Crc());
	DataInfoReader.Close();
}

void MmoController::OnMessage(int MsgID, void *pRawMsg, int ClientID)
{
	if(!pRawMsg)
		return;

	for(auto& pComponent : m_Components.m_paComponents)
		pComponent->OnMessage(MsgID, pRawMsg, ClientID);
}

void MmoController::ResetClientData(int ClientID)
{
	for (auto& pComponent : m_Components.m_paComponents)
		pComponent->OnResetClient(ClientID);
}

// saving account
void MmoController::SaveAccount(CPlayer *pPlayer, int Table)
{
	if(!pPlayer->IsAuthed())
		return;

	if(Table == SAVE_STATS)
	{
		const int EquipDiscord = pPlayer->GetEquippedItemID(EQUIP_DISCORD);
		SJK.UD("tw_accounts_data", "Level = '%d', Exp = '%d', DiscordEquip = '%d' WHERE ID = '%d'",
			pPlayer->Acc().m_Level, pPlayer->Acc().m_Exp, EquipDiscord, pPlayer->Acc().m_AccountID);
	}
	else if(Table == SAVE_UPGRADES)
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

		SJK.UD("tw_accounts_data", "Upgrade = '%d' %s WHERE ID = '%d'", pPlayer->Acc().m_Upgrade, Buffer.buffer(), pPlayer->Acc().m_AccountID);
		Buffer.clear();
	}
	else if(Table == SAVE_PLANT_DATA)
	{
		char aBuf[64];
		dynamic_string Buffer;
		for(int i = 0; i < NUM_JOB_ACCOUNTS_STATS; i++)
		{
			const char *pFieldName = pPlayer->Acc().m_aPlantData[i].getFieldName();
			const int JobValue = pPlayer->Acc().m_aPlantData[i];
			str_format(aBuf, sizeof(aBuf), "%s = '%d' %s", pFieldName, JobValue, (i == NUM_JOB_ACCOUNTS_STATS-1 ? "" : ", "));
			Buffer.append_at(Buffer.length(), aBuf);
		}

		SJK.UD("tw_accounts_plants", "%s WHERE AccountID = '%d'", Buffer.buffer(), pPlayer->Acc().m_AccountID);
		Buffer.clear();
	}
	else if(Table == SAVE_MINER_DATA)
	{
		char aBuf[64];
		dynamic_string Buffer;
		for(int i = 0; i < NUM_JOB_ACCOUNTS_STATS; i++)
		{
			const char* pFieldName = pPlayer->Acc().m_aMiningData[i].getFieldName();
			const int JobValue = pPlayer->Acc().m_aMiningData[i];
			str_format(aBuf, sizeof(aBuf), "%s = '%d' %s", pFieldName, JobValue, (i == NUM_JOB_ACCOUNTS_STATS-1 ? "" : ", "));
			Buffer.append_at(Buffer.length(), aBuf);
		}

		SJK.UD("tw_accounts_miner", "%s WHERE AccountID = '%d'", Buffer.buffer(), pPlayer->Acc().m_AccountID);
		Buffer.clear();
	}
	else if(Table == SAVE_GUILD_DATA)
	{
		SJK.UD("tw_accounts_data", "GuildID = '%d', GuildRank = '%d' WHERE ID = '%d'", pPlayer->Acc().m_GuildID, pPlayer->Acc().m_GuildRank, pPlayer->Acc().m_AccountID);
	}
	else if(Table == SAVE_POSITION)
	{
		int LatestCorrectWorldID = Account()->GetHistoryLatestCorrectWorldID(pPlayer);
		SJK.UD("tw_accounts_data", "WorldID = '%d' WHERE ID = '%d'", LatestCorrectWorldID, pPlayer->Acc().m_AccountID);
	}
	else if(Table == SAVE_LANGUAGE)
	{
		SJK.UD("tw_accounts", "Language = '%s' WHERE ID = '%d'", pPlayer->GetLanguage(), pPlayer->Acc().m_AccountID);
	}
	else
	{
		SJK.UD("tw_accounts", "Username = '%s' WHERE ID = '%d'", pPlayer->Acc().m_aLogin, pPlayer->Acc().m_AccountID);
	}
}

void MmoController::LoadLogicWorld()
{
	ResultPtr pRes = SJK.SD("*", "tw_logicworld", "WHERE WorldID = '%d'", GS()->GetWorldID());
	while(pRes->next())
	{
		const int Type = (int)pRes->getInt("MobID"), Mode = (int)pRes->getInt("Mode"), Health = (int)pRes->getInt("ParseInt");
		const vec2 Position = vec2(pRes->getInt("PosX"), pRes->getInt("PosY"));
		GS()->m_pController->CreateLogic(Type, Mode, Position, Health);
	}
}

char SaveNick[32];
const char* MmoController::PlayerName(int AccountID)
{
	ResultPtr pRes = SJK.SD("Nick", "tw_accounts_data", "WHERE ID = '%d'", AccountID);
	if(pRes->next())
	{
		str_copy(SaveNick, pRes->getString("Nick").c_str(), sizeof(SaveNick));
		return SaveNick;
	}
	return "No found!";
}

void MmoController::ShowLoadingProgress(const char* pLoading, int Size)
{
	char aLoadingBuf[128];
	str_format(aLoadingBuf, sizeof(aLoadingBuf), "[Loaded %d %s] :: WorldID %d.", Size, pLoading, GS()->GetWorldID());
	GS()->Console()->Print(IConsole::OUTPUT_LEVEL_ADDINFO, "LOAD DB", aLoadingBuf);
}

void MmoController::ShowTopList(CPlayer* pPlayer, int TypeID)
{
	const int ClientID = pPlayer->GetCID();
	pPlayer->m_VoteColored = SMALL_LIGHT_GRAY_COLOR;
	if(TypeID == GUILDS_LEVELING)
	{
		ResultPtr pRes = SJK.SD("*", "tw_guilds", "ORDER BY Level DESC, Experience DESC LIMIT 10");
		while (pRes->next())
		{
			char NameGuild[64];
			const int Rank = pRes->getRow();
			const int Level = pRes->getInt("Level");
			const int Experience = pRes->getInt("Experience");
			str_copy(NameGuild, pRes->getString("GuildName").c_str(), sizeof(NameGuild));
			GS()->AVL(ClientID, "null", "{INT}. {STR} :: Level {INT} : Exp {INT}", Rank, NameGuild, Level, Experience);
		}
	}
	else if (TypeID == GUILDS_WEALTHY)
	{
		ResultPtr pRes = SJK.SD("*", "tw_guilds", "ORDER BY Bank DESC LIMIT 10");
		while (pRes->next())
		{
			char NameGuild[64];
			const int Rank = pRes->getRow();
			const int Gold = pRes->getInt("Bank");
			str_copy(NameGuild, pRes->getString("GuildName").c_str(), sizeof(NameGuild));
			GS()->AVL(ClientID, "null", "{INT}. {STR} :: Gold {INT}", Rank, NameGuild, Gold);
		}
	}
	else if (TypeID == PLAYERS_LEVELING)
	{
		ResultPtr pRes = SJK.SD("*", "tw_accounts_data", "ORDER BY Level DESC, Exp DESC LIMIT 10");
		while (pRes->next())
		{
			char Nick[64];
			const int Rank = pRes->getRow();
			const int Level = pRes->getInt("Level");
			const int Experience = pRes->getInt("Exp");
			str_copy(Nick, pRes->getString("Nick").c_str(), sizeof(Nick));
			GS()->AVL(ClientID, "null", "{INT}. {STR} :: Level {INT} : Exp {INT}", Rank, Nick, Level, Experience);
		}
	}
	else if (TypeID == PLAYERS_WEALTHY)
	{
		ResultPtr pRes = SJK.SD("*", "tw_accounts_items", "WHERE ItemID = '%d' ORDER BY Count DESC LIMIT 10", (int)itGold);
		while (pRes->next())
		{
			char Nick[64];
			const int Rank = pRes->getRow();
			const int Gold = pRes->getInt("Count");
			const int OwnerID = pRes->getInt("OwnerID");
			str_copy(Nick, PlayerName(OwnerID), sizeof(Nick));
			GS()->AVL(ClientID, "null", "{INT}. {STR} :: Gold {INT}", Rank, Nick, Gold);
		}
	}
}