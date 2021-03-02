/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#ifndef GAME_SERVER_SKILLJOB_H
#define GAME_SERVER_SKILLJOB_H
#include <game/server/mmocore/MmoComponent.h>

#include "Skill.h"

class SkillsJob : public MmoComponent
{
	~SkillsJob()
	{
		ms_aSkillsData.clear();
		ms_aSkills.clear();
	};

	void OnInit() override;
	void OnInitAccount(CPlayer* pPlayer) override;
	void OnResetClient(int ClientID) override;
	bool OnHandleTile(CCharacter* pChr, int IndexCollision) override;
	bool OnHandleMenulist(CPlayer* pPlayer, int Menulist, bool ReplaceMenu) override;
	bool OnHandleVoteCommands(CPlayer* pPlayer, const char* CMD, const int VoteID, const int VoteID2, int Get, const char* GetText) override;

public:
	static std::map < int, CSkillInformation > ms_aSkillsData;
	static std::map < int, std::map < int, CSkill > > ms_aSkills;
	void ParseEmoticionSkill(CPlayer* pPlayer, int EmoticionID);

private:
	void ShowMailSkillList(CPlayer* pPlayer, bool Pasive);
	void SkillSelected(CPlayer* pPlayer, int SkillID);
};

#endif
 