/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#include "WorldSwapCore.h"

#include <game/server/gamecontext.h>

void CWorldSwapCore::OnInit()
{
	SJK.SDT("*", "tw_world_swap", [&](ResultPtr pRes)
	{
		while(pRes->next())
		{
			const int ID = pRes->getInt("ID");
			CWorldSwapData::ms_aWorldSwap[ID].m_RequiredQuestID = pRes->getInt("RequiredQuestID");
			CWorldSwapData::ms_aWorldSwap[ID].m_PositionX = pRes->getInt("PositionX");
			CWorldSwapData::ms_aWorldSwap[ID].m_PositionY = pRes->getInt("PositionY");
			CWorldSwapData::ms_aWorldSwap[ID].m_WorldID = pRes->getInt("WorldID");
			CWorldSwapData::ms_aWorldSwap[ID].m_TwoPositionX = pRes->getInt("TwoPositionX");
			CWorldSwapData::ms_aWorldSwap[ID].m_TwoPositionY = pRes->getInt("TwoPositionY");
			CWorldSwapData::ms_aWorldSwap[ID].m_TwoWorldID = pRes->getInt("TwoWorldID");
		}

		for(const auto& swapw : CWorldSwapData::ms_aWorldSwap)
		{
			CWorldSwapPosition pPositionLogic;
			pPositionLogic.m_BaseWorldID = swapw.second.m_WorldID;
			pPositionLogic.m_FindWorldID = swapw.second.m_TwoWorldID;
			pPositionLogic.m_Position = vec2(swapw.second.m_TwoPositionX, swapw.second.m_TwoPositionY);
			CWorldSwapPosition::ms_aWorldPositionLogic.push_back(pPositionLogic);

			pPositionLogic.m_BaseWorldID = swapw.second.m_TwoWorldID;
			pPositionLogic.m_FindWorldID = swapw.second.m_WorldID;
			pPositionLogic.m_Position = vec2(swapw.second.m_PositionX, swapw.second.m_PositionY);
			CWorldSwapPosition::ms_aWorldPositionLogic.push_back(pPositionLogic);
		}
		Job()->ShowLoadingProgress("Worlds Swap Logic", CWorldSwapPosition::ms_aWorldPositionLogic.size());
		Job()->ShowLoadingProgress("Worlds Swap", CWorldSwapData::ms_aWorldSwap.size());
	});
}

void CWorldSwapCore::OnInitWorld(const char* pWhereLocalWorld)
{
	const int WorldID = GS()->GetWorldID();
	const CSqlString<32> world_name = CSqlString<32>(Server()->GetWorldName(WorldID));

	ResultPtr pRes = SJK.SD("RespawnWorld, MusicID", "enum_worlds", pWhereLocalWorld);
	if(pRes->next())
	{
		const int RespawnWorld = (int)pRes->getInt("RespawnWorld");
		const int MusicID = (int)pRes->getInt("MusicID");
		SJK.UD("enum_worlds", "Name = '%s' WHERE WorldID = '%d'", world_name.cstr(), WorldID);
		GS()->SetRespawnWorld(RespawnWorld);
		GS()->SetMapMusic(MusicID);
		return;
	}
	SJK.ID("enum_worlds", "(WorldID, Name) VALUES ('%d', '%s')", WorldID, world_name.cstr());
}

bool CWorldSwapCore::OnHandleTile(CCharacter *pChr, int IndexCollision)
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

int CWorldSwapCore::GetWorldType() const
{
	if(GS()->GetDungeonID())
		return WORLD_DUNGEON;
	return WORLD_STANDARD;
}

int CWorldSwapCore::GetNecessaryQuest(int WorldID) const
{
	int CheckWorldID = WorldID != -1 ? WorldID : GS()->GetWorldID();
	const auto& pItem = std::find_if(CWorldSwapData::ms_aWorldSwap.begin(), CWorldSwapData::ms_aWorldSwap.end(), [CheckWorldID](const std::pair<int, CWorldSwapData>& pWorldSwap)
	                                 { return pWorldSwap.second.m_TwoWorldID == CheckWorldID; });
	return pItem != CWorldSwapData::ms_aWorldSwap.end() ? pItem->second.m_RequiredQuestID : -1;
}

vec2 CWorldSwapCore::GetPositionQuestBot(int ClientID, QuestBotInfo QuestBot) const
{
	if(GS()->GetWorldID() == QuestBot.m_WorldID)
		return vec2(QuestBot.m_PositionX, QuestBot.m_PositionY);

	int TargetWorldID = QuestBot.m_WorldID;
	for(const auto& swp : CWorldSwapPosition::ms_aWorldPositionLogic)
	{
		if(TargetWorldID != swp.m_BaseWorldID)
			continue;

		TargetWorldID = swp.m_FindWorldID;
		if(GS()->GetWorldID() == TargetWorldID)
			return swp.m_Position;
	}
	return vec2(0, 0);
}

void CWorldSwapCore::CheckQuestingOpened(CPlayer* pPlayer, int QuestID) const
{
	const int ClientID = pPlayer->GetCID();
	for(const auto& sw : CWorldSwapData::ms_aWorldSwap)
	{
		if(QuestID == sw.second.m_RequiredQuestID)
			GS()->Chat(-1, "{STR} opened zone ({STR})!", Server()->ClientName(ClientID), Server()->GetWorldName(sw.second.m_TwoWorldID));
	}
}

bool CWorldSwapCore::ChangeWorld(CPlayer* pPlayer, vec2 Pos)
{
	const int WID = GetID(Pos);
	if(CWorldSwapData::ms_aWorldSwap.find(WID) != CWorldSwapData::ms_aWorldSwap.end())
	{
		const int ClientID = pPlayer->GetCID();
		const int StoryQuestNeeded = CWorldSwapData::ms_aWorldSwap[WID].m_RequiredQuestID;
		if(StoryQuestNeeded > 0 && !pPlayer->GetQuest(StoryQuestNeeded).IsComplected())
		{
			GS()->Broadcast(ClientID, BroadcastPriority::GAME_WARNING, 100, "Requires quest completion '{STR}'!", pPlayer->GetQuest(StoryQuestNeeded).Info().GetName());
			return false;
		}

		if(CWorldSwapData::ms_aWorldSwap[WID].m_WorldID == GS()->GetWorldID())
		{
			pPlayer->GetTempData().m_TempTeleportX = CWorldSwapData::ms_aWorldSwap[WID].m_TwoPositionX;
			pPlayer->GetTempData().m_TempTeleportY = CWorldSwapData::ms_aWorldSwap[WID].m_TwoPositionY;
			pPlayer->ChangeWorld(CWorldSwapData::ms_aWorldSwap[WID].m_TwoWorldID);
			return true;
		}

		pPlayer->GetTempData().m_TempTeleportX = CWorldSwapData::ms_aWorldSwap[WID].m_PositionX;
		pPlayer->GetTempData().m_TempTeleportY = CWorldSwapData::ms_aWorldSwap[WID].m_PositionY;
		pPlayer->ChangeWorld(CWorldSwapData::ms_aWorldSwap[WID].m_WorldID);
		return true;
	}
	return false;
}

int CWorldSwapCore::GetID(vec2 Pos) const
{
	for(const auto& sw : CWorldSwapData::ms_aWorldSwap)
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
