/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#include <limits.h>

#include <game/server/gamecontext.h>
#include "DungeonJob.h"

using namespace sqlstr;

std::map < int , DungeonJob::StructDungeon > DungeonJob::ms_aDungeon;

DungeonJob::DungeonJob()
{
	ResultPtr pRes = SJK.SD("*", "tw_dungeons");
	while (pRes->next())
	{
		const int ID = pRes->getInt("ID");
		str_copy(ms_aDungeon[ID].m_aName, pRes->getString("Name").c_str(), sizeof(ms_aDungeon[ID].m_aName));
		ms_aDungeon[ID].m_Level = pRes->getInt("Level");
		ms_aDungeon[ID].m_DoorX = pRes->getInt("DoorX");
		ms_aDungeon[ID].m_DoorY = pRes->getInt("DoorY");
		ms_aDungeon[ID].m_OpenQuestID = pRes->getInt("OpenQuestID");
		ms_aDungeon[ID].m_WorldID = pRes->getInt("WorldID");
		ms_aDungeon[ID].m_IsStory = (bool)pRes->getBoolean("Story");
	}
}

bool DungeonJob::IsDungeonWorld(int WorldID) const
{
	return std::find_if(ms_aDungeon.begin(), ms_aDungeon.end(),
		[WorldID](const std::pair<int, StructDungeon>& pDungeon) { return pDungeon.second.m_WorldID == WorldID; }) != ms_aDungeon.end();
}

void DungeonJob::SaveDungeonRecord(CPlayer* pPlayer, int DungeonID, int Seconds)
{
	ResultPtr pRes = SJK.SD("*", "tw_dungeons_records", "WHERE OwnerID = '%d' AND DungeonID = '%d'", pPlayer->Acc().m_AuthID, DungeonID);
	if (pRes->next())
	{
		if (pRes->getInt("Seconds") > Seconds)
			SJK.UD("tw_dungeons_records", "Seconds = '%d' WHERE OwnerID = '%d' AND DungeonID = '%d'", Seconds, pPlayer->Acc().m_AuthID, DungeonID);
		return;
	}
	SJK.ID("tw_dungeons_records", "(OwnerID, DungeonID, Seconds) VALUES ('%d', '%d', '%d')", pPlayer->Acc().m_AuthID, DungeonID, Seconds);
}

void DungeonJob::ShowDungeonTop(CPlayer* pPlayer, int DungeonID, int HideID)
{
	const int ClientID = pPlayer->GetCID();
	ResultPtr pRes = SJK.SD("*", "tw_dungeons_records", "WHERE DungeonID = '%d' ORDER BY Seconds ASC LIMIT 5", DungeonID);
	while (pRes->next())
	{
		const int Rank = pRes->getRow();
		const int OwnerID = pRes->getInt("OwnerID");
		const int BaseSeconds = pRes->getDouble("Seconds");

		const int Minutes = BaseSeconds / 60;
		const int Seconds = BaseSeconds - (BaseSeconds / 60 * 60);
		GS()->AVM(ClientID, "null", NOPE, HideID, "{INT}. {STR} : Time: {INT} minute(s) {INT} second(s)", &Rank, Job()->PlayerName(OwnerID), &Minutes, &Seconds);
	}
}

void DungeonJob::ShowDungeonsList(CPlayer* pPlayer, bool Story)
{
	const int ClientID = pPlayer->GetCID();
	for (const auto& dungeon : ms_aDungeon)
	{
		if(dungeon.second.m_IsStory != Story)
			continue;

		const int HideID = 7500 + dungeon.first;
		GS()->AVH(ClientID, HideID, LIGHT_GOLDEN_COLOR, "Lvl{INT} {STR} : Players {INT} : {STR} [{INT}%]",
			&dungeon.second.m_Level, dungeon.second.m_aName, &dungeon.second.m_Players, (dungeon.second.IsDungeonPlaying() ? "Active dungeon" : "Waiting players"), &dungeon.second.m_Progress);

		ShowDungeonTop(pPlayer, dungeon.first, HideID);

		const int NeededQuestID = dungeon.second.m_OpenQuestID;
		if(NeededQuestID <= 0 || pPlayer->GetQuest(NeededQuestID).IsComplected())
			GS()->AVM(ClientID, "DUNGEONJOIN", dungeon.first, HideID, "Join dungeon {STR}", dungeon.second.m_aName);
		else
			GS()->AVM(ClientID, "null", NOPE, HideID, "Need to complete quest {STR}", pPlayer->GetQuest(NeededQuestID).Info().GetName());
	}
}

void DungeonJob::ShowTankVotingDungeon(CPlayer* pPlayer)
{
	if(!GS()->IsDungeon())
		return;

	const int ClientID = pPlayer->GetCID();
	const int DungeonWorldID = ms_aDungeon[GS()->GetDungeonID()].m_WorldID;
	pPlayer->m_Colored = GRAY_COLOR;
	GS()->AVL(ClientID, "null", "Voting for the choice of tank!");
	pPlayer->m_Colored = LIGHT_GRAY_COLOR;
	for(int i = 0; i < MAX_PLAYERS; i++)
	{
		CPlayer* pSearchPlayer = GS()->GetPlayer(i, true);
		if(!pSearchPlayer || pSearchPlayer->GetPlayerWorldID() != DungeonWorldID)
			continue;

		GS()->AVM(ClientID, "DUNGEONVOTE", i, NOPE, "Vote for {STR} (Votes: {INT})", GS()->Server()->ClientName(i), &pSearchPlayer->GetTempData().m_TempTankVotingDungeon);
	}
}

void DungeonJob::CheckQuestingOpened(CPlayer *pPlayer, int QuestID)
{
	const int ClientID = pPlayer->GetCID();
	for (const auto& dungeon : ms_aDungeon)
	{
		if (QuestID == dungeon.second.m_OpenQuestID)
			GS()->Chat(-1, "{STR} opened dungeon ({STR})!", GS()->Server()->ClientName(ClientID), dungeon.second.m_aName);
	}
}

bool DungeonJob::OnHandleMenulist(CPlayer* pPlayer, int Menulist, bool ReplaceMenu)
{
	const int ClientID = pPlayer->GetCID();
	if (ReplaceMenu)
	{
		return false;
	}

	if (Menulist == MenuList::MENU_DUNGEONS)
	{
		pPlayer->m_LastVoteMenu = MenuList::MAIN_MENU;
		GS()->AVH(ClientID, TAB_INFO_DUNGEON, GREEN_COLOR, "Dungeons Information");
		GS()->AVM(ClientID, "null", NOPE, TAB_INFO_DUNGEON, "In this section you can choose a dungeon");
		GS()->AVM(ClientID, "null", NOPE, TAB_INFO_DUNGEON, "View the fastest players on the passage");
		GS()->AV(ClientID, "null");

		pPlayer->m_Colored = GOLDEN_COLOR;
		GS()->AVL(ClientID, "null", "Story dungeon's");
		ShowDungeonsList(pPlayer, true);
		GS()->AV(ClientID, "null");

		pPlayer->m_Colored = GOLDEN_COLOR;
		GS()->AVL(ClientID, "null", "Alternative story dungeon's");
		ShowDungeonsList(pPlayer, false);

		if (GS()->IsDungeon())
		{
			GS()->AV(ClientID, "null");
			ShowTankVotingDungeon(pPlayer);
			GS()->AV(ClientID, "null");
			pPlayer->m_Colored = { 30, 8, 8 };
			GS()->AVL(ClientID, "DUNGEONEXIT", "Exit dungeon {STR} (warning)", ms_aDungeon[GS()->GetDungeonID()].m_aName);
		}
		GS()->AddBackpage(ClientID);
		return true;
	}
	return false;
}

bool DungeonJob::OnParsingVoteCommands(CPlayer* pPlayer, const char* CMD, const int VoteID, const int VoteID2, int Get, const char* GetText)
{
	const int ClientID = pPlayer->GetCID();
	if (!pPlayer->GetCharacter() || !pPlayer->GetCharacter()->IsAlive())
		return false;

	if (PPSTR(CMD, "DUNGEONJOIN") == 0)
	{
		if(GS()->IsPlayerEqualWorldID(ClientID, ms_aDungeon[VoteID].m_WorldID))
		{
			GS()->Chat(ClientID, "You are already in this dungeon!");
			GS()->StrongUpdateVotes(ClientID, MenuList::MENU_DUNGEONS);
			return true;
		}
		if (ms_aDungeon[VoteID].IsDungeonPlaying())
		{
			GS()->Chat(ClientID, "At the moment players are passing this dungeon!");
			GS()->StrongUpdateVotes(ClientID, MenuList::MENU_DUNGEONS);
			return true;
		}

		if (pPlayer->Acc().m_Level < ms_aDungeon[VoteID].m_Level)
		{
			GS()->Chat(ClientID, "Your level is low to pass this dungeon!");
			GS()->StrongUpdateVotes(ClientID, MenuList::MENU_DUNGEONS);
			return true;
		}

		if(!GS()->IsDungeon())
		{
			pPlayer->GetTempData().m_TempTeleportX = pPlayer->GetCharacter()->m_Core.m_Pos.x;
			pPlayer->GetTempData().m_TempTeleportY = pPlayer->GetCharacter()->m_Core.m_Pos.y;
			GS()->Mmo()->SaveAccount(pPlayer, SaveType::SAVE_POSITION);
		}

		GS()->Chat(-1, "{STR} joined to Dungeon {STR}!", GS()->Server()->ClientName(ClientID), ms_aDungeon[VoteID].m_aName);
		GS()->Chat(ClientID, "You can vote for the choice of tank (Dungeon Tab)!");
		pPlayer->ChangeWorld(ms_aDungeon[VoteID].m_WorldID);
		return true;
	}
	
	// dungeon exit
	else if (PPSTR(CMD, "DUNGEONEXIT") == 0)
	{
		int LatestCorrectWorldID = Job()->Account()->GetHistoryLatestCorrectWorldID(pPlayer);
		pPlayer->ChangeWorld(LatestCorrectWorldID);
		return true;
	}

	// dungeon voting
	else if(PPSTR(CMD, "DUNGEONVOTE") == 0)
	{
		CPlayer* pSearchPlayer = GS()->GetPlayer(VoteID, true);
		if(!pSearchPlayer)
		{
			GS()->StrongUpdateVotes(ClientID, pPlayer->m_OpenVoteMenu);
			return true;
		}

		if(VoteID == ClientID)
		{
			GS()->Chat(ClientID, "You can't vote for yourself!");
			GS()->StrongUpdateVotes(ClientID, pPlayer->m_OpenVoteMenu);
			return true;
		}

		if(pPlayer->GetTempData().m_TempAlreadyVotedDungeon)
		{
			GS()->Chat(ClientID, "You already voted!");
			GS()->StrongUpdateVotes(ClientID, pPlayer->m_OpenVoteMenu);
			return true;
		}

		pPlayer->GetTempData().m_TempAlreadyVotedDungeon = true;
		pSearchPlayer->GetTempData().m_TempTankVotingDungeon++;
		GS()->ChatWorldID(pPlayer->GetPlayerWorldID(), "[Dungeon]", "{STR} voted for {STR}.", GS()->Server()->ClientName(ClientID), GS()->Server()->ClientName(VoteID));
		GS()->StrongUpdateVotesForAll(pPlayer->m_OpenVoteMenu);
		return true;
	}
	return false;
}

int DungeonJob::SyncFactor()
{
	int MaxFactor = 0;
	int MinFactor = INT_MAX;
	for (int i = MAX_PLAYERS; i < MAX_CLIENTS; i++)
	{
		CPlayerBot* pBotPlayer = static_cast<CPlayerBot*>(GS()->m_apPlayers[i]);
		if(!pBotPlayer || pBotPlayer->GetBotType() != BotsTypes::TYPE_BOT_MOB || pBotPlayer->GetPlayerWorldID() != GS()->GetWorldID())
			continue;

		int LevelDisciple = pBotPlayer->GetLevelAllAttributes();
		MinFactor = min(MinFactor, LevelDisciple);
		MaxFactor = max(MaxFactor, LevelDisciple);
	}
	return (MaxFactor + MinFactor) / 2;
}