/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#include <engine/shared/config.h>
#include <game/server/gamecontext.h>
#include "SkillJob.h"

#include <game/server/mmocore/GameEntities/Skills/healthturret/healer-health.h>
#include <game/server/mmocore/GameEntities/Skills/sleepygravity/sleepygravity.h>

using namespace sqlstr;

std::map < int , SkillJob::StructSkillInformation > SkillJob::SkillData;
std::map < int , std::map < int , SkillJob::StructSkills > > SkillJob::Skill;

// Инициализация класса
void SkillJob::OnInit()
{ 
	// загрузить список скиллов
	boost::scoped_ptr<ResultSet> RES(SJK.SD("*", "tw_skills_list"));
	while(RES->next())
	{
		const int SkillID = (int)RES->getInt("ID");
		str_copy(SkillData[SkillID].m_SkillName, RES->getString("SkillName").c_str(), sizeof(SkillData[SkillID].m_SkillName));
		str_copy(SkillData[SkillID].m_SkillDesc, RES->getString("SkillDesc").c_str(), sizeof(SkillData[SkillID].m_SkillDesc));
		str_copy(SkillData[SkillID].m_SkillBonusInfo, RES->getString("BonusInfo").c_str(), sizeof(SkillData[SkillID].m_SkillBonusInfo));
		SkillData[SkillID].m_ManaProcent = (int)RES->getInt("ManaProcent");
		SkillData[SkillID].m_SkillPrice = (int)RES->getInt("Price");
		SkillData[SkillID].m_SkillMaxLevel = (int)RES->getInt("MaxLevel");
		SkillData[SkillID].m_BonusCount = (int)RES->getInt("BonusCount");
		SkillData[SkillID].m_Passive = (bool)RES->getBoolean("Passive");
	}
}

void SkillJob::OnInitAccount(CPlayer *pPlayer)
{
	const int ClientID = pPlayer->GetCID();
	boost::scoped_ptr<ResultSet> RES(SJK.SD("SkillID, SkillLevel, SelectedEmoticion", "tw_accounts_skills", "WHERE OwnerID = '%d'", pPlayer->Acc().AuthID));
	while(RES->next())
	{
		const int SkillID = (int)RES->getInt("SkillID");
		Skill[ClientID][SkillID].m_SkillLevel = (int)RES->getInt("SkillLevel");
		Skill[ClientID][SkillID].m_SelectedEmoticion = (int)RES->getInt("SelectedEmoticion");
	}		
}

void SkillJob::OnResetClient(int ClientID)
{
	if (Skill.find(ClientID) != Skill.end())
		Skill.erase(ClientID);
}

bool SkillJob::OnHandleMenulist(CPlayer* pPlayer, int Menulist, bool ReplaceMenu)
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

bool SkillJob::OnHandleTile(CCharacter* pChr, int IndexCollision)
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
		GS()->Chat(ClientID, "You have left the active zone, menu is restored!");
		pChr->m_Core.m_ProtectHooked = pChr->m_NoAllowDamage = false;
		GS()->ResetVotes(ClientID, MenuList::MAIN_MENU);
		return true;
	}

	return false;
}

bool SkillJob::OnVotingMenu(CPlayer* pPlayer, const char* CMD, const int VoteID, const int VoteID2, int Get, const char* GetText)
{
	const int ClientID = pPlayer->GetCID();
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

		SJK.UD("tw_accounts_skills", "SelectedEmoticion = '%d' WHERE SkillID = '%d' AND OwnerID = '%d'", Skill[ClientID][SkillID].m_SelectedEmoticion, SkillID, pPlayer->Acc().AuthID);
		GS()->VResetVotes(ClientID, pPlayer->m_OpenVoteMenu);
		return true;
	}
	return false;
}

int SkillJob::GetSkillBonus(int ClientID, int SkillID) const
{
	if(Skill[ClientID].find(SkillID) != Skill[ClientID].end())
		return (Skill[ClientID][SkillID].m_SkillLevel * SkillData[SkillID].m_BonusCount);
	return 0;
}

int SkillJob::GetSkillLevel(int ClientID, int SkillID) const
{
	if(Skill[ClientID].find(SkillID) != Skill[ClientID].end())
		return Skill[ClientID][SkillID].m_SkillLevel;
	return 0;
}

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

void SkillJob::SkillSelected(CPlayer *pPlayer, int SkillID)
{
	if(SkillData.find(SkillID) == SkillData.end()) 
		return;

	const int ClientID = pPlayer->GetCID();
	const int LevelOwn = GetSkillLevel(ClientID, SkillID);
	const int BonusSkill = GetSkillBonus(ClientID, SkillID) + SkillData[SkillID].m_BonusCount;
	const bool Passive = SkillData[SkillID].m_Passive;
	const int HideID = NUM_TAB_MENU + ItemJob::ItemsInfo.size() + SkillID;
	GS()->AVHI(ClientID, "skill", HideID, LIGHT_BLUE_COLOR, "{STR} :: {INT}SP ({INT}/{INT})", SkillData[SkillID].m_SkillName, &SkillData[SkillID].m_SkillPrice, &LevelOwn, &SkillData[SkillID].m_SkillMaxLevel);
	if(Passive)
	{
		GS()->AVM(ClientID, "null", NOPE, HideID, "Next level +{INT} {STR}", &BonusSkill, SkillData[SkillID].m_SkillBonusInfo);
		GS()->AVM(ClientID, "null", NOPE, HideID, "{STR}", SkillData[SkillID].m_SkillDesc);
		GS()->AVM(ClientID, "SKILLLEARN", SkillID, HideID, "Learn {STR}", SkillData[SkillID].m_SkillName);
		return;
	}

	// если обычный скилл
	GS()->AVM(ClientID, "null", NOPE, HideID, "Mana required (-{INT}%)", &SkillData[SkillID].m_ManaProcent);
	GS()->AVM(ClientID, "null", NOPE, HideID, "{STR}", SkillData[SkillID].m_SkillDesc);
	GS()->AVM(ClientID, "null", NOPE, HideID, "Next level +{INT} {STR}", &BonusSkill, SkillData[SkillID].m_SkillBonusInfo);
	if (LevelOwn >= 1)
	{
		GS()->AVM(ClientID, "null", NOPE, HideID, "F1 Bind: (bind 'key' say \"/useskill {INT}\")", &SkillID);
		GS()->AVM(ClientID, "SKILLCHANGEEMOTICION", SkillID, HideID, "Used on {STR}", GetSelectedEmoticion(Skill[ClientID][SkillID].m_SelectedEmoticion));
	}
	GS()->AVM(ClientID, "SKILLLEARN", SkillID, HideID, "Learn {STR}", SkillData[SkillID].m_SkillName);
}

bool SkillJob::UpgradeSkill(CPlayer *pPlayer, int SkillID)
{
	const int ClientID = pPlayer->GetCID();
	boost::scoped_ptr<ResultSet> RES(SJK.SD("*", "tw_accounts_skills", "WHERE SkillID = '%d' AND OwnerID = '%d'", SkillID, pPlayer->Acc().AuthID));
	if (RES->next())
	{
		if (Skill[ClientID][SkillID].m_SkillLevel >= SkillData[SkillID].m_SkillMaxLevel)
		{
			GS()->Chat(ClientID, "This a skill already maximum level");
			return false;
		}

		if (pPlayer->CheckFailMoney(SkillData[SkillID].m_SkillPrice, itSkillPoint))
			return false;

		Skill[ClientID][SkillID].m_SkillLevel++;
		SJK.UD("tw_accounts_skills", "SkillLevel = '%d' WHERE SkillID = '%d' AND OwnerID = '%d'", Skill[ClientID][SkillID].m_SkillLevel, SkillID, pPlayer->Acc().AuthID);
		GS()->Chat(ClientID, "You have increased the skill [{STR} level to {INT}]!", SkillData[SkillID].m_SkillName, &Skill[ClientID][SkillID].m_SkillLevel);
		return true;
	}

	if (pPlayer->CheckFailMoney(SkillData[SkillID].m_SkillPrice, itSkillPoint))
		return false;

	Skill[ClientID][SkillID].m_SkillLevel = 1;
	Skill[ClientID][SkillID].m_SelectedEmoticion = -1;
	SJK.ID("tw_accounts_skills", "(SkillID, OwnerID, SkillLevel) VALUES ('%d', '%d', '1');", SkillID, pPlayer->Acc().AuthID);
	GS()->Chat(ClientID, "You have learned a new skill [{STR}]", SkillData[SkillID].m_SkillName);
	return true;
}

bool SkillJob::UseSkill(CPlayer *pPlayer, int SkillID)
{
	if(!pPlayer || !pPlayer->GetCharacter() || GetSkillLevel(pPlayer->GetCID(), SkillID) <= 0) 
		return false;

	// проверяем ману
	const int SkillProcent = SkillData[SkillID].m_ManaProcent;
	const int ManaPrice = kurosio::translate_to_procent_rest(pPlayer->GetStartMana(), SkillProcent);
	CCharacter* pChr = pPlayer->GetCharacter();
	if(ManaPrice > 0 && pChr->CheckFailMana(ManaPrice))
		return false;

	// скилл турель здоровья
	const vec2 PlayerPosition = pChr->GetPos();
	const int ClientID = pPlayer->GetCID();
	const int SkillBonus = GetSkillBonus(ClientID, SkillID);
	if(SkillID == Skill::SkillHeartTurret)
	{
		for(CHealthHealer *pHh = (CHealthHealer*)GS()->m_World.FindFirst(CGameWorld::ENTYPE_SKILLTURRETHEART); pHh; pHh = (CHealthHealer *)pHh->TypeNext())
		{
			if(pHh->m_pPlayer->GetCID() == pPlayer->GetCID())
			{
				pHh->Reset();
				break;
			}
		}
		const int PowerLevel = ManaPrice;
		new CHealthHealer(&GS()->m_World, pPlayer, SkillBonus, PowerLevel, PlayerPosition);
		return true;
	}

	// скилл турель гравитации
	if(SkillID == Skill::SkillSleepyGravity)
	{
		for(CSleepyGravity *pHh = (CSleepyGravity*)GS()->m_World.FindFirst(CGameWorld::ENTYPE_SLEEPYGRAVITY); pHh; pHh = (CSleepyGravity *)pHh->TypeNext())
		{
			if(pHh->m_pPlayer->GetCID() == pPlayer->GetCID())
			{
				pHh->Reset();
				break;
			}
		}
		const int PowerLevel = ManaPrice;
		new CSleepyGravity(&GS()->m_World, pPlayer, SkillBonus, PowerLevel, PlayerPosition);
		return true;
	}
	
	// скилл восстановить патроны
	if(SkillID == Skill::SkillBlessingGodWar)
	{
		for(int i = 0; i < MAX_PLAYERS; i++)
		{
			CPlayer* pPlayerSearch = GS()->GetPlayer(i, true, true);
			if(!pPlayerSearch || GS()->Server()->GetWorldID(i) != GS()->Server()->GetWorldID(ClientID) || distance(PlayerPosition, pPlayerSearch->GetCharacter()->GetPos()) > 800 
				|| pPlayerSearch->GetCharacter()->IsAllowedPVP(ClientID))
				continue;

			const int RealAmmo = 10 + pPlayerSearch->GetAttributeCount(Stats::StAmmo);
			const int RestoreAmmo = kurosio::translate_to_procent_rest(RealAmmo, min(SkillBonus, 100));
			for(int i = WEAPON_GUN; i <= WEAPON_LASER; i++)
			{
				pPlayerSearch->GetCharacter()->GiveWeapon(i, RestoreAmmo);
				GS()->CreateWorldSound(PlayerPosition, SOUND_CTF_GRAB_PL);
				GS()->CreateDeath(PlayerPosition, i);
			}
		}

		GS()->CreateText(NULL, false, vec2(PlayerPosition.x, PlayerPosition.y - 96.0f), vec2(0, 0), 40, "RECOVERY AMMO", GS()->GetWorldID());
		return true;
	}

	return false;
}

void SkillJob::ParseEmoticionSkill(CPlayer *pPlayer, int EmoticionID)
{
	int ClientID = pPlayer->GetCID();
	for (const auto& skillplayer : Skill[ClientID])
	{
		if (skillplayer.second.m_SelectedEmoticion == EmoticionID)
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