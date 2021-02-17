/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#ifdef CONF_DISCORD

#ifndef ENGINE_DISCORD_COMMANDS_SERVER_H
#define ENGINE_DISCORD_COMMANDS_SERVER_H

#include <sleepy_discord/message.h>

enum
{
	CMD_INVISIBLE = 0,
	CMD_IMPORTANT = 1 << 0,
	CMD_GAME = 1 << 1,
	CMD_FUN = 1 << 2,

	ACCESS_EVERYONE = 0,
	ACCESS_VERIFIED = 1 << 0,
	ACCESS_ADMIN = 1 << 1,
	ACCESS_OWNER = 1 << 2
};

typedef std::function<void(void*, class DiscordJob*, SleepyDiscord::Message)> CommandCallback;
class DiscordCommands
{
	/************************************************************************/
	/*  Important commands                                                  */
	/************************************************************************/
	static void ComHelp(void* pResult, class DiscordJob* pDiscord, SleepyDiscord::Message message);
	static void ComConnect(void* pResult, class DiscordJob* pDiscord, SleepyDiscord::Message message);


	/************************************************************************/
	/*  Game server commands                                                */
	/************************************************************************/
	static void ComOnline(void* pResult, class DiscordJob* pDiscord, SleepyDiscord::Message message);
	static void ComStats(void* pResult, class DiscordJob* pDiscord, SleepyDiscord::Message message);
	static void ComRanking(void* pResult, class DiscordJob* pDiscord, SleepyDiscord::Message message);


	/************************************************************************/
	/*  Fun commands                                                        */
	/************************************************************************/
	static void ComAvatar(void* pResult, class DiscordJob* pDiscord, SleepyDiscord::Message message);


	/************************************************************************/
	/*  Admin commands                                                      */
	/************************************************************************/


	/************************************************************************/
	/*  Engine commands                                                     */
	/************************************************************************/
public:
	struct Command
	{
		char m_aCommand[32];
		char m_aCommandDesc[256];
		char m_aCommandArgs[128];
		int64 m_AccessFlags;
		int64 m_TypeFlags;

		CommandCallback m_pCallback;
	};

	static void InitCommands();
	static void RegisterCommand(const char* pName, const char* pArgs, const char* pDesc, CommandCallback pCallback, int64 FlagsType, int64 Flags = ACCESS_EVERYONE);
	static bool ExecuteCommand(class DiscordJob* pDiscord, SleepyDiscord::Message message);

private:
	static std::vector < Command > m_aCommands;
};

#endif
#endif