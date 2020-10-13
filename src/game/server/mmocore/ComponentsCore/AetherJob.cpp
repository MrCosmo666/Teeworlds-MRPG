/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#include <engine/shared/config.h>
#include <game/server/gamecontext.h>
#include "AetherJob.h"

using namespace sqlstr;
std::map < int , AetherJob::StructTeleport > AetherJob::ms_aTeleport;

void AetherJob::OnInit()
{
	SJK.SDT("*", "tw_aethers", [&](ResultSet* RES)
	{
		while(RES->next())
		{
			const int ID = RES->getInt("ID");
			str_copy(ms_aTeleport[ID].m_aTeleName, RES->getString("TeleName").c_str(), sizeof(ms_aTeleport[ID].m_aTeleName));
			ms_aTeleport[ID].m_TeleX = RES->getInt("TeleX");
			ms_aTeleport[ID].m_TeleY = RES->getInt("TeleY");
			ms_aTeleport[ID].m_WorldID = RES->getInt("WorldID");
		}
		Job()->ShowLoadingProgress("Aethers", ms_aTeleport.size());
	});
}

void AetherJob::OnInitAccount(CPlayer *pPlayer)
{
	std::shared_ptr<ResultSet> RES(SJK.SD("*", "tw_accounts_locations", "WHERE OwnerID = '%d'", pPlayer->Acc().m_AuthID));
	while(RES->next())
	{
		int TeleportID = RES->getInt("TeleportID");
		pPlayer->Acc().m_aAetherLocation[TeleportID] = true;
	}
}

bool AetherJob::OnVotingMenu(CPlayer *pPlayer, const char *CMD, const int VoteID, const int VoteID2, int Get, const char *GetText)
{
	const int ClientID = pPlayer->GetCID();

	// teleport
	if(PPSTR(CMD, "TELEPORT") == 0)
	{
		const int TeleportID = VoteID;
		const int Price = VoteID2;
		if(Price > 0 && !pPlayer->SpendCurrency(Price))
			return true;

		const vec2 Position = vec2(ms_aTeleport[TeleportID].m_TeleX, ms_aTeleport[TeleportID].m_TeleY);
		if(!GS()->IsPlayerEqualWorldID(ClientID, ms_aTeleport[TeleportID].m_WorldID))
		{
			pPlayer->GetTempData().m_TempTeleportX = Position.x;
			pPlayer->GetTempData().m_TempTeleportY = Position.y;
			pPlayer->ChangeWorld(ms_aTeleport[TeleportID].m_WorldID);
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
		GS()->Chat(ClientID, "You can see menu in the votes!");
		pChr->m_Core.m_ProtectHooked = pChr->m_NoAllowDamage = true;
		UnlockLocation(pChr->GetPlayer(), pChr->m_Core.m_Pos);
		GS()->UpdateVotes(ClientID, pPlayer->m_OpenVoteMenu);
		return true;
	}
	else if (pChr->GetHelper()->TileExit(IndexCollision, TILE_AETHER_TELEPORT))
	{
		GS()->Chat(ClientID, "You left the active zone, menu is restored!");
		pChr->m_Core.m_ProtectHooked = pChr->m_NoAllowDamage = false;
		GS()->UpdateVotes(ClientID, pPlayer->m_OpenVoteMenu);
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

		if (pChr->GetHelper()->BoolIndex(TILE_AETHER_TELEPORT))
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
	for (const auto& tl : ms_aTeleport)
	{
		if (distance(vec2(tl.second.m_TeleX, tl.second.m_TeleY), Pos) > 100 || pPlayer->Acc().m_aAetherLocation.find(tl.first) != pPlayer->Acc().m_aAetherLocation.end())
			continue;

		SJK.ID("tw_accounts_locations", "(OwnerID, TeleportID) VALUES ('%d', '%d')", pPlayer->Acc().m_AuthID, tl.first);
		GS()->Chat(ClientID, "You unlock aether {STR}!", ms_aTeleport[tl.first].m_aTeleName);
		GS()->ChatDiscord("14671083", GS()->Server()->ClientName(ClientID), "Adventure unlock aether {STR}", ms_aTeleport[tl.first].m_aTeleName);

		pPlayer->Acc().m_aAetherLocation[tl.first] = true;
		return;
	}
}

void AetherJob::ShowTeleportList(CCharacter* pChar)
{
	CPlayer* pPlayer = pChar->GetPlayer();
	const int ClientID = pPlayer->GetCID();
	GS()->ShowItemValueInformation(pPlayer);
	GS()->AV(ClientID, "null");
	
	GS()->AVH(ClientID, TAB_AETHER, GOLDEN_COLOR, "Available aethers");
	if (Job()->Member()->GetGuildHouseID(pPlayer->Acc().m_GuildID) >= 1)
		GS()->AVM(ClientID, "MSPAWN", NOPE, TAB_AETHER, "Move to Guild House - free");
	if (Job()->House()->PlayerHouseID(pPlayer) >= 1)
		GS()->AVM(ClientID, "HSPAWN", NOPE, TAB_AETHER, "Move to Your House - free");

	for (const auto& tl : ms_aTeleport)
	{
		if (pPlayer->Acc().m_aAetherLocation.find(tl.first) == pPlayer->Acc().m_aAetherLocation.end())
			continue;

		const bool LocalTeleport = (GS()->IsPlayerEqualWorldID(ClientID, tl.second.m_WorldID) &&
			distance(pPlayer->GetCharacter()->m_Core.m_Pos, vec2(tl.second.m_TeleX, tl.second.m_TeleY)) < 120);
		if (LocalTeleport)
		{
			GS()->AVM(ClientID, "null", tl.first, TAB_AETHER, "[Local {STR}] : {STR}", tl.second.m_aTeleName, GS()->Server()->GetWorldName(tl.second.m_WorldID));
			continue;
		}

		int Price = g_Config.m_SvPriceTeleport * (tl.second.m_WorldID + 1);
		GS()->AVD(ClientID, "TELEPORT", tl.first, Price, TAB_AETHER, "[{STR}] : {STR} - {INT}gold",
			tl.second.m_aTeleName, GS()->Server()->GetWorldName(tl.second.m_WorldID), &Price);
	}
	GS()->AV(ClientID, "null");
}