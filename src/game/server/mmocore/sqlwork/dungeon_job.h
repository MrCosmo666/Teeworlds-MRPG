/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#ifndef GAME_SERVER_DUNGEONS_SQL_H
#define GAME_SERVER_DUNGEONS_SQL_H

#include "../component.h"
 
class DungeonJob : public CMmoComponent
{
	~DungeonJob()
	{
		Dungeon.clear();
	};

	struct StructDungeon
	{
		char Name[64];
		int Level;
		int WorldID;

		int DoorX;
		int DoorY;

		int Players;
		int Progress;
		int State;
	};


public:
	typedef std::map < int, StructDungeon > DungeonType;
	static DungeonType Dungeon;

	DungeonJob();

	virtual bool OnParseVotingMenu(CPlayer* pPlayer, const char* CMD, const int VoteID, const int VoteID2, int Get, const char* GetText);
	virtual bool OnPlayerHandleMainMenu(CPlayer* pPlayer, int Menulist);

	void SaveDungeonRecord(CPlayer* pPlayer, int DungeonID, int Seconds);
	void ShowDungeonTop(CPlayer* pPlayer, int DungeonID, int HideID);
	void ShowDungeonsList(CPlayer* pPlayer);
};
#endif