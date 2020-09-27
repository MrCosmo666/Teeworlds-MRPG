/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#include <game/server/gamecontext.h>
#include "WorldSwapJob.h"

using namespace sqlstr;
std::map < int , WorldSwapJob::StructSwapWorld > WorldSwapJob::ms_aWorldSwap;
std::list < WorldSwapJob::StructPositionLogic > WorldSwapJob::ms_aWorldPositionLogic;

void WorldSwapJob::OnInit()
{
	SJK.SDT("*", "tw_world_swap", [&](ResultSet* RES)
	{
		while(RES->next())
		{
			const int ID = RES->getInt("ID");
			ms_aWorldSwap[ID].m_OpenQuestID = RES->getInt("OpenQuestID");
			ms_aWorldSwap[ID].m_PositionX = RES->getInt("PositionX");
			ms_aWorldSwap[ID].m_PositionY = RES->getInt("PositionY");
			ms_aWorldSwap[ID].m_WorldID = RES->getInt("WorldID");
			ms_aWorldSwap[ID].m_TwoPositionX = RES->getInt("TwoPositionX");
			ms_aWorldSwap[ID].m_TwoPositionY = RES->getInt("TwoPositionY");
			ms_aWorldSwap[ID].m_TwoWorldID = RES->getInt("TwoWorldID");
		}

		for(const auto& swapw : ms_aWorldSwap)
		{
			StructPositionLogic pPositionLogic;
			pPositionLogic.m_BaseWorldID = swapw.second.m_WorldID;
			pPositionLogic.m_FindWorldID = swapw.second.m_TwoWorldID;
			pPositionLogic.m_Position = vec2(swapw.second.m_TwoPositionX, swapw.second.m_TwoPositionY);
			ms_aWorldPositionLogic.push_back(pPositionLogic);

			pPositionLogic.m_BaseWorldID = swapw.second.m_TwoWorldID;
			pPositionLogic.m_FindWorldID = swapw.second.m_WorldID;
			pPositionLogic.m_Position = vec2(swapw.second.m_PositionX, swapw.second.m_PositionY);
			ms_aWorldPositionLogic.push_back(pPositionLogic);
		}
		Job()->ShowLoadingProgress("Worlds Swap Logic", ms_aWorldPositionLogic.size());
		Job()->ShowLoadingProgress("Worlds Swap", ms_aWorldSwap.size());
	});
}

void WorldSwapJob::OnInitWorld(const char* pWhereLocalWorld)
{
	const int WorldID = GS()->GetWorldID();
	const CSqlString<32> world_name = CSqlString<32>(GS()->Server()->GetWorldName(WorldID));
	SJK.SDT("RespawnWorld, MusicID", "ENUM_WORLDS", [&](ResultSet* RES)
	{
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
	}, pWhereLocalWorld);
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
	for(const auto& sw : ms_aWorldSwap)
	{
		if (sw.second.m_WorldID == GS()->GetWorldID() || sw.second.m_TwoWorldID == GS()->GetWorldID())
		{
			vec2 SwapPosition = vec2(sw.second.m_PositionX, sw.second.m_PositionY);
			vec2 SwapPosition2 = vec2(sw.second.m_TwoPositionX, sw.second.m_TwoPositionY);
			if (distance(SwapPosition, Pos) < 400 || distance(SwapPosition2, Pos) < 400)
				return sw.first;
		}
	}
	return -1;
}

void WorldSwapJob::CheckQuestingOpened(CPlayer* pPlayer, int QuestID)
{
	const int ClientID = pPlayer->GetCID();
	for (const auto& sw : ms_aWorldSwap)
	{
		if (QuestID == sw.second.m_OpenQuestID)
			GS()->Chat(-1, "{STR} opened zone ({STR})!", GS()->Server()->ClientName(ClientID), GS()->Server()->GetWorldName(sw.second.m_TwoWorldID));
	}
}

int WorldSwapJob::GetNecessaryQuest(int WorldID) const
{
	int CheckWorldID = ((WorldID <= -1 || WorldID >= COUNT_WORLD) ? GS()->GetWorldID() : WorldID);
	for (const auto& sw : ms_aWorldSwap)
	{
		if (sw.second.m_TwoWorldID == CheckWorldID)
			return sw.second.m_OpenQuestID;
	}
	return -1;
}

bool WorldSwapJob::ChangeWorld(CPlayer *pPlayer, vec2 Pos)
{
	const int WID = GetID(Pos);
	if (ms_aWorldSwap.find(WID) != ms_aWorldSwap.end())
	{
		const int ClientID = pPlayer->GetCID();
		const int StoryQuestNeeded = ms_aWorldSwap[WID].m_OpenQuestID;
		if (StoryQuestNeeded > 0 && !GS()->Mmo()->Quest()->IsCompletedQuest(ClientID, StoryQuestNeeded))
		{
			GS()->SBL(ClientID, BroadcastPriority::BROADCAST_GAME_WARNING, 100, "Requires quest completion '{STR}'!", GS()->Mmo()->Quest()->GetQuestName(StoryQuestNeeded));
			return false;
		}

		if (ms_aWorldSwap[WID].m_WorldID == GS()->GetWorldID())
		{
			pPlayer->GetTempData().m_TempTeleportX = ms_aWorldSwap[WID].m_TwoPositionX;
			pPlayer->GetTempData().m_TempTeleportY = ms_aWorldSwap[WID].m_TwoPositionY;
			pPlayer->ChangeWorld(ms_aWorldSwap[WID].m_TwoWorldID);
			return true;
		}

		pPlayer->GetTempData().m_TempTeleportX = ms_aWorldSwap[WID].m_PositionX;
		pPlayer->GetTempData().m_TempTeleportY = ms_aWorldSwap[WID].m_PositionY;
		pPlayer->ChangeWorld(ms_aWorldSwap[WID].m_WorldID);
		return true;
	}
	return false;
}

vec2 WorldSwapJob::GetPositionQuestBot(int ClientID, int QuestID)
{
	const int playerTalkProgress = QuestJob::ms_aQuests[ClientID][QuestID].m_Progress;
	BotJob::QuestBotInfo *FindBot = Job()->Quest()->GetQuestBot(QuestID, playerTalkProgress);
	if(FindBot)
	{
		if(GS()->GetWorldID() == FindBot->m_WorldID)
			return vec2(FindBot->m_PositionX, FindBot->m_PositionY);

		int TargetWorldID = FindBot->m_WorldID;
		for(const auto& swp : ms_aWorldPositionLogic)
		{
			if(TargetWorldID != swp.m_BaseWorldID) 
				continue;
			TargetWorldID = swp.m_FindWorldID;
			if(GS()->GetWorldID() == swp.m_FindWorldID)
				return swp.m_Position;
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