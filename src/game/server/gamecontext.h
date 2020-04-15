/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#ifndef GAME_SERVER_GAMECONTEXT_H
#define GAME_SERVER_GAMECONTEXT_H

#include <base/another/kurhelper.h>

#include <engine/server.h>
#include <engine/storage.h>
#include <engine/console.h>
#include <engine/shared/memheap.h>
#include <teeother/components/localization.h>
#include <game/server/enum_context.h>
#include <game/server/typedef_struct.h>
#include <game/layers.h>
#include <game/voting.h>

#include "eventhandler.h"
#include "gamecontroller.h"
#include "gameworld.h"

#include "player_bot.h"
#include "player.h"

#include <game/server/mmocore/controller.h>


#ifdef _MSC_VER
typedef __int32 int32_t;
typedef unsigned __int32 uint32_t;
typedef __int64 int64_t;
typedef unsigned __int64 uint64_t;
#else
#include <stdint.h>
#endif

class CGS : public IGameServer
{
	/* #########################################################################
		VAR AND OBJECT GAMECONTEX DATA 
	######################################################################### */
	int m_WorldID;
	class SqlController *pMmoController;

	IServer *m_pServer;
	class IConsole *m_pConsole;
	
	CLayers m_Layers;
	CCollision m_Collision;

	CNetObjHandler m_NetObjHandler;
	CTuningParams m_Tuning;

	CGS(int Resetting);
	void Construct(int Resetting);
	bool m_Resetting;

	// данжы
	int m_DungeonID;

public:
	IServer *Server() const { return m_pServer; }
	class IConsole *Console() { return m_pConsole; }

	CCollision *Collision() { return &m_Collision; }
	CTuningParams *Tuning() { return &m_Tuning; }
	
	CGS();
	~CGS();
	void Clear();

	SqlController *Mmo() { return pMmoController; }

	CEventHandler m_Events;
	CPlayer *m_apPlayers[MAX_CLIENTS];
	class IGameController *m_pController;
	CGameWorld m_World;

	/* #########################################################################
		SWAP GAMECONTEX DATA 
	######################################################################### */


	// - - - - - - - - - - - -
	struct StructParsing
	{
		int ParsingLifeTick;
		int ParsingClientID;
		int ParsingSaveInt;
		char ParsingType[32];
	};
	typedef std::map < int , StructParsing > InteractiveType;
	static InteractiveType Interactive;
	// - - - - - - - - - - - -
	struct StructInteractiveSub
	{
		// обычные что не можно раскидать на все
		int TempID;
		int TempID2;
		int TempID3;

		// остальное все
		ContextBots::QuestBotInfo QBI;
		ShopMailSql::AuctionItem AuctionItem;

		char RankName[32];
		char GuildName[32];
	};
	typedef std::map < int , StructInteractiveSub > InteractiveSubType;
	static InteractiveSubType InteractiveSub;
	// - - - - - - - - - - - -
	struct StructDungeon
	{
		char Name[64];
		int Level;
		int WorldID;

		int DoorX;
		int DoorY;

		int Players;
		int Progress;
	};
	typedef std::map < int , StructDungeon > DungeonType;
	static DungeonType Dungeon;	
	// - - - - - - - - - - - -

	// - - - - - - - - - - - -
	typedef std::map < int , std::map < std::string , int > > EffectType;
	static EffectType Effects;
	// - - - - - - - - - - - -
	struct StructAttribut
	{
		char Name[32];
		char FieldName[32];
		int ProcentID;
		int UpgradePrice;
		int AtType;
	};
	typedef std::map < int , StructAttribut > AttributType;
	static AttributType AttributInfo;

	/* #########################################################################
		HELPER PLAYER FUNCTION 
	######################################################################### */
	class CCharacter *GetPlayerChar(int ClientID);
	CPlayer *GetPlayer(int ClientID, bool CheckAuthed = false, bool CheckCharacter = false);
	const char* LevelString(int max, int value, int step, char ch1, char ch2);
	ItemSql::ItemInformation &GetItemInfo(int ItemID) const;

	/* #########################################################################
		EVENTS 
	######################################################################### */
	void CreateDamage(vec2 Pos, int Id, vec2 Source, int HealthAmount, int ArmorAmount, bool Self);
	void CreateDamageTranslate(vec2 Pos, int Id, vec2 Source, int HealthAmount, int ArmorAmount, bool Self);
	void CreateHammerHit(vec2 Pos);
	void CreateExplosion(vec2 Pos, int Owner, int Weapon, int MaxDamage);
	void CreatePlayerSpawn(vec2 Pos);
	void CreateDeath(vec2 Pos, int Who);
	void CreateSound(vec2 Pos, int Sound, int64 Mask=-1);
	void CreatePlayerSound(int ClientID, int Sound);
	void SendMmoEffect(vec2 Pos, int EffectID);
	void SendMmoPotion(vec2 Pos, const char *Potion, bool Added);

	/* #########################################################################
		CHAT FUNCTIONS 
	######################################################################### */
private:
	void SendChat(int ChatterClientID, int Mode, int To, const char *pText);

public:
	void Chat(int ClientID, const char* pText, ...);
	void ChatFollow(int ClientID, const char* pText, ...);
	void ChatAccountID(int AccountID, const char* pText, ...);
	void ChatDiscord(bool Icon, const char *Color, const char *Title, const char* pText, ...);
	void ChatDiscordChannel(bool Icon, const char *pChanel, const char *Color, const char *Title, const char* pText, ...);
	void ChatGuild(int GuildID, const char* pText, ...);
	void ChatWorldID(int WorldID, const char *Suffix, const char* pText, ...);
	void Motd(int ClientID, const char* Text, ...);

	/* #########################################################################
		BROADCAST FUNCTIONS 
	######################################################################### */
private:
	struct CBroadcastState
	{
		int m_NoChangeTick;
		char m_PrevMessage[1024];
		
		int m_Priority;
		char m_NextMessage[1024];
		
		int m_LifeSpanTick;
		int m_TimedPriority;
		char m_TimedMessage[1024];
	};
	CBroadcastState m_BroadcastStates[MAX_PLAYERS];

public:
	void AddBroadcast(int ClientID, const char* pText, int Priority, int LifeSpan);
	void SendBroadcast(const char *pText, int ClientID, int Priority, int LifeSpan);
	void SBL(int ClientID, int Priority, int LifeSpan, const char *pText, ...);
	void BroadcastWorldID(int WorldID, int Priority, int LifeSpan, const char *pText, ...);
	void BroadcastTick(int ClientID);

	/* #########################################################################
		PACKET MESSAGE FUNCTIONS 
	######################################################################### */
	void SendEmoticon(int ClientID, int Emoticon);
	void SendWeaponPickup(int ClientID, int Weapon);
	void SendMotd(int ClientID);
	void SendSettings(int ClientID);
	void SendSkinChange(int ClientID, int TargetID);
	void SendEquipItem(int ClientID, int TargetID);
	void SendTeam(int ClientID, int Team, bool DoChatMsg, int TargetID);
	void SendGameMsg(int GameMsgID, int ClientID);
	void SendGameMsg(int GameMsgID, int ParaI1, int ClientID);
	void SendGameMsg(int GameMsgID, int ParaI1, int ParaI2, int ParaI3, int ClientID);
	void SendTuningParams(int ClientID);
	void SendTalkText(int OwnID, int TalkingID, bool PlayerTalked, const char *Message, int Style = -1, int TalkingEmote = -1);
	void ClearTalkText(int OwnID);
	int CheckPlayerMessageWorldID(int ClientID);
	int64 MaskWorldID();

	/* #########################################################################
		ENGINE GAMECONTEXT 
	######################################################################### */	
	virtual void OnInit(int WorldID);
	virtual void OnConsoleInit();
	virtual void OnShutdown();

	virtual void OnTick();
	void OnTickLocalWorld();
	virtual void OnPreSnap();
	virtual void OnSnap(int ClientID);
	virtual void OnPostSnap();
	
	virtual void OnMessage(int MsgID, CUnpacker *pUnpacker, int ClientID);
	virtual void OnClientConnected(int ClientID);

	virtual void OnClientEnter(int ClientID);
	virtual void OnClientDrop(int ClientID, const char *pReason, bool ChangeWorld = false);
	virtual void OnClientDirectInput(int ClientID, void *pInput);
	virtual void OnClientPredictedInput(int ClientID, void *pInput);
	virtual void ChangeWorld(int ClientID);

	virtual bool IsClientReady(int ClientID) const;
	virtual bool IsClientPlayer(int ClientID) const;

	bool CheckClient(int ClientID) const;
	virtual const char *GameType() const;
	virtual const char *Version() const;
	virtual const char *NetVersion() const;

	virtual void ClearClientData(int ClientID);
	virtual void UpdateWorld();
	virtual int GetRank(int AuthID);

	/* #########################################################################
		CONSOLE GAMECONTEXT 
	######################################################################### */
private:
	static void ConParseSkin(IConsole::IResult *pResult, void *pUserData);
	static void ConGiveItem(IConsole::IResult *pResult, void *pUserData);
	static void ConTuneParam(IConsole::IResult *pResult, void *pUserData);
	static void ConTuneReset(IConsole::IResult *pResult, void *pUserData);
	static void ConTuneDump(IConsole::IResult *pResult, void *pUserData);
	static void ConSay(IConsole::IResult *pResult, void *pUserData);
	static void ConAddCharacter(IConsole::IResult *pResult, void *pUserData);
	static void ConchainSpecialMotdupdate(IConsole::IResult *pResult, void *pUserData, IConsole::FCommandCallback pfnCallback, void *pCallbackUserData);
	static void ConchainSettingUpdate(IConsole::IResult *pResult, void *pUserData, IConsole::FCommandCallback pfnCallback, void *pCallbackUserData);
	static void ConchainGameinfoUpdate(IConsole::IResult *pResult, void *pUserData, IConsole::FCommandCallback pfnCallback, void *pCallbackUserData);

	/* #########################################################################
		VOTING MMO GAMECONTEXT 
	######################################################################### */
	struct CVoteOptions
	{
		char m_aDescription[VOTE_DESC_LENGTH];
		char m_aCommand[VOTE_CMD_LENGTH];
		int m_TempID;
		int m_TempID2;
	};
	std::list<CVoteOptions> m_PlayerVotes[MAX_PLAYERS];

public:
	void ClearVotes(int ClientID);
	void AV(int To, const char *Cmd, const char *Desc, const int ID = -1, const int ID2 = -1, const char *Icon = "unused");
	void AVL(int To, const char* aCmd, const char* pText, ...);
	void AVH(int To, const int ID, vec3 Color, const char* pText, ...);
	void AVHI(int To, const char *Icon, const int ID, vec3 Color, const char* pText, ...);

	void AVM(int To, const char* Type, const int ID, const int HideID, const char* pText, ...);
	void AVMI(int To, const char *Icon, const char *Type, const int ID, const int HideID, const char *pText, ...);
	void AVD(int To, const char* Type, const int ID, const int ID2, const int HideID, const char* pText, ...);

	void ResetVotes(int ClientID, int MenuList);
	void VResetVotes(int ClientID, int MenuID);
	void AddBack(int ClientID);
	void ShowPlayerStats(CPlayer *pPlayer);
	bool ParseVote(int ClientID, const char *CMD, const int VoteID, const int VoteID2, int Get, const char *Text);

	/* #########################################################################
		MMO GAMECONTEXT 
	######################################################################### */
	void CreateBot(short SpawnPoint, int BotID, int SubID);
	virtual void ClearQuestsBot(int QuestID, int Step);
	void CreateText(CEntity *pParent, bool Follow, vec2 Pos, vec2 Vel, int Lifespan, const char *pText, int WorldID);
	void CreateDropBonuses(vec2 Pos, int Type, int Count, int BonusCount = 1);
	void CreateDropItem(vec2 Pos, int ClientID, int ItemID, int Count, int Enchant = 0);
	void CreateDropItem(vec2 Pos, int ClientID, ItemSql::ItemPlayer &PlayerItem, int Count);
	void CreateDropQuest(const ContextBots::QuestBotInfo &BotInfo, int ClientID);
	bool TakeItemCharacter(int ClientID);
	void SendInbox(int ClientID, const char* Name, const char* Desc, int ItemID = -1, int Count = -1, int Enchant = -1);

private:
	void SendDayInfo(int ClientID);

public:
	void ChangeEquipSkin(int ClientID, int ItemID);
	void ClearInteractiveSub(int ClientID);

	int GetWorldID() const { return m_WorldID; }
	int DungeonID() const { return m_DungeonID; }
	int IncreaseCountRaid(int IncreaseCount) const;
	bool GetPlayerCliped(vec2 Pos, float Distance) const;

private:
	int ItDungeon(int WorldID) const;

	/* #########################################################################
		FUNCTIONS PLAYER ITEMS 
	######################################################################### */
	int m_DayEnumType;
	static int m_RaidExp;
};

inline int64 CmaskAll() { return -1; }
inline int64 CmaskOne(int ClientID) { return (int64)1<<ClientID; }
inline int64 CmaskAllExceptOne(int ClientID) { return CmaskAll()^CmaskOne(ClientID); }
inline bool CmaskIsSet(int64 Mask, int ClientID) { return (Mask&CmaskOne(ClientID)) != 0; }

#endif
