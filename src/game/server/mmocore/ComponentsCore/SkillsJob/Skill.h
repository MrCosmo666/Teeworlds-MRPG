/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#ifndef GAME_SERVER_SKILLJOB_SKILL_H
#define GAME_SERVER_SKILLJOB_SKILL_H

#include "SkillInformation.h"

class CSkill
{
	class CGS* m_pGS;
	class CPlayer* m_pPlayer;
	CGS* GS() const { return m_pGS; }

public:
	int m_SkillID;
	int m_Level;
	int m_SelectedEmoticion;

	CSkillInformation& Info() const;
	void SetSkillOwner(CPlayer* pPlayer);
	int GetID() const { return m_SkillID; }
	int GetBonus() const { return m_Level * Info().m_BonusCount; };
	bool IsLearned() const { return m_Level > 0; }
	const char* GetControlEmoteStateName() const { return Info().GetControlEmoteStateName(m_SelectedEmoticion); }

	void SelectNextControlEmote();
	bool Upgrade();
	bool Use();
};

#endif
 