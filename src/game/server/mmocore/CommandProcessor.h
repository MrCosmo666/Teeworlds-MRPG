#ifndef GAME_SERVER_MMOCORE_COMMAND_PROCESSOR_H
#define GAME_SERVER_MMOCORE_COMMAND_PROCESSOR_H

class CCommandProcessor
{
	CGS* m_pGS;

	void LastChat(CGS* pGS, CPlayer* pPlayer);

	bool IsLeaderPlayer(CGS* pGS, CPlayer* pPlayer, int Access) const;
	void ExitGuild(CGS* pGS, int AccountID);
	void CreateGuild(CGS* pGS, int ClientID, const char* pName);
	void ChangeStateDoor(CGS* pGS, int HouseID);
	int PlayerHouseID(CGS* pGS, CPlayer* pPlayer) const;

	bool UseSkill(CGS* pGS, CPlayer* pPlayer, int SkillID) const;

public:
	CCommandProcessor(CGS* pGS);

	CGS* GS() { return m_pGS; }

	void ChatCmd(CNetMsg_Cl_Say *pMsg, CPlayer *pPlayer);
	void AddCommand(const char* pName, const char* pParams, IConsole::FCommandCallback pfnFunc, void* pUser, const char* pHelp);

	static void ConChatLogin(IConsole::IResult* pResult, void* pUserData);
};

#endif
