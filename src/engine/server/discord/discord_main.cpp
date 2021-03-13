/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#ifdef CONF_DISCORD
#include <base/stdafx.h>

#include "discord_main.h"
#include "discord_commands.h"

#include <engine/shared/config.h>
#include <game/server/gamecontext.h>

std::mutex ml_mutex_task;

using namespace sqlstr;
DiscordJob::DiscordJob(IServer* pServer) : SleepyDiscord::DiscordClient(g_Config.m_SvDiscordToken, SleepyDiscord::USER_CONTROLED_THREADS)
{
	m_pServer = pServer;
	setIntents(SleepyDiscord::Intent::SERVER_MESSAGES);

	std::thread(&DiscordJob::run, this).detach(); // start thread discord event bot
	std::thread(&DiscordJob::HandlerThreadTasks, this).detach(); // start handler bridge teeworlds - discord bot

	// initilize commands
	DiscordCommands::InitCommands();
}

/************************************************************************/
/* Discord main events functions                                        */
/************************************************************************/
void DiscordJob::onAddMember(SleepyDiscord::Snowflake<SleepyDiscord::Server> serverID, SleepyDiscord::ServerMember member)
{
	// welcome messages
	std::vector < std::string > ArrayWelcomes;
	ArrayWelcomes.push_back("I can't believe my eyes! Wipe my window " + member.user.showMention());
	ArrayWelcomes.push_back(member.user.showMention() + ", i bet you haven't seen me as long as i had!");
	ArrayWelcomes.push_back("Welcome, " + member.user.showMention() + ". We hope you're not coming to us without pizza!");
	ArrayWelcomes.push_back("The raptor "+ member.user.showMention() + " appeared. Watch out!");
	std::string RulesStr("\n**Don't forget to read the rules <#708092196024352768>!**");

	const int RandomID = random_int() % ArrayWelcomes.size();
	std::string Fullmessage(ArrayWelcomes[RandomID] + RulesStr);

	SleepyDiscord::Embed EmbedWelcome;
	EmbedWelcome.color = 3553599;
	EmbedWelcome.description = Fullmessage;
	sendMessage(g_Config.m_SvDiscordWelcomeChannel, "\0", EmbedWelcome);

	// give member role
	addRole(serverID, member.ID, g_Config.m_SvDiscordMemberRole);
}

void DiscordJob::onMessage(SleepyDiscord::Message message)
{
	// skip empty and messages from the bot itself
	if(message.length() <= 0 || message.author == getCurrentUser().cast())
	 	return;

	// send from the discord chat to server chat
	if(str_comp(std::string(message.channelID).c_str(), g_Config.m_SvDiscordServerChatChannel) == 0)
	{
		std::string Nickname("D|" + message.author.username);
		m_pServer->GameServer(FAKE_DISCORD_WORLD_ID)->FakeChat(Nickname.c_str(), message.content.c_str());
		return;
	}

	// suggestions-voting
	if(str_comp(std::string(message.channelID).c_str(), g_Config.m_SvDiscordSuggestionChannel) == 0)
	{
		deleteMessage(message.channelID, message);

		SleepyDiscord::Embed EmbedIdeas;
		EmbedIdeas.title = std::string("Suggestion");
		EmbedIdeas.description = "From: " + message.author.showMention() + "\n" + message.content;
		EmbedIdeas.color = 431050;
		EmbedIdeas.thumbnail.url = message.author.avatarUrl();
		EmbedIdeas.thumbnail.proxyUrl = message.author.avatarUrl();
		EmbedIdeas.footer.text = "Use reactions for voting!";
		EmbedIdeas.footer.iconUrl = message.author.avatarUrl();
		EmbedIdeas.footer.proxyIconUrl = message.author.avatarUrl();

		SleepyDiscord::Message pMessage = sendMessage(message.channelID, "\0", EmbedIdeas);
		addReaction(message.channelID, pMessage, "%E2%9C%85");
		addReaction(message.channelID, pMessage, "%E2%9D%8C");
		return;
	}

	// command processor
	if(DiscordCommands::ExecuteCommand(this, message))
		return;
}

/************************************************************************/
/* Discord main functions                                               */
/************************************************************************/
void DiscordJob::SendSuccesfulMessage(SleepyDiscord::Snowflake<SleepyDiscord::Channel> channelID, std::string Message)
{
	SleepyDiscord::Embed EmbedSuccessful;
	EmbedSuccessful.description = Message;
	EmbedSuccessful.color = string_to_number(DC_DISCORD_BOT, 0, 1410065407);
	sendMessage(channelID, "\0", EmbedSuccessful);
}

void DiscordJob::SendWarningMessage(SleepyDiscord::Snowflake<SleepyDiscord::Channel> channelID, std::string Message)
{
	SleepyDiscord::Embed EmbedWarning;
	EmbedWarning.description = Message;
	EmbedWarning.color = string_to_number(DC_SERVER_WARNING, 0, 1410065407);
	sendMessage(channelID, "\0", EmbedWarning);
}

bool DiscordJob::SendGenerateMessage(SleepyDiscord::User UserRequestFrom, std::string Channel, std::string Title, std::string SearchNickname, std::string Color, bool MultipleSearch)
{
	CSqlString<64> SearchNick(std::string("%" + SearchNickname + "%").c_str());
	ResultPtr pRes = SJK.SD("*", "tw_accounts_data", "WHERE Nick LIKE '%s' LIMIT %d", SearchNick.cstr(), (MultipleSearch ? 3 : 1));
	while(pRes->next())
	{
		const int AccountID = pRes->getInt("ID");
		const int Rank = Server()->GameServer()->GetRank(AccountID);
		std::string Nickname(pRes->getString("Nick").c_str());
		std::string PhpArguments = "?player=" + Nickname + "&dicid=" + std::to_string(pRes->getInt("DiscordEquip")) + "&rank=" + std::to_string(Rank);
		std::string ImageUrl = std::string(g_Config.m_SvDiscordGenerateURL) + PhpArguments;

		SleepyDiscord::Embed embed;
		embed.title = Title;
		embed.image.height = 800;
		embed.image.width = 600;
		embed.image.url = ImageUrl;
		embed.image.proxyUrl = ImageUrl;
		embed.color = Color[0] != '\0' ? string_to_number(Color.c_str(), 0, 1410065407) : 53380;
		std::string DiscordID(pRes->getString("DiscordID").c_str());
		if(DiscordID.compare("null") != 0)
		{
			SleepyDiscord::User UserAccountOwner = getUser(DiscordID).cast();
			if(!UserAccountOwner.invalid())
			{
				embed.description = "Owns this account: " + UserAccountOwner.showMention() + "\n\nPersonal card MRPG of this user"; // should we ping a person if another person is looking at his card?
				embed.thumbnail.url = UserAccountOwner.avatarUrl();
				embed.thumbnail.proxyUrl = UserAccountOwner.avatarUrl();
				embed.thumbnail.height = 32;
				embed.thumbnail.width = 32;
			}
		}
		if(!UserRequestFrom.invalid())
		{
			embed.footer.text = "Request from " + UserRequestFrom.username;
			embed.footer.iconUrl = UserRequestFrom.avatarUrl();
			embed.footer.proxyIconUrl = UserRequestFrom.avatarUrl();
		}

		this->sendMessage(Channel, "\0", embed);
	}
	const bool Founded = (pRes->rowsCount() > 0);
	return Founded;
}

bool DiscordJob::SendGenerateMessageAccountID(SleepyDiscord::User UserRequestFrom, std::string Chanal, std::string Title, int AccountID, std::string Color)
{
	CGS* pGS = (CGS*)Server()->GameServer(MAIN_WORLD_ID);
	std::string Nickname(pGS->Mmo()->PlayerName(AccountID));
	return SendGenerateMessage(UserRequestFrom, Chanal, Title, Nickname.c_str(), Color, false);
}

/************************************************************************/
/* Discord teeworlds server side                                        */
/************************************************************************/
void DiscordJob::HandlerThreadTasks()
{
	while(true)
	{
		ml_mutex_task.lock();
		if(m_pThreadHandler.empty())
		{
			ml_mutex_task.unlock();
			std::this_thread::sleep_for(std::chrono::seconds(1));
			continue;
		}

		for(DiscordHandle* pHandler : m_pThreadHandler)
		{
			pHandler->m_pEvent();
			delete pHandler;
		}
		m_pThreadHandler.clear();
		ml_mutex_task.unlock();
	}
}

void DiscordJob::AddThreadTask(DiscordTask Task)
{
	ml_mutex_task.lock();
	m_pThreadHandler.push_back(new DiscordHandle(Task));
	ml_mutex_task.unlock();
}

#endif