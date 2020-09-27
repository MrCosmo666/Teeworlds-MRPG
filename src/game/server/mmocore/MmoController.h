/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#ifndef GAME_SERVER_MMOCONTROLLER_H
#define GAME_SERVER_MMOCONTROLLER_H

/*
	At the end, distribute the components in MmoController.cpp
	And distribute where they are required
	This will affect the size of the output file
*/
#include "ComponentsCore/AccountMainJob.h"
#include "ComponentsCore/AccountMinerJob.h"
#include "ComponentsCore/AccountPlantJob.h"
#include "ComponentsCore/BotJob.h"
#include "ComponentsCore/ItemJob.h"
#include "ComponentsCore/QuestJob.h"
#include "ComponentsCore/ShopJob.h"
#include "ComponentsCore/StorageJob.h"

#include "ComponentsCore/SkillJob.h"
#include "ComponentsCore/WorldSwapJob.h"
#include "ComponentsCore/CraftJob.h"
#include "ComponentsCore/DungeonJob.h"
#include "ComponentsCore/HouseJob.h"
#include "ComponentsCore/MailBoxJob.h"
#include "ComponentsCore/GuildJob.h"

#include <engine/server/sql_connect_pool.h>
#include <engine/server/sql_string_helpers.h>

class MmoController
{
	class CStack
	{
	public:
		void add(class MmoComponent *pComponent)
		{
 			m_paComponents.push_back(pComponent);
		}
		
		void clear()
		{
			m_paComponents.clear();
		}

		std::list < class MmoComponent *> m_paComponents;
	};
	CStack m_Components;

	class AccountMainJob *m_pAccMain;
	class BotJob *m_pBotsInfo;
	class ItemJob *m_pItemWork;
	class AccountMinerJob *m_pAccMiner;
	class AccountPlantJob *m_pAccPlant;
	class QuestJob *m_pQuest;
	class ShopJob *m_pShopmail;
	class StorageJob *m_pStorageWork;

	class GuildJob* m_pGuildJob;
	class CraftJob* m_pCraftJob;
	class DungeonJob* m_pDungeonJob;
	class HouseJob* m_pHouseJob;
	class MailBoxJob* m_pMailBoxJob;
	class SkillJob* m_pSkillJob;
	class WorldSwapJob* m_pWorldSwapJob;

public:
	explicit MmoController(CGS *pGameServer);
	~MmoController();

	CGS *m_pGameServer;
	CGS *GS() const { return m_pGameServer; }

	AccountMainJob *Account() const { return m_pAccMain; }
	BotJob *BotsData() const { return m_pBotsInfo; }
	ItemJob *Item() const { return m_pItemWork; }
	AccountMinerJob *MinerAcc() const { return m_pAccMiner; }
	AccountPlantJob *PlantsAcc() const { return m_pAccPlant; }
	QuestJob *Quest() const { return m_pQuest; }
	ShopJob *Auction() const { return m_pShopmail; }
	StorageJob *Storage() const { return m_pStorageWork; }

	CraftJob* Craft() const { return m_pCraftJob; }
	DungeonJob* Dungeon() const { return m_pDungeonJob; }
	HouseJob* House() const { return m_pHouseJob; }
	MailBoxJob* Inbox() const { return m_pMailBoxJob; }
	GuildJob* Member() const { return m_pGuildJob; }
	SkillJob* Skills() const { return m_pSkillJob; }
	WorldSwapJob *WorldSwap() const { return m_pWorldSwapJob; }

	// global systems
	void OnTick();
	bool OnPlayerHandleTile(CCharacter *pChr, int IndexCollision);
	bool OnPlayerHandleMainMenu(int ClientID, int Menulist, bool ReplaceMenu);
	void OnInitAccount(int ClientID);
	void OnMessage(int MsgID, void *pRawMsg, int ClientID);
	bool OnParseFullVote(CPlayer *pPlayer, const char *CMD, const int VoteID, const int VoteID2, int Get, const char *GetText);
	void ResetClientData(int ClientID);

	// 
	void LoadLogicWorld();
	static const char* PlayerName(int AccountID);
	void SaveAccount(CPlayer *pPlayer, int Table);
	void ShowLoadingProgress(const char* pLoading, int Size);
	void ShowTopList(CPlayer* pPlayer, int TypeID);
};

#endif