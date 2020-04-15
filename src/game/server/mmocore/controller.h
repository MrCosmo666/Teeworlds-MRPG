/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#ifndef GAME_SERVER_SQLTEST_H
#define GAME_SERVER_SQLTEST_H
#include <thread>

#include "sqlwork/account_main.h"
#include "sqlwork/account_miner.h"
#include "sqlwork/account_plant.h"
#include "sqlwork/account_relax.h"
#include "sqlwork/botsinfo.h"
#include "sqlwork/craft.h"
#include "sqlwork/home.h"
#include "sqlwork/inbox.h"
#include "sqlwork/items.h"
#include "sqlwork/member.h"
#include "sqlwork/questing.h"
#include "sqlwork/shopmail.h"
#include "sqlwork/skills.h"
#include "sqlwork/storage.h"
#include "sqlwork/teleports.h"
#include "sqlwork/worldswap.h"

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
	class CraftSql *m_pCraftWork;
	class HouseSql *m_pHouseWork;
	class InboxSql *m_pInbox;
	class ItemSql *m_pItemWork;
	class MemberSql *m_pMemberWork;
	class MinerAccSql *m_pAccMiner;
	class PlantsAccSql *m_pAccPlant;
	class QuestBase *m_pQuest;
	class ShopMailSql *m_pShopmail;
	class SkillsSql *m_pSkillsWork;
	class SpaRelaxSql *m_pAccRelax;
	class StorageSql *m_pStorageWork;
	class TeleportsSql *m_pTeleportsWork;
	class WorldSwapSql *m_pWorldSwapWork;

	void CheckTimePayment();

public:
	explicit SqlController(CGS *pGameServer);
	~SqlController();

	CGS *m_pGameServer;
	CGS *GS() const { return m_pGameServer; }

	// ссылки на объекты класса Sql Work
	AccountMainSql *Account() const { return m_pAccMain; }
	ContextBots *BotsData() const { return m_pBotsInfo; }
	CraftSql *Craft() const { return m_pCraftWork; }
	HouseSql *House() const { return m_pHouseWork; }
	InboxSql *Inbox() const { return m_pInbox; }
	ItemSql *Item() const { return m_pItemWork; }
	MemberSql *Member() const { return m_pMemberWork; }
	MinerAccSql *MinerAcc() const { return m_pAccMiner; }
	PlantsAccSql *PlantsAcc() const { return m_pAccPlant; }
	QuestBase *Quest() const { return m_pQuest; }
	ShopMailSql *Auction() const { return m_pShopmail; }
	SkillsSql *Skills() const { return m_pSkillsWork; }
	SpaRelaxSql *SpaAcc() const { return m_pAccRelax; }
	StorageSql *Storage() const { return m_pStorageWork; }
	TeleportsSql *Teleports() const { return m_pTeleportsWork; }
	WorldSwapSql *WorldSwap() const { return m_pWorldSwapWork; }

	// global systems
	void LoadFullSystems();
	void OnTick();
	bool OnPlayerHandleTile(CCharacter *pChr, int IndexCollision);
	void OnPlayerHandleMainMenu(int ClientID, int Menulist);
	void OnInitAccount(int ClientID);
	bool OnMessage(int MsgID, void *pRawMsg, int ClientID);
	bool OnParseFullVote(CPlayer *pPlayer, const char *CMD, const int VoteID, const int VoteID2, int Get, const char *GetText);

	// 
	void LoadLogicWorld();
	void LoadDungeons();
	void ShowBussinesHousesSell(CPlayer *pPlayer);
	
	//
	const char* PlayerName(int AccountID);

	// Аккаунт
	void SaveAccount(CPlayer *pPlayer, int Table);

	//
	void ShowLoadingProgress(const char *Loading, int LoadCount);

	void ShowDungeonTop(CPlayer* pPlayer, int DungeonID, int HideID);
	void SaveDungeonRecord(CPlayer* pPlayer, int DungeonID, int Seconds);
	void ShowDungeonsList(CPlayer* pPlayer);
	void ShowTopList(CPlayer* pPlayer, int TypeID);
};

#endif