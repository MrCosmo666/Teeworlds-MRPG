/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#ifndef GAME_SERVER_SQLTEST_H
#define GAME_SERVER_SQLTEST_H
#include <thread>

#include "sqlwork/account_main.h"
#include "sqlwork/account_miner.h"
#include "sqlwork/account_plant.h"
#include "sqlwork/botsinfo.h"
#include "sqlwork/items.h"
#include "sqlwork/questing.h"
#include "sqlwork/shopmail.h"
#include "sqlwork/storage.h"
#include "sqlwork/teleports.h"

#include "sqlwork/skill_job.h"
#include "sqlwork/world_swap_job.h"
#include "sqlwork/craft_job.h"
#include "sqlwork/dungeon_job.h"
#include "sqlwork/house_job.h"
#include "sqlwork/mailbox_job.h"
#include "sqlwork/guild_job.h"

#include <engine/server/sql_connect_pool.h>
#include <engine/server/sql_string_helpers.h>

class SqlController
{
	class CStack
	{
	public:
		void add(class CMmoComponent *pComponent)
		{
 			m_paComponents.push_back(pComponent);
		}
		
		void clear()
		{
			m_paComponents.clear();
		}

		std::list < class CMmoComponent *> m_paComponents;
	};
	CStack m_Components;

	class AccountMainSql *m_pAccMain;
	class ContextBots *m_pBotsInfo;
	class ItemSql *m_pItemWork;
	class MinerAccSql *m_pAccMiner;
	class PlantsAccSql *m_pAccPlant;
	class QuestBase *m_pQuest;
	class ShopMailSql *m_pShopmail;
	class StorageSql *m_pStorageWork;
	class TeleportsSql *m_pTeleportsWork;

	class GuildJob* m_pGuildJob;
	class CraftJob* m_pCraftJob;
	class DungeonJob* m_pDungeonJob;
	class HouseJob* m_pHouseJob;
	class MailBoxJob* m_pMailBoxJob;
	class SkillJob* m_pSkillJob;
	class WorldSwapJob* m_pWorldSwapJob;

public:
	explicit SqlController(CGS *pGameServer);
	~SqlController();

	CGS *m_pGameServer;
	CGS *GS() const { return m_pGameServer; }

	// ссылки на объекты класса Sql Work
	AccountMainSql *Account() const { return m_pAccMain; }
	ContextBots *BotsData() const { return m_pBotsInfo; }
	ItemSql *Item() const { return m_pItemWork; }
	MinerAccSql *MinerAcc() const { return m_pAccMiner; }
	PlantsAccSql *PlantsAcc() const { return m_pAccPlant; }
	QuestBase *Quest() const { return m_pQuest; }
	ShopMailSql *Auction() const { return m_pShopmail; }
	StorageSql *Storage() const { return m_pStorageWork; }
	TeleportsSql *Teleports() const { return m_pTeleportsWork; }

	CraftJob* Craft() const { return m_pCraftJob; }
	DungeonJob* Dungeon() const { return m_pDungeonJob; }
	HouseJob* House() const { return m_pHouseJob; }
	MailBoxJob* Inbox() const { return m_pMailBoxJob; }
	GuildJob* Member() const { return m_pGuildJob; }
	SkillJob* Skills() const { return m_pSkillJob; }
	WorldSwapJob *WorldSwap() const { return m_pWorldSwapJob; }

	// global systems
	void LoadFullSystems();
	void OnTick();
	bool OnPlayerHandleTile(CCharacter *pChr, int IndexCollision);
	bool OnPlayerHandleMainMenu(int ClientID, int Menulist, bool ReplaceMenu);
	void OnInitAccount(int ClientID);
	bool OnMessage(int MsgID, void *pRawMsg, int ClientID);
	bool OnParseFullVote(CPlayer *pPlayer, const char *CMD, const int VoteID, const int VoteID2, int Get, const char *GetText);
	void ResetClientData(int ClientID);

	// 
	void LoadLogicWorld();
	
	//
	const char* PlayerName(int AccountID);

	// Аккаунт
	void SaveAccount(CPlayer *pPlayer, int Table);

	//
	void ShowLoadingProgress(const char *Loading, int LoadCount);
	void ShowTopList(CPlayer* pPlayer, int TypeID);
};

#endif