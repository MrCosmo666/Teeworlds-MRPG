/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#include <engine/shared/config.h>
#include <game/server/gamecontext.h>
#include "world_swap_job.h"

using namespace sqlstr;
std::map < int , WorldSwapJob::StructSwapWorld > WorldSwapJob::WorldSwap;
std::list < WorldSwapJob::StructPositionLogic > WorldSwapJob::WorldPositionLogic;

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

void WorldSwapJob::OnInitGlobal() 
{ 
	boost::scoped_ptr<ResultSet> RES(SJK.SD("*", "tw_world_swap"));
	while(RES->next())
	{
		const int ID = RES->getInt("ID");
		WorldSwap[ID].Level = RES->getInt("Level");
		WorldSwap[ID].PositionX = RES->getInt("PositionX");
		WorldSwap[ID].PositionY = RES->getInt("PositionY");
		WorldSwap[ID].WorldID = RES->getInt("WorldID");
		WorldSwap[ID].SwapID = RES->getInt("SwapID");
	}

	for(const auto& sw1 : WorldSwap)
	{
		for(const auto& sw2 : WorldSwap)
		{
			if(sw1.second.WorldID == sw2.second.WorldID || sw1.second.SwapID != sw2.second.SwapID)
				continue;
		
			StructPositionLogic pPositionLogic;
			pPositionLogic.BaseWorldID = sw1.second.WorldID;
			pPositionLogic.FindWorldID = sw2.second.WorldID;
			pPositionLogic.Position = vec2(sw2.second.PositionX, sw2.second.PositionY);
			WorldPositionLogic.push_back(pPositionLogic);
		}
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
		if(ChangingWorld(pPlayer->GetCID(), pChr->m_Core.m_Pos))
			return true;
	}
	return false;
}

/* #########################################################################
	FUNCTION TELEPORT CLASS
######################################################################### */
int WorldSwapJob::GetSwapID(vec2 Pos)
{
	for(const auto& sw : WorldSwap)
	{
		vec2 SwapPosition = vec2(sw.second.PositionX, sw.second.PositionY);
		if(distance(SwapPosition, Pos) < 400 && sw.second.WorldID == GS()->GetWorldID())
			return sw.second.SwapID;
	}
	return -1;
}

bool WorldSwapJob::ChangingWorld(int ClientID, vec2 Pos)
{
	CPlayer *pPlayer = GS()->GetPlayer(ClientID);
	if(!pPlayer) return true;

	for(const auto& sw : WorldSwap)
	{
		int SwapID = GetSwapID(Pos);
		if(sw.second.WorldID == GS()->GetWorldID() || SwapID != sw.second.SwapID)
			continue;

		if(pPlayer->Acc().Level < sw.second.Level)
		{
			GS()->SBL(ClientID, BroadcastPriority::BROADCAST_GAME_WARNING, 100, "Required {INT} level!", &sw.second.Level);
			return false;
		}

		pPlayer->Acc().TeleportX = sw.second.PositionX;
		pPlayer->Acc().TeleportY = sw.second.PositionY;
		GS()->Server()->ChangeWorld(ClientID, sw.second.WorldID);
		return true;
	}
	return false;
}

vec2 WorldSwapJob::GetPositionQuestBot(int ClientID, int QuestID)
{
	int playerTalkProgress = QuestBase::Quests[ClientID][QuestID].Progress;
	ContextBots::QuestBotInfo FindBot = Job()->Quest()->GetQuestBot(QuestID, playerTalkProgress);
	if(FindBot.IsActive())
	{
		if(GS()->GetWorldID() == FindBot.WorldID)
			return vec2(FindBot.PositionX, FindBot.PositionY);

		int TargetWorldID = FindBot.WorldID;
		for(const auto& swp : WorldPositionLogic)
		{
			if(TargetWorldID != swp.BaseWorldID) continue;
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
	else 
		return WORLD_STANDARD;
}