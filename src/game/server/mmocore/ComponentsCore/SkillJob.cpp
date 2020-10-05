/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#include <game/server/gamecontext.h>
#include "SkillJob.h"

#include <game/server/mmocore/GameEntities/Skills/healthturret/healer-health.h>
#include <game/server/mmocore/GameEntities/Skills/sleepygravity/sleepygravity.h>
#include <game/server/mmocore/GameEntities/Skills/noctislucisteleport/noctis_teleport.h>

using namespace sqlstr;

std::map < int , SkillJob::StructSkillInformation > SkillJob::ms_aSkillsData;
std::map < int , std::map < int , SkillJob::StructSkills > > SkillJob::ms_aSkills;

void SkillJob::OnInit()
{ 
	SJK.SDT("*", "tw_skills_list", [&](ResultSet* RES)
	{
		while(RES->next())
		{
			const int SkillID = (int)RES->getInt("ID");
			str_copy(ms_aSkillsData[SkillID].m_aName, RES->getString("SkillName").c_str(), sizeof(ms_aSkillsData[SkillID].m_aName));
			str_copy(ms_aSkillsData[SkillID].m_aDesc, RES->getString("SkillDesc").c_str(), sizeof(ms_aSkillsData[SkillID].m_aDesc));
			str_copy(ms_aSkillsData[SkillID].m_aBonusInfo, RES->getString("BonusInfo").c_str(), sizeof(ms_aSkillsData[SkillID].m_aBonusInfo));
			ms_aSkillsData[SkillID].m_ManaProcent = (int)RES->getInt("ManaProcent");
			ms_aSkillsData[SkillID].m_PriceSP = (int)RES->getInt("Price");
			ms_aSkillsData[SkillID].m_MaxLevel = (int)RES->getInt("MaxLevel");
			ms_aSkillsData[SkillID].m_BonusCount = (int)RES->getInt("BonusCount");
			ms_aSkillsData[SkillID].m_Passive = (bool)RES->getBoolean("Passive");
		}
	});
}

void SkillJob::OnInitAccount(CPlayer *pPlayer)
{
	const int ClientID = pPlayer->GetCID();
	std::shared_ptr<ResultSet> RES(SJK.SD("SkillID, SkillLevel, SelectedEmoticion", "tw_accounts_skills", "WHERE OwnerID = '%d'", pPlayer->Acc().m_AuthID));
	while(RES->next())
	{
		const int SkillID = (int)RES->getInt("SkillID");
		ms_aSkills[ClientID][SkillID].m_Level = (int)RES->getInt("SkillLevel");
		ms_aSkills[ClientID][SkillID].m_SelectedEmoticion = (int)RES->getInt("SelectedEmoticion");
	}		
}

void SkillJob::OnResetClient(int ClientID)
{
	if (ms_aSkills.find(ClientID) != ms_aSkills.end())
		ms_aSkills.erase(ClientID);
}

bool SkillJob::OnHandleMenulist(CPlayer* pPlayer, int Menulist, bool ReplaceMenu)
{
	if (ReplaceMenu)
	{
		CCharacter* pChr = pPlayer->GetCharacter();
		if (!pChr || !pChr->IsAlive()) 
			return false;

		if (pChr->GetHelper()->BoolIndex(TILE_LEARN_SKILL))
		{
			const int ClientID = pPlayer->GetCID();
			GS()->AVH(ClientID, TAB_INFO_SKILL, GREEN_COLOR, "Skill Learn Information");
			GS()->AVM(ClientID, "null", NOPE, TAB_INFO_SKILL, "Here you can learn passive and active skills");
			GS()->AVM(ClientID, "null", NOPE, TAB_INFO_SKILL, "You can bind active skill any button using the console");
			GS()->AV(ClientID, "null", "");
			GS()->ShowItemValueInformation(pPlayer, itSkillPoint);
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
		GS()->Chat(ClientID, "You can see menu in the votes!");
		pChr->m_Core.m_ProtectHooked = pChr->m_NoAllowDamage = true;
		GS()->ResetVotes(ClientID, pPlayer->m_OpenVoteMenu);
		return true;
	}
	else if (pChr->GetHelper()->TileExit(IndexCollision, TILE_LEARN_SKILL))
	{
		GS()->Chat(ClientID, "You left the active zone, menu is restored!");
		pChr->m_Core.m_ProtectHooked = pChr->m_NoAllowDamage = false;
		GS()->ResetVotes(ClientID, pPlayer->m_OpenVoteMenu);
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
			GS()->UpdateVotes(ClientID, pPlayer->m_OpenVoteMenu);
		return true;
	}

	if (PPSTR(CMD, "SKILLCHANGEEMOTICION") == 0)
	{
		const int SkillID = VoteID;
		ms_aSkills[ClientID][SkillID].m_SelectedEmoticion++;
		if (ms_aSkills[ClientID][SkillID].m_SelectedEmoticion >= NUM_EMOTICONS)
			ms_aSkills[ClientID][SkillID].m_SelectedEmoticion = -1;

		SJK.UD("tw_accounts_skills", "SelectedEmoticion = '%d' WHERE SkillID = '%d' AND OwnerID = '%d'", ms_aSkills[ClientID][SkillID].m_SelectedEmoticion, SkillID, pPlayer->Acc().m_AuthID);
		GS()->UpdateVotes(ClientID, pPlayer->m_OpenVoteMenu);
		return true;
	}
	return false;
}

int SkillJob::GetSkillBonus(int ClientID, int SkillID) const
{
	if(ms_aSkills[ClientID].find(SkillID) != ms_aSkills[ClientID].end())
		return (ms_aSkills[ClientID][SkillID].m_Level * ms_aSkillsData[SkillID].m_BonusCount);
	return 0;
}

int SkillJob::GetSkillLevel(int ClientID, int SkillID) const
{
	if(ms_aSkills[ClientID].find(SkillID) != ms_aSkills[ClientID].end())
		return ms_aSkills[ClientID][SkillID].m_Level;
	return 0;
}

void SkillJob::ShowMailSkillList(CPlayer *pPlayer, bool Passive)
{
	const int ClientID = pPlayer->GetCID();
	pPlayer->m_Colored = BLUE_COLOR;
	GS()->AVL(ClientID, "null", "{STR} skill's", (Passive ? "Passive" : "Active"));
	for (const auto& sk : ms_aSkillsData)
	{
		if(sk.second.m_Passive == Passive)
			SkillSelected(pPlayer, sk.first);
	}
	GS()->AV(ClientID, "null", "");
}

void SkillJob::SkillSelected(CPlayer *pPlayer, int SkillID)
{
	if(ms_aSkillsData.find(SkillID) == ms_aSkillsData.end()) 
		return;

	const int ClientID = pPlayer->GetCID();
	const int LevelOwn = GetSkillLevel(ClientID, SkillID);
	const int BonusSkill = GetSkillBonus(ClientID, SkillID) + ms_aSkillsData[SkillID].m_BonusCount;
	const bool Passive = ms_aSkillsData[SkillID].m_Passive;
	const int HideID = NUM_TAB_MENU + InventoryJob::ms_aItemsInfo.size() + SkillID;
	GS()->AVHI(ClientID, "skill", HideID, LIGHT_BLUE_COLOR, "{STR} - {INT}SP ({INT}/{INT})", ms_aSkillsData[SkillID].m_aName, &ms_aSkillsData[SkillID].m_PriceSP, &LevelOwn, &ms_aSkillsData[SkillID].m_MaxLevel);
	if(Passive)
	{
		GS()->AVM(ClientID, "null", NOPE, HideID, "Next level +{INT} {STR}", &BonusSkill, ms_aSkillsData[SkillID].m_aBonusInfo);
		GS()->AVM(ClientID, "null", NOPE, HideID, "{STR}", ms_aSkillsData[SkillID].m_aDesc);
		GS()->AVM(ClientID, "SKILLLEARN", SkillID, HideID, "Learn {STR}", ms_aSkillsData[SkillID].m_aName);
		return;
	}

	GS()->AVM(ClientID, "null", NOPE, HideID, "Mana required (-{INT}%)", &ms_aSkillsData[SkillID].m_ManaProcent);
	GS()->AVM(ClientID, "null", NOPE, HideID, "{STR}", ms_aSkillsData[SkillID].m_aDesc);
	GS()->AVM(ClientID, "null", NOPE, HideID, "Next level +{INT} {STR}", &BonusSkill, ms_aSkillsData[SkillID].m_aBonusInfo);
	if (LevelOwn >= 1)
	{
		GS()->AVM(ClientID, "null", NOPE, HideID, "F1 Bind: (bind 'key' say \"/useskill {INT}\")", &SkillID);
		GS()->AVM(ClientID, "SKILLCHANGEEMOTICION", SkillID, HideID, "Used on {STR}", GetSelectedEmoticion(ms_aSkills[ClientID][SkillID].m_SelectedEmoticion));
	}
	GS()->AVM(ClientID, "SKILLLEARN", SkillID, HideID, "Learn {STR}", ms_aSkillsData[SkillID].m_aName);
}

bool SkillJob::UpgradeSkill(CPlayer *pPlayer, int SkillID)
{
	const int ClientID = pPlayer->GetCID();
	std::shared_ptr<ResultSet> RES(SJK.SD("*", "tw_accounts_skills", "WHERE SkillID = '%d' AND OwnerID = '%d'", SkillID, pPlayer->Acc().m_AuthID));
	if (RES->next())
	{
		if (ms_aSkills[ClientID][SkillID].m_Level >= ms_aSkillsData[SkillID].m_MaxLevel)
		{
			GS()->Chat(ClientID, "Skill already maximum level!");
			return false;
		}

		if (pPlayer->CheckFailMoney(ms_aSkillsData[SkillID].m_PriceSP, itSkillPoint))
			return false;

		ms_aSkills[ClientID][SkillID].m_Level++;
		SJK.UD("tw_accounts_skills", "SkillLevel = '%d' WHERE SkillID = '%d' AND OwnerID = '%d'", ms_aSkills[ClientID][SkillID].m_Level, SkillID, pPlayer->Acc().m_AuthID);
		GS()->Chat(ClientID, "Increased the skill [{STR} level to {INT}]", ms_aSkillsData[SkillID].m_aName, &ms_aSkills[ClientID][SkillID].m_Level);
		return true;
	}

	if (pPlayer->CheckFailMoney(ms_aSkillsData[SkillID].m_PriceSP, itSkillPoint))
		return false;

	ms_aSkills[ClientID][SkillID].m_Level = 1;
	ms_aSkills[ClientID][SkillID].m_SelectedEmoticion = -1;
	SJK.ID("tw_accounts_skills", "(SkillID, OwnerID, SkillLevel) VALUES ('%d', '%d', '1');", SkillID, pPlayer->Acc().m_AuthID);
	GS()->Chat(ClientID, "Learned a new skill [{STR}]", ms_aSkillsData[SkillID].m_aName);
	return true;
}

bool SkillJob::UseSkill(CPlayer *pPlayer, int SkillID)
{
	if(!pPlayer || !pPlayer->GetCharacter() || GetSkillLevel(pPlayer->GetCID(), SkillID) <= 0) 
		return false;

	// mana check
	const int SkillProcent = ms_aSkillsData[SkillID].m_ManaProcent;
	const int ManaPrice = kurosio::translate_to_procent_rest(pPlayer->GetStartMana(), SkillProcent);
	CCharacter* pChr = pPlayer->GetCharacter();
	if(ManaPrice > 0 && pChr->CheckFailMana(ManaPrice))
		return false;

	const vec2 PlayerPosition = pChr->GetPos();
	const int ClientID = pPlayer->GetCID();
	const int SkillBonus = GetSkillBonus(ClientID, SkillID);
	if(SkillID == Skill::SkillHeartTurret)
	{
		for(CHealthHealer *pHh = (CHealthHealer*)GS()->m_World.FindFirst(CGameWorld::ENTYPE_SKILLTURRETHEART); pHh; pHh = (CHealthHealer *)pHh->TypeNext())
		{
			if(pHh->m_pPlayer->GetCID() != ClientID)
				continue;

			pHh->Reset();
			break;
		}
		const int PowerLevel = ManaPrice;
		new CHealthHealer(&GS()->m_World, pPlayer, SkillBonus, PowerLevel, PlayerPosition);
		return true;
	}

	if(SkillID == Skill::SkillSleepyGravity)
	{
		for(CSleepyGravity *pHh = (CSleepyGravity*)GS()->m_World.FindFirst(CGameWorld::ENTYPE_SLEEPYGRAVITY); pHh; pHh = (CSleepyGravity *)pHh->TypeNext())
		{
			if(pHh->m_pPlayer->GetCID() != ClientID)
				continue;
		
			pHh->Reset();
			break;
		}
		const int PowerLevel = ManaPrice;
		new CSleepyGravity(&GS()->m_World, pPlayer, SkillBonus, PowerLevel, PlayerPosition);
		return true;
	}

	if(SkillID == Skill::SkillNoctisTeleport)
	{
		new CNoctisTeleport(&GS()->m_World, PlayerPosition, pChr, SkillBonus);
		return true;
	}

	if(SkillID == Skill::SkillBlessingGodWar)
	{
		for(int i = 0; i < MAX_PLAYERS; i++)
		{
			CPlayer* pPlayerSearch = GS()->GetPlayer(i, true, true);
			if(!pPlayerSearch || !GS()->IsClientEqualWorldID(ClientID) || distance(PlayerPosition, pPlayerSearch->GetCharacter()->GetPos()) > 800
				|| (pPlayerSearch->GetCharacter()->IsAllowedPVP(ClientID) && i != ClientID))
				continue;

			const int RealAmmo = 10 + pPlayerSearch->GetAttributeCount(Stats::StAmmo);
			const int RestoreAmmo = kurosio::translate_to_procent_rest(RealAmmo, min(SkillBonus, 100));
			for(int i = WEAPON_GUN; i <= WEAPON_LASER; i++)
			{
				pPlayerSearch->GetCharacter()->GiveWeapon(i, RestoreAmmo);
				GS()->CreateSound(PlayerPosition, SOUND_CTF_GRAB_PL);
				GS()->CreateDeath(PlayerPosition, i);
			}
		}

		GS()->CreateText(NULL, false, vec2(PlayerPosition.x, PlayerPosition.y - 96.0f), vec2(0, 0), 40, "RECOVERY AMMO");
		return true;
	}

	return false;
}

void SkillJob::ParseEmoticionSkill(CPlayer *pPlayer, int EmoticionID)
{
	if(!pPlayer || !pPlayer->GetCharacter()) 
		return;

	const int ClientID = pPlayer->GetCID();
	for (const auto& skillplayer : ms_aSkills[ClientID])
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