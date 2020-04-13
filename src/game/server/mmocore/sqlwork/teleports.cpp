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
		Teleport[ID].TeleX = RES->getInt("TeleX");
		Teleport[ID].TeleY = RES->getInt("TeleY");
		Teleport[ID].WorldID = RES->getInt("WorldID");
		str_copy(Teleport[ID].TeleName, RES->getString("TeleName").c_str(), sizeof(Teleport[ID].TeleName));
	}
	Job()->ShowLoadingProgress("Teleports", Teleport.size());	
}

void TeleportsSql::OnInitAccount(CPlayer *pPlayer)
{
	boost::scoped_ptr<ResultSet> RES(SJK.SD("*", "tw_location", "WHERE OwnerID = '%d'", pPlayer->Acc().AuthID));
	while(RES->next())
	{
		int TeleportID = RES->getInt("TeleportID");
		pPlayer->Acc().AetherLocation[TeleportID] = true;
	}
}

bool TeleportsSql::OnPlayerHandleTile(CCharacter *pChr, int IndexCollision)
{
	CPlayer *pPlayer = pChr->GetPlayer();
	int ClientID = pPlayer->GetCID();

	if(pChr->GetHelper()->TileEnter(IndexCollision, TILE_AETHER))
	{
		GS()->Chat(ClientID, "List of your Aether Locations, you can see on vote!");
		UnlockLocation(ClientID, pChr->m_Core.m_Pos);
		pChr->m_Core.m_ProtectHooked = true;
		pChr->m_NoAllowDamage = true;

		GS()->VResetVotes(ClientID, MAINMENU);
		return true;
	}
	else if(pChr->GetHelper()->TileExit(IndexCollision, TILE_AETHER))
	{
		pChr->m_Core.m_ProtectHooked = false;
		pChr->m_NoAllowDamage = false;
		return true;
	}

	return false;
}

bool TeleportsSql::OnPlayerHandleMainMenu(CPlayer *pPlayer, int Menulist)
{
	CCharacter *pChr = pPlayer->GetCharacter();
	if(!pChr) return false;

	if(Menulist == MAINMENU && pChr->GetHelper()->BoolIndex(TILE_AETHER))
	{
		ShowTeleportList(pPlayer);	
		return true;
	}
	return false;
}


/* #########################################################################
	FUNCTION TELEPORT CLASS 
######################################################################### */
// Разблокировать телепорт
void TeleportsSql::UnlockLocation(int ClientID, vec2 Pos)
{
	CPlayer *pPlayer = GS()->GetPlayer(ClientID);
	if(!pPlayer) return;

	for(const auto& tl : Teleport)
	{
		if(distance(vec2(tl.second.TeleX, tl.second.TeleY), Pos) > 100 || 
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

// Показать лист телепортов
void TeleportsSql::ShowTeleportList(CPlayer *pPlayer)
{
	if(!pPlayer->GetCharacter()) return;

	const int ClientID = pPlayer->GetCID();

	// весь список телепортов	
	GS()->AVH(ClientID, HTELEPORTLIST, vec3(50, 10, 5), _("Available teleports"), NULL);
	
	// если имеется дом организации
	if(Job()->Member()->GetMemberHouseID(pPlayer->Acc().MemberID) >= 1)
		GS()->AVM(ClientID, "MSPAWN", NOPE, HTELEPORTLIST, _("Move to Member House - free"), NULL);\

	// если имеется дом
	if(Job()->House()->PlayerHouseID(pPlayer) >= 1)
		GS()->AVM(ClientID, "HSPAWN", NOPE, HTELEPORTLIST, _("Move to Your House - free"), NULL);

	for(const auto& tl : Teleport)
	{
		if(pPlayer->Acc().AetherLocation.find(tl.first) == pPlayer->Acc().AetherLocation.end()) 
			continue;

		// проверяем локальное место
		bool LocalTeleport = (Teleport[tl.first].WorldID == GS()->Server()->GetWorldID(ClientID) && 
			distance(pPlayer->GetCharacter()->m_Core.m_Pos, vec2(tl.second.TeleX, tl.second.TeleY)) < 120);	
		if(LocalTeleport)
		{
			GS()->AVM(ClientID, "null", tl.first, HTELEPORTLIST, _("[Local] {s:name}"), "name", tl.second.TeleName, NULL);	
			continue;
		}
		
		// если не локальное то пишем цену и добавляем возможность телепорта
		int Price = g_Config.m_SvPriceTeleport*tl.second.WorldID;
		GS()->AVM(ClientID, "TELEPORT", tl.first, HTELEPORTLIST, _("{s:name} {s:world} - {i:price}gold"), 
			"name", tl.second.TeleName, "world", GS()->Server()->GetWorldName(tl.second.WorldID),  "price", &Price, NULL);	
	}
}

// парсинг голосований
bool TeleportsSql::OnParseVotingMenu(CPlayer *pPlayer, const char *CMD, const int VoteID, const int VoteID2, int Get, const char *GetText)
{
	const int ClientID = pPlayer->GetCID();

	// телепорт
	if(PPSTR(CMD, "TELEPORT") == 0)
	{
		// проверяем золото
		const int Price = g_Config.m_SvPriceTeleport * Teleport[VoteID].WorldID;
		if(Price > 0 && pPlayer->CheckFailMoney(Price))
			return true;

		// телепортируем если мир не установленного телепорта
		vec2 Position = vec2(Teleport[VoteID].TeleX, Teleport[VoteID].TeleY);
		if(Teleport[VoteID].WorldID != GS()->Server()->GetWorldID(ClientID))
		{
			pPlayer->Acc().TeleportX = Position.x;
			pPlayer->Acc().TeleportY = Position.y;
			GS()->Server()->ChangeWorld(ClientID, Teleport[VoteID].WorldID);
			return true;
		}

		// меняем позицию если локальный телепорт
		pPlayer->GetCharacter()->ChangePosition(Position);
		GS()->ResetVotes(ClientID, pPlayer->m_OpenVoteMenu);
		return true;
	}
	return false; 
}