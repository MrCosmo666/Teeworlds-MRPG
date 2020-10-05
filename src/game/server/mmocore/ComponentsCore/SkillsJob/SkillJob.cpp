/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#include <game/server/gamecontext.h>
#include "SkillJob.h"

using namespace sqlstr;

std::map < int , CSkillInformation > SkillJob::ms_aSkillsData;
std::map < int , std::map < int , CSkillPlayer > > SkillJob::ms_aSkills;

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
	std::shared_ptr<ResultSet> RES(SJK.SD("*", "tw_accounts_skills", "WHERE OwnerID = '%d'", pPlayer->Acc().m_AuthID));
	while(RES->next())
	{
		const int SkillID = (int)RES->getInt("SkillID");
		ms_aSkills[ClientID][SkillID].SetSkillOwner(pPlayer);
		ms_aSkills[ClientID][SkillID].m_SkillID = SkillID;
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
		if (pPlayer->GetSkill(SkillID).Upgrade())
			GS()->UpdateVotes(ClientID, pPlayer->m_OpenVoteMenu);
		return true;
	}

	if (PPSTR(CMD, "SKILLCHANGEEMOTICION") == 0)
	{
		const int SkillID = VoteID;
		pPlayer->GetSkill(SkillID).SelectNextControlEmote();
		GS()->UpdateVotes(ClientID, pPlayer->m_OpenVoteMenu);
		return true;
	}
	return false;
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
	CSkillPlayer& pSkill = pPlayer->GetSkill(SkillID);
	const int ClientID = pPlayer->GetCID();
	const bool Passive = pSkill.Info().m_Passive;
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

	if(!Passive)
	{
		GS()->AVM(ClientID, "null", NOPE, HideID, "Mana required {INT}%", &pSkill.Info().m_ManaProcent);
		GS()->AVM(ClientID, "null", NOPE, HideID, "{STR}", pSkill.Info().m_aDesc);
		if(pSkill.m_Level >= 1)
		{
			GS()->AVM(ClientID, "null", NOPE, HideID, "F1 Bind: (bind 'key' say \"/useskill {INT}\")", &SkillID);
			GS()->AVM(ClientID, "SKILLCHANGEEMOTICION", SkillID, HideID, "Used on {STR}", pSkill.GetControlEmoteStateName());
		}
	}

	if(!IsMaxLevel)
		GS()->AVM(ClientID, "SKILLLEARN", SkillID, HideID, "Learn {STR}", pSkill.Info().m_aName);
}

void SkillJob::ParseEmoticionSkill(CPlayer *pPlayer, int EmoticionID)
{
	if(!pPlayer || !pPlayer->IsAuthed() || !pPlayer->GetCharacter())
		return;

	const int ClientID = pPlayer->GetCID();
	for (auto& skillplayer : ms_aSkills[ClientID])
	{
		CSkillPlayer& pSkill = pPlayer->GetSkill(skillplayer.first);
		if (pSkill.m_SelectedEmoticion == EmoticionID)
			pSkill.Use();
	}
}
