/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#include <engine/shared/config.h>
#include <game/server/gamecontext.h>
#include "account_relax.h"

using namespace sqlstr;

// Загрузка данных игрока
void SpaRelaxSql::OnInitAccount(CPlayer *pPlayer)
{
	boost::scoped_ptr<ResultSet> RES(SJK.SD("*", "tw_accounts_relax", "WHERE AccountID = '%d'", pPlayer->Acc().AuthID));
	if(RES->next())
	{
		for(int i = 0; i < RELAX::NUM_RELAX; i++)
			pPlayer->Acc().Relax[i] = RES->getInt(str_RELAX((RELAX) i));
		return;
	}
	pPlayer->Acc().Relax[RlxLevel] = 1;
	pPlayer->Acc().Relax[RlxExpBonus] = 3;
	pPlayer->Acc().Relax[RlxMoneyBonus] = 3;
	pPlayer->Acc().Relax[RlxSpaBonus] = 1;
	SJK.ID("tw_accounts_relax", "(AccountID) VALUES ('%d')", pPlayer->Acc().AuthID);	
	return;	
}

// работа в спа
void SpaRelaxSql::Work(int ClientID)
{
	CPlayer *pPlayer = GS()->GetPlayer(ClientID, true);
	if(!pPlayer)
		return;

	// выдача опыта для спа
	int Exp = 1+pPlayer->Acc().Relax[RlxSpaBonus];
	pPlayer->Acc().Relax[RlxExp] += Exp;
	for( ; pPlayer->Acc().Relax[RlxExp] >= ExpNeed(pPlayer->Acc().Relax[RlxLevel]) ; ) 
	{
		pPlayer->Acc().Relax[RlxExp] -= ExpNeed(pPlayer->Acc().Relax[RlxLevel]);
		pPlayer->Acc().Relax[RlxLevel]++, pPlayer->Acc().Relax[RlxUpgrade]++;

		int ClientID = pPlayer->GetCID();
		if(GS()->GetPlayer(ClientID, true, true))
		{
			GS()->CreateSound(pPlayer->GetCharacter()->m_Core.m_Pos, 4);
			GS()->CreateDeath(pPlayer->GetCharacter()->m_Core.m_Pos, ClientID);
			GS()->CreateText(pPlayer->GetCharacter(), false, vec2(0, -40), vec2(0, -1), 40, "relax up", GS()->Server()->GetWorldID(ClientID));
		}
		GS()->ChatFollow(ClientID, "Relax Level UP. Now Level {INT}!", &pPlayer->Acc().Relax[RlxLevel]);
	}
	
	pPlayer->AddExp(pPlayer->Acc().Relax[RlxExpBonus]);
	pPlayer->ProgressBar("Relax", pPlayer->Acc().Relax[RlxLevel], pPlayer->Acc().Relax[RlxExp], 
									ExpNeed(pPlayer->Acc().Relax[RlxLevel]), Exp);
	pPlayer->AddMoney(pPlayer->Acc().Relax[RlxMoneyBonus]);
	GS()->VResetVotes(ClientID, UPGRADES);	

	if(GS()->GetPlayer(ClientID, true, true))
	{
		GS()->SendMmoEffect(vec2(pPlayer->GetCharacter()->m_Core.m_Pos.x+rand()%15-rand()%30, 
			pPlayer->GetCharacter()->m_Core.m_Pos.y-35+rand()%15), EFFECT_SPASALON);
	}

	if(rand()%10 == 1) GS()->Mmo()->SaveAccount(pPlayer, SAVESPAACCOUNT);
}

// меню релакса
void SpaRelaxSql::ShowMenu(int ClientID)
{
	CPlayer *pPlayer = GS()->GetPlayer(ClientID, true);
	if(!pPlayer)
		return;

	int NeedExp = ExpNeed(pPlayer->Acc().Relax[RlxLevel]);
	GS()->AVM(ClientID, "null", NOPE, HJOBUPGRADE, "[Relax Point: {INT}] Level: {INT} Exp: {INT}/{INT}", 
		&pPlayer->Acc().Relax[RlxUpgrade], &pPlayer->Acc().Relax[RlxLevel], &pPlayer->Acc().Relax[RlxExp], &NeedExp);
	GS()->AVD(ClientID, "RELAXUPGRADE", RlxSpaBonus, 12, HJOBUPGRADE, "[Price 12P]Experience Relax +1 (Active {INT})", &pPlayer->Acc().Relax[RlxSpaBonus]);
	GS()->AVD(ClientID, "RELAXUPGRADE", RlxExpBonus, 5, HJOBUPGRADE, "[Price 5P]Experience Account +1 (Active {INT})", &pPlayer->Acc().Relax[RlxExpBonus]);
	GS()->AVD(ClientID, "RELAXUPGRADE", RlxMoneyBonus, 12, HJOBUPGRADE, _("[Price 12P]Money +1 (Active {INT})"), &pPlayer->Acc().Relax[RlxMoneyBonus]);
}

// парсинг голосований релакса
bool SpaRelaxSql::OnParseVotingMenu(CPlayer *pPlayer, const char *CMD, const int VoteID, const int VoteID2, int Get, const char *GetText)
{
	int ClientID = pPlayer->GetCID();
	if(PPSTR(CMD, "RELAXUPGRADE") == 0)
	{
		char aBuf[32];
		str_format(aBuf, sizeof(aBuf), "Relax '%s'", str_RELAX((RELAX)VoteID));
		if(pPlayer->Upgrade(Get, &pPlayer->Acc().Relax[VoteID], 
								 &pPlayer->Acc().Relax[RlxUpgrade], VoteID2, 1000, aBuf))
		{
			GS()->Mmo()->SaveAccount(pPlayer, SAVESPAACCOUNT);
			GS()->VResetVotes(ClientID, UPGRADES);
		}
		return true;
	}
	return false; 
}

// подсчет опыта для лвл апа
int SpaRelaxSql::ExpNeed(int Level) const { return (g_Config.m_SvRelaxLeveling+Level*2)*(Level*Level); };
