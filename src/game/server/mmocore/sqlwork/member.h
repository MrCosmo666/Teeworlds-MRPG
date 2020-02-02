/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#ifndef GAME_SERVER_SQLMEMBER_H
#define GAME_SERVER_SQLMEMBER_H

#include "../component.h"
#include <map>

class MemberDoor;
class MemberSql : public CMmoComponent
{
/* #########################################################################
	VAR AND OBJECTS MEMBER 
######################################################################### */
	struct MemberStruct
	{
		char mMemberName[32];
		int mLevel;
		int mExperience;
		int mOwnerID;
		int mBank;
		int mUpgrades[EMEMBERUPGRADE::NUM_EMEMBERUPGRADE];
		int mScore;
	};
	
	struct MemberStructHouse
	{
		int mX;
		int mY;
		int mDoorX;
		int mDoorY;
		int mTextX;
		int mTextY;
		int mWorldID;
		int mPrice;
		int mEveryDay;
		int mOwnerMemberID;
		MemberDoor *m_Door;
	};

	struct MemberStructRank
	{
		char Rank[32];
		int MemberID;
		int Access;
	};

	typedef std::map < int , MemberStruct > MemberType;
	static MemberType Member;

	typedef std::map < int , MemberStructHouse > MemberHouseType;
	static MemberHouseType MemberHouse;

	typedef std::map < int , MemberStructRank > MemberRankType;
	static MemberRankType MemberRank;

/* #########################################################################
	LOADING MEMBER 
######################################################################### */
	void LoadMemberRank(int MemberID);

public:
	virtual void OnInitGlobal();
	virtual void OnInitLocal(const char *pLocal);
	virtual void OnTick();
	virtual void OnPaymentTime();

/* #########################################################################
	GET CHECK MEMBER 
######################################################################### */
	const char *MemberName(int MemberID) const;
	bool IsLeaderPlayer(CPlayer *pPlayer, int Access = -1) const;
	int GetMemberChairBonus(int MemberID, int Field) const;
private:
	std::string UpgradeNames(int Field, bool DataTable = false);
	int ExpForLevel(int Level);
public:
/* #########################################################################
	FUNCTIONS MEMBER MEMBER 
######################################################################### */
	void CreateGuild(int ClientID, const char *MemberName);
	void JoinGuild(int AuthID, int MemberID);
	void ExitGuild(int AccountID);
	void ShowMenuGuild(CPlayer *pPlayer);

	void AddExperience(int MemberID);	
	bool AddMoneyBank(int MemberID, int Money);
	bool RemoveMoneyBank(int MemberID, int Money);
	bool UpgradeGuild(int MemberID, int Field);

/* #########################################################################
	GET CHECK MEMBER RANK MEMBER 
######################################################################### */
	const char *AccessNames(int Access);
	const char *GetMemberRank(int MemberID, int RankID);
	int FindMemberRank(int MemberID, const char *Rank) const;

/* #########################################################################
	FUNCTIONS MEMBER RANK MEMBER 
######################################################################### */
	void AddRank(int MemberID, const char *Rank);
	void DeleteRank(int RankID, int MemberID);
	void ChangeRank(int RankID, int MemberID, const char *NewRank);
	void ChangeRankAccess(int RankID);
	void ChangePlayerRank(int AuthID, int RankID);
	void ShowMenuRank(CPlayer *pPlayer);

/* #########################################################################
	GET CHECK MEMBER INVITE MEMBER 
######################################################################### */
	int GetGuildPlayerCount(int MemberID);

/* #########################################################################
	FUNCTIONS MEMBER INVITE MEMBER 
######################################################################### */
	bool AddInviteGuild(int MemberID, int OwnerID);
	void ShowInvitesGuilds(int ClientID, int MemberID);
	void ShowFinderGuilds(int ClientID);

/* #########################################################################
	FUNCTIONS MEMBER HISTORY MEMBER 
######################################################################### */
	void ShowHistoryGuild(int ClientID, int MemberID);
	void AddHistoryGuild(int MemberID, const char *Buffer, ...);

/* #########################################################################
	GET CHECK MEMBER HOUSING MEMBER 
######################################################################### */
	int GetHouseMemberID(int HouseID) const;
	int GetHouseWorldID(int HouseID) const;
	int GetPosHouseID(vec2 Pos) const;

	bool GetMemberDoor(int MemberID) const;
	vec2 GetPositionHouse(int MemberID) const;
	int GetMemberHouseID(int MemberID) const;

/* #########################################################################
	FUNCTIONS MEMBER HOUSING MEMBER 
######################################################################### */
	void BuyGuildHouse(int MemberID, int HouseID);
	void SellGuildHouse(int MemberID);
	void ShowBuyHouse(CPlayer *pPlayer, int MID);
	void CheckTimePayment();
	void ChangeStateDoor(int MemberID);

/* #########################################################################
	GLOBAL MEMBER  
######################################################################### */
	virtual bool OnParseVotingMenu(CPlayer *pPlayer, const char *CMD, const int VoteID, const int VoteID2, int Get, const char *GetText);
};

/* #########################################################################
	HOUSE ENTITIES MEMBER  
######################################################################### */
class MemberDoor : public CEntity
{
	int m_MemberID;
	vec2 m_To;
	
public:
	MemberDoor(CGameWorld *pGameWorld, vec2 Pos, int HouseID);
	~MemberDoor();

	virtual void Tick();
	virtual void Snap(int SnappingClient);
};

#endif
 