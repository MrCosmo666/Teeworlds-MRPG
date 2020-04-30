/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#ifndef GAME_SERVER_CRAFTJOB_H
#define GAME_SERVER_CRAFTJOB_H

#include "../MmoComponent.h"
 
class CraftJob : public MmoComponent
{
 	// создаем экземпляр класса лишь 1 шт так что можно использовать глобально
	struct CraftStruct
	{
		int ItemNeedID[3];
		int	ItemNeedCount[3];
		int GetItemID;
		int GetItemCount;
		int Price;
		int WorldID;
	};
	typedef std::map < int , CraftStruct > CraftType;
	static CraftType Craft;

	void CraftItem(CPlayer* pPlayer, int CraftID);
	
	bool ItEmptyType(int SelectType) const;
	void ShowCraftList(CPlayer* pPlayer, const char* TypeName, int SelectType);

public:
	virtual void OnInitGlobal();
	virtual bool OnPlayerHandleTile(CCharacter* pChr, int IndexCollision);
	virtual bool OnParseVotingMenu(CPlayer* pPlayer, const char* CMD, const int VoteID, const int VoteID2, int Get, const char* GetText);
	virtual bool OnPlayerHandleMainMenu(CPlayer* pPlayer, int Menulist, bool ReplaceMenu);

};

#endif