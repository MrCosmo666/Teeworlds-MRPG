/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#include <engine/shared/config.h>
#include <game/server/gamecontext.h>
#include "AccountPlantJob.h"

using namespace sqlstr;
std::map < int , AccountPlantJob::StructPlants > AccountPlantJob::ms_aPlants;

void AccountPlantJob::OnInitWorld(const char* pWhereLocalWorld)
{
	std::shared_ptr<ResultSet> RES(SJK.SD("*", "tw_position_plant", pWhereLocalWorld));
	while(RES->next())
	{
		const int ID = RES->getInt("ID");
		ms_aPlants[ID].m_ItemID = RES->getInt("ItemID");
		ms_aPlants[ID].m_Level = RES->getInt("Level");
		ms_aPlants[ID].m_PositionX = RES->getInt("PositionX");
		ms_aPlants[ID].m_PositionY = RES->getInt("PositionY");	
		ms_aPlants[ID].m_Distance = RES->getInt("Distance");
	}
}

void AccountPlantJob::OnInitAccount(CPlayer *pPlayer)
{
	std::shared_ptr<ResultSet> RES(SJK.SD("*", "tw_accounts_plants", "WHERE AccountID = '%d'", pPlayer->Acc().m_AuthID));
	if(RES->next())
	{
		for(int i = 0; i < NUM_PLANT; i++)
			pPlayer->Acc().m_aPlant[i] = RES->getInt(str_PLANT((PLANT) i));
		return;
	}
	pPlayer->Acc().m_aPlant[PlLevel] = 1;
	pPlayer->Acc().m_aPlant[PlCounts] = 1;
	SJK.ID("tw_accounts_plants", "(AccountID) VALUES ('%d')", pPlayer->Acc().m_AuthID);	
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

void AccountPlantJob::ShowMenu(int ClientID)
{
	CPlayer *pPlayer = GS()->GetPlayer(ClientID, true);
	if(!pPlayer)
		return;

	int ExperienceNeed = kurosio::computeExperience(pPlayer->Acc().m_aPlant[PlLevel]);
	GS()->AVM(ClientID, "null", NOPE, TAB_UPGR_JOB, "Plants Point: {INT} :: Level: {INT} Exp: {INT}/{INT}", 
		&pPlayer->Acc().m_aPlant[PlUpgrade], &pPlayer->Acc().m_aPlant[PlLevel], &pPlayer->Acc().m_aPlant[PlExp], &ExperienceNeed);
	GS()->AVD(ClientID, "PLANTUPGRADE", PlCounts, 20, TAB_UPGR_JOB, "Quantity +{INT} (Price 20P)", &pPlayer->Acc().m_aPlant[PlCounts]);
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
	const int MultiplierExperience = kurosio::computeExperience(Level) / g_Config.m_SvPlantingIncreaseLevel;
	pPlayer->Acc().m_aPlant[MnrExp] += clamp(MultiplierExperience, 1, MultiplierExperience);

	int ExperienceNeed = kurosio::computeExperience(pPlayer->Acc().m_aPlant[PlLevel]);
	for (; pPlayer->Acc().m_aPlant[MnrExp] >= ExperienceNeed; )
	{
		pPlayer->Acc().m_aPlant[PlExp] -= ExperienceNeed;
		pPlayer->Acc().m_aPlant[PlLevel]++;
		pPlayer->Acc().m_aPlant[PlUpgrade]++;
		if(pPlayer->GetCharacter() && pPlayer->GetCharacter()->IsAlive())
		{
			GS()->CreateSound(pPlayer->GetCharacter()->m_Core.m_Pos, 4);
			GS()->CreateDeath(pPlayer->GetCharacter()->m_Core.m_Pos, ClientID);
			GS()->CreateText(pPlayer->GetCharacter(), false, vec2(0, -40), vec2(0, -1), 40, "plants up");
		}
		ExperienceNeed = kurosio::computeExperience(pPlayer->Acc().m_aPlant[PlLevel]);
		GS()->ChatFollow(ClientID, "Plants Level UP. Now Level {INT}!", &pPlayer->Acc().m_aPlant[PlLevel]);
	}
	pPlayer->ProgressBar("Plants", pPlayer->Acc().m_aPlant[PlLevel], pPlayer->Acc().m_aPlant[PlExp], ExperienceNeed, MultiplierExperience);
	Job()->SaveAccount(pPlayer, SAVE_PLANT_DATA);
}

bool AccountPlantJob::OnParsingVoteCommands(CPlayer *pPlayer, const char *CMD, const int VoteID, const int VoteID2, int Get, const char *GetText)
{
	const int ClientID = pPlayer->GetCID();	
	if(PPSTR(CMD, "PLANTUPGRADE") == 0)
	{
		char aBuf[32];
		str_format(aBuf, sizeof(aBuf), "Harvest '%s'", str_PLANT((PLANT)VoteID));
		if(pPlayer->Upgrade(Get, &pPlayer->Acc().m_aPlant[VoteID], &pPlayer->Acc().m_aPlant[PlUpgrade], VoteID2, 3, aBuf))
		{
			GS()->Mmo()->SaveAccount(pPlayer, SaveType::SAVE_PLANT_DATA);
			GS()->StrongUpdateVotes(ClientID, MenuList::MENU_UPGRADE);
		}
		return true;
	}	

	return false;
}