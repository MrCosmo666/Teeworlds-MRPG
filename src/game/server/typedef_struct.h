/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#ifndef GAME_TYPEDEF_CONTEX_H
#define GAME_TYPEDEF_CONTEX_H

enum SafeList
{
	Empty = -1
};

struct StructSkillInformation
{
	char sSkillName[32];
	char sSkillDesc[64];
	char sBonusName[64];
	int sBonusCount;
	int sManaCost;
	int sManaUseCost;
	int sSkillPrice;
	int sSkillMaxLevel;
	bool sPassive;
};
typedef StructSkillInformation SkillInfo;

struct StructSkills
{
	int sBonusLevel;
};
typedef StructSkills SkillPlayer;




#endif
