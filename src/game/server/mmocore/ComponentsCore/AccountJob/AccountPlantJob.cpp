/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#include <engine/shared/config.h>
#include <game/server/gamecontext.h>
#include "AccountPlantJob.h"

using namespace sqlstr;
std::map < int , AccountPlantJob::StructPlants > AccountPlantJob::ms_aPlants;

void AccountPlantJob::OnInitWorld(const char* pWhereLocalWorld)
{
	ResultPtr pRes = SJK.SD("*", "tw_position_plant", pWhereLocalWorld);
	while(pRes->next())
	{
		const int ID = pRes->getInt("ID");
		ms_aPlants[ID].m_ItemID = pRes->getInt("ItemID");
		ms_aPlants[ID].m_Level = pRes->getInt("Level");
		ms_aPlants[ID].m_PositionX = pRes->getInt("PositionX");
		ms_aPlants[ID].m_PositionY = pRes->getInt("PositionY");
		ms_aPlants[ID].m_Distance = pRes->getInt("Distance");
	}
}

void AccountPlantJob::OnInitAccount(CPlayer *pPlayer)
{
	ResultPtr pRes = SJK.SD("*", "tw_accounts_plants", "WHERE AccountID = '%d'", pPlayer->Acc().m_AccountID);
	if(pRes->next())
	{
		for(int i = 0; i < NUM_JOB_ACCOUNTS_STATS; i++)
		{
			const char* pFieldName = pPlayer->Acc().m_aPlant[i].m_aFieldName;
			pPlayer->Acc().m_aPlant[i].m_Value = pRes->getInt(pFieldName);
		}
		return;
	}
	pPlayer->Acc().m_aPlant[JOB_LEVEL].m_Value = 1;
	pPlayer->Acc().m_aPlant[JOB_UPGR_COUNTS].m_Value = 1;
	SJK.ID("tw_accounts_plants", "(AccountID) VALUES ('%d')", pPlayer->Acc().m_AccountID);	
}

int AccountPlantJob::GetPlantLevel(vec2 Pos)
{
	for(const auto & Plant : ms_aPlants)
	{
		vec2 Position = vec2(Plant.second.m_PositionX, Plant.second.m_PositionY);
		if(distance(Position, Pos) < Plant.second.m_Distance)
			return Plant.second.m_Level;
	}
	return -1;
}

int AccountPlantJob::GetPlantItemID(vec2 Pos)
{
	for(const auto & Plant : ms_aPlants)
	{
		vec2 Position = vec2(Plant.second.m_PositionX, Plant.second.m_PositionY);
		if(distance(Position, Pos) < Plant.second.m_Distance)
			return Plant.second.m_ItemID;
	}
	return -1;
}

void AccountPlantJob::ShowMenu(CPlayer* pPlayer)
{
	const int ClientID = pPlayer->GetCID();
	const int JobLevel = pPlayer->Acc().m_aPlant[JOB_LEVEL].m_Value;
	const int JobExperience = pPlayer->Acc().m_aPlant[JOB_EXPERIENCE].m_Value;
	const int JobUpgrades = pPlayer->Acc().m_aPlant[JOB_UPGRADES].m_Value;
	const int JobUpgrCounts = pPlayer->Acc().m_aPlant[JOB_UPGR_COUNTS].m_Value;
	const int ExperienceNeed = computeExperience(JobExperience);

	GS()->AVM(ClientID, "null", NOPE, TAB_UPGR_JOB, "Plants Point: {INT} :: Level: {INT} Exp: {INT}/{INT}", &JobUpgrades, &JobLevel, &JobExperience, &ExperienceNeed);
	GS()->AVD(ClientID, "PLANTUPGRADE", JOB_UPGR_COUNTS, 20, TAB_UPGR_JOB, "Quantity +{INT} (Price 20P)", &JobUpgrCounts);
}

void AccountPlantJob::ShowPlantsItems(int ClientID)
{
	CPlayer *pPlayer = GS()->GetPlayer(ClientID, true);
	if(!pPlayer)
		return;

	Job()->Item()->ListInventory(pPlayer, FUNCTION_PLANTS, true);
}

void AccountPlantJob::Work(CPlayer* pPlayer, int Level)
{
	const int ClientID = pPlayer->GetCID();
	const int MultiplierExperience = computeExperience(Level) / g_Config.m_SvPlantingIncreaseLevel;
	pPlayer->Acc().m_aPlant[JOB_EXPERIENCE].m_Value += clamp(MultiplierExperience, 1, MultiplierExperience);

	int ExperienceNeed = computeExperience(pPlayer->Acc().m_aPlant[JOB_LEVEL].m_Value);
	for (; pPlayer->Acc().m_aPlant[JOB_EXPERIENCE].m_Value >= ExperienceNeed; )
	{
		pPlayer->Acc().m_aPlant[JOB_EXPERIENCE].m_Value -= ExperienceNeed;
		pPlayer->Acc().m_aPlant[JOB_LEVEL].m_Value++;
		pPlayer->Acc().m_aPlant[JOB_UPGRADES].m_Value++;

		if(pPlayer->GetCharacter() && pPlayer->GetCharacter()->IsAlive())
		{
			GS()->CreateSound(pPlayer->GetCharacter()->m_Core.m_Pos, 4);
			GS()->CreateDeath(pPlayer->GetCharacter()->m_Core.m_Pos, ClientID);
			GS()->CreateText(pPlayer->GetCharacter(), false, vec2(0, -40), vec2(0, -1), 40, "plants up");
		}

		ExperienceNeed = computeExperience(pPlayer->Acc().m_aPlant[JOB_LEVEL].m_Value);
		GS()->ChatFollow(ClientID, "Plants Level UP. Now Level {INT}!", &pPlayer->Acc().m_aPlant[JOB_LEVEL].m_Value);
	}

	pPlayer->ProgressBar("Plants", pPlayer->Acc().m_aPlant[JOB_LEVEL].m_Value, pPlayer->Acc().m_aPlant[JOB_EXPERIENCE].m_Value, ExperienceNeed, MultiplierExperience);
	Job()->SaveAccount(pPlayer, SAVE_PLANT_DATA);
}

bool AccountPlantJob::OnHandleVoteCommands(CPlayer *pPlayer, const char *CMD, const int VoteID, const int VoteID2, int Get, const char *GetText)
{
	const int ClientID = pPlayer->GetCID();	
	if(PPSTR(CMD, "PLANTUPGRADE") == 0)
	{
		if(pPlayer->Upgrade(Get, &pPlayer->Acc().m_aPlant[VoteID].m_Value, &pPlayer->Acc().m_aPlant[JOB_UPGRADES].m_Value, VoteID2, 3))
		{
			GS()->Mmo()->SaveAccount(pPlayer, SaveType::SAVE_PLANT_DATA);
			GS()->StrongUpdateVotes(ClientID, MenuList::MENU_UPGRADE);
		}
		return true;
	}	

	return false;
}