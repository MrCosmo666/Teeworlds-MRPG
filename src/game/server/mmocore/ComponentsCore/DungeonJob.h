/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#ifndef GAME_SERVER_DUNGEONS_SQL_H
#define GAME_SERVER_DUNGEONS_SQL_H

#include "../MmoComponent.h"
 
class DungeonJob : public MmoComponent
{
	~DungeonJob()
	{
		Dungeon.clear();
	};

	struct StructDungeon
	{
		char m_aName[64];
		int m_Level;
		int m_OpenQuestID;
		int m_WorldID;

		int m_DoorX;
		int m_DoorY;

		int m_Players;
		int m_Progress;
		int m_State;
		bool m_IsStory;
	};


public:
	typedef std::map < int, StructDungeon > DungeonType;
	static DungeonType Dungeon;

	DungeonJob();

	virtual bool OnVotingMenu(CPlayer* pPlayer, const char* CMD, const int VoteID, const int VoteID2, int Get, const char* GetText);
	virtual bool OnHandleMenulist(CPlayer* pPlayer, int Menulist, bool ReplaceMenu);

	void SaveDungeonRecord(CPlayer* pPlayer, int DungeonID, int Seconds);
	void ShowDungeonTop(CPlayer* pPlayer, int DungeonID, int HideID);
	void ShowDungeonsList(CPlayer* pPlayer, bool Story);
	void CheckQuestingOpened(CPlayer* pPlayer, int QuestID);
	void ShowTankVotingDungeon(CPlayer* pPlayer);
	int SyncFactor();
};
#endif