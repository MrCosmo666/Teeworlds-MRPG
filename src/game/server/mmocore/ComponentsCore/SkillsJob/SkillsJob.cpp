/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#include <game/server/gamecontext.h>
#include "SkillsJob.h"

using namespace sqlstr;

std::map < int , CSkillInformation > SkillsJob::ms_aSkillsData;
std::map < int , std::map < int , CSkill > > SkillsJob::ms_aSkills;

void SkillsJob::OnInit()
{ 
	SJK.SDT("*", "tw_skills_list", [&](ResultPtr pRes)
	{
		while(pRes->next())
		{
			const int SkillID = (int)pRes->getInt("ID");
			str_copy(ms_aSkillsData[SkillID].m_aName, pRes->getString("SkillName").c_str(), sizeof(ms_aSkillsData[SkillID].m_aName));
			str_copy(ms_aSkillsData[SkillID].m_aDesc, pRes->getString("SkillDesc").c_str(), sizeof(ms_aSkillsData[SkillID].m_aDesc));
			str_copy(ms_aSkillsData[SkillID].m_aBonusInfo, pRes->getString("BonusInfo").c_str(), sizeof(ms_aSkillsData[SkillID].m_aBonusInfo));
			ms_aSkillsData[SkillID].m_ManaProcent = (int)pRes->getInt("ManaProcent");
			ms_aSkillsData[SkillID].m_PriceSP = (int)pRes->getInt("Price");
			ms_aSkillsData[SkillID].m_MaxLevel = (int)pRes->getInt("MaxLevel");
			ms_aSkillsData[SkillID].m_BonusCount = (int)pRes->getInt("BonusCount");
			ms_aSkillsData[SkillID].m_Passive = (bool)pRes->getBoolean("Passive");
		}
	});
}

void SkillsJob::OnInitAccount(CPlayer *pPlayer)
{
	const int ClientID = pPlayer->GetCID();
	ResultPtr pRes = SJK.SD("*", "tw_accounts_skills", "WHERE OwnerID = '%d'", pPlayer->Acc().m_AuthID);
	while(pRes->next())
	{
		const int SkillID = (int)pRes->getInt("SkillID");
		ms_aSkills[ClientID][SkillID].SetSkillOwner(pPlayer);
		ms_aSkills[ClientID][SkillID].m_SkillID = SkillID;
		ms_aSkills[ClientID][SkillID].m_Level = (int)pRes->getInt("SkillLevel");
		ms_aSkills[ClientID][SkillID].m_SelectedEmoticion = (int)pRes->getInt("SelectedEmoticion");
	}		
}

void SkillsJob::OnResetClient(int ClientID)
{
	ms_aSkills.erase(ClientID);
}

bool SkillsJob::OnHandleMenulist(CPlayer* pPlayer, int Menulist, bool ReplaceMenu)
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
			GS()->AV(ClientID, "null");
			GS()->ShowVotesItemValueInformation(pPlayer, itSkillPoint);
			GS()->AV(ClientID, "null");
			
			ShowMailSkillList(pPlayer, false);
			ShowMailSkillList(pPlayer, true);
			return true;
		}
		return false;
	}
	return false;
}

bool SkillsJob::OnHandleTile(CCharacter* pChr, int IndexCollision)
{
	CPlayer* pPlayer = pChr->GetPlayer();
	const int ClientID = pPlayer->GetCID();
	if (pChr->GetHelper()->TileEnter(IndexCollision, TILE_LEARN_SKILL))
	{
		GS()->Chat(ClientID, "You can see menu in the votes!");
		pChr->m_Core.m_ProtectHooked = pChr->m_SkipDamage = true;
		GS()->ResetVotes(ClientID, pPlayer->m_OpenVoteMenu);
		return true;
	}
	else if (pChr->GetHelper()->TileExit(IndexCollision, TILE_LEARN_SKILL))
	{
		GS()->Chat(ClientID, "You left the active zone, menu is restored!");
		pChr->m_Core.m_ProtectHooked = pChr->m_SkipDamage = false;
		GS()->ResetVotes(ClientID, pPlayer->m_OpenVoteMenu);
		return true;
	}
	return false;
}

bool SkillsJob::OnHandleVoteCommands(CPlayer* pPlayer, const char* CMD, const int VoteID, const int VoteID2, int Get, const char* GetText)
{
	const int ClientID = pPlayer->GetCID();
	if (PPSTR(CMD, "SKILLLEARN") == 0)
	{
		const int SkillID = VoteID;
		if (pPlayer->GetSkill(SkillID).Upgrade())
			GS()->StrongUpdateVotes(ClientID, pPlayer->m_OpenVoteMenu);
		return true;
	}

	if (PPSTR(CMD, "SKILLCHANGEEMOTICION") == 0)
	{
		const int SkillID = VoteID;
		pPlayer->GetSkill(SkillID).SelectNextControlEmote();
		GS()->StrongUpdateVotes(ClientID, pPlayer->m_OpenVoteMenu);
		return true;
	}
	return false;
}

void SkillsJob::ShowMailSkillList(CPlayer *pPlayer, bool Passive)
{
	const int ClientID = pPlayer->GetCID();
	pPlayer->m_Colored = BLUE_COLOR;
	GS()->AVL(ClientID, "null", "{STR} skill's", (Passive ? "Passive" : "Active"));
	for (const auto& sk : ms_aSkillsData)
	{
		if(sk.second.m_Passive == Passive)
			SkillSelected(pPlayer, sk.first);
	}
	GS()->AV(ClientID, "null");
}

void SkillsJob::SkillSelected(CPlayer *pPlayer, int SkillID)
{
	CSkill& pSkill = pPlayer->GetSkill(SkillID);
	const int ClientID = pPlayer->GetCID();
	const bool IsPassive = pSkill.Info().m_Passive;
	const bool IsMaxLevel = pSkill.m_Level >= pSkill.Info().m_MaxLevel;
	const int HideID = NUM_TAB_MENU + InventoryJob::ms_aItemsInfo.size() + SkillID;

	GS()->AVHI(ClientID, "skill", HideID, LIGHT_BLUE_COLOR, "{STR} - {INT}SP ({INT}/{INT})", pSkill.Info().m_aName, &pSkill.Info().m_PriceSP, &pSkill.m_Level, &pSkill.Info().m_MaxLevel);
	if(!IsMaxLevel)
	{
		const int NewBonus = pSkill.GetBonus() + pSkill.Info().m_BonusCount;
		GS()->AVM(ClientID, "null", NOPE, HideID, "Next level {INT} {STR}", &NewBonus, pSkill.Info().m_aBonusInfo);
	}
	else
	{
		const int ActiveBonus = pSkill.GetBonus();
		GS()->AVM(ClientID, "null", NOPE, HideID, "Max level {INT} {STR}", &ActiveBonus, pSkill.Info().m_aBonusInfo);
	}
	GS()->AVM(ClientID, "null", NOPE, HideID, "{STR}", pSkill.Info().m_aDesc);

	if(!IsPassive)
	{
		GS()->AVM(ClientID, "null", NOPE, HideID, "Mana required {INT}%", &pSkill.Info().m_ManaProcent);
		if(pSkill.IsLearned())
		{
			GS()->AVM(ClientID, "null", NOPE, HideID, "F1 Bind: (bind 'key' say \"/useskill {INT}\")", &SkillID);
			GS()->AVM(ClientID, "SKILLCHANGEEMOTICION", SkillID, HideID, "Used on {STR}", pSkill.GetControlEmoteStateName());
		}
	}

	if(!IsMaxLevel)
		GS()->AVM(ClientID, "SKILLLEARN", SkillID, HideID, "Learn {STR}", pSkill.Info().m_aName);
}

void SkillsJob::ParseEmoticionSkill(CPlayer *pPlayer, int EmoticionID)
{
	if(!pPlayer || !pPlayer->IsAuthed() || !pPlayer->GetCharacter())
		return;

	const int ClientID = pPlayer->GetCID();
	for (auto& skillplayer : ms_aSkills[ClientID])
	{
		CSkill& pSkill = pPlayer->GetSkill(skillplayer.first);
		if (pSkill.m_SelectedEmoticion == EmoticionID)
			pSkill.Use();
	}
}
