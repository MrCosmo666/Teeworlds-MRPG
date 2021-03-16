/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#ifndef GAME_SERVER_COMPONENT_SKILL_DATA_INFO_H
#define GAME_SERVER_COMPONENT_SKILL_DATA_INFO_H

class CSkillDataInfo
{
public:
	char m_aName[32];
	char m_aDesc[64];
	char m_aBonusInfo[64];
	int m_BonusCount;
	int m_ManaProcent;
	int m_PriceSP;
	int m_MaxLevel;
	bool m_Passive;

	static const char* GetControlEmoteStateName(int EmoticionID);
	static std::map< int, CSkillDataInfo > ms_aSkillsData;
};

#endif
