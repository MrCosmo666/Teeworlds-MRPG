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
	
	CTuningParams m_PrevTuningParams;
	CTuningParams m_NextTuningParams;

	virtual int GetBotID() override                         { return m_BotID; };
	virtual int GetSpawnBot() override                      { return m_SpawnPointBot; };
	virtual bool IsBot() override                           { return true; };
	virtual int GetBotSub() override                        { return m_SubBotID; };
	virtual	int GetHealth() override                        { return m_BotHealth; };
	virtual int GetTeam() override                          { return TEAM_BLUE; };
	virtual void SetStandart(int Health, int Mana) override { m_BotHealth = Health; };
	virtual void SetDungeonAllowedSpawn(bool Spawn) override{ m_DungeonAllowedSpawn = Spawn; };

	virtual void Tick() override;
	virtual int GetStartHealth() override;
	virtual int GetAttributeCount(int BonusID, bool Really = false) override;
	virtual void Snap(int SnappingClient) override;
	virtual bool CheckQuestSnapPlayer(int SnappingClient, bool SnapData) override;

private:
	virtual void TryRespawn() override;
	int GetBotLevel() const;
	int GetMoodNameplacesType(int SnappingClient);
	bool GetActiveQuestsID(int SnapClientID);
};

#endif
