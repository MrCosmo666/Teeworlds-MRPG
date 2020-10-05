/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#ifndef GAME_SERVER_SKILLJOB_H
#define GAME_SERVER_SKILLJOB_H

#include "SkillPlayer.h"
#include "SkillInformation.h"

#include <game/server/mmocore/MmoComponent.h>

class SkillJob : public MmoComponent
{
public:
	static std::map < int, CSkillInformation > ms_aSkillsData;
	static std::map < int, std::map < int, CSkillPlayer > > ms_aSkills;

	void OnInit() override;
	virtual void OnInitAccount(CPlayer *pPlayer);
	virtual void OnResetClient(int ClientID);
	virtual bool OnHandleTile(CCharacter* pChr, int IndexCollision);
	virtual bool OnHandleMenulist(CPlayer* pPlayer, int Menulist, bool ReplaceMenu);
	virtual bool OnVotingMenu(CPlayer* pPlayer, const char* CMD, const int VoteID, const int VoteID2, int Get, const char* GetText);

	int SkillsSize() const { return ms_aSkillsData.size(); };
	void ParseEmoticionSkill(CPlayer* pPlayer, int EmoticionID);

private:
	void ShowMailSkillList(CPlayer* pPlayer, bool Pasive);
	void SkillSelected(CPlayer* pPlayer, int SkillID);
};

#endif
 