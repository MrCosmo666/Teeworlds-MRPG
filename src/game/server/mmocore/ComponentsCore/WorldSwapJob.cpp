/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#include <engine/shared/config.h>
#include <game/server/gamecontext.h>
#include "WorldSwapJob.h"

using namespace sqlstr;
std::map < int , WorldSwapJob::StructSwapWorld > WorldSwapJob::WorldSwap;
std::list < WorldSwapJob::StructPositionLogic > WorldSwapJob::WorldPositionLogic;

void WorldSwapJob::OnInit()
{ 
	boost::scoped_ptr<ResultSet> RES(SJK.SD("*", "tw_world_swap"));
	while(RES->next())
	{
		const int ID = RES->getInt("ID");
		WorldSwap[ID].OpenQuestID = RES->getInt("OpenQuestID");
		WorldSwap[ID].PositionX = RES->getInt("PositionX");
		WorldSwap[ID].PositionY = RES->getInt("PositionY");
		WorldSwap[ID].WorldID = RES->getInt("WorldID");		
		WorldSwap[ID].TwoPositionX = RES->getInt("TwoPositionX");
		WorldSwap[ID].TwoPositionY = RES->getInt("TwoPositionY");
		WorldSwap[ID].TwoWorldID = RES->getInt("TwoWorldID");
	}

	for(const auto& swapw : WorldSwap)
	{
		StructPositionLogic pPositionLogic;
		pPositionLogic.BaseWorldID = swapw.second.WorldID;
		pPositionLogic.FindWorldID = swapw.second.TwoWorldID;
		pPositionLogic.Position = vec2(swapw.second.TwoPositionX, swapw.second.TwoPositionY);
		WorldPositionLogic.push_back(pPositionLogic);
			
		pPositionLogic.BaseWorldID = swapw.second.TwoWorldID;
		pPositionLogic.FindWorldID = swapw.second.WorldID;
		pPositionLogic.Position = vec2(swapw.second.PositionX, swapw.second.PositionY);
		WorldPositionLogic.push_back(pPositionLogic);
	}
	Job()->ShowLoadingProgress("Worlds Swap", WorldSwap.size());
	Job()->ShowLoadingProgress("Worlds Swap Logic", WorldPositionLogic.size());
}

void WorldSwapJob::OnInitWorld(const char* pWhereLocalWorld)
{
	const int WorldID = GS()->GetWorldID();
	CSqlString<32> world_name = CSqlString<32>(GS()->Server()->GetWorldName(WorldID));
	boost::scoped_ptr<ResultSet> RES(SJK.SD("RespawnWorld, MusicID", "ENUM_WORLDS", pWhereLocalWorld));
	if(RES->next())
	{
		const int RespawnWorld = (int)RES->getInt("RespawnWorld");
		const int MusicID = (int)RES->getInt("MusicID");
		SJK.UD("ENUM_WORLDS", "Name = '%s' WHERE WorldID = '%d'", world_name.cstr(), WorldID);
		GS()->SetRespawnWorld(RespawnWorld);
		GS()->SetMapMusic(MusicID);
		return;
	}
	SJK.ID("ENUM_WORLDS", "(WorldID, Name) VALUES ('%d', '%s')", WorldID, world_name.cstr());
}

bool WorldSwapJob::OnHandleTile(CCharacter *pChr, int IndexCollision)
{
	CPlayer *pPlayer = pChr->GetPlayer();
	if(pChr->GetHelper()->TileEnter(IndexCollision, TILE_WORLD_SWAP))
	{
		pChr->m_Core.m_ProtectHooked = pChr->m_NoAllowDamage = true;
		ChangeWorld(pPlayer, pChr->m_Core.m_Pos);
		return true;
	}
	else if(pChr->GetHelper()->TileExit(IndexCollision, TILE_WORLD_SWAP))
	{
		pChr->m_Core.m_ProtectHooked = pChr->m_NoAllowDamage = false;
		return true;	
	}
	return false;
}

int WorldSwapJob::GetID(vec2 Pos)
{
	for(const auto& sw : WorldSwap)
	{
		if (sw.second.WorldID == GS()->GetWorldID() || sw.second.TwoWorldID == GS()->GetWorldID())
		{
			vec2 SwapPosition = vec2(sw.second.PositionX, sw.second.PositionY);
			vec2 SwapPosition2 = vec2(sw.second.TwoPositionX, sw.second.TwoPositionY);
			if (distance(SwapPosition, Pos) < 400 || distance(SwapPosition2, Pos) < 400)
				return sw.first;
		}
	}
	return -1;
}

void WorldSwapJob::CheckQuestingOpened(CPlayer* pPlayer, int QuestID)
{
	const int ClientID = pPlayer->GetCID();
	for (const auto& sw : WorldSwap)
	{
		if (QuestID == sw.second.OpenQuestID)
			GS()->Chat(-1, "{STR} opened zone ({STR})!", GS()->Server()->ClientName(ClientID), GS()->Server()->GetWorldName(sw.second.TwoWorldID));
	}
}

int WorldSwapJob::GetNecessaryQuest(int WorldID) const
{
	int CheckWorldID = ((WorldID <= -1 || WorldID >= COUNT_WORLD) ? GS()->GetWorldID() : WorldID);
	for (const auto& sw : WorldSwap)
	{
		if (sw.second.TwoWorldID == CheckWorldID)
			return sw.second.OpenQuestID;
	}
	return -1;
}

bool WorldSwapJob::ChangeWorld(CPlayer *pPlayer, vec2 Pos)
{
	const int WID = GetID(Pos);
	if (WorldSwap.find(WID) != WorldSwap.end())
	{
		const int ClientID = pPlayer->GetCID();
		int StoryQuestNeeded = WorldSwap[WID].OpenQuestID;
		if (StoryQuestNeeded > 0 && !GS()->Mmo()->Quest()->IsCompletedQuest(ClientID, StoryQuestNeeded))
		{
			GS()->SBL(ClientID, BroadcastPriority::BROADCAST_GAME_WARNING, 100, "Requires quest completion '{STR}'!", GS()->Mmo()->Quest()->GetQuestName(StoryQuestNeeded));
			return false;
		}

		if (WorldSwap[WID].WorldID == GS()->GetWorldID())
		{
			pPlayer->GetTempData().TempTeleportX = WorldSwap[WID].TwoPositionX;
			pPlayer->GetTempData().TempTeleportY = WorldSwap[WID].TwoPositionY;
			pPlayer->ChangeWorld(WorldSwap[WID].TwoWorldID);
			return true;
		}

		pPlayer->GetTempData().TempTeleportX = WorldSwap[WID].PositionX;
		pPlayer->GetTempData().TempTeleportY = WorldSwap[WID].PositionY;
		pPlayer->ChangeWorld(WorldSwap[WID].WorldID);
		return true;
	}
	return false;
}

vec2 WorldSwapJob::GetPositionQuestBot(int ClientID, int QuestID)
{
	const int playerTalkProgress = QuestJob::Quests[ClientID][QuestID].Progress;
	BotJob::QuestBotInfo *FindBot = Job()->Quest()->GetQuestBot(QuestID, playerTalkProgress);
	if(FindBot)
	{
		if(GS()->GetWorldID() == FindBot->WorldID)
			return vec2(FindBot->PositionX, FindBot->PositionY);

		int TargetWorldID = FindBot->WorldID;
		for(const auto& swp : WorldPositionLogic)
		{
			if(TargetWorldID != swp.BaseWorldID) 
				continue;
			TargetWorldID = swp.FindWorldID;
			if(GS()->GetWorldID() == swp.FindWorldID)
				return swp.Position;
		}
	}
	return vec2(0.0f, 0.0f);
}

int WorldSwapJob::GetWorldType() const
{
	if(GS()->DungeonID())
		return WORLD_DUNGEON;
	return WORLD_STANDARD;
}