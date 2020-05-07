/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#include <engine/shared/config.h>
#include <game/server/gamecontext.h>
#include "AetherJob.h"

using namespace sqlstr;
std::map < int , AetherJob::StructTeleport > AetherJob::Teleport;

void AetherJob::OnInit()
{ 
	boost::scoped_ptr<ResultSet> RES(SJK.SD("*", "tw_teleports"));
	while(RES->next())
	{
		int ID = RES->getInt("ID");
		str_copy(Teleport[ID].TeleName, RES->getString("TeleName").c_str(), sizeof(Teleport[ID].TeleName));
		Teleport[ID].TeleX = RES->getInt("TeleX");
		Teleport[ID].TeleY = RES->getInt("TeleY");
		Teleport[ID].WorldID = RES->getInt("WorldID");
	}
	Job()->ShowLoadingProgress("Teleports", Teleport.size());	
}

void AetherJob::OnInitAccount(CPlayer *pPlayer)
{
	boost::scoped_ptr<ResultSet> RES(SJK.SD("*", "tw_accounts_locations", "WHERE OwnerID = '%d'", pPlayer->Acc().AuthID));
	while(RES->next())
	{
		int TeleportID = RES->getInt("TeleportID");
		pPlayer->Acc().AetherLocation[TeleportID] = true;
	}
}

bool AetherJob::OnVotingMenu(CPlayer *pPlayer, const char *CMD, const int VoteID, const int VoteID2, int Get, const char *GetText)
{
	const int ClientID = pPlayer->GetCID();

	// телепорт
	if(PPSTR(CMD, "TELEPORT") == 0)
	{
		const int TeleportID = VoteID;
		const int Price = VoteID2;
		if(Price > 0 && pPlayer->CheckFailMoney(Price))
			return true;

		vec2 Position = vec2(Teleport[TeleportID].TeleX, Teleport[TeleportID].TeleY);
		if(Teleport[TeleportID].WorldID != GS()->Server()->GetWorldID(ClientID))
		{
			pPlayer->Acc().TeleportX = Position.x;
			pPlayer->Acc().TeleportY = Position.y;
			GS()->Server()->ChangeWorld(ClientID, Teleport[TeleportID].WorldID);
			return true;
		}

		pPlayer->GetCharacter()->ChangePosition(Position);
		GS()->ResetVotes(ClientID, pPlayer->m_OpenVoteMenu);
		return true;
	}

	return false; 
}

bool AetherJob::OnHandleTile(CCharacter* pChr, int IndexCollision)
{
	CPlayer* pPlayer = pChr->GetPlayer();
	const int ClientID = pPlayer->GetCID();

	if (pChr->GetHelper()->TileEnter(IndexCollision, TILE_AETHER_TELEPORT))
	{
		pChr->m_Core.m_ProtectHooked = pChr->m_NoAllowDamage = true;
		GS()->Chat(ClientID, "List of your Aether Locations, you can see on vote!");
		UnlockLocation(pChr->GetPlayer(), pChr->m_Core.m_Pos);
		GS()->VResetVotes(ClientID, MenuList::MAIN_MENU);
		return true;
	}
	else if (pChr->GetHelper()->TileExit(IndexCollision, TILE_AETHER_TELEPORT))
	{
		pChr->m_Core.m_ProtectHooked = pChr->m_NoAllowDamage = false;
		GS()->VResetVotes(ClientID, MenuList::MAIN_MENU);
		return true;
	}

	return false;
}

bool AetherJob::OnHandleMenulist(CPlayer* pPlayer, int Menulist, bool ReplaceMenu)
{
	if (ReplaceMenu)
	{
		CCharacter* pChr = pPlayer->GetCharacter();
		if (!pChr || !pChr->IsAlive()) 
			return false;

		if (Menulist == MenuList::MAIN_MENU && pChr->GetHelper()->BoolIndex(TILE_AETHER_TELEPORT))
		{
			ShowTeleportList(pChr);
			return true;
		}
		return false;
	}


	return false;
}

void AetherJob::UnlockLocation(CPlayer *pPlayer, vec2 Pos)
{
	const int ClientID = pPlayer->GetCID();
	for (const auto& tl : Teleport)
	{
		if (distance(vec2(tl.second.TeleX, tl.second.TeleY), Pos) > 100 || pPlayer->Acc().AetherLocation.find(tl.first) != pPlayer->Acc().AetherLocation.end())
			continue;

		SJK.ID("tw_accounts_locations", "(OwnerID, TeleportID) VALUES ('%d', '%d')", pPlayer->Acc().AuthID, tl.first);
		GS()->Chat(ClientID, "You unlock new location {STR}!", Teleport[tl.first].TeleName);
		GS()->ChatDiscord("14671083", GS()->Server()->ClientName(ClientID),
			"Adventure unlock new location {STR}", Teleport[tl.first].TeleName);

		pPlayer->Acc().AetherLocation[tl.first] = true;
		return;
	}
}

void AetherJob::ShowTeleportList(CCharacter* pChar)
{
	CPlayer* pPlayer = pChar->GetPlayer();
	const int ClientID = pPlayer->GetCID();

	GS()->AVH(ClientID, TAB_AETHER, GOLDEN_COLOR, "Available teleports");
	if (Job()->Member()->GetGuildHouseID(pPlayer->Acc().GuildID) >= 1)
		GS()->AVM(ClientID, "MSPAWN", NOPE, TAB_AETHER, "Move to Guild House - free");
	if (Job()->House()->PlayerHouseID(pPlayer) >= 1)
		GS()->AVM(ClientID, "HSPAWN", NOPE, TAB_AETHER, "Move to Your House - free");

	for (const auto& tl : Teleport)
	{
		if (pPlayer->Acc().AetherLocation.find(tl.first) == pPlayer->Acc().AetherLocation.end())
			continue;

		bool LocalTeleport = (tl.second.WorldID == GS()->Server()->GetWorldID(ClientID) &&
			distance(pPlayer->GetCharacter()->m_Core.m_Pos, vec2(tl.second.TeleX, tl.second.TeleY)) < 120);
		if (LocalTeleport)
		{
			GS()->AVM(ClientID, "null", tl.first, TAB_AETHER, "[Local] {STR}", tl.second.TeleName);
			continue;
		}

		int Price = g_Config.m_SvPriceTeleport * (tl.second.WorldID + 1);
		GS()->AVD(ClientID, "TELEPORT", tl.first, Price, TAB_AETHER, "{STR} {STR} - {INT}gold",
			tl.second.TeleName, GS()->Server()->GetWorldName(tl.second.WorldID), &Price);
	}
}