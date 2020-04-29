/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#ifndef GAME_SERVER_PLAYER_H
#define GAME_SERVER_PLAYER_H

#include "mmocore/sqlwork/account_main.h"
#include "mmocore/sqlwork/items.h"
#include "mmocore/sqlwork/botsinfo.h"

#include "entities/character.h"

enum
{
	WEAPON_GAME = -3, // team switching etc
	WEAPON_SELF = -2, // console kill command
	WEAPON_WORLD = -1, // death tiles etc
};

class CPlayer
{
	/* #########################################################################
		VAR AND OBJECTS PLAYER 
	######################################################################### */
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
	CCharacter *m_pCharacter;
	CGS *m_pGS;

	IServer *Server() const;
	int m_ClientID;

public:
	CGS *GS() const { return m_pGS; }
	vec2 m_ViewPos;
	int m_PlayerFlags;
	int m_PlayerTick[TickState::NUM_TICK];
	bool m_Flymode;
	int m_SyncFactor;
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

	short m_SecurCheckCode;
	vec3 m_Colored;

private:
	char m_FormatTalkQuest[512];
	char m_aLanguage[16];
	std::map < int , bool > m_HidenMenu;

	/* #########################################################################
		FUNCTIONS PLAYER ENGINE 
	######################################################################### */
	void TickOnlinePlayer();
	void TickSystemTalk();

	void HandleTuningParams();
public:
	CPlayer(CGS *pGS, int ClientID);
	virtual ~CPlayer();

	bool IsBot() 
	{ 
		return (bool)(m_ClientID >= MAX_PLAYERS && m_ClientID < MAX_CLIENTS); 
	};
	virtual int GetBotID()                                               { return -1; };
	virtual int GetSpawnBot()                                            { return -1; };
	virtual int GetBotSub()                                              { return -1; };
	virtual bool CheckQuestSnapPlayer(int SnappingClient, bool SnapData) { return true; };
	virtual	int GetHealth()                                              { return Acc().PlayerHealth;};
	virtual void SetDungeonAllowedSpawn(bool Spawn)                      { return; };
	virtual void SetStandart(int Health, int Mana);
	virtual int GetMoodNameplacesType(int SnappingClient);

	virtual void Tick();
	virtual int GetTeam();
	virtual int GetStartHealth();
	virtual int GetAttributeCount(int BonusID, bool Really = false);
	virtual void Snap(int SnappingClient);
	virtual int GetItemEquip(int EquipID, int SkipItemID = -1) const;
	
private:
	virtual void TryRespawn();

public:
	void PostTick();

	CCharacter *GetCharacter();

	void KillCharacter(int Weapon = WEAPON_GAME);
	void OnDisconnect();
	void OnDirectInput(CNetObj_PlayerInput *NewInput);
	void OnPredictedInput(CNetObj_PlayerInput *NewInput);

	int GetCID() const { return m_ClientID; };
	/* #########################################################################
		FUNCTIONS PLAYER HELPER 
	######################################################################### */
	void ProgressBar(const char *Name, int MyLevel, int MyExp, int ExpNeed, int GivedExp);
	bool Upgrade(int Count, int *Upgrade, int *Useless, int Price, int MaximalUpgrade, const char *UpgradeName);
	const char *AtributeName(int BonusID) const;

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

	int ExpNeed(int Level);
	int GetStartMana();
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
	ItemSql::ItemPlayer &GetItem(int ItemID);

	AccountMainSql::StructData &Acc() 
	{ 
		return AccountMainSql::Data[m_ClientID]; 
	};
	int GetLevelDisciple(int Class);

	// разговоры с нпс функции
	void SetTalking(int TalkedID, bool ToProgress);
	void ClearTalking();
	int GetTalkedID() const { return m_TalkingNPC.m_TalkedID; };

	// форматирование текста функции
	const char *FormatedTalkedText() const { return m_FormatTalkQuest; };
	void FormatTextQuest(int DataBotID, const char *pText);
	void ClearFormatQuestText();
};

#endif
