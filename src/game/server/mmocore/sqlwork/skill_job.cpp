/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#include <engine/shared/config.h>
#include <game/server/gamecontext.h>

#include <game/server/player.h>
#include "skill_job.h"

#include "skills/healthturret/healer-health.h"
#include "skills/sleepygravity/sleepygravity.h"

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

// Загрузка данных игрока
void SkillJob::OnInitAccount(CPlayer *pPlayer)
{
	int ClientID = pPlayer->GetCID();
	boost::scoped_ptr<ResultSet> RES(SJK.SD("SkillID, SkillLevel", "tw_skills", "WHERE OwnerID = '%d'", pPlayer->Acc().AuthID));
	while(RES->next())
	{
		const int SkillID = (int)RES->getInt("SkillID");
		Skill[ClientID][SkillID].m_SkillLevel = (int)RES->getInt("SkillLevel");
	}		
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
void SkillJob::ShowMailSkillList(CPlayer *pPlayer)
{
	const int ClientID = pPlayer->GetCID();
	GS()->AVH(ClientID, HSKILLLEARN, vec3(35,80,40), "Skill Learn Information");
	GS()->AVM(ClientID, "null", NOPE, HSKILLLEARN, "Here you can learn passive and active skills");
	GS()->AVM(ClientID, "null", NOPE, HSKILLLEARN, "You can bind active skill any button using the console");
	GS()->AV(ClientID, "null", "");

	for(const auto& sk : SkillData)
		SkillSelected(pPlayer, sk.first);
}

// показать выбранные скиллы
void SkillJob::SkillSelected(CPlayer *pPlayer, int SkillID)
{
	if(SkillData.find(SkillID) == SkillData.end()) return;

	const bool Passive = SkillData[SkillID].m_Passive;
	const int ClientID = pPlayer->GetCID();
	int LevelOwn = GetSkillLevel(ClientID, SkillID);
	int BonusSkill = GetSkillBonus(ClientID, SkillID) + SkillData[SkillID].m_BonusCount;

	// меню выводим
	pPlayer->m_Colored = { 5,10,1 };
	GS()->AVM(ClientID, "SKILLLEARN", SkillID, NOPE, "{STR} [{INT}/{INT}] {STR} [Price {INT}SP]", 
		(Passive ? "Passive" : "Active"), &LevelOwn, &SkillData[SkillID].m_SkillMaxLevel, SkillData[SkillID].m_SkillName, &SkillData[SkillID].m_SkillPrice);

	// пасивный скилл
	if(Passive)
	{
		GS()->AVM(ClientID, "null", NOPE, NOPE, "Next level +{INT} {STR}", &BonusSkill, SkillData[SkillID].m_SkillBonusInfo);
		GS()->AVM(ClientID, "null", NOPE, NOPE, "{STR}", SkillData[SkillID].m_SkillDesc);
		GS()->AV(ClientID, "null", "");
		return;
	}

	// если обычный скилл
	GS()->AVM(ClientID, "null", NOPE, NOPE, "Mana required (first use -{INT} support -{INT})", &SkillData[SkillID].m_ManaCost, &SkillData[SkillID].m_ManaSupport);
	GS()->AVM(ClientID, "null", NOPE, NOPE, "{STR}", SkillData[SkillID].m_SkillDesc);
	GS()->AVM(ClientID, "null", NOPE, NOPE, "Next level +{INT} {STR}", &BonusSkill, SkillData[SkillID].m_SkillBonusInfo);
	GS()->AVM(ClientID, "null", NOPE, NOPE, "F1 Bind: (bind 'key' say \"/useskill {INT}\")", &SkillID);
	GS()->AV(ClientID, "null", "");
}

// улучшение скилла
bool SkillJob::UpgradeSkill(CPlayer *pPlayer, int SkillID)
{
	// если такой скилл есть повышаем уровень
	const int ClientID = pPlayer->GetCID();
	if(Skill[ClientID][SkillID].m_SkillLevel >= SkillData[SkillID].m_SkillMaxLevel)
	{
		GS()->Chat(ClientID, "This a skill already maximum level");
		return false;				
	}

	// проверяем хватает ли денег
	if(pPlayer->CheckFailMoney(SkillData[SkillID].m_SkillPrice, itSkillPoint)) 
		return false;

	if(Skill[ClientID].find(SkillID) != Skill[ClientID].end())
	{
		// добавляем уровень
		Skill[ClientID][SkillID].m_SkillLevel++;
		SJK.UD("tw_skills", "SkillLevel = '%d' WHERE SkillID = '%d' AND OwnerID = '%d'", Skill[ClientID][SkillID].m_SkillLevel, SkillID, pPlayer->Acc().AuthID);
		GS()->Chat(ClientID, "You have increased the skill [{STR} level to {INT}]!", SkillData[SkillID].m_SkillName, &Skill[ClientID][SkillID].m_SkillLevel);
		return true;	
	}

	// создаем неовый скилл
	Skill[ClientID][SkillID].m_SkillLevel = 1;
	SJK.ID("tw_skills", "(SkillID, OwnerID, SkillLevel) VALUES ('%d', '%d', '1');", SkillID, pPlayer->Acc().AuthID);
	GS()->Chat(ClientID, "You have learned a new skill [{STR}]", SkillData[SkillID].m_SkillName);
	return true;
}

// использовать скилл
bool SkillJob::UseSkill(CPlayer *pPlayer, int SkillID)
{
	// проверяем есть ли игрок обьект чар, уровень скилла и имеется ли скилл вообщем в списках
	if(!pPlayer || !pPlayer->GetCharacter() || GetSkillLevel(pPlayer->GetCID(), SkillID) <= 0 ||
		Skill[pPlayer->GetCID()].find(SkillID) == Skill[pPlayer->GetCID()].end()) return false;

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
		// ищем удаляем обьект если такой владелец уже имеется
		for(CHealthHealer *pHh = (CHealthHealer*)GS()->m_World.FindFirst(CGameWorld::ENTYPE_SKILLTURRETHEART); 
			pHh; pHh = (CHealthHealer *)pHh->TypeNext())
		{
			if(pHh->m_pPlayer->GetCID() != pPlayer->GetCID()) continue;
			pHh->Reset();
		}
		// создаем обьект
		new CHealthHealer(&GS()->m_World, pPlayer, SkillLevel, ManaUsePrice, pChr->m_Core.m_Pos);
	}

	// скилл турель гравитации
	if(SkillID == Skill::SkillSleepyGravity)
	{
		// ищем удаляем обьект если такой владелец уже имеется
		for(CSleepyGravity *pHh = (CSleepyGravity*)GS()->m_World.FindFirst(CGameWorld::ENTYPE_SLEEPYGRAVITY); 
			pHh; pHh = (CSleepyGravity *)pHh->TypeNext())
		{
			if(pHh->m_pPlayer->GetCID() != pPlayer->GetCID()) continue;
			pHh->Reset();
		}
		// создаем обьект
		new CSleepyGravity(&GS()->m_World, pPlayer, SkillLevel, ManaUsePrice, pChr->m_Core.m_Pos);
	}

	return false;
}

// парсинг голосований
bool SkillJob::OnParseVotingMenu(CPlayer *pPlayer, const char *CMD, const int VoteID, const int VoteID2, int Get, const char *GetText)
{
	int ClientID = pPlayer->GetCID();

	// улучшение скилла
	if(PPSTR(CMD, "SKILLLEARN") == 0)
	{
		const int SkillID = VoteID;

		// улучшение
		if(UpgradeSkill(pPlayer, SkillID))
		{
			GS()->VResetVotes(ClientID, pPlayer->m_OpenVoteMenu);
		}
		return true;				
	}
	return false;
}
