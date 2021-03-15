/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#include <base/stdafx.h>

#include "AccountMinerCore.h"

#include <engine/shared/config.h>
#include <game/server/gamecontext.h>

using namespace sqlstr;
std::map < int , CAccountMinerCore::StructOres > CAccountMinerCore::ms_aOre;

void CAccountMinerCore::ShowMenu(CPlayer *pPlayer)
{
	const int ClientID = pPlayer->GetCID();
	const int JobLevel = pPlayer->Acc().m_aMiningData[JOB_LEVEL];
	const int JobExperience = pPlayer->Acc().m_aMiningData[JOB_EXPERIENCE];
	const int JobUpgrades = pPlayer->Acc().m_aMiningData[JOB_UPGRADES];
	const int JobUpgrCounts = pPlayer->Acc().m_aMiningData[JOB_UPGR_COUNTS];
	const int ExperienceNeed = computeExperience(JobLevel);

	GS()->AVM(ClientID, "null", NOPE, TAB_UPGR_JOB, "Miner Point: {INT} :: Level: {INT} Exp: {INT}/{INT}", JobUpgrades, JobLevel, JobExperience, ExperienceNeed);
	GS()->AVD(ClientID, "MINERUPGRADE", JOB_UPGR_COUNTS, 20, TAB_UPGR_JOB, "Quantity +{INT} (Price 20P)", JobUpgrCounts);
}

int CAccountMinerCore::GetOreLevel(vec2 Pos) const
{
	for(const auto& ore : ms_aOre)
	{
		vec2 Position = vec2(ore.second.m_PositionX, ore.second.m_PositionY);
		if(distance(Position, Pos) < ore.second.m_Distance)
			return ore.second.m_Level;
	}
	return -1;
}

int CAccountMinerCore::GetOreItemID(vec2 Pos) const
{
	for(const auto& ore : ms_aOre)
	{
		vec2 Position = vec2(ore.second.m_PositionX, ore.second.m_PositionY);
		if(distance(Position, Pos) < ore.second.m_Distance)
			return ore.second.m_ItemID;
	}
	return -1;
}

int CAccountMinerCore::GetOreHealth(vec2 Pos) const
{
	for(const auto& ore : ms_aOre)
	{
		vec2 Position = vec2(ore.second.m_PositionX, ore.second.m_PositionY);
		if(distance(Position, Pos) < ore.second.m_Distance)
			return ore.second.m_StartHealth;
	}
	return -1;
}

void CAccountMinerCore::Work(CPlayer *pPlayer, int Level)
{
	const int ClientID = pPlayer->GetCID();
	const int MultiplierExperience = computeExperience(Level) / g_Config.m_SvMiningIncreaseLevel;
	pPlayer->Acc().m_aMiningData[JOB_EXPERIENCE] += clamp(MultiplierExperience, 1, MultiplierExperience);

	int ExperienceNeed = computeExperience((int)pPlayer->Acc().m_aMiningData[JOB_LEVEL]);
	for( ; (int)pPlayer->Acc().m_aMiningData[JOB_EXPERIENCE] >= ExperienceNeed; )
	{
		pPlayer->Acc().m_aMiningData[JOB_EXPERIENCE] -= ExperienceNeed;
		pPlayer->Acc().m_aMiningData[JOB_LEVEL]++;
		pPlayer->Acc().m_aMiningData[JOB_UPGR_COUNTS]++;

		if(pPlayer->GetCharacter() && pPlayer->GetCharacter()->IsAlive())
		{
			GS()->CreateSound(pPlayer->GetCharacter()->m_Core.m_Pos, 4);
			GS()->CreateDeath(pPlayer->GetCharacter()->m_Core.m_Pos, ClientID);
			GS()->CreateText(pPlayer->GetCharacter(), false, vec2(0, -40), vec2(0, -1), 40, "miner up");
		}

		const int NewLevel = pPlayer->Acc().m_aMiningData[JOB_LEVEL];
		ExperienceNeed = computeExperience(NewLevel);
		GS()->ChatFollow(ClientID, "Miner Level UP. Now Level {INT}!", NewLevel);
	}

	pPlayer->ProgressBar("Miner", pPlayer->Acc().m_aMiningData[JOB_LEVEL], pPlayer->Acc().m_aMiningData[JOB_EXPERIENCE], ExperienceNeed, MultiplierExperience);
	Job()->SaveAccount(pPlayer, SAVE_MINER_DATA);
}

void CAccountMinerCore::OnInitAccount(CPlayer* pPlayer)
{
	ResultPtr pRes = SJK.SD("*", "tw_accounts_miner", "WHERE AccountID = '%d'", pPlayer->Acc().m_AccountID);
	if (pRes->next())
	{
		for(int i = 0; i < NUM_JOB_ACCOUNTS_STATS; i++)
		{
			const char* pFieldName = pPlayer->Acc().m_aMiningData[i].getFieldName();
			pPlayer->Acc().m_aMiningData[i] = pRes->getInt(pFieldName);
		}
		return;
	}
	pPlayer->Acc().m_aMiningData[JOB_LEVEL] = 1;
	pPlayer->Acc().m_aMiningData[JOB_UPGR_COUNTS] = 1;
	SJK.ID("tw_accounts_miner", "(AccountID) VALUES ('%d')", pPlayer->Acc().m_AccountID);
}

void CAccountMinerCore::OnInitWorld(const char* pWhereLocalWorld)
{
	ResultPtr pRes = SJK.SD("*", "tw_position_miner", pWhereLocalWorld);
	while (pRes->next())
	{
		const int ID = pRes->getInt("ID");
		ms_aOre[ID].m_ItemID = pRes->getInt("ItemID");
		ms_aOre[ID].m_Level = pRes->getInt("Level");
		ms_aOre[ID].m_StartHealth = pRes->getInt("Health");
		ms_aOre[ID].m_PositionX = pRes->getInt("PositionX");
		ms_aOre[ID].m_PositionY = pRes->getInt("PositionY");
		ms_aOre[ID].m_Distance = pRes->getInt("Distance");
	}
}

bool CAccountMinerCore::OnHandleVoteCommands(CPlayer* pPlayer, const char* CMD, const int VoteID, const int VoteID2, int Get, const char* GetText)
{
	const int ClientID = pPlayer->GetCID();
	if (PPSTR(CMD, "MINERUPGRADE") == 0)
	{
		if (pPlayer->Upgrade(Get, &pPlayer->Acc().m_aMiningData[VoteID], &pPlayer->Acc().m_aMiningData[JOB_UPGRADES], VoteID2, 3))
		{
			GS()->Mmo()->SaveAccount(pPlayer, SaveType::SAVE_MINER_DATA);
			GS()->StrongUpdateVotes(ClientID, MenuList::MENU_UPGRADE);
		}
		return true;
	}

	return false;
}