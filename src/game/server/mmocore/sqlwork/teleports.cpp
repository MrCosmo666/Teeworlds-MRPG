/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#include <engine/shared/config.h>
#include <game/server/gamecontext.h>
#include "teleports.h"

using namespace sqlstr;
std::map < int , TeleportsSql::StructTeleport > TeleportsSql::Teleport;

void TeleportsSql::OnInitGlobal() 
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

void TeleportsSql::OnInitAccount(CPlayer *pPlayer)
{
	boost::scoped_ptr<ResultSet> RES(SJK.SD("*", "tw_accounts_locations", "WHERE OwnerID = '%d'", pPlayer->Acc().AuthID));
	while(RES->next())
	{
		int TeleportID = RES->getInt("TeleportID");
		pPlayer->Acc().AetherLocation[TeleportID] = true;
	}
}

// парсинг голосований
bool TeleportsSql::OnParseVotingMenu(CPlayer *pPlayer, const char *CMD, const int VoteID, const int VoteID2, int Get, const char *GetText)
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

bool TeleportsSql::OnPlayerHandleTile(CCharacter* pChr, int IndexCollision)
{
	CPlayer* pPlayer = pChr->GetPlayer();
	const int ClientID = pPlayer->GetCID();

	if (pChr->GetHelper()->TileEnter(IndexCollision, TILE_AETHER))
	{
		GS()->Chat(ClientID, "List of your Aether Locations, you can see on vote!");
		UnlockLocation(ClientID, pChr->m_Core.m_Pos);
		pChr->m_Core.m_ProtectHooked = true;
		pChr->m_NoAllowDamage = true;

		GS()->VResetVotes(ClientID, MAINMENU);
		return true;
	}
	else if (pChr->GetHelper()->TileExit(IndexCollision, TILE_AETHER))
	{
		pChr->m_Core.m_ProtectHooked = false;
		pChr->m_NoAllowDamage = false;
		GS()->VResetVotes(ClientID, MAINMENU);
		return true;
	}

	return false;
}

bool TeleportsSql::OnPlayerHandleMainMenu(CPlayer* pPlayer, int Menulist, bool ReplaceMenu)
{
	if (ReplaceMenu)
	{
		CCharacter* pChr = pPlayer->GetCharacter();
		if (!pChr || !pChr->IsAlive()) return false;

		if (Menulist == MAINMENU && pChr->GetHelper()->BoolIndex(TILE_AETHER))
		{
			ShowTeleportList(pChr);
			return true;
		}
		return false;
	}


	return false;
}

/* #########################################################################
	FUNCTION TELEPORT CLASS
######################################################################### */
void TeleportsSql::UnlockLocation(int ClientID, vec2 Pos)
{
	CPlayer* pPlayer = GS()->GetPlayer(ClientID);
	if (!pPlayer) return;

	for (const auto& tl : Teleport)
	{
		if (distance(vec2(tl.second.TeleX, tl.second.TeleY), Pos) > 100 ||
			pPlayer->Acc().AetherLocation.find(tl.first) != pPlayer->Acc().AetherLocation.end())
			continue;

		SJK.ID("tw_location", "(OwnerID, TeleportID) VALUES ('%d', '%d')", pPlayer->Acc().AuthID, tl.first);
		GS()->Chat(ClientID, "You unlock new location {STR}!", Teleport[tl.first].TeleName);
		GS()->ChatDiscord(false, "14671083", GS()->Server()->ClientName(ClientID),
			"Adventure unlock new location {STR}", Teleport[tl.first].TeleName);

		pPlayer->Acc().AetherLocation[tl.first] = true;
		return;
	}
}

void TeleportsSql::ShowTeleportList(CCharacter* pChar)
{
	CPlayer* pPlayer = pChar->GetPlayer();
	const int ClientID = pPlayer->GetCID();

	GS()->AVH(ClientID, HTELEPORTLIST, GOLDEN_COLOR, "Available teleports");
	if (Job()->Member()->GetGuildHouseID(pPlayer->Acc().GuildID) >= 1)
		GS()->AVM(ClientID, "MSPAWN", NOPE, HTELEPORTLIST, "Move to Member House - free");
	if (Job()->House()->PlayerHouseID(pPlayer) >= 1)
		GS()->AVM(ClientID, "HSPAWN", NOPE, HTELEPORTLIST, "Move to Your House - free");

	for (const auto& tl : Teleport)
	{
		if (pPlayer->Acc().AetherLocation.find(tl.first) == pPlayer->Acc().AetherLocation.end())
			continue;

		bool LocalTeleport = (tl.second.WorldID == GS()->Server()->GetWorldID(ClientID) &&
			distance(pPlayer->GetCharacter()->m_Core.m_Pos, vec2(tl.second.TeleX, tl.second.TeleY)) < 120);
		if (LocalTeleport)
		{
			GS()->AVM(ClientID, "null", tl.first, HTELEPORTLIST, "[Local] {STR}", tl.second.TeleName);
			continue;
		}

		int Price = g_Config.m_SvPriceTeleport * (tl.second.WorldID + 1);
		GS()->AVD(ClientID, "TELEPORT", tl.first, Price, HTELEPORTLIST, "{STR} {STR} - {INT}gold",
			tl.second.TeleName, GS()->Server()->GetWorldName(tl.second.WorldID), &Price);
	}
}