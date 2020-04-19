/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#include <engine/shared/config.h>

#include "entities/botai/botaijob.h"

#include "gamecontext.h"
#include "player_bot.h"

MACRO_ALLOC_POOL_ID_IMPL(CPlayerBot, MAX_CLIENTS*COUNT_WORLD+MAX_CLIENTS)

IServer* CPlayer::Server() const { return m_pGS->Server(); };

CPlayerBot::CPlayerBot(CGS *pGS, int ClientID, int BotID, int SubBotID, int SpawnPoint)
: CPlayer(pGS, ClientID), m_SpawnPointBot(SpawnPoint), m_BotID(BotID), m_SubBotID(SubBotID), m_BotHealth(0)
{
	m_Spawned = true;
	m_DungeonAllowedSpawn = false;
	m_PlayerTick[TickState::Respawn] = Server()->Tick();
	GS()->SendInformationBot(this);
}

CPlayerBot::~CPlayerBot() 
{ 
	delete m_pCharacter;
	m_pCharacter = NULL;
}

void CPlayerBot::Tick()
{
	if(!Server()->ClientIngame(m_ClientID))
		return;

	Server()->SetClientScore(m_ClientID, 1);

	if(m_pCharacter && !m_pCharacter->IsAlive())
	{
		delete m_pCharacter;
		m_pCharacter = NULL;
	}

	if(m_pCharacter)
	{
		if(m_pCharacter->IsAlive())
			m_ViewPos = m_pCharacter->GetPos();
	} 
	else if(m_Spawned && m_PlayerTick[TickState::Respawn]+Server()->TickSpeed()*3 <= Server()->Tick())
		TryRespawn();

}

int CPlayerBot::GetStartHealth()
{
	if(m_SpawnPointBot == SPAWNMOBS)
		return ContextBots::MobBot[m_SubBotID].Health*10;
	return 10;	
}

int CPlayerBot::GetAttributeCount(int BonusID, bool Really)
{
	// если бот то возращаем в зависимости от обьема установленного здоровья
	if(CGS::AttributInfo.find(BonusID) == CGS::AttributInfo.end()) return 0;

	if(m_SpawnPointBot == SPAWNMOBS)
	{
		// сила рейда
		int Power = GS()->IncreaseCountRaid(ContextBots::MobBot[m_SubBotID].Health);
		return (int)(Power / (BonusID == Stats::StStrength ? 15 : 8));
	} // если не моб то 10
	return 10;
}

// Спавн игрока
void CPlayerBot::TryRespawn()
{
	// разрешить спавн в данже только по запросу
	if (GS()->DungeonID() > 0 && !m_DungeonAllowedSpawn)
		return;

	vec2 SpawnPos;
	const int SpawnType = m_SpawnPointBot;
	
	// если бот обычный моб
	if(SpawnType == SPAWNMOBS)
	{
		if(GS()->GetWorldID() != ContextBots::MobBot[m_SubBotID].WorldID)
			return;

		vec2 MobPos = vec2(ContextBots::MobBot[m_SubBotID].PositionX, ContextBots::MobBot[m_SubBotID].PositionY);
		if(!GS()->m_pController->CanSpawn(m_SpawnPointBot, &SpawnPos, MobPos))
			return;
	}
	// если бот обычный NPC
	else if(SpawnType == SPAWNNPC)
	{
		if(GS()->GetWorldID() != ContextBots::NpcBot[m_SubBotID].WorldID)
			return;

		SpawnPos = vec2(ContextBots::NpcBot[m_SubBotID].PositionX, ContextBots::NpcBot[m_SubBotID].PositionY);		
	}
	// если бот квестовый NPC
	else if(SpawnType == SPAWNQUESTNPC)
	{
		if(GS()->GetWorldID() != ContextBots::QuestBot[m_SubBotID].WorldID)
			return;

		SpawnPos = vec2(ContextBots::QuestBot[m_SubBotID].PositionX, ContextBots::QuestBot[m_SubBotID].PositionY);			
	}
	
	// создаем бота
	int savecidmem = MAX_CLIENTS*GS()->GetWorldID()+m_ClientID;
	m_pCharacter = new(savecidmem) BotAI(&GS()->m_World);
	m_pCharacter->Spawn(this, SpawnPos);

	// чтобы не было видно эффектов что НПС не видемый для одного игрока был видем другому
	if(SpawnType != SPAWNQUESTNPC) 
		GS()->CreatePlayerSpawn(SpawnPos);

	// сбросить респавн в данжах если он был разрешен
	if (SpawnType == SPAWNMOBS && GS()->DungeonID() > 0 && m_DungeonAllowedSpawn)
		m_DungeonAllowedSpawn = false;

}


// Проверить рисорку под квест игрока
bool CPlayerBot::CheckQuestSnapPlayer(int SnappingClient, bool SnapData)
{
	CPlayer *pSnap = GS()->m_apPlayers[SnappingClient];
	if(m_SpawnPointBot != SPAWNQUESTNPC || !pSnap)
		return SnapData;

	// все переменные
	const int QuestID = ContextBots::QuestBot[m_SubBotID].QuestID;	
	const int TalkProgress = ContextBots::QuestBot[m_SubBotID].Progress;
	if(!pSnap->IsAuthed()) return false;

	// ищем игрока что квест равен боту и не равен завершенному и не выполнен по сбору предметов или прогресс разговора не является концу квеста
	if(QuestBase::Quests[SnappingClient].find(QuestID) == QuestBase::Quests[SnappingClient].end()) return false;

	// если квест завершен
	if(QuestBase::Quests[SnappingClient][QuestID].Type == QUESTFINISHED)
		return false;

	// если прогресс разговора не равен прогрессу бота
	if(TalkProgress != QuestBase::Quests[SnappingClient][QuestID].Progress)
		return false;

	return true;
}

// Рисовка игрока как бота
void CPlayerBot::Snap(int SnappingClient)
{
	if(!Server()->ClientIngame(m_ClientID) || !CheckQuestSnapPlayer(SnappingClient, true))
		return;

	CNetObj_PlayerInfo *pPlayerInfo = static_cast<CNetObj_PlayerInfo *>(Server()->SnapNewItem(NETOBJTYPE_PLAYERINFO, m_ClientID, sizeof(CNetObj_PlayerInfo)));
	if(!pPlayerInfo)
		return;

	pPlayerInfo->m_PlayerFlags = PLAYERFLAG_READY;
	pPlayerInfo->m_Latency = 0;
	pPlayerInfo->m_Score = (m_SpawnPointBot == SPAWNMOBS ? ContextBots::MobBot[m_SubBotID].Level : 1);

	// --------------------- CUSTOM ----------------------
	// ---------------------------------------------------
	if(!GS()->CheckClient(SnappingClient))
		return;

	CNetObj_Mmo_ClientInfo *pClientInfo = static_cast<CNetObj_Mmo_ClientInfo *>(Server()->SnapNewItem(NETOBJTYPE_MMO_CLIENTINFO, m_ClientID, sizeof(CNetObj_Mmo_ClientInfo)));
	if(!pClientInfo)
		return;

	bool local_ClientID = (m_ClientID == SnappingClient);
	pClientInfo->m_Local = local_ClientID;
	pClientInfo->m_MoodType = GetMoodNameplacesType(SnappingClient);
	pClientInfo->m_WorldType = GS()->Mmo()->WorldSwap()->GetWorldType();
	pClientInfo->m_HealthStart = GetStartHealth();
	pClientInfo->m_Health = GetHealth();
	pClientInfo->m_Level = GetBotLevel();
	pClientInfo->m_ActiveQuest = GetActiveQuestsID(SnappingClient);

	if (m_SpawnPointBot == SPAWNQUESTNPC)
	{
		int QuestID = ContextBots::QuestBot[m_SubBotID].QuestID;
		StrToInts(pClientInfo->m_Guildname, 12, GS()->Mmo()->Quest()->GetQuestName(QuestID));
	}
	else 
		StrToInts(pClientInfo->m_Guildname, 12, "\0");

	for(int p = 0; p < 6; p++)
	{
		StrToInts(pClientInfo->m_aaSkinPartNames[p], 6, ContextBots::DataBot[m_BotID].SkinNameBot[p]);
		pClientInfo->m_aUseCustomColors[p] = ContextBots::DataBot[m_BotID].UseCustomBot[p];
		pClientInfo->m_aSkinPartColors[p] = ContextBots::DataBot[m_BotID].SkinColorBot[p];
	}
}

int CPlayerBot::GetMoodNameplacesType(int SnappingClient) 
{
	if(GetSpawnBot() == SPAWNMOBS)
	{
		BotAI *pChr = (BotAI *)m_pCharacter;
		if(pChr && pChr->GetBotTarget() != m_ClientID)
		{
			if(pChr->GetBotTarget() == SnappingClient)
				return MOOD_AGRESSED_TANK;
			else 
				return MOOD_AGRESSED_OTHER;
		}
		else 
			return MOOD_ANGRY;
	}
	else if(GetSpawnBot() == SPAWNNPC)
		return MOOD_FRIENDLY;
	else if(GetSpawnBot() == SPAWNQUESTNPC)
		return MOOD_QUESTING;
	return MOOD_NORMAL;
}

int CPlayerBot::GetBotLevel() const
{
	return (m_SpawnPointBot == SPAWNMOBS ? ContextBots::MobBot[m_SubBotID].Level : 1);
}

bool CPlayerBot::GetActiveQuestsID(int SnapClientID)
{
	if (SnapClientID >= MAX_PLAYERS || SnapClientID < 0 || m_SpawnPointBot != SPAWNNPC)
		return false;

	for (const auto& talk : ContextBots::NpcBot[m_SubBotID].m_Talk)
	{
		if (talk.m_GivingQuest <= 0 || GS()->Mmo()->Quest()->GetQuestState(SnapClientID, talk.m_GivingQuest) != QUESTNOACCEPT)
			continue;
		return true;
	}
	return false;
}
