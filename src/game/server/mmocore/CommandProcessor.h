#ifndef GAME_SERVER_MMOCORE_COMMAND_PROCESSOR_H
#define GAME_SERVER_MMOCORE_COMMAND_PROCESSOR_H

class CCommandProcessor
{
	CGS* m_pGS;

	void LastChat(CGS* pGS, CPlayer* pPlayer);

public:
	CCommandProcessor(CGS* pGS);

	CGS* GS() { return m_pGS; }

	void ChatCmd(CNetMsg_Cl_Say *pMsg, CPlayer *pPlayer);
	void AddCommand(const char* pName, const char* pParams, IConsole::FCommandCallback pfnFunc, void* pUser, const char* pHelp);

	static void ConChatLogin(IConsole::IResult* pResult, void* pUserData);
	static void ConChatRegister(IConsole::IResult* pResult, void* pUserData);
	static void ConChatGuildExit(IConsole::IResult* pResult, void* pUserData);
	static void ConChatGuildCreate(IConsole::IResult* pResult, void* pUserData);
	static void ConChatDoorHouse(IConsole::IResult* pResult, void* pUserData);
	static void ConChatSellHouse(IConsole::IResult* pResult, void* pUserData);
	static void ConChatPosition(IConsole::IResult* pResult, void* pUserData);
	static void ConChatSound(IConsole::IResult* pResult, void* pUserData);
	static void ConChatUseItem(IConsole::IResult* pResult, void* pUserData);
	static void ConChatUseSkill(IConsole::IResult* pResult, void* pUserData);
	static void ConChatCmdList(IConsole::IResult* pResult, void* pUserData);
	static void ConChatRules(IConsole::IResult* pResult, void* pUserData);

#ifdef CONF_DISCORD
	static void ConChatDiscordConnect(IConsole::IResult* pResult, void* pUserData);
#endif
};

#endif
