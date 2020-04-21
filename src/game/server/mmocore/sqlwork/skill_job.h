/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#ifndef GAME_SERVER_SKILLJOB_H
#define GAME_SERVER_SKILLJOB_H

#include "../component.h"

class SkillJob : public CMmoComponent
{
/* #########################################################################
	GLOBAL SKILL CLASS 
######################################################################### */
	struct StructSkillInformation
	{
		char m_SkillName[32];
		char m_SkillDesc[64];
		char m_SkillBonusInfo[64];
		int m_BonusCount;
		int m_ManaCost;
		int m_ManaSupport;
		int m_SkillPrice;
		int m_SkillMaxLevel;
		bool m_Passive;
	};
	typedef StructSkillInformation SkillInfo;

	struct StructSkills
	{
		int m_SkillLevel;
	};
	typedef StructSkills SkillPlayer;

	static std::map < int, SkillInfo > SkillData;
	static std::map < int, std::map < int, SkillPlayer > > Skill;

public:
	virtual void OnInitGlobal();
	virtual void OnInitAccount(CPlayer *pPlayer);
	virtual void OnResetClientData(int ClientID);
	virtual bool OnPlayerHandleTile(CCharacter* pChr, int IndexCollision);
	virtual bool OnPlayerHandleMainMenu(CPlayer* pPlayer, int Menulist, bool ReplaceMenu);
	virtual bool OnParseVotingMenu(CPlayer* pPlayer, const char* CMD, const int VoteID, const int VoteID2, int Get, const char* GetText);

	int GetSkillBonus(int ClientID, int SkillID) const;
	int GetSkillLevel(int ClientID, int SkillID) const;
	int SkillsSize() const { return SkillData.size(); };

	void SkillSelected(CPlayer *pPlayer, int SkillID);
	bool UpgradeSkill(CPlayer *pPlayer, int SkillID);
	bool UseSkill(CPlayer *pPlayer, int SkillID);

private:
	void ShowMailSkillList(CPlayer* pPlayer);
};

#endif
 