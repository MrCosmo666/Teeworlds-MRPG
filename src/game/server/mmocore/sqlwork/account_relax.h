/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#ifndef GAME_SERVER_SQLSPARELAX_H
#define GAME_SERVER_SQLSPARELAX_H

#include "../component.h"
 
class SpaRelaxSql : public CMmoComponent
{
	// Приватные методы
	int ExpNeed(int Level) const;

public:
	virtual void OnInitAccount(CPlayer *pPlayer);

	// Основное 
	void Work(int ClientID);
	void ShowMenu(int ClientID);

	// Парсинг голосований 
	virtual bool OnParseVotingMenu(CPlayer *pPlayer, const char *CMD, const int VoteID, const int VoteID2, int Get, const char *GetText);
};

#endif