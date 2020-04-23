/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#ifndef GAME_SERVER_CRAFTJOB_H
#define GAME_SERVER_CRAFTJOB_H

#include "../component.h"
 
class CraftJob : public CMmoComponent
{
 	// создаем экземпляр класса лишь 1 шт так что можно использовать глобально
	struct CraftStruct
	{
		int Level;
		int ItemID;
		int ItemCount;
		int Money;
		int Need[3];
		int	Count[3];
		int Tab;
		bool Rare;
	};
	typedef std::map < int , CraftStruct > CraftType;
	static CraftType Craft;

	void ShowCraftList(CPlayer* pPlayer, int CraftType);

public:
	virtual void OnInitGlobal();


	// действия с крафтами
	void StartCraftItem(CPlayer *pPlayer, int CraftID);
	virtual bool OnPlayerHandleTile(CCharacter* pChr, int IndexCollision);
	virtual bool OnParseVotingMenu(CPlayer *pPlayer, const char *CMD, const int VoteID, const int VoteID2, int Get, const char *GetText);
	virtual bool OnPlayerHandleMainMenu(CPlayer* pPlayer, int Menulist, bool ReplaceMenu);

};

#endif