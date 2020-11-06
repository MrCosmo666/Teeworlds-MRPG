/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#include <base/math.h>
#include <base/system.h>
#include <engine/shared/config.h>
#include <engine/server/sql_connect_pool.h>
#include <engine/server/sql_string_helpers.h>
#include <teeother/components/localization.h>
#include <game/server/enum_context.h>
#include <game/server/enum_global.h>

#include <engine/server.h>
#include "discord_main.h"

#ifdef CONF_DISCORD
DiscordJob::DiscordJob(const char *token, int threads) : SleepyDiscord::DiscordClient(token, SleepyDiscord::USER_CONTROLED_THREADS)
{
	std::thread t1(&DiscordJob::run, this);
	t1.detach();
}

void DiscordJob::SetServer(IServer *pServer)
{
	m_pServer = pServer;
}

void DiscordJob::onMessage(SleepyDiscord::Message message) 
{
	if(message.length() <= 0 || !g_Config.m_SvCreateDiscordBot || message.author == getCurrentUser().cast())
	 	return;

	// statistics
	if (message.startsWith("!mstats"))
	{
		// if the number of characters is less than the required
		std::string messagecont = message.content;
		if(messagecont.size() <= 8)
		{
			SendMessage(std::string(message.channelID).c_str(), DC_SERVER_WARNING, "Not right!",
			std::string("Use **!mstats <nick full or not>. Minimal symbols 1.**!!!"));
			return;
		}

		// search data
		bool Found = false;
		std::string input = "%" + messagecont.substr(8, messagecont.length() - 8) + "%";
		sqlstr::CSqlString<64> cDiscordIDorNick = sqlstr::CSqlString<64>(input.c_str());

		// user lookup
		ResultPtr pRes = SJK.SD("*", "tw_accounts_data", "WHERE Nick LIKE '%s'LIMIT 5", cDiscordIDorNick.cstr());
		while(pRes->next())
		{
			const int AuthID = pRes->getInt("ID");
			const int RandomColor = 1000+random_int()%10000000;
			const int Rank = Server()->GameServer()->GetRank(AuthID);

			char aBuf[256];
			str_format(aBuf, sizeof(aBuf), "?player=%s&rank=%d&dicid=%d", pRes->getString("Nick").c_str(), Rank, pRes->getInt("DiscordEquip"));
			SendGenerateMessage(std::string(message.channelID).c_str(), std::to_string(RandomColor).c_str(), "Discord MRPG Card", aBuf);
			Found = true;
		}

		if(!Found)
		{
			SendMessage(std::string(message.channelID).c_str(), DC_SERVER_WARNING, "Sorry!",
				"**This account not found in database!**");
		}
		return;
	}

	else if (message.startsWith("!mconnect"))
	{	
		// get the user's ID
		SleepyDiscord::Snowflake<SleepyDiscord::User> userAuth = getUser(message.author).cast();
		SendMessage(std::string(message.channelID).c_str(), DC_DISCORD_INFO, "Connector Information",  
			"```ini\n[Warning] Do not connect other people's discords to your account in the game\nThis is similar to hacking your account in the game\n# Use in-game for connect your personal discord:\n# Did '" + userAuth + "'\n# Command in-game: /discord_connect <did>```");

		// check for user search
		std::string Nick = "Refresh please.";
		std::string UserID = userAuth;

		// search criteria
		bool Found = false;
		sqlstr::CSqlString<64> cDiscordID = sqlstr::CSqlString<64>(UserID.c_str());

		// get connected
		ResultPtr pRes = SJK.SD("Nick", "tw_accounts_data", "WHERE DiscordID = '%s'", cDiscordID.cstr());
		while(pRes->next())
		{
			// send a connected message
			Nick = pRes->getString("Nick").c_str();
			SendMessage(std::string(message.channelID).c_str(), DC_DISCORD_BOT, "Good work :)", "**Your account is enabled: Nickname in-game: " + Nick + "**");
			Found = true;
		}

		// end connection
		if(!Found)
			SendMessage(std::string(message.channelID).c_str(), DC_SERVER_WARNING, "Fail in work :(", "**Fail connect. See !mconnect.\nUse in-game /discord_connect <DID>..**");
		return;
	}

	// list all players
	else if (message.startsWith("!monline"))
	{
		dynamic_string Buffer;
		for(int i = 0; i < MAX_PLAYERS; i++) 
		{
			if(!Server()->ClientIngame(i)) 
				continue;

			Buffer.append_at(Buffer.length(), Server()->ClientName(i));
			Buffer.append_at(Buffer.length(), "\n");
		}
		if(Buffer.length() <= 0)
		{
			SendMessage(std::string(message.channelID).c_str(), DC_DISCORD_BOT, "Online Server", "Server is empty");
			return;
		}
		SendMessage(std::string(message.channelID).c_str(), DC_DISCORD_BOT, "Online Server", Buffer.buffer());
		Buffer.clear();
		return;
	}
	// help
	else if (message.startsWith("!mhelp"))
	{
		SendMessage(std::string(message.channelID).c_str(), DC_DISCORD_INFO, "Commands / Information", 
		"`!mconnect` - Info for connect your discord and account in game."
		"\n`!mstats <symbol>` - See stats players. Minimal 1 symbols."
		"\n`!monline` - Show players ingame.");
	}
	// send from the discord chat to server chat
	else if(str_comp(std::string(message.channelID).c_str(), g_Config.m_SvDiscordChanal) == 0)
	{
		std::string Nickname("D|" + message.author.username);
		m_pServer->GameServer(FAKE_DISCORD_WORLD_ID)->FakeChat(Nickname.c_str(), message.content.c_str());
	}
	// ideas-voting
	else if(str_comp(std::string(message.channelID).c_str(), g_Config.m_SvDiscordIdeasChanal) == 0)
	{
		deleteMessage(message.channelID, message);

		SleepyDiscord::Embed embed;
		embed.title = std::string("Suggestion");
		embed.color = 431050;

		SleepyDiscord::EmbedThumbnail embedthumb;
		embedthumb.url = message.author.avatarUrl();
		embedthumb.proxyUrl = message.author.avatarUrl();
		embed.thumbnail = embedthumb;

		SleepyDiscord::EmbedFooter embedfooter;
		embedfooter.text = "Use reactions for voting!";
		embedfooter.iconUrl = message.author.avatarUrl();
		embedfooter.proxyIconUrl = message.author.avatarUrl();
		embed.footer = embedfooter;
		embed.description = "From:" + message.author.showUser() + "!\n" + message.content;

		SleepyDiscord::Message pMessage = sendMessage(message.channelID, "\0", embed);
		addReaction(message.channelID, pMessage, "%E2%9C%85");
		addReaction(message.channelID, pMessage, "%E2%9D%8C");
	}

	// ranking
	else if (message.startsWith("!mranking") || message.startsWith("!mgoldranking"))
	{
		SleepyDiscord::Embed embed;
		embed.title = message.startsWith("!mgoldranking") ? "Ranking by Gold" : "Ranking by Level";
		embed.color = 422353;

		int Rank = 1;
		std::string Names = "";
		std::string Levels = "";
		std::string GoldValues = "";
		ResultPtr pRes = SJK.SD("a.ID, ad.Nick AS `Nick`, ad.Level AS `Level`, g.Count AS `Gold`", "tw_accounts a", "JOIN tw_accounts_data ad ON a.ID = ad.ID LEFT JOIN tw_accounts_items g ON a.ID = g.OwnerID AND g.ItemID = 1 ORDER BY %s LIMIT 10", (message.startsWith("!mgoldranking") ? "g.Count DESC, ad.Level DESC" : "ad.Level DESC, g.Count DESC"));
		while (pRes->next())
		{
			Names += std::to_string(Rank) + ". **" + pRes->getString("Nick").c_str() + "**\n";
			Rank++;

			Levels += std::to_string(pRes->getInt("Level")) + "\n";
			GoldValues += std::to_string(pRes->getInt("Gold")) + "\n";
		}

		embed.fields.push_back(SleepyDiscord::EmbedField("Name", Names, true));
		embed.fields.push_back(SleepyDiscord::EmbedField("Level", Levels, true));
		embed.fields.push_back(SleepyDiscord::EmbedField("Gold", GoldValues, true));

		SleepyDiscord::Message pMessage = sendMessage(message.channelID, "\0", embed);
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

void DiscordJob::SendStatus(const char *Status, int Type)
{
	if(!g_Config.m_SvCreateDiscordBot) return;
	this->updateStatus(Status, Type);
}

void DiscordJob::SendGenerateMessage(const char *pChanal, const char *Color, const char *Title, const char *pPhpArg)
{
	if(!g_Config.m_SvCreateDiscordBot) return;

	SleepyDiscord::Embed embed;
	embed.title = std::string(Title);
	embed.color = string_to_number(Color, 0, 1410065407);

	char aBuf[128];
	str_format(aBuf, sizeof(aBuf), "%s/%s/gentee.php%s", g_Config.m_SvSiteUrl, g_Config.m_SvGenerateURL, pPhpArg);

	SleepyDiscord::EmbedImage embedimage;
	embedimage.height = 800;
	embedimage.width = 600;
	embedimage.url = std::string(aBuf);
	embedimage.proxyUrl = std::string(aBuf);
	embed.image = embedimage;
	this->sendMessage(pChanal, "\0", embed);
}

void DiscordJob::SendMessage(const char *pChanal, const char *Color, const char *Title, std::string pMsg)
{
	if(!g_Config.m_SvCreateDiscordBot) return;

	SleepyDiscord::Embed embed;
	embed.title = std::string(Title);
	embed.color = string_to_number(Color, 0, 1410065407);
	embed.description = pMsg;
	this->sendMessage(pChanal, "\0", embed);
}
#endif