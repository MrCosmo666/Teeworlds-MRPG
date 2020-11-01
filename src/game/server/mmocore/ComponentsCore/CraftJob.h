/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#ifndef GAME_SERVER_CRAFTJOB_H
#define GAME_SERVER_CRAFTJOB_H

#include "../MmoComponent.h"
 
class CraftJob : public MmoComponent
{
 	// create an instance of the class so that you can use it globally
	struct CraftStruct
	{
		int m_aItemNeedID[3];
		int	m_aItemNeedCount[3];
		int m_ReceivedItemID;
		int m_ReceivedItemCount;
		int m_Price;
		int m_WorldID;
	};
	static std::map <int, CraftStruct> ms_aCraft;

	void OnInit() override;
	bool OnHandleTile(CCharacter* pChr, int IndexCollision) override;
	bool OnHandleVoteCommands(CPlayer* pPlayer, const char* CMD, const int VoteID, const int VoteID2, int Get, const char* GetText) override;
	bool OnHandleMenulist(CPlayer* pPlayer, int Menulist, bool ReplaceMenu) override;

	int GetFinalPrice(CPlayer* pPlayer, int CraftID) const;
	void CraftItem(CPlayer* pPlayer, int CraftID);
	void ShowCraftList(CPlayer* pPlayer, const char* TypeName, int SelectType);
};

#endif