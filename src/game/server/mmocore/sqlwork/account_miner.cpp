/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#include <engine/shared/config.h>
#include <game/server/gamecontext.h>
#include "account_miner.h"

using namespace sqlstr;
std::map < int , MinerAccSql::StructOres > MinerAccSql::DataOre;

// Инициализация класса
void MinerAccSql::OnInitLocal(const char *pLocal)
{
	// загружаем руду
	boost::scoped_ptr<ResultSet> RES(SJK.SD("*", "tw_position_miner", pLocal));
	while(RES->next())
	{
		const int ID = RES->getInt("ID");
		DataOre[ ID ].ItemID = RES->getInt("ItemID");
		DataOre[ ID ].Level = RES->getInt("Level");
		DataOre[ ID ].Health = RES->getInt("Health");
		DataOre[ ID ].PositionX = RES->getInt("PositionX");
		DataOre[ ID ].PositionY = RES->getInt("PositionY");	
		DataOre[ ID ].Distance = RES->getInt("Distance");
	}
}

// Загрузка данных игрока
void MinerAccSql::OnInitAccount(CPlayer *pPlayer)
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

// меню крафта
void MinerAccSql::ShowMenu(int ClientID)
{
	CPlayer *pPlayer = GS()->GetPlayer(ClientID, true);
	if(!pPlayer)
		return;

	int NeedExp = ExpNeed(pPlayer->Acc().Miner[MnrLevel]);
	GS()->AVM(ClientID, "null", NOPE, HJOBUPGRADE, _("[Miner Point: {i:point}] Level: {i:level} Exp: {i:exp}/{i:needexp}"), 
		"point", &pPlayer->Acc().Miner[MnrUpgrade], "level", &pPlayer->Acc().Miner[MnrLevel], 
		"exp", &pPlayer->Acc().Miner[MnrExp], "needexp", &NeedExp, NULL);

	GS()->AVD(ClientID, "MINERUPGRADE", MnrCount, 20, HJOBUPGRADE, _("[Price 20P]Mining bonus +1(Active {i:upgradebonus})"),
		 "upgradebonus", &pPlayer->Acc().Miner[MnrCount], NULL);
}

// получить уровень руды
int MinerAccSql::GetOreLevel(vec2 Pos) const
{
	for(const auto& ore : DataOre)
	{
		vec2 Position = vec2(ore.second.PositionX, ore.second.PositionY);
		if(distance(Position, Pos) < ore.second.Distance)
			return ore.second.Level;
	}
	return -1;
}

// получить предмет руды
int MinerAccSql::GetOreItemID(vec2 Pos) const
{
	for(const auto& ore : DataOre)
	{
		vec2 Position = vec2(ore.second.PositionX, ore.second.PositionY);
		if(distance(Position, Pos) < ore.second.Distance)
			return ore.second.ItemID;
	}
	return -1;
}

// получить здоровье руды
int MinerAccSql::GetOreHealth(vec2 Pos) const
{
	for(const auto& ore : DataOre)
	{
		vec2 Position = vec2(ore.second.PositionX, ore.second.PositionY);
		if(distance(Position, Pos) < ore.second.Distance)
			return ore.second.Health;
	}
	return -1;
}

// Работа получение опыта miner
void MinerAccSql::Work(CPlayer *pPlayer, int Exp)
{
	const int ClientID = pPlayer->GetCID();
	pPlayer->Acc().Miner[MnrExp] += Exp;
	for( ; pPlayer->Acc().Miner[MnrExp] >= ExpNeed(pPlayer->Acc().Miner[MnrLevel]) ; ) 
	{
		pPlayer->Acc().Miner[MnrExp] -= ExpNeed(pPlayer->Acc().Miner[MnrLevel]);
		pPlayer->Acc().Miner[MnrLevel]++, pPlayer->Acc().Miner[MnrUpgrade]++;

		if(pPlayer->GetCharacter() && pPlayer->GetCharacter()->IsAlive())
		{
			GS()->CreateSound(pPlayer->GetCharacter()->m_Core.m_Pos, 4);
			GS()->CreateDeath(pPlayer->GetCharacter()->m_Core.m_Pos, ClientID);
			GS()->CreateText(pPlayer->GetCharacter(), false, vec2(0, -40), vec2(0, -1), 40, "miner up", GS()->Server()->GetWorldID(ClientID));
		}
		GS()->ChatFollow(ClientID, "Miner Level UP. Now Level {INT}!", &pPlayer->Acc().Miner[MnrLevel]);
	}
	pPlayer->ProgressBar("Miner", pPlayer->Acc().Miner[MnrLevel], 
						pPlayer->Acc().Miner[MnrExp], ExpNeed(pPlayer->Acc().Miner[MnrLevel]), Exp);
	Job()->SaveAccount(pPlayer, SAVEMINERACCOUNT);
}

// Парсинг голосований Минера
bool MinerAccSql::OnParseVotingMenu(CPlayer *pPlayer, const char *CMD, const int VoteID, const int VoteID2, int Get, const char *GetText)
{
	const int ClientID = pPlayer->GetCID();	
	if(PPSTR(CMD, "MINERUPGRADE") == 0)
	{
		char aBuf[32];
		str_format(aBuf, sizeof(aBuf), "Mining '%s'", str_MINER((MINER)VoteID));
		if(pPlayer->Upgrade(Get, &pPlayer->Acc().Miner[VoteID], &pPlayer->Acc().Miner[MnrUpgrade], VoteID2, 1000, aBuf))
		{
			GS()->Mmo()->SaveAccount(pPlayer, SAVEMINERACCOUNT);
			GS()->VResetVotes(ClientID, UPGRADES);
		}
		return true;
	}	
	return false;
}

// подсчет опыта для лвл апа
int MinerAccSql::ExpNeed(int Level) const { return (g_Config.m_SvMinerLeveling+Level*2)*(Level*Level); } 
