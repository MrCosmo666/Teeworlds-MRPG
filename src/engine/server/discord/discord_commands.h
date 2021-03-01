/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#ifdef CONF_DISCORD
#ifndef ENGINE_DISCORD_COMMANDS_SERVER_H
#define ENGINE_DISCORD_COMMANDS_SERVER_H

#include <engine/console.h>

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

class DiscordCommands
{
	typedef void (*CommandCallback)(class IConsole::IResult* pResult, class DiscordJob*, SleepyDiscord::Message);

	/************************************************************************/
	/*  Important commands                                                  */
	/************************************************************************/
	static void ComHelp(class IConsole::IResult* pResult, class DiscordJob* pDiscord, SleepyDiscord::Message message);
	static void ComConnect(class IConsole::IResult* pResult, class DiscordJob* pDiscord, SleepyDiscord::Message message);


	/************************************************************************/
	/*  Game server commands                                                */
	/************************************************************************/
	static void ComOnline(class IConsole::IResult* pResult, class DiscordJob* pDiscord, SleepyDiscord::Message message);
	static void ComStats(class IConsole::IResult* pResult, class DiscordJob* pDiscord, SleepyDiscord::Message message);
	static void ComRanking(class IConsole::IResult* pResult, class DiscordJob* pDiscord, SleepyDiscord::Message message);


	/************************************************************************/
	/*  Fun commands                                                        */
	/************************************************************************/
	static void ComAvatar(class IConsole::IResult* pResult, class DiscordJob* pDiscord, SleepyDiscord::Message message);


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
		int m_AccessFlags;
		int m_TypeFlags;
		CommandCallback m_pCallback;
	};

	static void InitCommands();
	static void RegisterCommand(const char* pName, const char* pArgs, const char* pDesc, CommandCallback pCallback, int FlagsType, int Flags = ACCESS_EVERYONE);
	static bool ExecuteCommand(class DiscordJob* pDiscord, SleepyDiscord::Message message);

private:
	static std::vector < Command > m_aCommands;
};

#endif
#endif