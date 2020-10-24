/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#include <engine/shared/config.h>
#include <game/server/gamecontext.h>
#include "AccountMinerJob.h"

using namespace sqlstr;
std::map < int , AccountMinerJob::StructOres > AccountMinerJob::ms_aOre;

void AccountMinerJob::ShowMenu(CPlayer *pPlayer)
{
	const int ClientID = pPlayer->GetCID();
	int ExperienceNeed = kurosio::computeExperience(pPlayer->Acc().m_aMiner[MnrLevel]);
	GS()->AVM(ClientID, "null", NOPE, TAB_UPGR_JOB, "Miner Point: {INT} :: Level: {INT} Exp: {INT}/{INT}", 
		&pPlayer->Acc().m_aMiner[MnrUpgrade], &pPlayer->Acc().m_aMiner[MnrLevel], &pPlayer->Acc().m_aMiner[MnrExp], &ExperienceNeed);
	GS()->AVD(ClientID, "MINERUPGRADE", MnrCount, 20, TAB_UPGR_JOB, "Quantity +{INT} (Price 20P)", &pPlayer->Acc().m_aMiner[MnrCount]);
}

int AccountMinerJob::GetOreLevel(vec2 Pos) const
{
	for(const auto& ore : ms_aOre)
	{
		vec2 Position = vec2(ore.second.m_PositionX, ore.second.m_PositionY);
		if(distance(Position, Pos) < ore.second.m_Distance)
			return ore.second.m_Level;
	}
	return -1;
}

int AccountMinerJob::GetOreItemID(vec2 Pos) const
{
	for(const auto& ore : ms_aOre)
	{
		vec2 Position = vec2(ore.second.m_PositionX, ore.second.m_PositionY);
		if(distance(Position, Pos) < ore.second.m_Distance)
			return ore.second.m_ItemID;
	}
	return -1;
}

int AccountMinerJob::GetOreHealth(vec2 Pos) const
{
	for(const auto& ore : ms_aOre)
	{
		vec2 Position = vec2(ore.second.m_PositionX, ore.second.m_PositionY);
		if(distance(Position, Pos) < ore.second.m_Distance)
			return ore.second.m_StartHealth;
	}
	return -1;
}

void AccountMinerJob::Work(CPlayer *pPlayer, int Level)
{
	const int ClientID = pPlayer->GetCID();
	const int MultiplierExperience = kurosio::computeExperience(Level) / g_Config.m_SvMiningIncreaseLevel;
	pPlayer->Acc().m_aMiner[MnrExp] += clamp(MultiplierExperience, 1, MultiplierExperience);

	int ExperienceNeed = kurosio::computeExperience(pPlayer->Acc().m_aMiner[MnrLevel]);
	for( ; pPlayer->Acc().m_aMiner[MnrExp] >= ExperienceNeed; )
	{
		pPlayer->Acc().m_aMiner[MnrExp] -= ExperienceNeed;
		pPlayer->Acc().m_aMiner[MnrLevel]++;
		pPlayer->Acc().m_aMiner[MnrUpgrade]++;

		if(pPlayer->GetCharacter() && pPlayer->GetCharacter()->IsAlive())
		{
			GS()->CreateSound(pPlayer->GetCharacter()->m_Core.m_Pos, 4);
			GS()->CreateDeath(pPlayer->GetCharacter()->m_Core.m_Pos, ClientID);
			GS()->CreateText(pPlayer->GetCharacter(), false, vec2(0, -40), vec2(0, -1), 40, "miner up");
		}
		ExperienceNeed = kurosio::computeExperience(pPlayer->Acc().m_aMiner[MnrLevel]);
		GS()->ChatFollow(ClientID, "Miner Level UP. Now Level {INT}!", &pPlayer->Acc().m_aMiner[MnrLevel]);
	}
	pPlayer->ProgressBar("Miner", pPlayer->Acc().m_aMiner[MnrLevel], pPlayer->Acc().m_aMiner[MnrExp], ExperienceNeed, MultiplierExperience);
	Job()->SaveAccount(pPlayer, SAVE_MINER_DATA);
}

void AccountMinerJob::OnInitAccount(CPlayer* pPlayer)
{
	std::shared_ptr<ResultSet> RES(SJK.SD("*", "tw_accounts_miner", "WHERE AccountID = '%d'", pPlayer->Acc().m_AuthID));
	if (RES->next())
	{
		for (int i = 0; i < NUM_MINER; i++)
			pPlayer->Acc().m_aMiner[i] = RES->getInt(str_MINER((MINER)i));
		return;
	}
	pPlayer->Acc().m_aMiner[MnrLevel] = 1;
	pPlayer->Acc().m_aMiner[MnrCount] = 1;
	SJK.ID("tw_accounts_miner", "(AccountID) VALUES ('%d')", pPlayer->Acc().m_AuthID);
}

void AccountMinerJob::OnInitWorld(const char* pWhereLocalWorld)
{
	std::shared_ptr<ResultSet> RES(SJK.SD("*", "tw_position_miner", pWhereLocalWorld));
	while (RES->next())
	{
		const int ID = RES->getInt("ID");
		ms_aOre[ID].m_ItemID = RES->getInt("ItemID");
		ms_aOre[ID].m_Level = RES->getInt("Level");
		ms_aOre[ID].m_StartHealth = RES->getInt("Health");
		ms_aOre[ID].m_PositionX = RES->getInt("PositionX");
		ms_aOre[ID].m_PositionY = RES->getInt("PositionY");
		ms_aOre[ID].m_Distance = RES->getInt("Distance");
	}
}

bool AccountMinerJob::OnParsingVoteCommands(CPlayer* pPlayer, const char* CMD, const int VoteID, const int VoteID2, int Get, const char* GetText)
{
	const int ClientID = pPlayer->GetCID();
	if (PPSTR(CMD, "MINERUPGRADE") == 0)
	{
		char aBuf[32];
		str_format(aBuf, sizeof(aBuf), "Mining '%s'", str_MINER((MINER)VoteID));
		if (pPlayer->Upgrade(Get, &pPlayer->Acc().m_aMiner[VoteID], &pPlayer->Acc().m_aMiner[MnrUpgrade], VoteID2, 3, aBuf))
		{
			GS()->Mmo()->SaveAccount(pPlayer, SaveType::SAVE_MINER_DATA);
			GS()->StrongUpdateVotes(ClientID, MenuList::MENU_UPGRADE);
		}
		return true;
	}

	return false;
}