/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#include <engine/shared/config.h>
#include <game/server/gamecontext.h>
#include "AccountMinerJob.h"

using namespace sqlstr;
std::map < int , AccountMinerJob::StructOres > AccountMinerJob::Ore;

void AccountMinerJob::OnInitLocal(const char *pLocal)
{
	boost::scoped_ptr<ResultSet> RES(SJK.SD("*", "tw_position_miner", pLocal));
	while(RES->next())
	{
		const int ID = RES->getInt("ID");
		Ore[ ID ].ItemID = RES->getInt("ItemID");
		Ore[ ID ].Level = RES->getInt("Level");
		Ore[ ID ].Health = RES->getInt("Health");
		Ore[ ID ].PositionX = RES->getInt("PositionX");
		Ore[ ID ].PositionY = RES->getInt("PositionY");	
		Ore[ ID ].Distance = RES->getInt("Distance");
	}
}

void AccountMinerJob::OnInitAccount(CPlayer *pPlayer)
{
	boost::scoped_ptr<ResultSet> RES(SJK.SD("*", "tw_accounts_miner", "WHERE AccountID = '%d'", pPlayer->Acc().AuthID));
	if(RES->next()) 
	{
		for(int i = 0; i < NUM_MINER; i++)
			pPlayer->Acc().Miner[i] = RES->getInt(str_MINER((MINER) i));
		return;
	}
	pPlayer->Acc().Miner[MnrLevel] = 1;
	pPlayer->Acc().Miner[MnrCount] = 1;
	SJK.ID("tw_accounts_miner", "(AccountID) VALUES ('%d')", pPlayer->Acc().AuthID);	
	return;	
}

void AccountMinerJob::ShowMenu(CPlayer *pPlayer)
{
	const int ClientID = pPlayer->GetCID();
	int NeedExp = ExpNeed(pPlayer->Acc().Miner[MnrLevel]);
	GS()->AVM(ClientID, "null", NOPE, TAB_UPGR_JOB, "[Miner Point: {INT}] Level: {INT} Exp: {INT}/{INT}", 
		&pPlayer->Acc().Miner[MnrUpgrade], &pPlayer->Acc().Miner[MnrLevel], &pPlayer->Acc().Miner[MnrExp], &NeedExp);

	GS()->AVD(ClientID, "MINERUPGRADE", MnrCount, 20, TAB_UPGR_JOB, "[Price 20P]Mining bonus +1(Active {INT})", &pPlayer->Acc().Miner[MnrCount]);
}

int AccountMinerJob::GetOreLevel(vec2 Pos) const
{
	for(const auto& ore : Ore)
	{
		vec2 Position = vec2(ore.second.PositionX, ore.second.PositionY);
		if(distance(Position, Pos) < ore.second.Distance)
			return ore.second.Level;
	}
	return -1;
}

int AccountMinerJob::GetOreItemID(vec2 Pos) const
{
	for(const auto& ore : Ore)
	{
		vec2 Position = vec2(ore.second.PositionX, ore.second.PositionY);
		if(distance(Position, Pos) < ore.second.Distance)
			return ore.second.ItemID;
	}
	return -1;
}

int AccountMinerJob::GetOreHealth(vec2 Pos) const
{
	for(const auto& ore : Ore)
	{
		vec2 Position = vec2(ore.second.PositionX, ore.second.PositionY);
		if(distance(Position, Pos) < ore.second.Distance)
			return ore.second.Health;
	}
	return -1;
}

void AccountMinerJob::Work(CPlayer *pPlayer, int Exp)
{
	const int ClientID = pPlayer->GetCID();
	pPlayer->Acc().Miner[MnrExp] += Exp;
	for( ; pPlayer->Acc().Miner[MnrExp] >= ExpNeed(pPlayer->Acc().Miner[MnrLevel]) ; ) 
	{
		pPlayer->Acc().Miner[MnrExp] -= ExpNeed(pPlayer->Acc().Miner[MnrLevel]);
		pPlayer->Acc().Miner[MnrLevel]++;
		pPlayer->Acc().Miner[MnrUpgrade]++;

		if(pPlayer->GetCharacter() && pPlayer->GetCharacter()->IsAlive())
		{
			GS()->CreateSound(pPlayer->GetCharacter()->m_Core.m_Pos, 4);
			GS()->CreateDeath(pPlayer->GetCharacter()->m_Core.m_Pos, ClientID);
			GS()->CreateText(pPlayer->GetCharacter(), false, vec2(0, -40), vec2(0, -1), 40, "miner up", GS()->Server()->GetWorldID(ClientID));
		}
		GS()->ChatFollow(ClientID, "Miner Level UP. Now Level {INT}!", &pPlayer->Acc().Miner[MnrLevel]);
	}
	pPlayer->ProgressBar("Miner", pPlayer->Acc().Miner[MnrLevel], pPlayer->Acc().Miner[MnrExp], ExpNeed(pPlayer->Acc().Miner[MnrLevel]), Exp);
	Job()->SaveAccount(pPlayer, SAVE_MINER_DATA);
}

// Парсинг голосований Минера
bool AccountMinerJob::OnParseVotingMenu(CPlayer *pPlayer, const char *CMD, const int VoteID, const int VoteID2, int Get, const char *GetText)
{
	const int ClientID = pPlayer->GetCID();	
	if(PPSTR(CMD, "MINERUPGRADE") == 0)
	{
		char aBuf[32];
		str_format(aBuf, sizeof(aBuf), "Mining '%s'", str_MINER((MINER)VoteID));
		if(pPlayer->Upgrade(Get, &pPlayer->Acc().Miner[VoteID], &pPlayer->Acc().Miner[MnrUpgrade], VoteID2, 1000, aBuf))
		{
			GS()->Mmo()->SaveAccount(pPlayer, SaveType::SAVE_MINER_DATA);
			GS()->VResetVotes(ClientID, MenuList::MENU_UPGRADE);
		}
		return true;
	}	

	return false;
}

// подсчет опыта для лвл апа
int AccountMinerJob::ExpNeed(int Level) const { return (g_Config.m_SvMinerLeveling+Level*2)*(Level*Level); } 