/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#ifdef CONF_DISCORD

#ifndef ENGINE_DISCORD_MAIN_SERVER_H
#define ENGINE_DISCORD_MAIN_SERVER_H

#include <sleepy_discord/websocketpp_websocket.h>

typedef std::function<void()> DiscordTask;
class DiscordJob : public SleepyDiscord::DiscordClient
{
public:
	using SleepyDiscord::DiscordClient::DiscordClient;
	DiscordJob(class IServer* pServer);

private:
	/************************************************************************/
	/* Discord main functions                                               */
	/************************************************************************/
	void onAddMember(SleepyDiscord::Snowflake<SleepyDiscord::Server> serverID, SleepyDiscord::ServerMember member) override;
	void onRemoveMember(SleepyDiscord::Snowflake<SleepyDiscord::Server> serverID, SleepyDiscord::User user) override;
	void onMessage(SleepyDiscord::Message message) override;
	bool SendGenerateMessage(SleepyDiscord::User UserRequestFrom, std::string Chanal, std::string Title, std::string SearchNickname, std::string Color = "\0", bool MultipleSearch = true);
	bool SendGenerateMessageAuthID(SleepyDiscord::User UserRequestFrom, std::string Chanal, std::string Title, int AuthID, std::string Color = "\0");

	/************************************************************************/
	/* Discord teeworlds server side                                        */
	/************************************************************************/
	// Allow access only from the CServer 
	friend class CServer;
	struct DiscordHandle
	{
		DiscordTask m_pEvent;
		DiscordHandle(DiscordTask Event) : m_pEvent(Event) {};
	};
	std::list<DiscordHandle*> m_pThreadHandler;
	
	class IServer* m_pServer;
	IServer* Server() const { return m_pServer; }

	void HandlerThreadTasks();
	void AddThreadTask(DiscordTask Task);
};

#endif
#endif