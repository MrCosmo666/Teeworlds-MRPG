/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#include <base/math.h>
#include <base/system.h>
#include <engine/shared/config.h>
#include <game/server/gamecontext.h>

#include "discord_main.h"

#ifdef CONF_DISCORD

using namespace sqlstr;
DiscordJob::DiscordJob(const char *token, int threads) : SleepyDiscord::DiscordClient(token, SleepyDiscord::USER_CONTROLED_THREADS)
{
	m_pServer = nullptr;
	std::thread(&DiscordJob::run, this).detach();
}

void DiscordJob::SetServer(IServer *pServer)
{
	m_pServer = pServer;
}

void DiscordJob::onMessage(SleepyDiscord::Message message) 
{
	if(message.length() <= 0 || message.author == getCurrentUser().cast())
	 	return;

	// send from the discord chat to server chat
	if(str_comp(std::string(message.channelID).c_str(), g_Config.m_SvDiscordChanal) == 0)
	{
		std::string Nickname("D|" + message.author.username);
		m_pServer->GameServer(FAKE_DISCORD_WORLD_ID)->FakeChat(Nickname.c_str(), message.content.c_str());
	}

	// help
	else if(message.startsWith("!mhelp"))
	{
		SleepyDiscord::Embed EmbedHelp;
		EmbedHelp.title = "Commands / Information";
		EmbedHelp.description = "**!mconnect** - Info for connect your discord to account in game."
			"\n**!monline** - Show players online."
			"\n**!mranking** - Show ranking by level."
			"\n**!mgoldranking** - Show ranking by gold."
			"\n**!mstats <symbols>** - Search for player accounts cards. Minimum 1 symbol.";
		EmbedHelp.color = string_to_number(DC_DISCORD_INFO, 0, 1410065407);
		sendMessage(message.channelID, "\0", EmbedHelp);
	}

	// statistics
	else if (message.startsWith("!mstats"))
	{
		if(message.content.size() <= 8)
		{
			SleepyDiscord::Embed EmbedWarning;
			EmbedWarning.title = "Not right!";
			EmbedWarning.description = "Use: **!mstats <symbols>**. Minimum 1 symbol!";
			EmbedWarning.color = string_to_number(DC_SERVER_WARNING, 0, 1410065407);
			sendMessage(message.channelID, "\0", EmbedWarning);
			return;
		}

		std::string InputNick = "%" + message.content.substr(8, message.content.length() - 8) + "%";
		bool Found = SendGenerateMessage(message.author, std::string(message.channelID).c_str(), "Discord MRPG Card", InputNick.c_str());
		if(!Found)
		{
			SleepyDiscord::Embed EmbedWarning;
			EmbedWarning.title = "Sorry!";
			EmbedWarning.description = "**This account not found in database!**";
			EmbedWarning.color = string_to_number(DC_SERVER_WARNING, 0, 1410065407);
			sendMessage(message.channelID, "\0", EmbedWarning);
		}
	}

	// ranking
	else if(message.startsWith("!mranking") || message.startsWith("!mgoldranking"))
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
		sendMessage(message.channelID, "\0", EmbedRanking);
	}

	// connect discordid to database
	else if (message.startsWith("!mconnect"))
	{	
		SleepyDiscord::Snowflake<SleepyDiscord::User> userAuth = getUser(message.author).cast();

		SleepyDiscord::Embed EmbedConnectInfo;
		EmbedConnectInfo.title = "Connector Information";
		EmbedConnectInfo.description = "```ini\n[Warning] Do not connect other people's discords to your account in the game"
			"\nThis is similar to hacking your account in the game"
			"\n# Use in-game for connect your personal discord:"
			"\n# Did '" + userAuth +
			"'\n# Command in-game: /discord_connect <did>```";
		EmbedConnectInfo.color = string_to_number(DC_DISCORD_INFO, 0, 1410065407);
		sendMessage(message.channelID, "\0", EmbedConnectInfo);

		SleepyDiscord::Embed EmbedConnectResult;
		std::string Nick = "Refresh please.";
		std::string UserID = userAuth;
		sqlstr::CSqlString<64> DiscordID = sqlstr::CSqlString<64>(UserID.c_str());
		ResultPtr pRes = SJK.SD("Nick", "tw_accounts_data", "WHERE DiscordID = '%s'", DiscordID.cstr());
		if(!pRes->rowsCount())
		{
			EmbedConnectResult.title = "Fail in work :(";
			EmbedConnectResult.description = "**Fail connect. See !mconnect.\nUse in-game /discord_connect <DID>..**";
			EmbedConnectResult.color = string_to_number(DC_SERVER_WARNING, 0, 1410065407);
			sendMessage(message.channelID, "\0", EmbedConnectResult);
		}
		while(pRes->next())
		{
			Nick = pRes->getString("Nick").c_str();

			EmbedConnectResult.title = "Good work :)";
			EmbedConnectResult.description = "**Your account is enabled: Nickname in-game: " + Nick + "**";
			EmbedConnectResult.color = string_to_number(DC_DISCORD_BOT, 0, 1410065407);
			sendMessage(message.channelID, "\0", EmbedConnectResult);
		}
	}

	// list all players
	else if (message.startsWith("!monline"))
	{
		std::string Onlines = "";
		CGS* pGS = (CGS*)Server()->GameServer(MAIN_WORLD_ID);
		for(int i = 0; i < MAX_PLAYERS; i++) 
		{
			// THREAD_PLAYER_DATA_SAFE(i) // this thread working on detach we can got (Big ass)
			CPlayer* pPlayer = pGS->GetPlayer(i);
			if(pPlayer)
			{
				std::string Nickname(Server()->ClientName(i));
				std::string Team(pPlayer->GetTeam() != TEAM_SPECTATORS ? "Ingame" : "Spectator");
				Onlines += "**" + Nickname + "** - " + Team + "\n";
			}
		}
		if(Onlines.empty())
			Onlines = "Server is empty";

		SleepyDiscord::Embed EmbedOnlines;
		EmbedOnlines.title = "Online Server";
		EmbedOnlines.description = Onlines;
		EmbedOnlines.color = string_to_number(DC_DISCORD_BOT, 0, 1410065407);
		sendMessage(message.channelID, "\0", EmbedOnlines);
	}

	// ideas-voting
	if(str_comp(std::string(message.channelID).c_str(), g_Config.m_SvDiscordIdeasChanal) == 0)
	{
		deleteMessage(message.channelID, message);

		SleepyDiscord::Embed EmbedIdeas;
		EmbedIdeas.title = std::string("Suggestion");
		EmbedIdeas.description = "From: " + message.author.showUser() + "\n" + message.content;
		EmbedIdeas.color = 431050;
		EmbedIdeas.thumbnail.url = message.author.avatarUrl();
		EmbedIdeas.thumbnail.proxyUrl = message.author.avatarUrl();
		EmbedIdeas.footer.text = "Use reactions for voting!";
		EmbedIdeas.footer.iconUrl = message.author.avatarUrl();
		EmbedIdeas.footer.proxyIconUrl = message.author.avatarUrl();

		SleepyDiscord::Message pMessage = sendMessage(message.channelID, "\0", EmbedIdeas);
		addReaction(message.channelID, pMessage, "%E2%9C%85");
		addReaction(message.channelID, pMessage, "%E2%9D%8C");
	}
}

void DiscordJob::onReaction(SleepyDiscord::Snowflake<SleepyDiscord::User> userID, SleepyDiscord::Snowflake<SleepyDiscord::Channel> channelID, 
SleepyDiscord::Snowflake<SleepyDiscord::Message> messageID, SleepyDiscord::Emoji emoji)
{ 
}

void DiscordJob::onDeleteReaction(SleepyDiscord::Snowflake<SleepyDiscord::User> userID, SleepyDiscord::Snowflake<SleepyDiscord::Channel> channelID, 
SleepyDiscord::Snowflake<SleepyDiscord::Message> messageID, SleepyDiscord::Emoji emoji)
{
}

void DiscordJob::UpdateMessageIdeas(SleepyDiscord::Snowflake<SleepyDiscord::User> userID, SleepyDiscord::Snowflake<SleepyDiscord::Channel> channelID, 
SleepyDiscord::Snowflake<SleepyDiscord::Message> messageID)
{
	if(userID == getCurrentUser().cast())
		return;
}

void DiscordJob::UpdateStatus(const char* Status)
{
	this->updateStatus(Status);
}

bool DiscordJob::SendGenerateMessage(SleepyDiscord::User UserRequestFrom, const char* pChannel, const char *pTitle, const char* pSearchNickname, const char* pColor, bool MultipleSearch)
{
	CSqlString<64> SearchNick(std::string("%" + std::string(pSearchNickname) + "%").c_str());
	ResultPtr pRes = SJK.SD("*", "tw_accounts_data", "WHERE Nick LIKE '%s' LIMIT %d", SearchNick.cstr(), (MultipleSearch ? 3 : 1));
	while(pRes->next())
	{
		const int AuthID = pRes->getInt("ID");
		const int Rank = Server()->GameServer()->GetRank(AuthID);
		std::string Nickname(pRes->getString("Nick").c_str());
		std::string PhpArguments = "?player=" + Nickname + "&dicid=" + std::to_string(pRes->getInt("DiscordEquip")) + "&rank=" + std::to_string(Rank);
		std::string ImageUrl = std::string(g_Config.m_SvGenerateURL) + PhpArguments;

		SleepyDiscord::Embed embed;
		embed.title = pTitle;
		embed.image.height = 800;
		embed.image.width = 600;
		embed.image.url = ImageUrl;
		embed.image.proxyUrl = ImageUrl;
		embed.color = pColor[0] != '\0' ? string_to_number(pColor, 0, 1410065407) : 1703301;
		std::string DiscordID(pRes->getString("DiscordID").c_str());
		if(DiscordID.compare("null") != 0)
		{
			SleepyDiscord::User userAuth = getUser(DiscordID).cast();
			embed.description = "Owns this account: " + userAuth.showUser() + "\n\nPersonal card MRPG of this user"; // should we ping a person if another person is looking at his card?
			embed.thumbnail.url = userAuth.avatarUrl();
			embed.thumbnail.proxyUrl = userAuth.avatarUrl();
			embed.thumbnail.height = 32;
			embed.thumbnail.width = 32;
		}
		if(!UserRequestFrom.empty())
		{
			embed.footer.text = "Request from " + UserRequestFrom.username;
			embed.footer.iconUrl = UserRequestFrom.avatarUrl();
			embed.footer.proxyIconUrl = UserRequestFrom.avatarUrl();
		}

		this->sendMessage(std::string(pChannel), "\0", embed);
	}
	const bool Founded = (pRes->rowsCount() > 0);
	return Founded;
}

bool DiscordJob::SendGenerateMessage(SleepyDiscord::User UserRequestFrom, const char* pChanal, const char* pTitle, const int AuthID, const char* pColor)
{
	CGS* pGS = (CGS*)Server()->GameServer(MAIN_WORLD_ID);
	std::string Nickname(pGS->Mmo()->PlayerName(AuthID));
	return SendGenerateMessage(UserRequestFrom, pChanal, pTitle, Nickname.c_str(), pColor, false);
}

void DiscordJob::SendEmbedMessage(const char *pChanal, const char *Color, const char *Title, std::string pMsg)
{
	SleepyDiscord::Embed embed;
	embed.title = std::string(Title);
	embed.color = string_to_number(Color, 0, 1410065407);
	embed.description = pMsg;
	this->sendMessage(pChanal, "\0", embed);
}
#endif