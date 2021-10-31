/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#ifdef CONF_DISCORD
#include "discord_main.h"

#include "discord_github_api_watcher.h"

DiscordGithubAPIRepoWatcher::DiscordGithubAPIRepoWatcher(std::string Link, SleepyDiscord::Snowflake<SleepyDiscord::Channel> Channel, SleepyDiscord::DiscordClient& client)
{
	m_RepoCommitsLink = Link;
	m_RepoCommitsChannel = Channel;
	asio::post([=, &client]()
	{
		// TODO: this code is used twice, make it a function
		auto response = cpr::Get(cpr::Url{ m_RepoCommitsLink });
		if(response.status_code != 200)
			return;

		rapidjson::Document document;
		document.Parse(response.text.c_str(), response.text.length());
		if(document.HasParseError())
			return;

		auto commits = document.GetArray();
		auto& lastCommit = commits[0];
		auto sha = lastCommit.FindMember("sha");
		if(sha == lastCommit.MemberEnd() || !sha->value.IsString())
			return;

		m_LastCommitSha = std::string{ sha->value.GetString(), sha->value.GetStringLength() };
		PollTomarrow(client);
	});
}

void DiscordGithubAPIRepoWatcher::PollTomarrow(SleepyDiscord::DiscordClient& client)
{
	client.schedule([=, &client]()
	{
		PollImplementation(client);
		PollTomarrow(client);
	}, 7200000);
}

void DiscordGithubAPIRepoWatcher::PollImplementation(SleepyDiscord::DiscordClient& client)
{
	asio::post([=, &client]()
	{
		SleepyDiscord::Snowflake<SleepyDiscord::Channel> channel = m_RepoCommitsChannel;

		auto response = cpr::Get(cpr::Url{ m_RepoCommitsLink });
		if(response.status_code != 200)
			return;

		rapidjson::Document document;
		document.Parse(response.text.c_str(), response.text.length());
		if(document.HasParseError())
			return;

		auto commits = document.GetArray();
		auto lastCommitIterator = commits.end();
		int index = 0;
		for(auto& commit : commits)
		{
			auto sha = commit.FindMember("sha");
			if(sha == commit.MemberEnd() || !sha->value.IsString())
			{
				index += 1;
				continue;
			}

			// TODO: use string_view
			std::string shaStr{ sha->value.GetString(), sha->value.GetStringLength() };
			if(m_LastCommitSha == shaStr)
			{
				lastCommitIterator = commits.begin() + index;
				break;
			}
			index += 1;
		}

		if(lastCommitIterator == commits.begin()) //no new commits
			return;

		SleepyDiscord::Embed embed;
		embed.color = 4235236774;

		// since the commits are sorted newest first, we need to go backwards to make it
		// fit Discord's message order being oldest first/top.
		for(auto commit = lastCommitIterator - 1; commit != commits.begin() - 1; commit -= 1)
		{
			auto sha = commit->FindMember("sha");
			if(sha == commit->MemberEnd() || !sha->value.IsString())
				continue;

			if(commit == commits.begin())
				m_LastCommitSha = std::string{ sha->value.GetString(), sha->value.GetStringLength() };

			auto data = commit->FindMember("commit");
			if(data == commit->MemberEnd() || !data->value.IsObject())
				continue;

			auto messageMember = data->value.FindMember("message");
			if(messageMember == data->value.MemberEnd() || !messageMember->value.IsString())
				continue;

			auto htmlLinkMember = commit->FindMember("html_url");
			if(htmlLinkMember == commit->MemberEnd() || !htmlLinkMember->value.IsString())
				continue;

			std::string hashDisplay{ sha->value.GetString(), sha->value.GetStringLength() <= 9 ? sha->value.GetStringLength() : 9 };
			std::string message{ messageMember->value.GetString(), messageMember->value.GetStringLength() };

			std::string commitTitle;
			size_t newLinePOS = message.find_first_of('\n');
			if(newLinePOS == std::string::npos)
				commitTitle = message;
			else
				commitTitle = message.substr(0, newLinePOS);

			std::string commitLink;
			size_t linkSize = 1 + hashDisplay.length() + 2 + htmlLinkMember->value.GetStringLength() + 1;
			commitLink += '[';
			commitLink += hashDisplay;
			commitLink += "](";
			commitLink += htmlLinkMember->value.GetString();
			commitLink += ')';
			embed.fields.push_back(SleepyDiscord::EmbedField{ commitTitle, commitLink });

			// check if we are over the embed limits
			// TODO: list the embed limits in the library
			if(25 <= embed.fields.size())
			{
				client.sendMessage(channel, "", embed, {}, SleepyDiscord::TTS::Default, SleepyDiscord::Async);
				embed = SleepyDiscord::Embed{};
			}
		}

		client.sendMessage(channel, "", embed, {}, SleepyDiscord::TTS::Default, SleepyDiscord::Async);
	});
}

#endif