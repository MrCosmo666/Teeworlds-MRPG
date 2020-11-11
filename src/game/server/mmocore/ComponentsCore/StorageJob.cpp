/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#include <game/server/gamecontext.h>
#include "StorageJob.h"

using namespace sqlstr;
std::map < int, StorageJob::SturctStorage > StorageJob::ms_aStorage;

void StorageJob::OnInit()
{
	SJK.SDT("*", "tw_storages", [&](ResultPtr pRes)
		{
			while(pRes->next())
			{
				const int ID = (int)pRes->getInt("ID");
				ms_aStorage[ID].m_PosX = (int)pRes->getInt("PosX");
				ms_aStorage[ID].m_PosY = (int)pRes->getInt("PosY");
				ms_aStorage[ID].m_Currency = (int)pRes->getInt("Currency");
				ms_aStorage[ID].m_WorldID = (int)pRes->getInt("WorldID");
				str_copy(ms_aStorage[ID].m_aName, pRes->getString("Name").c_str(), sizeof(ms_aStorage[ID].m_aName));
			}
		});
}

bool StorageJob::OnHandleTile(CCharacter* pChr, int IndexCollision)
{
	CPlayer* pPlayer = pChr->GetPlayer();
	const int ClientID = pPlayer->GetCID();
	if(pChr->GetHelper()->TileEnter(IndexCollision, TILE_SHOP_ZONE))
	{
		GS()->Chat(ClientID, "You can see menu in the votes!");
		pChr->m_Core.m_ProtectHooked = pChr->m_SkipDamage = true;
		GS()->ResetVotes(ClientID, pPlayer->m_OpenVoteMenu);
		return true;
	}
	else if(pChr->GetHelper()->TileExit(IndexCollision, TILE_SHOP_ZONE))
	{
		GS()->Chat(ClientID, "You left the active zone, menu is restored!");
		pChr->m_Core.m_ProtectHooked = pChr->m_SkipDamage = false;
		GS()->ResetVotes(ClientID, pPlayer->m_OpenVoteMenu);
		return true;
	}

	return false;
}

bool StorageJob::OnHandleVoteCommands(CPlayer* pPlayer, const char* CMD, const int VoteID, const int VoteID2, int Get, const char* GetText)
{
	const int ClientID = pPlayer->GetCID();
	if(PPSTR(CMD, "REPAIRITEMS") == 0)
	{
		Job()->Item()->RepairDurabilityItems(pPlayer);
		GS()->Chat(ClientID, "You repaired all items.");
		return true;
	}

	return false;
}

int StorageJob::GetStorageID(vec2 Pos) const
{
	for (const auto& st : ms_aStorage)
	{
		const vec2 PosStorage(st.second.m_PosX, st.second.m_PosY);
		if (distance(PosStorage, Pos) < 200) 
			return st.first;
	}
	return -1;
}

void StorageJob::ShowStorageMenu(CPlayer* pPlayer, int StorageID)
{
	const int ClientID = pPlayer->GetCID();
	if(ms_aStorage.find(StorageID) == ms_aStorage.end())
	{
		GS()->AV(ClientID, "null", "Storage Don't work");
		return;
	}
	
	GS()->AVH(ClientID, TAB_STORAGE, GOLDEN_COLOR, "Shop :: {STR}", ms_aStorage[StorageID].m_aName);
	GS()->AVM(ClientID, "REPAIRITEMS", StorageID, TAB_STORAGE, "Repair all items - FREE");
	GS()->AV(ClientID, "null");
	GS()->ShowVotesItemValueInformation(pPlayer, ms_aStorage[StorageID].m_Currency);
	GS()->AV(ClientID, "null");
}
