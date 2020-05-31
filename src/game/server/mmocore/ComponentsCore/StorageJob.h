/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#ifndef GAME_SERVER_SQLSTORAGE_H
#define GAME_SERVER_SQLSTORAGE_H

#include "../MmoComponent.h"

class StorageJob : public MmoComponent
{
	struct SturctStorage
	{
		char Name[32];
		int PosX;
		int PosY;
		int Count;
		int Currency;
		int WorldID;
	};
	static std::map < int, SturctStorage > Storage;

public:
	virtual void OnInit();
	virtual bool OnHandleTile(CCharacter* pChr, int IndexCollision);
	virtual bool OnVotingMenu(CPlayer* pPlayer, const char* CMD, const int VoteID, const int VoteID2, int Get, const char* GetText);
	
	void ShowStorageMenu(CPlayer *pPlayer, int StorageID);
	int GetStorageID(vec2 Pos) const;
};

#endif