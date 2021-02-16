/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#ifdef CONF_DISCORD

#include <base/system.h>
#include <engine/shared/config.h>
#include <game/server/gamecontext.h>
#include "discord_main.h"

#include "discord_commands.h"

std::vector < DiscordCommands::Command > DiscordCommands::m_aCommands;

void DiscordCommands::InitCommands()
{
	// commands important
	DiscordCommands::RegisterCommand("!mhelp", "get help on commands.", ComHelp, TYPE_IMPORTANT);
	DiscordCommands::RegisterCommand("!mconnect", "help for connect your discord to account in game.", ComConnect, TYPE_IMPORTANT|TYPE_GAME);

	// commands game server
	DiscordCommands::RegisterCommand("!monline", "show a list of players on the server.", ComOnline, TYPE_GAME);
	DiscordCommands::RegisterCommand("!mstats", "searching for players and displaying their personal MRPG cards.", ComStats, TYPE_GAME);
	DiscordCommands::RegisterCommand("!mranking", "show the ranking of players by level.", ComRanking, TYPE_GAME);
	DiscordCommands::RegisterCommand("!mgoldranking", "show the ranking of players by gold.", ComRanking, TYPE_GAME);

	// commands fun
	DiscordCommands::RegisterCommand("!mavatar", "show user avatars.", ComAvatar, TYPE_FUN);

	// commands admin
	DiscordCommands::RegisterCommand("!madminhelp", "help on admin commands.", ComAvatar, -1, ACCESS_ADMIN);
}

/************************************************************************/
/*  Important commands                                                  */
/************************************************************************/
void DiscordCommands::ComHelp(DiscordJob *pDiscord, SleepyDiscord::Message message)
{
	std::string ImportantCmd("_**Important commands:**_");
	std::string RelatedGameServerCmd("\n\n_**Related game server commands:**_");
	std::string EntertainmentFunCmd("\n\n_**Entertainment / Fun**_");
	std::string AdminToolsCmd = "";
	for(auto& pCommand : DiscordCommands::m_aCommands)
	{
		if(pCommand.m_TypeFlags & TYPE_IMPORTANT)
			ImportantCmd += "\n- **" + std::string(pCommand.m_aCommand) + "** - " + std::string(pCommand.m_aCommandDesc);
		else if(pCommand.m_TypeFlags & TYPE_GAME)
			RelatedGameServerCmd += "\n- **" + std::string(pCommand.m_aCommand) + "** - " + std::string(pCommand.m_aCommandDesc);
		else if(pCommand.m_TypeFlags & TYPE_FUN)
			EntertainmentFunCmd += "\n- **" + std::string(pCommand.m_aCommand) + "** - " + std::string(pCommand.m_aCommandDesc);
	}

	SleepyDiscord::Embed EmbedHelp;
	EmbedHelp.title = "Commands / Information";
	EmbedHelp.description = ImportantCmd + RelatedGameServerCmd + EntertainmentFunCmd + AdminToolsCmd;
	EmbedHelp.color = 1233333;
	pDiscord->sendMessage(message.channelID, "\0", EmbedHelp);
}

void DiscordCommands::ComConnect(class DiscordJob* pDiscord, SleepyDiscord::Message message)
{
	sqlstr::CSqlString<64> DiscordID = sqlstr::CSqlString<64>(std::string(message.author.ID).c_str());
	ResultPtr pRes = SJK.SD("Nick", "tw_accounts_data", "WHERE DiscordID = '%s'", DiscordID.cstr());
	if(!pRes->rowsCount())
	{
		SleepyDiscord::Embed EmbedConnectInfo;
		EmbedConnectInfo.title = "Information on how to connect your account";
		EmbedConnectInfo.description = "_**Warning:**_"
			"\nDo not connect other people's discord accounts to your in-game account. This is similar to hacking your account in the game."
			"\n\n_**How to connect:**_"
			"\nYou need to enter in the game the command \"__/discord_connect <did>__\"."
			"\nYour personal Discord ID: " + message.author.ID;
		EmbedConnectInfo.color = string_to_number(DC_DISCORD_INFO, 0, 1410065407);
		pDiscord->sendMessage(message.channelID, "\0", EmbedConnectInfo);

		pDiscord->SendWarningMessage(message.channelID, "Your account is not connected to the game account.\nSee the information in !mconnect, to connect your account to Discord.");
		return;
	}

	if(pRes->next())
	{
		std::string Nick(pRes->getString("Nick").c_str());
		pDiscord->SendSuccesfulMessage(message.channelID, "Your account is connected to the game nickname [" + Nick + "].");
	}
}


/************************************************************************/
/*  Game server commands                                                */
/************************************************************************/
void DiscordCommands::ComOnline(class DiscordJob* pDiscord, SleepyDiscord::Message message)
{
	std::string Onlines = "";
	CGS* pGS = (CGS*)pDiscord->Server()->GameServer(MAIN_WORLD_ID);
	for(int i = 0; i < MAX_PLAYERS; i++)
	{
		CPlayer* pPlayer = pGS->GetPlayer(i);
		if(pPlayer)
		{
			std::string Nickname(pDiscord->Server()->ClientName(i));
			std::string Team(pPlayer->GetTeam() != TEAM_SPECTATORS ? "Ingame" : "Spectator");
			Onlines += "**" + Nickname + "** - " + Team + "\n";
		}
	}

	SleepyDiscord::Embed EmbedOnlines;
	EmbedOnlines.title = "Online Server";
	EmbedOnlines.description = Onlines.empty() ? "Server is empty" : Onlines;
	EmbedOnlines.color = string_to_number(DC_DISCORD_BOT, 0, 1410065407);
	pDiscord->sendMessage(message.channelID, "\0", EmbedOnlines);
}

void DiscordCommands::ComStats(class DiscordJob* pDiscord, SleepyDiscord::Message message)
{
	if(message.content.size() <= 8)
	{
		pDiscord->SendWarningMessage(message.channelID, "You must enter at least 1 character to search for accounts.");
		return;
	}

	std::string InputNick = "%" + message.content.substr(8, message.content.length() - 8) + "%";
	bool Found = pDiscord->SendGenerateMessage(message.author, message.channelID, "Discord MRPG Card", InputNick.c_str());
	if(!Found)
		pDiscord->SendWarningMessage(message.channelID, "Accounts containing [" + InputNick + "] among nicknames were not found on the server.");
}

void DiscordCommands::ComRanking(class DiscordJob* pDiscord, SleepyDiscord::Message message)
{
	SleepyDiscord::Embed EmbedRanking;
	EmbedRanking.title = message.startsWith("!mgoldranking") ? "Ranking by Gold" : "Ranking by Level";
	EmbedRanking.color = 422353;

	std::string Names = "";
	std::string Levels = "";
	std::string GoldValues = "";
	ResultPtr pRes = SJK.SD("a.ID, ad.Nick AS `Nick`, ad.Level AS `Level`, g.Count AS `Gold`", "tw_accounts a", "JOIN tw_accounts_data ad ON a.ID = ad.ID LEFT JOIN tw_accounts_items g ON a.ID = g.OwnerID AND g.ItemID = 1 ORDER BY %s LIMIT 10", (message.startsWith("!mgoldranking") ? "g.Count DESC, ad.Level DESC" : "ad.Level DESC, g.Count DESC"));
	while(pRes->next())
	{
		Names += std::to_string((int)pRes->getRow()) + ". **" + pRes->getString("Nick").c_str() + "**\n";
		Levels += std::to_string(pRes->getInt("Level")) + "\n";
		GoldValues += std::to_string(pRes->getInt("Gold")) + "\n";
	}

	EmbedRanking.fields.push_back(SleepyDiscord::EmbedField("Name", Names, true));
	EmbedRanking.fields.push_back(SleepyDiscord::EmbedField("Level", Levels, true));
	EmbedRanking.fields.push_back(SleepyDiscord::EmbedField("Gold", GoldValues, true));
	pDiscord->sendMessage(message.channelID, "\0", EmbedRanking);
}


/************************************************************************/
/*  Fun commands                                                        */
/************************************************************************/
void DiscordCommands::ComAvatar(class DiscordJob* pDiscord, SleepyDiscord::Message message)
{
	if(message.mentions.empty())
	{
		pDiscord->sendMessage(message.channelID, message.author.avatarUrl());
		return;
	}
	else if(message.mentions.size() > 2)
	{
		pDiscord->SendWarningMessage(message.channelID, "Prohibited to use this command with more than 2 people.");
		return;
	}

	for(auto& pMention : message.mentions)
		pDiscord->sendMessage(message.channelID, pMention.avatarUrl());
}


/************************************************************************/
/*  Fun commands                                                        */
/************************************************************************/
void DiscordCommands::ComAdminHelp(class DiscordJob* pDiscord, SleepyDiscord::Message message)
{

}

/************************************************************************/
/*  Engine commands                                                     */
/************************************************************************/
void DiscordCommands::RegisterCommand(const char* pName, const char* pDesc, CommandCallback pCallback, int64 FlagsType, int64 Flags)
{
	DiscordCommands::Command NewCommand;
	str_copy(NewCommand.m_aCommand, pName, sizeof(NewCommand.m_aCommand));
	str_copy(NewCommand.m_aCommandDesc, pDesc, sizeof(NewCommand.m_aCommandDesc));
	NewCommand.m_TypeFlags = FlagsType;
	NewCommand.m_AccessFlags = Flags;
	NewCommand.m_pCallback = pCallback;
	DiscordCommands::m_aCommands.push_back(NewCommand);
}

bool DiscordCommands::ExecuteÑommand(DiscordJob* pDiscord, SleepyDiscord::Message message)
{
	for(auto& pCommand : DiscordCommands::m_aCommands)
	{
		if(message.startsWith(pCommand.m_aCommand))
		{
			SleepyDiscord::Server Server = pDiscord->getServer(message.serverID).cast();
			if(!(pCommand.m_AccessFlags & ACCESS_BASE) && 
				((pCommand.m_AccessFlags & ACCESS_VERIFIED && !message.author.verified)
					|| (pCommand.m_AccessFlags & ACCESS_ADMIN && message.author.ID != Server.ownerID)
					|| (pCommand.m_AccessFlags & ACCESS_NITRO_CLASSIC && message.author.premiumType < SleepyDiscord::User::PremiumType::Nitro_Classic)
					|| (pCommand.m_AccessFlags & ACCESS_NITRO && message.author.premiumType < SleepyDiscord::User::PremiumType::Nitro)
				))
			{
				pDiscord->SendWarningMessage(message.channelID, "You do not have enough rights to execute this command.");
				return true;
			}

			pCommand.m_pCallback(pDiscord, message);
			return true;
		}
	}
	return false;
}

#endif