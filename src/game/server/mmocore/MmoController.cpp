/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#include <thread>
#include <engine/shared/config.h>

#include <game/server/gamecontext.h>
#include "MmoController.h"

#include "ComponentsCore/AetherJob.h"

using namespace sqlstr;

// список хранения всех компонентов Sql Work
MmoController::MmoController(CGS *pGameServer) : m_pGameServer(pGameServer)
{
	// order
	m_Components.add(m_pBotsInfo = new BotJob());
	m_Components.add(m_pItemWork = new ItemJob());
	m_Components.add(m_pCraftJob = new CraftJob());
	m_Components.add(m_pStorageWork = new StorageJob());
	m_Components.add(m_pShopmail = new ShopJob());
	m_Components.add(m_pQuest = new QuestJob());
	m_Components.add(m_pDungeonJob = new DungeonJob());
	m_Components.add(new AetherJob());
	m_Components.add(m_pWorldSwapJob = new WorldSwapJob());
	m_Components.add(m_pHouseJob = new HouseJob());
	m_Components.add(m_pGuildJob = new GuildJob());
	m_Components.add(m_pSkillJob = new SkillJob());
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

		char aLocalSelect[128];
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
	// весь тик компонентов
	for(auto& component : m_Components.m_paComponents)
		component->OnTick();
}

void MmoController::OnInitAccount(int ClientID)
{
	CPlayer *pPlayer = GS()->GetPlayer(ClientID);
	if(!pPlayer || !pPlayer->IsAuthed()) return;

	for(auto& component : m_Components.m_paComponents)
		component->OnInitAccount(pPlayer);
}

bool MmoController::OnPlayerHandleMainMenu(int ClientID, int Menulist, bool ReplaceMenu)
{
	CPlayer *pPlayer = GS()->GetPlayer(ClientID);
	if(!pPlayer || !pPlayer->IsAuthed()) return false;

	for(auto& component : m_Components.m_paComponents)
	{
		if(component->OnHandleMenulist(pPlayer, Menulist, ReplaceMenu))
			return true;
	}
	return false;
}

bool MmoController::OnPlayerHandleTile(CCharacter *pChr, int IndexCollision)
{
	if(!pChr) return true;
	for(auto & component : m_Components.m_paComponents)
	{
		if(component->OnHandleTile(pChr, IndexCollision))
			return true;
	}
	return false;
}

bool MmoController::OnParseFullVote(CPlayer *pPlayer, const char *CMD, const int VoteID, const int VoteID2, int Get, const char *GetText)
{
	for(auto& component : m_Components.m_paComponents)
	{
		if(component->OnVotingMenu(pPlayer, CMD, VoteID, VoteID2, Get, GetText))
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

// Сохранение аккаунта
void MmoController::SaveAccount(CPlayer *pPlayer, int Table)
{
	const int ClientID = pPlayer->GetCID();
	if(!pPlayer->IsAuthed()) return;
	
	// сохранение статистики
	if(Table == SaveType::SAVE_STATS)
	{
		const int EquipDiscord = pPlayer->GetItemEquip(EQUIP_DISCORD);
		SJK.UD("tw_accounts_data", "Level = '%d', Exp = '%d', DiscordEquip = '%d' WHERE ID = '%d'",
			pPlayer->Acc().Level, pPlayer->Acc().Exp, EquipDiscord, pPlayer->Acc().AuthID);
		return;
	}

	// сохранение апгрейдов
	else if(Table == SaveType::SAVE_UPGRADES)
	{
		char aBuf[64];
		dynamic_string Buffer;
		for(const auto& at : CGS::AttributInfo)
		{
			if(str_comp_nocase(at.second.FieldName, "unfield") == 0) 
				continue;
			str_format(aBuf, sizeof(aBuf), ", %s = '%d' ", at.second.FieldName, pPlayer->Acc().Stats[at.first]);
			Buffer.append_at(Buffer.length(), aBuf);
		}

		SJK.UD("tw_accounts_data", "Upgrade = '%d' %s WHERE ID = '%d'", pPlayer->Acc().Upgrade, Buffer.buffer(), pPlayer->Acc().AuthID);
		Buffer.clear();
		return;
	}

	// сохранение плант аккаунта
	else if(Table == SaveType::SAVE_PLANT_DATA)
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
	else if(Table == SaveType::SAVE_MINER_DATA)
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
	else if(Table == SaveType::SAVE_GUILD_DATA)
	{
		SJK.UD("tw_accounts_data", "GuildID = '%d', GuildRank = '%d' WHERE ID = '%d'", pPlayer->Acc().GuildID, pPlayer->Acc().GuildRank, pPlayer->Acc().AuthID);
		return;			
	}

	// сохранение мира позиции
	else if(Table == SaveType::SAVE_POSITION)
	{
		// запрет в данже сохранять позицию
		if (GS()->DungeonID() > 0) 
			return;

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

void MmoController::LoadLogicWorld()
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
const char* MmoController::PlayerName(int AccountID)
{
	boost::scoped_ptr<ResultSet> RES(SJK.SD("Nick", "tw_accounts_data", "WHERE ID = '%d'", AccountID));
	if(RES->next())
	{
		str_copy(SaveNick, RES->getString("Nick").c_str(), sizeof(SaveNick));
		return SaveNick;
	}
	return "No found!";
}

void MmoController::ShowLoadingProgress(const char *Loading, int LoadCount)
{
	char aLoadingBuf[128];
	str_format(aLoadingBuf, sizeof(aLoadingBuf), "Loaded %d %s | CK WorldID %d.", LoadCount, Loading, GS()->GetWorldID());
	GS()->Console()->Print(IConsole::OUTPUT_LEVEL_ADDINFO, "LOAD DB", aLoadingBuf);
}

void MmoController::ShowTopList(CPlayer* pPlayer, int TypeID)
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