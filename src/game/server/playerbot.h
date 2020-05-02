/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#ifndef GAME_SERVER_PLAYER_BOT_H
#define GAME_SERVER_PLAYER_BOT_H

#include "player.h"

class CPlayerBot : public CPlayer 
{
	MACRO_ALLOC_POOL_ID()

private:
	int m_SpawnPointBot;
	int m_BotID;
	int m_SubBotID;
	int m_BotHealth;
	int m_DungeonAllowedSpawn;


public:
	CPlayerBot(CGS *pGS, int ClientID, int BotID, int SubBotID, int SpawnPoint);
	virtual ~CPlayerBot() override;

	int GetBotID()													override { return m_BotID; };
	int GetSpawnBot()												override { return m_SpawnPointBot; };
	int GetBotSub()													override { return m_SubBotID; };
	int GetHealth() 												override { return m_BotHealth; };
	int GetTeam()													override { return TEAM_BLUE; };
	void SetStandart(int Health, int Mana)							override { m_BotHealth = Health; };
	void SetDungeonAllowedSpawn(bool Spawn)							override { m_DungeonAllowedSpawn = Spawn; };
	int GetMoodNameplacesType(int SnappingClient)					override;
	int GetItemEquip(int EquipID, int SkipItemID = -1) const		override;
	void Tick()														override;
	int GetStartHealth()											override;
	int GetAttributeCount(int BonusID, bool Really = false)			override;
	void Snap(int SnappingClient)									override;
	bool CheckQuestSnapPlayer(int SnappingClient, bool SnapData)	override;

private:
	void TryRespawn()												override;

	int GetBotLevel() const;
	bool GetActiveQuestsID(int SnapClientID);
	const char* GetStatusBot();
	void SendInformationBot();
};

#endif
