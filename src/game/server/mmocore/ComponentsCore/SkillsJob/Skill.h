/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#ifndef GAME_SERVER_SKILLJOB_SKILL_H
#define GAME_SERVER_SKILLJOB_SKILL_H

#include "SkillInformation.h"

class CSkill
{
	CGS* m_pGS;
	CPlayer* m_pPlayer;
	CGS* GS() { return m_pGS; }

public:
	CSkill() : m_pPlayer(nullptr)
	{
		m_SkillID = 0;
		m_Level = 0;
		m_SelectedEmoticion = -1;
	};

	int m_SkillID;
	int m_Level;
	int m_SelectedEmoticion;

	void SetSkillOwner(CPlayer* pPlayer);

	CSkillInformation& Info() const;
	bool IsLearned() const { return m_Level > 0; }
	int GetID() const { return m_SkillID; }
	int GetBonus() const { return m_Level * Info().m_BonusCount; };
	const char* GetControlEmoteStateName() const { return Info().GetControlEmoteStateName(m_SelectedEmoticion); }

	void SelectNextControlEmote();
	bool Upgrade();
	bool Use();
};

#endif
 