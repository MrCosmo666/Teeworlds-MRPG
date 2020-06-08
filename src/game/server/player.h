/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#ifndef GAME_SERVER_PLAYER_H
#define GAME_SERVER_PLAYER_H

#include "mmocore/ComponentsCore/AccountMainJob.h"
#include "mmocore/ComponentsCore/ItemJob.h"
#include "mmocore/ComponentsCore/BotJob.h"

#include "entities/character.h"

enum
{
	WEAPON_SELF = -2, // self die
	WEAPON_WORLD = -1, // swap world etc
};

class CPlayer
{
	MACRO_ALLOC_POOL_ID()

		struct StructLatency
	{
		int m_AccumMin;
		int m_AccumMax;
		int m_Min;
		int m_Max;
	};

	struct StructLastAction
	{
		int m_TargetX;
		int m_TargetY;
	};

	struct StructTalkNPC
	{
		bool m_FreezedProgress;
		int m_TalkedID;
		int m_TalkedProgress;
	};
	StructTalkNPC m_TalkingNPC;

protected:
	CCharacter* m_pCharacter;
	CGS* m_pGS;

	IServer* Server() const;
	int m_ClientID;
	void PotionsTick();

public:
	CGS* GS() const { return m_pGS; }
	vec2 m_ViewPos;
	int m_PlayerFlags;
	int m_PlayerTick[TickState::NUM_TICK];
	bool m_Flymode;
	int m_SyncDungeon;
	int m_SyncPlayers;
	int m_MoodState;

	StructLatency m_Latency;
	StructLastAction m_LatestActivity;

	/* #########################################################################
		VAR AND OBJECTS PLAYER MMO
	######################################################################### */
	CTuningParams m_PrevTuningParams;
	CTuningParams m_NextTuningParams;

	short m_SortTabs[NUMTABSORT];
	bool m_Spawned;
	short m_OpenVoteMenu;
	short m_LastVoteMenu;
	vec3 m_Colored;

private:
	char m_FormatTalkQuest[512];
	char m_aLanguage[16];
	std::map < int, bool > m_HidenMenu;

	/* #########################################################################
		FUNCTIONS PLAYER ENGINE
	######################################################################### */
	void TickOnlinePlayer();
	void TickSystemTalk();
public:
	CPlayer(CGS* pGS, int ClientID);
	virtual ~CPlayer();

	bool IsBot() const { return (bool)(m_ClientID >= MAX_PLAYERS && m_ClientID < MAX_CLIENTS); }
	virtual int GetTeam();
	virtual int GetBotID() const { return -1; };
	virtual int GetBotType() const { return -1; };
	virtual int GetBotSub() const { return -1; };
	virtual	int GetPlayerWorldID() const;

	virtual int GetStartHealth();
	int GetStartMana();
	virtual	int GetHealth() { return GetTempData().TempHealth; };
	virtual	int GetMana() { return GetTempData().TempMana; };

	virtual int IsActiveSnappingBot(int SnappingClient) const { return 2; };
	virtual void SetDungeonAllowedSpawn(bool Spawn) { return; };
	virtual int GetEquippedItem(int EquipID, int SkipItemID = -1) const;
	virtual int GetAttributeCount(int BonusID, bool Really = false, bool Searchclass = false);
	virtual void UpdateTempData(int Health, int Mana);
	virtual void SendClientInfo(int TargetID);

	virtual void Tick();
	void PostTick();

	virtual void Snap(int SnappingClient);
	void HandleTuningParams();
	
private:
	virtual void TryRespawn();

public:
	CCharacter *GetCharacter();

	void KillCharacter(int Weapon = WEAPON_WORLD);
	void OnDisconnect();
	void OnDirectInput(CNetObj_PlayerInput *NewInput);
	void OnPredictedInput(CNetObj_PlayerInput *NewInput);

	int GetCID() const { return m_ClientID; };
	/* #########################################################################
		FUNCTIONS PLAYER HELPER 
	######################################################################### */
	void ProgressBar(const char *Name, int MyLevel, int MyExp, int ExpNeed, int GivedExp);
	bool Upgrade(int Count, int *Upgrade, int *Useless, int Price, int MaximalUpgrade, const char *UpgradeName);

	/* #########################################################################
		FUNCTIONS PLAYER ACCOUNT 
	######################################################################### */
	bool CheckFailMoney(int Price, int ItemID = 1, bool CheckOnly = false);
	void GiveEffect(const char* Potion, int Sec, int Random = 0);
	void SetLanguage(const char* pLanguage);
	void AddExp(int Exp);
	void AddMoney(int Money);

	bool CheckEffect(const char* Potion);
	bool GetHidenMenu(int HideID) const;
	bool IsAuthed();
	int EnchantAttributes(int BonusID) const;
	int GetStartTeam();

	int ExpNeed(int Level) const;
	const char* GetLanguage();
	void ShowInformationStats();

	/* #########################################################################
		FUNCTIONS PLAYER PARSING 
	######################################################################### */
	bool SetParsing(int Sec, int InviteID, int SaveInt, const char* Interact);
	void ClearParsing(bool ClearVote = false, bool VotePass = true);
	bool ParseInteractive(int Vote);
	bool ParseItemsF3F4(int Vote);
  	bool ParseVoteUpgrades(const char *CMD, const int VoteID, const int VoteID2, int Get);

	/* #########################################################################
		FUNCTIONS PLAYER ITEMS 
	######################################################################### */
	ItemJob::InventoryItem& GetItem(int ItemID);
	AccountMainJob::StructTempPlayerData& GetTempData() { return AccountMainJob::PlayerTempData[m_ClientID]; }
	AccountMainJob::StructData& Acc() { return AccountMainJob::Data[m_ClientID]; }

	int GetLevelDisciple(int Class, bool SearchClass = false);

	// разговоры с нпс функции
	void SetTalking(int TalkedID, bool ToProgress);
	void ClearTalking();
	int GetTalkedID() const { return m_TalkingNPC.m_TalkedID; };

	// форматирование текста функции
	const char *FormatedTalkedText() const { return m_FormatTalkQuest; };
	void FormatTextQuest(int DataBotID, const char *pText);
	void ClearFormatQuestText();

	int GetMoodState() const { return MOOD_NORMAL; }

	void ChangeWorld(int WorldID);
};

#endif
