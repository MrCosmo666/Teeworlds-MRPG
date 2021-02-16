/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#ifdef CONF_DISCORD

#ifndef ENGINE_DISCORD_COMMANDS_SERVER_H
#define ENGINE_DISCORD_COMMANDS_SERVER_H

#include <sleepy_discord/message.h>

enum
{
	TYPE_IMPORTANT = 1 << 0,
	TYPE_GAME = 1 << 1,
	TYPE_FUN = 1 << 2,
};

enum
{
	ACCESS_BASE = 1 << 0,
	ACCESS_VERIFIED = 1 << 1,
	ACCESS_ADMIN = 1 << 2,
	ACCESS_NITRO_CLASSIC = 1 << 3,
	ACCESS_NITRO = 1 << 4,
};

typedef std::function<void(class DiscordJob*, SleepyDiscord::Message)> CommandCallback;
class DiscordCommands
{
	/************************************************************************/
	/*  Important commands                                                  */
	/************************************************************************/
	static void ComHelp(class DiscordJob* pDiscord, SleepyDiscord::Message message);
	static void ComConnect(class DiscordJob* pDiscord, SleepyDiscord::Message message);


	/************************************************************************/
	/*  Game server commands                                                */
	/************************************************************************/
	static void ComOnline(class DiscordJob* pDiscord, SleepyDiscord::Message message);
	static void ComStats(class DiscordJob* pDiscord, SleepyDiscord::Message message);
	static void ComRanking(class DiscordJob* pDiscord, SleepyDiscord::Message message);


	/************************************************************************/
	/*  Fun commands                                                        */
	/************************************************************************/
	static void ComAvatar(class DiscordJob* pDiscord, SleepyDiscord::Message message);


	/************************************************************************/
	/*  Admin commands                                                      */
	/************************************************************************/
	static void ComAdminHelp(class DiscordJob* pDiscord, SleepyDiscord::Message message);


	/************************************************************************/
	/*  Engine commands                                                     */
	/************************************************************************/
public:
	struct Command
	{
		char m_aCommand[32];
		char m_aCommandDesc[256];
		int64 m_AccessFlags;
		int64 m_TypeFlags;

		CommandCallback m_pCallback;
	};

	static void InitCommands();
	static void RegisterCommand(const char* pName, const char* pDesc, CommandCallback pCallback, int64 FlagsType, int64 Flags = ACCESS_BASE);
	static bool Execute—ommand(class DiscordJob* pDiscord, SleepyDiscord::Message message);

private:
	static std::vector < Command > m_aCommands;
};

#endif
#endif