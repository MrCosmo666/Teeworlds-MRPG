/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#include <engine/shared/config.h>
#include <game/server/gamecontext.h>
#include <game/server/Player.h>

#include "SkillJob.h"

#include <game/server/mmocore/GameEntities/Skills/healthturret/healer-health.h>
#include <game/server/mmocore/GameEntities/Skills/sleepygravity/sleepygravity.h>

using namespace sqlstr;

std::map < int , SkillJob::SkillInfo > SkillJob::SkillData;
std::map < int , std::map < int , SkillJob::SkillPlayer > > SkillJob::Skill;

// Инициализация класса
void SkillJob::OnInitGlobal() 
{ 
	// загрузить список скиллов
	boost::scoped_ptr<ResultSet> RES(SJK.SD("*", "tw_skills_list"));
	while(RES->next())
	{
		const int SkillID = (int)RES->getInt("ID");
		str_copy(SkillData[SkillID].m_SkillName, RES->getString("SkillName").c_str(), sizeof(SkillData[SkillID].m_SkillName));
		str_copy(SkillData[SkillID].m_SkillDesc, RES->getString("SkillDesc").c_str(), sizeof(SkillData[SkillID].m_SkillDesc));
		str_copy(SkillData[SkillID].m_SkillBonusInfo, RES->getString("BonusInfo").c_str(), sizeof(SkillData[SkillID].m_SkillBonusInfo));
		SkillData[SkillID].m_ManaCost = (int)RES->getInt("ManaCost");
		SkillData[SkillID].m_ManaSupport = (int)RES->getInt("ManaSupport");
		SkillData[SkillID].m_SkillPrice = (int)RES->getInt("Price");
		SkillData[SkillID].m_SkillMaxLevel = (int)RES->getInt("SkillMaxLevel");
		SkillData[SkillID].m_BonusCount = (int)RES->getInt("BonusCount");
		SkillData[SkillID].m_Passive = (bool)RES->getBoolean("Passive");
	}
}

void SkillJob::OnInitAccount(CPlayer *pPlayer)
{
	const int ClientID = pPlayer->GetCID();
	boost::scoped_ptr<ResultSet> RES(SJK.SD("SkillID, SkillLevel, SelectedEmoticion", "tw_skills", "WHERE OwnerID = '%d'", pPlayer->Acc().AuthID));
	while(RES->next())
	{
		const int SkillID = (int)RES->getInt("SkillID");
		Skill[ClientID][SkillID].m_SkillLevel = (int)RES->getInt("SkillLevel");
		Skill[ClientID][SkillID].m_SelectedEmoticion = (int)RES->getInt("SelectedEmoticion");
	}		
}

void SkillJob::OnResetClientData(int ClientID)
{
	if (Skill.find(ClientID) != Skill.end())
		Skill.erase(ClientID);
}

bool SkillJob::OnPlayerHandleMainMenu(CPlayer* pPlayer, int Menulist, bool ReplaceMenu)
{
	if (ReplaceMenu)
	{
		CCharacter* pChr = pPlayer->GetCharacter();
		if (!pChr) 
			return false;

		if (Menulist == MenuList::MAIN_MENU && pChr->GetHelper()->BoolIndex(TILE_LEARN_SKILL))
		{
			const int ClientID = pPlayer->GetCID();
			GS()->AVH(ClientID, TAB_INFO_SKILL, GREEN_COLOR, "Skill Learn Information");
			GS()->AVM(ClientID, "null", NOPE, TAB_INFO_SKILL, "Here you can learn passive and active skills");
			GS()->AVM(ClientID, "null", NOPE, TAB_INFO_SKILL, "You can bind active skill any button using the console");
			GS()->AV(ClientID, "null", "");

			ShowMailSkillList(pPlayer, false);
			ShowMailSkillList(pPlayer, true);
			return true;
		}
		return false;
	}
	return false;
}

bool SkillJob::OnPlayerHandleTile(CCharacter* pChr, int IndexCollision)
{
	CPlayer* pPlayer = pChr->GetPlayer();
	const int ClientID = pPlayer->GetCID();

	if (pChr->GetHelper()->TileEnter(IndexCollision, TILE_LEARN_SKILL))
	{
		GS()->Chat(ClientID, "You can see list of available skills in the voting!");
		pChr->m_Core.m_ProtectHooked = pChr->m_NoAllowDamage = true;
		GS()->ResetVotes(ClientID, MenuList::MAIN_MENU);
		return true;
	}
	else if (pChr->GetHelper()->TileExit(IndexCollision, TILE_LEARN_SKILL))
	{
		pChr->m_Core.m_ProtectHooked = pChr->m_NoAllowDamage = false;
		GS()->ResetVotes(ClientID, MenuList::MAIN_MENU);
		return true;
	}

	return false;
}

// парсинг голосований
bool SkillJob::OnParseVotingMenu(CPlayer* pPlayer, const char* CMD, const int VoteID, const int VoteID2, int Get, const char* GetText)
{
	int ClientID = pPlayer->GetCID();

	// улучшение скилла
	if (PPSTR(CMD, "SKILLLEARN") == 0)
	{
		const int SkillID = VoteID;
		if (UpgradeSkill(pPlayer, SkillID))
			GS()->VResetVotes(ClientID, pPlayer->m_OpenVoteMenu);

		return true;
	}

	if (PPSTR(CMD, "SKILLCHANGEEMOTICION") == 0)
	{
		const int SkillID = VoteID;
		Skill[ClientID][SkillID].m_SelectedEmoticion++;
		if (Skill[ClientID][SkillID].m_SelectedEmoticion >= NUM_EMOTICONS)
			Skill[ClientID][SkillID].m_SelectedEmoticion = -1;

		SJK.UD("tw_skills", "SelectedEmoticion = '%d' WHERE SkillID = '%d' AND OwnerID = '%d'", Skill[ClientID][SkillID].m_SelectedEmoticion, SkillID, pPlayer->Acc().AuthID);
		GS()->VResetVotes(ClientID, pPlayer->m_OpenVoteMenu);
		return true;
	}

	return false;
}

/* #########################################################################
	HELPER SKILL CLASS 
######################################################################### */
// получить бонус скилла
int SkillJob::GetSkillBonus(int ClientID, int SkillID) const
{
	if(Skill[ClientID].find(SkillID) != Skill[ClientID].end())
		return (Skill[ClientID][SkillID].m_SkillLevel * SkillData[SkillID].m_BonusCount);
	return 0;
}

// получить уровень скилла
int SkillJob::GetSkillLevel(int ClientID, int SkillID) const
{
	if(Skill[ClientID].find(SkillID) != Skill[ClientID].end())
		return Skill[ClientID][SkillID].m_SkillLevel;
	return 0;
}

/* #########################################################################
	FUNCTION SKILL CLASS 
######################################################################### */
// показать лист всех скиллов
void SkillJob::ShowMailSkillList(CPlayer *pPlayer, bool Passive)
{
	int ClientID = pPlayer->GetCID();
	pPlayer->m_Colored = BLUE_COLOR;
	GS()->AVL(ClientID, "null", "{STR} skill's | You have  [{INT}SP]", (Passive ? "Passive" : "Active"), &pPlayer->GetItem(itSkillPoint).Count);
	for (const auto& sk : SkillData)
	{
		if(sk.second.m_Passive == Passive)
		SkillSelected(pPlayer, sk.first);
	}
	GS()->AV(ClientID, "null", "");
}

// показать выбранные скиллы
void SkillJob::SkillSelected(CPlayer *pPlayer, int SkillID)
{
	if(SkillData.find(SkillID) == SkillData.end()) 
		return;

	const int ClientID = pPlayer->GetCID();
	int LevelOwn = GetSkillLevel(ClientID, SkillID);
	int BonusSkill = GetSkillBonus(ClientID, SkillID) + SkillData[SkillID].m_BonusCount;
	int HideID = NUM_TAB_MENU + ItemJob::ItemsInfo.size() + SkillID;

	// меню выводим
	const bool Passive = SkillData[SkillID].m_Passive;
	GS()->AVHI(ClientID, "skill_point", HideID, LIGHT_BLUE_COLOR, " ({INT}/{INT}) {STR} : {INT}SP",  &LevelOwn, &SkillData[SkillID].m_SkillMaxLevel, SkillData[SkillID].m_SkillName, &SkillData[SkillID].m_SkillPrice);
	if(Passive)
	{
		GS()->AVM(ClientID, "null", NOPE, HideID, "Next level +{INT} {STR}", &BonusSkill, SkillData[SkillID].m_SkillBonusInfo);
		GS()->AVM(ClientID, "null", NOPE, HideID, "{STR}", SkillData[SkillID].m_SkillDesc);
		GS()->AVM(ClientID, "SKILLLEARN", SkillID, HideID, "Learn {STR}", SkillData[SkillID].m_SkillName);
		return;
	}

	// если обычный скилл
	GS()->AVM(ClientID, "null", NOPE, HideID, "Mana required (first use -{INT} support -{INT})", &SkillData[SkillID].m_ManaCost, &SkillData[SkillID].m_ManaSupport);
	GS()->AVM(ClientID, "null", NOPE, HideID, "{STR}", SkillData[SkillID].m_SkillDesc);
	GS()->AVM(ClientID, "null", NOPE, HideID, "Next level +{INT} {STR}", &BonusSkill, SkillData[SkillID].m_SkillBonusInfo);
	if (LevelOwn >= 1)
	{
		GS()->AVM(ClientID, "null", NOPE, HideID, "F1 Bind: (bind 'key' say \"/useskill {INT}\")", &SkillID);
		GS()->AVM(ClientID, "SKILLCHANGEEMOTICION", SkillID, HideID, "Used on {STR}", GetSelectedEmoticion(Skill[ClientID][SkillID].m_SelectedEmoticion));
	}
	GS()->AVM(ClientID, "SKILLLEARN", SkillID, HideID, "Learn {STR}", SkillData[SkillID].m_SkillName);
}

// улучшение скилла
bool SkillJob::UpgradeSkill(CPlayer *pPlayer, int SkillID)
{
	const int ClientID = pPlayer->GetCID();
	boost::scoped_ptr<ResultSet> RES(SJK.SD("*", "tw_skills", "WHERE SkillID = '%d' AND OwnerID = '%d'", SkillID, pPlayer->Acc().AuthID));
	if (RES->next())
	{
		// если такой скилл есть повышаем уровень
		if (Skill[ClientID][SkillID].m_SkillLevel >= SkillData[SkillID].m_SkillMaxLevel)
		{
			GS()->Chat(ClientID, "This a skill already maximum level");
			return false;
		}

		// проверяем хватает ли скиллпоинтов
		if (pPlayer->CheckFailMoney(SkillData[SkillID].m_SkillPrice, itSkillPoint))
			return false;

		// добавляем уровень
		Skill[ClientID][SkillID].m_SkillLevel++;
		SJK.UD("tw_skills", "SkillLevel = '%d' WHERE SkillID = '%d' AND OwnerID = '%d'", Skill[ClientID][SkillID].m_SkillLevel, SkillID, pPlayer->Acc().AuthID);
		GS()->Chat(ClientID, "You have increased the skill [{STR} level to {INT}]!", SkillData[SkillID].m_SkillName, &Skill[ClientID][SkillID].m_SkillLevel);
		return true;
	}

	// проверяем хватает ли скиллпоинтов
	if (pPlayer->CheckFailMoney(SkillData[SkillID].m_SkillPrice, itSkillPoint))
		return false;

	// создаем неовый скилл
	Skill[ClientID][SkillID].m_SkillLevel = 1;
	Skill[ClientID][SkillID].m_SelectedEmoticion = -1;
	SJK.ID("tw_skills", "(SkillID, OwnerID, SkillLevel) VALUES ('%d', '%d', '1');", SkillID, pPlayer->Acc().AuthID);
	GS()->Chat(ClientID, "You have learned a new skill [{STR}]", SkillData[SkillID].m_SkillName);
	return true;
}

// использовать скилл
bool SkillJob::UseSkill(CPlayer *pPlayer, int SkillID)
{
	if(!pPlayer || !pPlayer->GetCharacter() || GetSkillLevel(pPlayer->GetCID(), SkillID) <= 0) 
		return false;

	// проверяем ману
	CCharacter *pChr = pPlayer->GetCharacter();
	const int ManaPrice = SkillData[SkillID].m_ManaCost;
	if(ManaPrice > 0 && pChr->CheckFailMana(ManaPrice))
		return true;

	const int ClientID = pPlayer->GetCID();
	const int ManaUsePrice = SkillData[SkillID].m_ManaSupport;
	const int SkillLevel = Skill[ClientID][SkillID].m_SkillLevel;

	// скилл турель здоровья
	if(SkillID == Skill::SkillHeartTurret)
	{
		for(CHealthHealer *pHh = (CHealthHealer*)GS()->m_World.FindFirst(CGameWorld::ENTYPE_SKILLTURRETHEART); 
			pHh; pHh = (CHealthHealer *)pHh->TypeNext())
		{
			if(pHh->m_pPlayer->GetCID() != pPlayer->GetCID()) 
				continue;
			pHh->Reset();
		}
		// создаем обьект
		new CHealthHealer(&GS()->m_World, pPlayer, SkillLevel, ManaUsePrice, pChr->m_Core.m_Pos);
	}

	// скилл турель гравитации
	if(SkillID == Skill::SkillSleepyGravity)
	{
		for(CSleepyGravity *pHh = (CSleepyGravity*)GS()->m_World.FindFirst(CGameWorld::ENTYPE_SLEEPYGRAVITY); 
			pHh; pHh = (CSleepyGravity *)pHh->TypeNext())
		{
			if(pHh->m_pPlayer->GetCID() != pPlayer->GetCID()) 
				continue;

			pHh->Reset();
		}
		new CSleepyGravity(&GS()->m_World, pPlayer, SkillLevel, ManaUsePrice, pChr->m_Core.m_Pos);
	}

	return false;
}

void SkillJob::ParseEmoticionSkill(CPlayer *pPlayer, int EmoticionID)
{
	int ClientID = pPlayer->GetCID();
	for (const auto& skillplayer : Skill[ClientID])
	{
		if (skillplayer.second.m_SelectedEmoticion != EmoticionID)
			continue;
		UseSkill(pPlayer, skillplayer.first);
	}
}

const char* SkillJob::GetSelectedEmoticion(int EmoticionID) const
{
	switch (EmoticionID)
	{
	case EMOTICON_OOP: return "Emoticion Ooop";
	case EMOTICON_EXCLAMATION: return "Emoticion Exclamation";
	case EMOTICON_HEARTS: return "Emoticion Hearts";
	case EMOTICON_DROP: return "Emoticion Drop";
	case EMOTICON_DOTDOT: return "Emoticion ...";
	case EMOTICON_MUSIC: return "Emoticion Music";
	case EMOTICON_SORRY: return "Emoticion Sorry";
	case EMOTICON_GHOST: return "Emoticion Ghost";
	case EMOTICON_SUSHI: return "Emoticion Sushi";
	case EMOTICON_SPLATTEE: return "Emoticion Splatee";
	case EMOTICON_DEVILTEE: return "Emoticion Deviltee";
	case EMOTICON_ZOMG: return "Emoticion Zomg";
	case EMOTICON_ZZZ: return "Emoticion Zzz";
	case EMOTICON_WTF: return "Emoticion Wtf";
	case EMOTICON_EYES: return "Emoticion Eyes";
	case EMOTICON_QUESTION: return "Emoticion Question";
	}
	return "Not selected";
}