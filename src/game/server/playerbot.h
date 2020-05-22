/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#ifndef GAME_SERVER_PLAYER_BOT_H
#define GAME_SERVER_PLAYER_BOT_H

#include "player.h"

class CPlayerBot : public CPlayer 
{
	MACRO_ALLOC_POOL_ID()

private:
	int m_BotType;
	int m_BotID;
	int m_SubBotID;
	int m_BotHealth;
	int m_DungeonAllowedSpawn;


public:
	CPlayerBot(CGS *pGS, int ClientID, int BotID, int SubBotID, int SpawnPoint);
	virtual ~CPlayerBot() override;

	int GetBotID() override { return m_BotID; };
	int GetBotType() override { return m_BotType; };
	int GetBotSub() override { return m_SubBotID; };
	int GetHealth() override { return m_BotHealth; };
	int GetMana() override { return 999; };
	int GetTeam() override { return TEAM_BLUE; };
	int GetStartHealth() override;
	int GetPlayerWorldID() const override;

	void SetDungeonAllowedSpawn(bool Spawn) override { m_DungeonAllowedSpawn = Spawn; };
	void UpdateTempData(int Health, int Mana) override { m_BotHealth = Health; };
	int IsActiveSnappingBot(int SnappingClient) override;
	int GetAttributeCount(int BonusID, bool Really = false, bool SearchClass = false) override;
	void SendClientInfo(int TargetID) override;
	int GetEquippedItem(int EquipID, int SkipItemID = -1) const override;

	void Tick() override;
	void Snap(int SnappingClient) override;

	int m_LastPosTick;
	int m_PathSize;
	vec2 m_CharPos;
	vec2 m_TargetPos;
	std::map<int, vec2> m_WayPoints;

private:
	void TryRespawn() override;

	void GenerateNick(char* buffer, int size_buffer);
	int GetBotLevel() const;
	bool IsActiveQuests(int SnapClientID);
	const char* GetStatusBot();
	int GetMoodState(int SnappingClient);

	// threading path finder
	void TickThreadMobsPathFinder();
};

#endif
