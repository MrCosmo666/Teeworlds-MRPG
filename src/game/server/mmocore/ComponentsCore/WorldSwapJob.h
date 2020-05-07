/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#ifndef GAME_SERVER_WORLDSWAPJOB_H
#define GAME_SERVER_WORLDSWAPJOB_H

#include "../MmoComponent.h"

class WorldSwapJob : public MmoComponent
{
	struct StructSwapWorld
	{
		int OpenQuestID;
		int PositionX;
		int PositionY;
		int WorldID;
		int TwoPositionX;
		int TwoPositionY;
		int TwoWorldID;
	};
	static std::map < int, StructSwapWorld > WorldSwap;

	struct StructPositionLogic
	{
		int BaseWorldID;
		int FindWorldID;
		vec2 Position;
	};
	static std::list < StructPositionLogic > WorldPositionLogic;
public:
	virtual void OnInit();
	virtual bool OnHandleTile(CCharacter* pChr, int IndexCollision);

	vec2 GetPositionQuestBot(int ClientID, int QuestID);
	int GetWorldType() const;
	int GetNecessaryQuest(int WorldID = -1) const;
	void CheckQuestingOpened(CPlayer* pPlayer, int QuestID);

private:
	bool ChangeWorld(CPlayer *pPlayer, vec2 Pos);
	void UpdateWorldsList();
	int GetID(vec2 Pos);
};

#endif