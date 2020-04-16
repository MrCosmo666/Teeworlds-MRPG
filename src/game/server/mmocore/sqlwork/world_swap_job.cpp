/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#include <engine/shared/config.h>
#include <game/server/gamecontext.h>
#include "world_swap_job.h"

using namespace sqlstr;
std::map < int , WorldSwapJob::StructSwapWorld > WorldSwapJob::WorldSwap;
std::list < WorldSwapJob::StructPositionLogic > WorldSwapJob::WorldPositionLogic;

void WorldSwapJob::OnInitGlobal() 
{ 
	boost::scoped_ptr<ResultSet> RES(SJK.SD("*", "tw_world_swap"));
	while(RES->next())
	{
		const int ID = RES->getInt("ID");
		WorldSwap[ID].Level = RES->getInt("Level");
		WorldSwap[ID].SwapID = RES->getInt("SwapID");
		WorldSwap[ID].PositionX = RES->getInt("PositionX");
		WorldSwap[ID].PositionY = RES->getInt("PositionY");
		WorldSwap[ID].WorldID = RES->getInt("WorldID");
		SJK.UD("tw_world_swap", "Name = '%s' WHERE ID = '%d'", GS()->Server()->GetWorldName(WorldSwap[ID].WorldID), ID);
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
	Job()->ShowLoadingProgress("Worlds Swap", WorldSwap.size());
	Job()->ShowLoadingProgress("Worlds Swap Logic", WorldPositionLogic.size());
}

bool WorldSwapJob::OnPlayerHandleTile(CCharacter *pChr, int IndexCollision)
{
	CPlayer *pPlayer = pChr->GetPlayer();
	int ClientID = pPlayer->GetCID();

	// парсинг на раз
	if(pChr->GetHelper()->TileEnter(IndexCollision, TILE_WORLDSWAP))
	{
		pChr->m_Core.m_ProtectHooked = true;
		pChr->m_NoAllowDamage = true;
		return true;
	}
	else if(pChr->GetHelper()->TileExit(IndexCollision, TILE_WORLDSWAP))
	{
		pChr->m_Core.m_ProtectHooked = false;
		pChr->m_NoAllowDamage = false;
		return true;	
	}

	// парсинг в секунду
	if(pChr->GetHelper()->BoolIndex(TILE_WORLDSWAP))
	{
		if(ChangingWorld(pPlayer->GetCID(), pChr->m_Core.m_Pos))
			return true;
	}
	return false;
}

// ------------------------------------------------------------------
// ------------------------------------------------------------------
// ------------------------------------------------------------------

// Получить айди смены мира по SwapID
int WorldSwapJob::CheckPosition(vec2 Pos)
{
	for(const auto& sw : WorldSwap)
	{
		vec2 SwapPosition = vec2(sw.second.PositionX, sw.second.PositionY);
		if(distance(SwapPosition, Pos) < 400 && sw.second.WorldID == GS()->GetWorldID())
			return sw.second.SwapID;
	}
	return -1;
}

// Смена мира игроку
bool WorldSwapJob::ChangingWorld(int ClientID, vec2 Pos)
{
	CPlayer *pPlayer = GS()->GetPlayer(ClientID);
	if(!pPlayer) return true;

	for(const auto& sw : WorldSwap)
	{
		int SwapID = CheckPosition(Pos);
		if(sw.second.WorldID == GS()->GetWorldID() || SwapID != sw.second.SwapID)
			continue;

		if(pPlayer->Acc().Level < sw.second.Level)
		{
			GS()->SBL(ClientID, 100000, 100, "Required {INT} level!", &sw.second.Level);
			return false;
		}

		pPlayer->Acc().TeleportX = sw.second.PositionX;
		pPlayer->Acc().TeleportY = sw.second.PositionY;
		GS()->Server()->ChangeWorld(ClientID, sw.second.WorldID);
		return true;
	}
	return false;
}

// Поиск путии до квеста к боту
vec2 WorldSwapJob::PositionQuestBot(int ClientID, int QuestID)
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
	if(GS()->GetWorldID() == CUTSCENEWELCOMEWORLD)
		return WORLD_CUTSCENE;
	else if(GS()->DungeonID())
		return WORLD_DUNGEON;
	else 
		return WORLD_STANDARD;
}