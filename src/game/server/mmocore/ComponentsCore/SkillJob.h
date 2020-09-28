/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#ifndef GAME_SERVER_SKILLJOB_H
#define GAME_SERVER_SKILLJOB_H

#include "../MmoComponent.h"

class SkillJob : public MmoComponent
{
	struct StructSkillInformation
	{
		char m_aName[32];
		char m_aDesc[64];
		char m_aBonusInfo[64];
		int m_BonusCount;
		int m_ManaProcent;
		int m_PriceSP;
		int m_MaxLevel;
		bool m_Passive;
	};
	struct StructSkills
	{
		int m_SelectedEmoticion;
		int m_Level;
	};

	static std::map < int, StructSkillInformation > ms_aSkillsData;
	static std::map < int, std::map < int, StructSkills > > ms_aSkills;

public:
	void OnInit() override;
	virtual void OnInitAccount(CPlayer *pPlayer);
	virtual void OnResetClient(int ClientID);
	virtual bool OnHandleTile(CCharacter* pChr, int IndexCollision);
	virtual bool OnHandleMenulist(CPlayer* pPlayer, int Menulist, bool ReplaceMenu);
	virtual bool OnVotingMenu(CPlayer* pPlayer, const char* CMD, const int VoteID, const int VoteID2, int Get, const char* GetText);

	int GetSkillBonus(int ClientID, int SkillID) const;
	int GetSkillLevel(int ClientID, int SkillID) const;
	int SkillsSize() const { return ms_aSkillsData.size(); };
	bool UseSkill(CPlayer *pPlayer, int SkillID);
	void ParseEmoticionSkill(CPlayer* pPlayer, int EmoticionID);

private:
	const char* GetSelectedEmoticion(int EmoticionID) const;
	void ShowMailSkillList(CPlayer* pPlayer, bool Pasive);
	void SkillSelected(CPlayer* pPlayer, int SkillID);
	bool UpgradeSkill(CPlayer* pPlayer, int SkillID);
};

#endif
 