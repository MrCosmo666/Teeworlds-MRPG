#include <engine/server.h>

#include <teeother/components/localization.h>
#include "game/server/gamecontext.h"
#include "CommandProcessor.h"

/*
	Later lead to quality
*/

void CommandProcessor::ChatCmd(CNetMsg_Cl_Say *Msg, CGS *GS, CPlayer *pPlayer)
{
	LastChat(GS, pPlayer);
	const int ClientID = pPlayer->GetCID();

	// АВТОРИЗАЦИЯ
	if(str_comp_num(Msg->m_pMessage, "/login", 6) == 0)
	{
		if (pPlayer->IsAuthed())
		{
			GS->Chat(ClientID, "You already authed.");
			return;
		}

		char Username[256], Password[256];
		if (sscanf(Msg->m_pMessage, "/login %s %s", Username, Password) != 2)
			return GS->ChatFollow(ClientID, "Use: /login <username> <password>");

		if (GS->Mmo()->Account()->LoginAccount(ClientID, Username, Password) == AUTH_LOGIN_GOOD)
			GS->Mmo()->Account()->LoadAccount(pPlayer, true);
		return;
	}

	// РЕГИСТРАЦИЯ
	else if(str_comp_num(Msg->m_pMessage, "/register", 9) == 0)
	{
		if (pPlayer->IsAuthed())
		{
			GS->Chat(ClientID, "Logout account and create!");
			return;
		}

		char Username[256], Password[256];
		if (sscanf(Msg->m_pMessage, "/register %s %s", Username, Password) != 2)
			return GS->ChatFollow(ClientID, "Use: /register <username> <password>");

		GS->Mmo()->Account()->RegisterAccount(ClientID, Username, Password);
		return;
	}

#ifdef CONF_DISCORD
	else if(str_comp_num(Msg->m_pMessage, "/discord_connect", 16) == 0)
	{
		if (!pPlayer->IsAuthed())
			return;

		char DiscordDID[256];
		if (sscanf(Msg->m_pMessage, "/discord_connect %s", DiscordDID) != 1)
			return GS->ChatFollow(ClientID, "Use: /discord_connect <DID>");
		if (str_length(DiscordDID) > 30 || str_length(DiscordDID) < 10)
			return GS->ChatFollow(ClientID, "Discord ID must contain 10-30 characters.");

		GS->Mmo()->Account()->DiscordConnect(ClientID, DiscordDID);
		return;
	}
#endif

/////////////////////////////////////// MEMBER COMMAN //////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////
	else if(str_comp_num(Msg->m_pMessage, "/gexit", 5) == 0)
	{
		if(!pPlayer->IsAuthed())
			return;

		if(pPlayer->Acc().IsGuild())
		{
			int AuthID = pPlayer->Acc().AuthID;
			ExitGuild(GS, AuthID);
		}
		return;
	}
	else if(str_comp_num(Msg->m_pMessage, "/gcreate", 8) == 0)
	{
		if(!pPlayer->IsAuthed())
			return;

		if(!pPlayer->Acc().IsGuild())
		{
			char GuildName[256];
			if(sscanf(Msg->m_pMessage, "/gcreate %s", GuildName) != 1) 
				return GS->ChatFollow(ClientID, "Use: /gcreate <guildname>");
			if(str_length(GuildName) > 8 || str_length(GuildName) < 3)
				return GS->ChatFollow(ClientID, "Guild name must contain 3-8 characters");

			CreateGuild(GS, ClientID, GuildName);
		}
		return;
	}

//////////////////////////////////////// HOUSE COMMAN //////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////	
	else if (str_comp_num(Msg->m_pMessage, "/doorhouse", 10) == 0)
	{
		if(!pPlayer->IsAuthed())
			return;

		const int HouseID = PlayerHouseID(GS, pPlayer);
		ChangeStateDoor(GS, HouseID);
		return;
	}
	else if (str_comp_num(Msg->m_pMessage, "/sellhouse", 10) == 0)
	{
		if(!pPlayer->IsAuthed())
			return;

		// check owner house id
		const int HouseID = PlayerHouseID(GS, pPlayer);
		if(HouseID < 0)
		{
			GS->Chat(ClientID, "You have no home.");
			return;
		}
		// sell house
		GS->Mmo()->House()->SellHouse(HouseID);
		return;
	}

/////////////////////////////////////// HELPER COMMAND /////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////	
	else if (str_comp_num(Msg->m_pMessage, "/pos", 4) == 0)
	{
		if(!pPlayer->GetCharacter())
			return;

		const int PosX = pPlayer->GetCharacter()->m_Core.m_Pos.x/32;
		const int PosY = pPlayer->GetCharacter()->m_Core.m_Pos.y/32;
		GS->Chat(ClientID, "[{STR}] Position X: {INT} Y: {INT}.", GS->Server()->GetWorldName(GS->GetWorldID()), &PosX, &PosY);
		dbg_msg("test", "%0.f %0.f WorldID: %d", pPlayer->GetCharacter()->m_Core.m_Pos.x, pPlayer->GetCharacter()->m_Core.m_Pos.y, GS->GetWorldID());
		return;
	}
	else if (str_comp_num(Msg->m_pMessage, "/sd", 3) == 0 && GS->Server()->IsAuthed(ClientID))
	{
		int size = 0;
		if ((sscanf(Msg->m_pMessage, "/sd %d", &size)) != 1)
			return GS->ChatFollow(ClientID, "Please use: /sd <idsound>");

		const int soundid = clamp(size, 0, 40);
		if (GS->GetPlayerChar(ClientID))
			GS->CreateSound(pPlayer->GetCharacter()->m_Core.m_Pos, soundid);
		return;
	}

	else if (str_comp_num(Msg->m_pMessage, "/useitem", 8) == 0)
	{
		if(!pPlayer->IsAuthed())
			return;

		int sitemid = 0;
		if ((sscanf(Msg->m_pMessage, "/useitem %d", &sitemid)) != 1)
			return GS->ChatFollow(ClientID, "Please use: /useitem <itemid>");
		
		UseItems(GS, ClientID, sitemid, 1);
		return;
	}

	else if (str_comp_num(Msg->m_pMessage, "/useskill", 9) == 0)
	{
		if(!pPlayer->IsAuthed())
			return;

		int sskillid = 0;
		if ((sscanf(Msg->m_pMessage, "/useskill %d", &sskillid)) != 1)
			return GS->ChatFollow(ClientID, "Please use: /useitem <itemid>");

		UseSkill(GS, pPlayer, sskillid);
		return;
	}

///////////////////////////////////////// INFO COMMAN //////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////	
	else if(str_comp_num(Msg->m_pMessage, "/cmdlist", 8) == 0 || str_comp_num(Msg->m_pMessage, "/help", 5) == 0)
	{
		GS->ChatFollow(ClientID, "Command List / Help");
		GS->ChatFollow(ClientID, "/register <name> <pass> - new account.");
		GS->ChatFollow(ClientID, "/login <name> <pass> - log in account.");
		GS->ChatFollow(ClientID, "/lang <codes iso> - language (translation is not complete).");
		GS->ChatFollow(ClientID, "/rules - server rules.");
		GS->ChatFollow(ClientID, "Another information see Wiki Page.");
		return;
	}
	else if(str_comp_num(Msg->m_pMessage, "/rules", 6) == 0)
	{
		GS->ChatFollow(ClientID, "Server rules");
		GS->ChatFollow(ClientID, "- Don't use bugs");
		GS->ChatFollow(ClientID, "- Don't use bots and other hack soft");
		GS->ChatFollow(ClientID, "- Don't use dummy multi-account's");
		GS->ChatFollow(ClientID, "- Don't share self account data (login, password)");
		GS->ChatFollow(ClientID, "- Do not use ads, that is not part of the game");
		return;
	}
	if(str_comp_num(Msg->m_pMessage, "/", 1) == 0)
	{
		GS->ChatFollow(ClientID, "Command {STR} not found!", Msg->m_pMessage);
		return;
	}
}

void CommandProcessor::LastChat(CGS *GS, CPlayer *pPlayer)
{
	if(pPlayer->m_PlayerTick[TickState::LastChat] + GS->Server()->TickSpeed() <= GS->Server()->Tick())
		pPlayer->m_PlayerTick[TickState::LastChat] = GS->Server()->Tick();
}

bool CommandProcessor::IsLeaderPlayer(CGS *GS, CPlayer *pPlayer, int Access) const
{
	return GS->Mmo()->Member()->IsLeaderPlayer(pPlayer, Access);
}

void CommandProcessor::ExitGuild(CGS *GS, int AccountID)
{
	GS->Mmo()->Member()->ExitGuild(AccountID);
}

void CommandProcessor::CreateGuild(CGS *GS, int ClientID, const char *pName)
{
	GS->Mmo()->Member()->CreateGuild(ClientID, pName);
}

void CommandProcessor::ChangeStateDoor(CGS *GS, int HouseID)
{
	GS->Mmo()->House()->ChangeStateDoor(HouseID);
}

int CommandProcessor::PlayerHouseID(CGS *GS, CPlayer *pPlayer) const
{
	return GS->Mmo()->House()->PlayerHouseID(pPlayer);
}

void CommandProcessor::UseItems(CGS *GS, int ClientID, int ItemID, int Count)
{
	GS->Mmo()->Item()->UseItem(ClientID, ItemID, Count);
}

bool CommandProcessor::UseSkill(CGS *GS, CPlayer *pPlayer, int SkillID) const
{
	return GS->Mmo()->Skills()->UseSkill(pPlayer, SkillID);
}