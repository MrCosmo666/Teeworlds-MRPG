/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#include <engine/shared/config.h>
#include <game/server/gamecontext.h>
#include "AccountPlantJob.h"

using namespace sqlstr;
std::map < int , AccountPlantJob::StructPlants > AccountPlantJob::Plants;

// Инициализация класса
void AccountPlantJob::OnInitWorld(const char* pWhereLocalWorld)
{
	// загрузить плантации
	boost::scoped_ptr<ResultSet> RES(SJK.SD("*", "tw_position_plant", pWhereLocalWorld));
	while(RES->next())
	{
		int ID = RES->getInt("ID");
		Plants[ ID ].ItemID = RES->getInt("ItemID");
		Plants[ ID ].Level = RES->getInt("Level");
		Plants[ ID ].PositionX = RES->getInt("PositionX");
		Plants[ ID ].PositionY = RES->getInt("PositionY");	
		Plants[ ID ].Distance = RES->getInt("Distance");
	}
}

// Загрузка данных игрока
void AccountPlantJob::OnInitAccount(CPlayer *pPlayer)
{
	boost::scoped_ptr<ResultSet> RES(SJK.SD("*", "tw_accounts_plants", "WHERE AccountID = '%d'", pPlayer->Acc().AuthID));
	if(RES->next())
	{
		for(int i = 0; i < NUM_PLANT; i++)
			pPlayer->Acc().Plant[i] = RES->getInt(str_PLANT((PLANT) i));
		return;
	}
	pPlayer->Acc().Plant[PlLevel] = 1;
	pPlayer->Acc().Plant[PlCounts] = 1;
	SJK.ID("tw_accounts_plants", "(AccountID) VALUES ('%d')", pPlayer->Acc().AuthID);	
	return;	
}

int AccountPlantJob::GetPlantLevel(vec2 Pos)
{
	for(auto plant = Plants.begin(); plant != Plants.end(); plant++)
	{
		vec2 Position = vec2(plant->second.PositionX, plant->second.PositionY);
		if(distance(Position, Pos) < plant->second.Distance)
			return plant->second.Level;
	}
	return -1;
}

int AccountPlantJob::GetPlantItemID(vec2 Pos)
{
	for(auto plant = Plants.begin(); plant != Plants.end(); plant++)
	{
		vec2 Position = vec2(plant->second.PositionX, plant->second.PositionY);
		if(distance(Position, Pos) < plant->second.Distance)
			return plant->second.ItemID;
	}
	return -1;
}

// меню крафта
void AccountPlantJob::ShowMenu(int ClientID)
{
	CPlayer *pPlayer = GS()->GetPlayer(ClientID, true);
	if(!pPlayer)
		return;

	int NeedExp = ExpNeed(pPlayer->Acc().Plant[PlLevel]);
	GS()->AVM(ClientID, "null", NOPE, TAB_UPGR_JOB, _("[Plants Point: {INT}] Level: {INT} Exp: {INT}/{INT}"), 
		&pPlayer->Acc().Plant[PlUpgrade], &pPlayer->Acc().Plant[PlLevel], &pPlayer->Acc().Plant[PlExp], &NeedExp);
	GS()->AVD(ClientID, "PLANTUPGRADE", PlCounts, 20, TAB_UPGR_JOB, _("[Price 20P]Plants bonus +1(Active {INT})"), &pPlayer->Acc().Plant[PlCounts]);
}
// меню всех растений
void AccountPlantJob::ShowPlantsItems(int ClientID)
{
	CPlayer *pPlayer = GS()->GetPlayer(ClientID, true);
	if(!pPlayer)
		return;

	Job()->Item()->ListInventory(pPlayer, FUNCTION_PLANTS, true);
}

void AccountPlantJob::Work(int ClientID, int Exp)
{
	CPlayer *pPlayer = GS()->GetPlayer(ClientID, true);
	if(!pPlayer)
		return;

	pPlayer->Acc().Plant[PlExp] += Exp;
	for( ; pPlayer->Acc().Plant[PlExp] >= ExpNeed(pPlayer->Acc().Plant[PlLevel]) ; ) 
	{
		pPlayer->Acc().Plant[PlExp] -= ExpNeed(pPlayer->Acc().Plant[PlLevel]);
		pPlayer->Acc().Plant[PlLevel]++, pPlayer->Acc().Plant[PlUpgrade]++;

		int ClientID = pPlayer->GetCID();
		if(pPlayer->GetCharacter() && pPlayer->GetCharacter()->IsAlive())
		{
			GS()->CreateSound(pPlayer->GetCharacter()->m_Core.m_Pos, 4);
			GS()->CreateDeath(pPlayer->GetCharacter()->m_Core.m_Pos, ClientID);
			GS()->CreateText(pPlayer->GetCharacter(), false, vec2(0, -40), vec2(0, -1), 40, "plants up", GS()->Server()->GetWorldID(ClientID));
		}
		GS()->ChatFollow(ClientID, "Plants Level UP. Now Level {INT}!", &pPlayer->Acc().Plant[PlLevel]);
	}
	pPlayer->ProgressBar("Plants", pPlayer->Acc().Plant[PlLevel], pPlayer->Acc().Plant[PlExp], ExpNeed(pPlayer->Acc().Plant[PlLevel]), Exp);
	Job()->SaveAccount(pPlayer, SAVE_PLANT_DATA);
}

bool AccountPlantJob::OnVotingMenu(CPlayer *pPlayer, const char *CMD, const int VoteID, const int VoteID2, int Get, const char *GetText)
{
	int ClientID = pPlayer->GetCID();	
	if(PPSTR(CMD, "PLANTUPGRADE") == 0)
	{
		char aBuf[32];
		str_format(aBuf, sizeof(aBuf), "Harvest '%s'", str_PLANT((PLANT)VoteID));
		if(pPlayer->Upgrade(Get, &pPlayer->Acc().Plant[VoteID], 
								 &pPlayer->Acc().Plant[PlUpgrade], VoteID2, 1000, aBuf))
		{
			GS()->Mmo()->SaveAccount(pPlayer, SaveType::SAVE_PLANT_DATA);
			GS()->VResetVotes(ClientID, MenuList::MENU_UPGRADE);
		}
		return true;
	}	

	return false;
}

// подсчет опыта для лвл апа
int AccountPlantJob::ExpNeed(int Level) const { return (g_Config.m_SvPlantLeveling+Level*2)*(Level*Level); }