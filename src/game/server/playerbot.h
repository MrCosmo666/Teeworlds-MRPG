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

	int GetBotID() const override { return m_BotID; };
	int GetBotType() const override { return m_BotType; };
	int GetBotSub() const override { return m_SubBotID; };
	int GetTeam() override { return TEAM_BLUE; };
	int GetPlayerWorldID() const override;

	int GetStartHealth() override;
	int GetHealth() override { return m_BotHealth; };
	int GetMana() override { return 999; };

	void UpdateTempData(int Health, int Mana) override { m_BotHealth = Health; };
	void SendClientInfo(int TargetID) override;
	
	int IsActiveSnappingBot(int SnappingClient) const override;
	int GetEquippedItem(int EquipID, int SkipItemID = -1) const override;
	int GetAttributeCount(int BonusID, bool Really = false, bool SearchClass = false) override;

	void Tick() override;
	void Snap(int SnappingClient) override;

	int m_LastPosTick;
	int m_PathSize;
	vec2 m_CharPos;
	vec2 m_TargetPos;
	std::map<int, vec2> m_WayPoints;

	void SetDungeonAllowedSpawn(bool Spawn) { m_DungeonAllowedSpawn = Spawn; };
	
private:
	void TryRespawn() override;

	int GetBotLevel() const;
	const char* GetStatusBot() const;
	int GetMoodState(int SnappingClient) const;
	bool IsActiveQuests(int SnapClientID) const;

	void GenerateNick(char* buffer, int size_buffer);
	void TickThreadMobsPathFinder();
};

#endif
