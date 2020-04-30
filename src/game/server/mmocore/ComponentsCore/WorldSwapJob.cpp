/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#include <engine/shared/config.h>
#include <game/server/gamecontext.h>
#include "WorldSwapJob.h"

using namespace sqlstr;
std::map < int , WorldSwapJob::StructSwapWorld > WorldSwapJob::WorldSwap;
std::list < WorldSwapJob::StructPositionLogic > WorldSwapJob::WorldPositionLogic;

void WorldSwapJob::OnInitGlobal() 
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
	UpdateWorldsList();
	Job()->ShowLoadingProgress("Worlds Swap", WorldSwap.size());
	Job()->ShowLoadingProgress("Worlds Swap Logic", WorldPositionLogic.size());
}

bool WorldSwapJob::OnPlayerHandleTile(CCharacter *pChr, int IndexCollision)
{
	CPlayer *pPlayer = pChr->GetPlayer();
	if(pChr->GetHelper()->TileEnter(IndexCollision, TILE_WORLD_SWAP))
	{
		pChr->m_Core.m_ProtectHooked = pChr->m_NoAllowDamage = true;
		return true;
	}
	else if(pChr->GetHelper()->TileExit(IndexCollision, TILE_WORLD_SWAP))
	{
		pChr->m_Core.m_ProtectHooked = pChr->m_NoAllowDamage = false;
		return true;	
	}

	
	if(pChr->GetHelper()->BoolIndex(TILE_WORLD_SWAP))
	{
		if(ChangeWorld(pPlayer->GetCID(), pChr->m_Core.m_Pos))
			return true;
	}
	return false;
}

int WorldSwapJob::GetID(vec2 Pos)
{
	for(const auto& sw : WorldSwap)
	{
		if (sw.second.WorldID == GS()->GetWorldID())
		{
			vec2 SwapPosition = vec2(sw.second.PositionX, sw.second.PositionY);
			if (distance(SwapPosition, Pos) < 400)
				return sw.first;
			continue;
		}
		
		if (sw.second.TwoWorldID == GS()->GetWorldID())
		{
			vec2 SwapPosition = vec2(sw.second.TwoPositionX, sw.second.TwoPositionY);
			if (distance(SwapPosition, Pos) < 400)
				return sw.first;
			continue;
		}
	}
	return -1;
}

int WorldSwapJob::GetNecessaryQuest(int WorldID) const
{
	int CheckWorldID = ((WorldID <= -1 || WorldID >= COUNT_WORLD) ? GS()->GetWorldID() : WorldID);
	if (CheckWorldID == LOCALWORLD)
		return -1;

	for (const auto& sw : WorldSwap)
	{
		if (sw.second.TwoWorldID == CheckWorldID)
			return sw.second.OpenQuestID;
	}
	return -1;
}

bool WorldSwapJob::ChangeWorld(int ClientID, vec2 Pos)
{
	CPlayer *pPlayer = GS()->GetPlayer(ClientID);
	if(!pPlayer) 
		return true;

	int WID = GetID(Pos);
	if (WorldSwap.find(WID) != WorldSwap.end())
	{
		int StoryQuestNeeded = WorldSwap[WID].OpenQuestID;
		if (!GS()->Mmo()->Quest()->IsComplectedQuest(ClientID, StoryQuestNeeded))
		{
			GS()->SBL(ClientID, BroadcastPriority::BROADCAST_GAME_WARNING, 100, "Requires story quest '{STR}' completion!", GS()->Mmo()->Quest()->GetQuestName(StoryQuestNeeded));
			return false;
		}

		if (WorldSwap[WID].WorldID == GS()->GetWorldID())
		{
			pPlayer->Acc().TeleportX = WorldSwap[WID].TwoPositionX;
			pPlayer->Acc().TeleportY = WorldSwap[WID].TwoPositionY;
			GS()->Server()->ChangeWorld(ClientID, WorldSwap[WID].TwoWorldID);
			return true;
		}

		pPlayer->Acc().TeleportX = WorldSwap[WID].PositionX;
		pPlayer->Acc().TeleportY = WorldSwap[WID].PositionY;
		GS()->Server()->ChangeWorld(ClientID, WorldSwap[WID].WorldID);
		return true;
	}
	return false;
}

vec2 WorldSwapJob::GetPositionQuestBot(int ClientID, int QuestID)
{
	int playerTalkProgress = QuestJob::Quests[ClientID][QuestID].Progress;
	BotJob::QuestBotInfo FindBot = Job()->Quest()->GetQuestBot(QuestID, playerTalkProgress);
	if(FindBot.IsActive())
	{
		if(GS()->GetWorldID() == FindBot.WorldID)
			return vec2(FindBot.PositionX, FindBot.PositionY);

		int TargetWorldID = FindBot.WorldID;
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

void WorldSwapJob::UpdateWorldsList()
{
	for (int i = 0; i < COUNT_WORLD; i++)
	{
		CSqlString<32> world_name = CSqlString<32>(GS()->Server()->GetWorldName(i));
		boost::scoped_ptr<ResultSet> RES(SJK.SD("*", "ENUM_WORLDS", "WHERE WorldID = '%d'", i));
		if (!RES->next()) { SJK.ID("ENUM_WORLDS", "(WorldID, Name) VALUES ('%d', '%s')", i, world_name.cstr()); }
		else { SJK.UD("ENUM_WORLDS", "Name = '%s' WHERE WorldID = '%d'", world_name.cstr(), i); }
	}
}