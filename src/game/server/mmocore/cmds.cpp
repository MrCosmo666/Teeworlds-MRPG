#include <engine/shared/config.h>
#include <engine/server.h>

#include "game/server/gamecontext.h"
#include "cmds.h"

/*
	Later lead to quality standard code
*/

void CCmd::ChatCmd(CNetMsg_Cl_Say *Msg, CGS *GS, CPlayer *pPlayer)
{
	int ClientID = pPlayer->GetCID();

	// вход в аккаунт
	if(str_comp_num(Msg->m_pMessage, "/login", 6) == 0)
	{
		LastChat(GS, pPlayer); 	
		GS->m_pController->OnPlayerCommand(pPlayer, "login", Msg->m_pMessage);
		return;
	}

	// регистрация аккаунта
	 if(str_comp_num(Msg->m_pMessage, "/register", 9) == 0)
	{
		LastChat(GS, pPlayer);
		GS->m_pController->OnPlayerCommand(pPlayer, "register", Msg->m_pMessage);
		return;
	}

#ifdef CONF_DISCORD
	else if(str_comp_num(Msg->m_pMessage, "/discord_connect", 16) == 0)
	{
		LastChat(GS, pPlayer); 
		GS->m_pController->OnPlayerCommand(pPlayer, "discord_connect", Msg->m_pMessage);
		return;
	}
#endif

/////////////////////////////////////// MEMBER COMMAN //////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////
	else if(str_comp_num(Msg->m_pMessage, "/ginvite", 8) == 0)
	{
		LastChat(GS, pPlayer); 		

		// check authed
		if(!pPlayer->IsAuthed())
			return;

		// check leader account
		if(pPlayer->Acc().MemberID > 0)
		{
			if(!IsLeaderPlayer(GS, pPlayer, MACCESSINVITEKICK))
				return GS->Chat(ClientID, "You have no access.");

			int CID;
			if(sscanf(Msg->m_pMessage, "/ginvite %d", &CID) != 1) 
				return GS->Chat(ClientID, "Use: /ginvite <ClientID>");

			CPlayer *pPlayerCID = GS->m_apPlayers[CID];
			if(CID < 0 || CID > MAX_CLIENTS || !pPlayerCID || !pPlayerCID->IsAuthed() || ClientID == CID)
				return GS->Chat(ClientID, "Not entered the correct ID of the player.");
			
			// check member group player		
			if(pPlayerCID->Acc().MemberID > 0)
				return GS->Chat(ClientID, "Player already in member group.");

			// start parsing
			int MemberID = pPlayer->Acc().MemberID;
			if(pPlayerCID->SetParsing(10, ClientID, MemberID, "Member"))
			{
				GS->Chat(ClientID, "You invite in member {STR}.", GS->Server()->ClientName(CID));
				GS->Chat(ClientID, "{STR} offered enter in member {STR}.", GS->Server()->ClientName(ClientID), GS->Mmo()->Member()->MemberName(MemberID));
			}
		}
		return;
	}
	else if(str_comp_num(Msg->m_pMessage, "/gexit", 5) == 0)
	{
		LastChat(GS, pPlayer); 		

		// check authed
		if(!pPlayer->IsAuthed())
			return;

		// start parsing
		if(pPlayer->Acc().MemberID > 0)
		{
			int AuthID = pPlayer->Acc().AuthID;
			ExitGuild(GS, AuthID);
		}
		return;
	}
	else if(str_comp_num(Msg->m_pMessage, "/gcreate", 8) == 0)
	{
		LastChat(GS, pPlayer); 		

		// check authed
		if(!pPlayer->IsAuthed())
			return;

		// create member
		if(pPlayer->Acc().MemberID <= 0)
		{
			char MemberName[256];
			if(sscanf(Msg->m_pMessage, "/gcreate %s", MemberName) != 1) 
				return GS->ChatFollow(ClientID, "Use: /gcreate <guildname>");
			if(str_length(MemberName) > 8 || str_length(MemberName) < 3)
				return GS->ChatFollow(ClientID, "Guild name must contain 3-8 characters");

			CreateGuild(GS, ClientID, MemberName);
		}
		return;
	}

//////////////////////////////////////// HOUSE COMMAN //////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////	
	else if (str_comp_num(Msg->m_pMessage, "/doorhouse", 10) == 0)
	{
		LastChat(GS, pPlayer); 
		if(!pPlayer->IsAuthed())
			return;

		int HouseID = PlayerHouseID(GS, pPlayer);
		ChangeStateDoor(GS, HouseID);
		return;
	}
	else if (str_comp_num(Msg->m_pMessage, "/sellhouse", 10) == 0)
	{
		LastChat(GS, pPlayer); 

		// check authed
		if(!pPlayer->IsAuthed())
			return;

		// check owner house id
		int HouseID = PlayerHouseID(GS, pPlayer);
		if(HouseID < 0)
			return GS->Chat(ClientID, "You have no home.");

		// sell house
		GS->Mmo()->House()->SellHouse(HouseID);
		return;
	}
	else if (str_comp_num(Msg->m_pMessage, "/selltohouse", 12) == 0)
	{
		LastChat(GS, pPlayer); 

		// check authed
		if(!pPlayer->IsAuthed())
			return;

		int CID, Price;
		if(sscanf(Msg->m_pMessage, "/selltohouse %d %d", &CID, &Price) != 2) 
			return GS->ChatFollow(ClientID, "Use: /selltohouse <ClientID> <Price>");

		CPlayer *pPlayerCID = GS->m_apPlayers[CID];
		if(CID < 0 || CID > MAX_CLIENTS || Price < 100 || !pPlayerCID || !pPlayerCID->IsAuthed() || ClientID == CID)
			return GS->Chat(ClientID, "Not entered the correct ID of the player. House min price 100.");

		// start parsing
		if(pPlayerCID->SetParsing(10, ClientID, Price, "Deal"))
		{
			GS->Chat(ClientID, "You sent a request {STR} to sell house {INT} gold.", GS->Server()->ClientName(CID), &Price);
			GS->Chat(CID, "{STR} offered to buy a house {INT} gold.", GS->Server()->ClientName(ClientID),&Price);
		}
		return;
	}

/////////////////////////////////////// HELPER COMMAND /////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////	
	else if (str_comp_num(Msg->m_pMessage, "/pos", 4) == 0)
	{
		LastChat(GS, pPlayer); 
		if(!pPlayer->GetCharacter())
			return;

		int PosX = pPlayer->GetCharacter()->m_Core.m_Pos.x/32;
		int PosY = pPlayer->GetCharacter()->m_Core.m_Pos.y/32;
		GS->Chat(ClientID, "[{STR}] Position X: {INT} Y: {INT}.", GS->Server()->GetWorldName(GS->GetWorldID()), &PosX, &PosY);
		dbg_msg("test", "%0.f %0.f WorldID: %d", pPlayer->GetCharacter()->m_Core.m_Pos.x, pPlayer->GetCharacter()->m_Core.m_Pos.y, GS->GetWorldID());
		return;
	}
	else if (str_comp_num(Msg->m_pMessage, "/sd", 3) == 0 && GS->Server()->IsAuthed(ClientID))
	{
		LastChat(GS, pPlayer); 
		int size = 0;
		if ((sscanf(Msg->m_pMessage, "/sd %d", &size)) != 1)
			return GS->ChatFollow(ClientID, "Please use: /sd <idsound>");

		int soundid = clamp(size, 0, 40);
		if (GS->GetPlayerChar(ClientID))
			GS->CreateSound(pPlayer->GetCharacter()->m_Core.m_Pos, soundid);
		return;
	}

	else if (str_comp_num(Msg->m_pMessage, "/useitem", 8) == 0)
	{
		LastChat(GS, pPlayer);

		// check authed
		if(!pPlayer->IsAuthed())
			return;

		int sitemid = 0;
		if ((sscanf(Msg->m_pMessage, "/useitem %d", &sitemid)) != 1)
			return GS->ChatFollow(ClientID, "Please use: /useitem <itemid>");

		int sizeitems = ItemSql::ItemsInfo.size();
		int itemid = clamp(sitemid, 0, sizeitems);
		UseItems(GS, ClientID, itemid, 1);
		return;
	}

	else if (str_comp_num(Msg->m_pMessage, "/useskill", 9) == 0)
	{
		LastChat(GS, pPlayer);

		// check authed
		if(!pPlayer->IsAuthed())
			return;

		int sskillid = 0;
		if ((sscanf(Msg->m_pMessage, "/useskill %d", &sskillid)) != 1)
			return GS->ChatFollow(ClientID, "Please use: /useitem <itemid>");

		int sizeskills = SkillsSql::SkillData.size();
		int skillid = clamp(sskillid, 0, sizeskills);
		UseSkill(GS, pPlayer, skillid);
		return;
	}

///////////////////////////////////////// INFO COMMAN //////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////	
	else if(str_comp_num(Msg->m_pMessage, "/cmdlist", 8) == 0 || str_comp_num(Msg->m_pMessage, "/help", 5) == 0)
	{
		LastChat(GS, pPlayer);

		GS->m_pController->OnPlayerCommand(pPlayer, "cmdlist", Msg->m_pMessage);
		return;
	}

	/*else if(!strncmp(Msg->m_pMessage, "/lang", 5))
	{	
		char pLanguageCode[128];
		char aFinalLanguageCode[8];
		aFinalLanguageCode[0] = 0;

		if(sscanf(Msg->m_pMessage, "/lang %s", pLanguageCode) == 1)
		{ 
			if(str_comp_num_nocase(pLanguageCode, "ua") == 0)
				str_copy(aFinalLanguageCode, "uk", sizeof(aFinalLanguageCode));
			else
			{
				for(int i=0; i< GS->Server()->Localization()->m_pLanguages.size(); i++)
				{
					if(str_comp_num_nocase(pLanguageCode, GS->Server()->Localization()->m_pLanguages[i]->GetFilename()) == 0)
						str_copy(aFinalLanguageCode, pLanguageCode, sizeof(aFinalLanguageCode));
				}
			}
		}
		
		if(aFinalLanguageCode[0])
		{
			GS->Server()->SetClientLanguage(ClientID, aFinalLanguageCode);
			if(pPlayer)
				pPlayer->SetLanguage(aFinalLanguageCode);
				
			if(GS->Server()->ISCientLogged(ClientID)) GS->ResetVotes(ClientID, AUTH);
			else GS->ResetVotes(ClientID, NOAUTH);
		}
		else
		{
			const char* pLanguage = pPlayer->GetLanguage();
			dynamic_string BufferList;
			int BufferIter = 0;
			for(int i=0; i< GS->Server()->Localization()->m_pLanguages.size(); i++)
			{
				if(i>0)
					BufferIter = BufferList.append_at(BufferIter, ", ");
				BufferIter = BufferList.append_at(BufferIter, GS->Server()->Localization()->m_pLanguages[i]->GetFilename());
			}
			
			dynamic_string Buffer;
			GS->Server()->Localization()->Format_L(Buffer, pLanguage, _("Available languages: {str:ListOfLanguage}"), 
				"ListOfLanguage", BufferList.buffer(), NULL);
			GS->SendChatTarget(pPlayer->GetCID(), Buffer.buffer());
			Buffer.clear();
		}
		return;
	}*/
 
	if(str_comp_num(Msg->m_pMessage, "/", 1) == 0)
	{
		LastChat(GS, pPlayer); 
		GS->ChatFollow(ClientID, "Command {s:cmd} not found!", Msg->m_pMessage);
		return;
	}
}

void CCmd::LastChat(CGS *GS, CPlayer *pPlayer)
{
	pPlayer->m_PlayerTick[TickState::LastChat] = GS->Server()->Tick();
}

bool CCmd::IsLeaderPlayer(CGS *GS, CPlayer *pPlayer, int Access) const
{
	return GS->Mmo()->Member()->IsLeaderPlayer(pPlayer, Access);
}

void CCmd::ExitGuild(CGS *GS, int AccountID)
{
	GS->Mmo()->Member()->ExitGuild(AccountID);
}

void CCmd::CreateGuild(CGS *GS, int ClientID, const char *pName)
{
	GS->Mmo()->Member()->CreateGuild(ClientID, pName);
}

void CCmd::ChangeStateDoor(CGS *GS, int HouseID)
{
	GS->Mmo()->House()->ChangeStateDoor(HouseID);
}

int CCmd::PlayerHouseID(CGS *GS, CPlayer *pPlayer) const
{
	return GS->Mmo()->House()->PlayerHouseID(pPlayer);
}

void CCmd::UseItems(CGS *GS, int ClientID, int ItemID, int Count)
{
	GS->Mmo()->Item()->UsedItems(ClientID, ItemID, Count);
}

bool CCmd::UseSkill(CGS *GS, CPlayer *pPlayer, int SkillID) const
{
	return GS->Mmo()->Skills()->UseSkill(pPlayer, SkillID);
}