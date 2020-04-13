#ifndef GAME_SERVER_CMD_H
#define GAME_SERVER_CMD_H

class CCmd
{
public:
	void ChatCmd(CNetMsg_Cl_Say *Msg, CGS *GS, CPlayer *pPlayer);
private:
	void LastChat(CGS *GS, CPlayer *pPlayer); 

	bool IsLeaderPlayer(CGS *GS, CPlayer *pPlayer, int Access) const;
	void ExitGuild(CGS *GS, int AccountID);
	void CreateGuild(CGS *GS, int ClientID, const char *pName);
	void ChangeStateDoor(CGS *GS, int HouseID);
	int PlayerHouseID(CGS *GS, CPlayer *pPlayer) const;

	void UseItems(CGS *GS, int ClientID, int ItemID, int Count);
	bool UseSkill(CGS *GS, CPlayer *pPlayer, int SkillID) const;
};

#endif