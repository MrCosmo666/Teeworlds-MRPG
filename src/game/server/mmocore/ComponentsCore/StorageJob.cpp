/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#include <engine/shared/config.h>
#include <game/server/gamecontext.h>
#include "StorageJob.h"

using namespace sqlstr;
std::map < int, StorageJob::SturctStorage > StorageJob::Storage;

int StorageJob::GetStorageID(vec2 Pos) const
{
	for (const auto& st : Storage)
	{
		vec2 PosStorage = vec2(st.second.PosX, st.second.PosY);
		if (distance(PosStorage, Pos) > 200) 
			continue;
		return st.first;
	}
	return -1;
}

void StorageJob::ShowStorageMenu(CPlayer* pPlayer, int StorageID)
{
	const int ClientID = pPlayer->GetCID();
	if(StorageID < 0)
	{
		GS()->AV(ClientID, "null", "Storage Don't work");
		return;
	}
	GS()->AVH(ClientID, TAB_STORAGE, GOLDEN_COLOR, "Shop [{STR}/{INT}]", Storage[StorageID].Name, &Storage[StorageID].Count);
	GS()->AVM(ClientID, "REPAIRITEMS", StorageID, TAB_STORAGE, "Repair all items - FREE");
}

void StorageJob::OnInit()
{
	boost::scoped_ptr<ResultSet> RES(SJK.SD("*", "tw_storages"));
	while (RES->next())
	{
		const int ID = (int)RES->getInt("ID");
		Storage[ID].PosX = (int)RES->getInt("PosX");
		Storage[ID].PosY = (int)RES->getInt("PosY");
		Storage[ID].WorldID = (int)RES->getInt("WorldID");
		str_copy(Storage[ID].Name, RES->getString("Name").c_str(), sizeof(Storage[ID].Name));
	}
}

bool StorageJob::OnHandleTile(CCharacter* pChr, int IndexCollision)
{
	CPlayer* pPlayer = pChr->GetPlayer();
	const int ClientID = pPlayer->GetCID();
	if (pChr->GetHelper()->TileEnter(IndexCollision, TILE_SHOP_ZONE))
	{
		GS()->Chat(ClientID, "You can see menu in the votes!");
		pChr->m_Core.m_ProtectHooked = pChr->m_NoAllowDamage = true;
		GS()->ResetVotes(ClientID, MenuList::MAIN_MENU);
		return true;
	}
	else if (pChr->GetHelper()->TileExit(IndexCollision, TILE_SHOP_ZONE))
	{
		GS()->Chat(ClientID, "You have left the active zone, menu is restored!");
		pChr->m_Core.m_ProtectHooked = pChr->m_NoAllowDamage = false;
		GS()->ResetVotes(ClientID, MenuList::MAIN_MENU);
		return true;
	}

	return false;
}

bool StorageJob::OnVotingMenu(CPlayer* pPlayer, const char* CMD, const int VoteID, const int VoteID2, int Get, const char* GetText)
{
	const int ClientID = pPlayer->GetCID();
	if (PPSTR(CMD, "REPAIRITEMS") == 0)
	{
		Job()->Item()->RepairDurabilityFull(pPlayer);
		GS()->Chat(ClientID, "You repaired all your items.");
		return true;
	}

	return false;
}