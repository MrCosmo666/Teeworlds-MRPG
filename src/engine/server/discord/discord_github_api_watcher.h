/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#ifdef CONF_DISCORD
#ifndef ENGINE_DISCORD_API_GITHUB_WATCHER_SERVER_H
#define ENGINE_DISCORD_API_GITHUB_WATCHER_SERVER_H

class DiscordGithubAPIRepoWatcher
{
	std::string m_LastCommitSha;
	std::string m_RepoCommitsLink;
	SleepyDiscord::Snowflake<SleepyDiscord::Channel> m_RepoCommitsChannel;

	void PollImplementation(SleepyDiscord::DiscordClient& client);

public:
	DiscordGithubAPIRepoWatcher(std::string Link, SleepyDiscord::Snowflake<SleepyDiscord::Channel> Channel, SleepyDiscord::DiscordClient& client);
	void PollTomarrow(SleepyDiscord::DiscordClient& client);
};


#endif
#endif