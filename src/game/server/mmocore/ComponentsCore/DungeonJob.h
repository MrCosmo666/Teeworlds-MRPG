/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#ifndef GAME_SERVER_DUNGEONS_SQL_H
#define GAME_SERVER_DUNGEONS_SQL_H

#include "../MmoComponent.h"
 
class DungeonJob : public MmoComponent
{
	~DungeonJob()
	{
		ms_aDungeon.clear();
	};

	struct StructDungeon
	{
		char m_aName[64];
		int m_Level;
		int m_OpenQuestID;
		int m_DoorX;
		int m_DoorY;
		int m_WorldID;
		int m_Players;
		int m_Progress;
		int m_State;
		bool m_IsStory;

		bool IsDungeonPlaying() const { return m_State > 1; };
	};

	void OnInit() override;
	bool OnHandleVoteCommands(CPlayer* pPlayer, const char* CMD, const int VoteID, const int VoteID2, int Get, const char* GetText) override;
	bool OnHandleMenulist(CPlayer* pPlayer, int Menulist, bool ReplaceMenu) override;

public:
	static std::map < int, StructDungeon > ms_aDungeon;

	bool IsDungeonWorld(int WorldID) const;
	void SaveDungeonRecord(CPlayer* pPlayer, int DungeonID, int Seconds);
	void ShowDungeonTop(CPlayer* pPlayer, int DungeonID, int HideID);
	void ShowDungeonsList(CPlayer* pPlayer, bool Story);
	void CheckQuestingOpened(CPlayer* pPlayer, int QuestID);
	void ShowTankVotingDungeon(CPlayer* pPlayer);
};
#endif