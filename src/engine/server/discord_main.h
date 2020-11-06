/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#ifndef ENGINE_DISCORD_MAIN_SERVER_H
#define ENGINE_DISCORD_MAIN_SERVER_H

#ifdef CONF_DISCORD
#include <sleepy_discord/websocketpp_websocket.h>

class DiscordJob : public SleepyDiscord::DiscordClient
{
	class IServer* m_pServer;
	IServer*Server() const { return m_pServer; }

	class CGS *m_GameServer;
	CGS *GS() const { return m_GameServer; }

	// роли
	std::vector<SleepyDiscord::Role> RoleList;

	void onMessage(SleepyDiscord::Message message) override;
	void onReaction(SleepyDiscord::Snowflake<SleepyDiscord::User> userID, SleepyDiscord::Snowflake<SleepyDiscord::Channel> channelID, 
						SleepyDiscord::Snowflake<SleepyDiscord::Message> messageID, SleepyDiscord::Emoji emoji) override;
	void onDeleteReaction(SleepyDiscord::Snowflake<SleepyDiscord::User> userID, SleepyDiscord::Snowflake<SleepyDiscord::Channel> channelID, 
						SleepyDiscord::Snowflake<SleepyDiscord::Message> messageID, SleepyDiscord::Emoji emoji) override;
	void UpdateMessageIdeas(SleepyDiscord::Snowflake<SleepyDiscord::User> userID, SleepyDiscord::Snowflake<SleepyDiscord::Channel> channelID, 
						SleepyDiscord::Snowflake<SleepyDiscord::Message> messageID);


public:
	using SleepyDiscord::DiscordClient::DiscordClient;
	DiscordJob(const char *token, int threads);

	void SetServer(IServer *pServer);

	void SendMessage(const char *pChanal, const char *Color, const char *Title, std::string pMsg);
	void SendGenerateMessage(const char *pChanal, const char *Color, const char *Title, const char *pPhpArg);
	void SendStatus(const char* Status, int Type);
};

#endif

#endif