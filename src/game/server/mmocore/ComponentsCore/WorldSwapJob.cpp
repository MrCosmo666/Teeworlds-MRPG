/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#include <game/server/gamecontext.h>
#include "WorldSwapJob.h"

using namespace sqlstr;
std::map < int , WorldSwapJob::StructSwapWorld > WorldSwapJob::ms_aWorldSwap;
std::list < WorldSwapJob::StructPositionLogic > WorldSwapJob::ms_aWorldPositionLogic;

void WorldSwapJob::OnInit()
{
	SJK.SDT("*", "tw_world_swap", [&](ResultPtr pRes)
	{
		while(pRes->next())
		{
			const int ID = pRes->getInt("ID");
			ms_aWorldSwap[ID].m_OpenQuestID = pRes->getInt("OpenQuestID");
			ms_aWorldSwap[ID].m_PositionX = pRes->getInt("PositionX");
			ms_aWorldSwap[ID].m_PositionY = pRes->getInt("PositionY");
			ms_aWorldSwap[ID].m_WorldID = pRes->getInt("WorldID");
			ms_aWorldSwap[ID].m_TwoPositionX = pRes->getInt("TwoPositionX");
			ms_aWorldSwap[ID].m_TwoPositionY = pRes->getInt("TwoPositionY");
			ms_aWorldSwap[ID].m_TwoWorldID = pRes->getInt("TwoWorldID");
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
	const CSqlString<32> world_name = CSqlString<32>(Server()->GetWorldName(WorldID));

	ResultPtr pRes = SJK.SD("RespawnWorld, MusicID", "ENUM_WORLDS", pWhereLocalWorld);
	if(pRes->next())
	{
		const int RespawnWorld = (int)pRes->getInt("RespawnWorld");
		const int MusicID = (int)pRes->getInt("MusicID");
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
		pChr->m_Core.m_ProtectHooked = pChr->m_SkipDamage = true;
		ChangeWorld(pPlayer, pChr->m_Core.m_Pos);
		return true;
	}
	else if(pChr->GetHelper()->TileExit(IndexCollision, TILE_WORLD_SWAP))
	{
		pChr->m_Core.m_ProtectHooked = pChr->m_SkipDamage = false;
		return true;	
	}
	return false;
}

int WorldSwapJob::GetWorldType() const
{
	if(GS()->GetDungeonID())
		return WORLD_DUNGEON;
	return WORLD_STANDARD;
}

int WorldSwapJob::GetNecessaryQuest(int WorldID) const
{
	int CheckWorldID = WorldID != -1 ? WorldID : GS()->GetWorldID();
	const auto& pItem = std::find_if(ms_aWorldSwap.begin(), ms_aWorldSwap.end(), [CheckWorldID](const std::pair<int, StructSwapWorld>& pWorldSwap)
	{ return pWorldSwap.second.m_TwoWorldID == CheckWorldID; });
	return pItem != ms_aWorldSwap.end() ? pItem->second.m_OpenQuestID : -1;
}

vec2 WorldSwapJob::GetPositionQuestBot(int ClientID, BotJob::QuestBotInfo QuestBot) const
{
	if(GS()->GetWorldID() == QuestBot.m_WorldID)
		return vec2(QuestBot.m_PositionX, QuestBot.m_PositionY);

	int TargetWorldID = QuestBot.m_WorldID;
	for(const auto& swp : ms_aWorldPositionLogic)
	{
		if(TargetWorldID != swp.m_BaseWorldID) 
			continue;

		TargetWorldID = swp.m_FindWorldID;
		if(GS()->GetWorldID() == TargetWorldID)
			return swp.m_Position;
	}
	return vec2(0, 0);
}

void WorldSwapJob::CheckQuestingOpened(CPlayer* pPlayer, int QuestID) const
{
	const int ClientID = pPlayer->GetCID();
	for(const auto& sw : ms_aWorldSwap)
	{
		if(QuestID == sw.second.m_OpenQuestID)
			GS()->Chat(-1, "{STR} opened zone ({STR})!", Server()->ClientName(ClientID), Server()->GetWorldName(sw.second.m_TwoWorldID));
	}
}

bool WorldSwapJob::ChangeWorld(CPlayer* pPlayer, vec2 Pos)
{
	const int WID = GetID(Pos);
	if(ms_aWorldSwap.find(WID) != ms_aWorldSwap.end())
	{
		const int ClientID = pPlayer->GetCID();
		const int StoryQuestNeeded = ms_aWorldSwap[WID].m_OpenQuestID;
		if(StoryQuestNeeded > 0 && !pPlayer->GetQuest(StoryQuestNeeded).IsComplected())
		{
			GS()->Broadcast(ClientID, BroadcastPriority::BROADCAST_GAME_WARNING, 100, "Requires quest completion '{STR}'!", pPlayer->GetQuest(StoryQuestNeeded).Info().GetName());
			return false;
		}

		if(ms_aWorldSwap[WID].m_WorldID == GS()->GetWorldID())
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

int WorldSwapJob::GetID(vec2 Pos) const
{
	for(const auto& sw : ms_aWorldSwap)
	{
		if(sw.second.m_WorldID == GS()->GetWorldID() || sw.second.m_TwoWorldID == GS()->GetWorldID())
		{
			vec2 SwapPosition = vec2(sw.second.m_PositionX, sw.second.m_PositionY);
			vec2 SwapPosition2 = vec2(sw.second.m_TwoPositionX, sw.second.m_TwoPositionY);
			if(distance(SwapPosition, Pos) < 400 || distance(SwapPosition2, Pos) < 400)
				return sw.first;
		}
	}
	return -1;
}
