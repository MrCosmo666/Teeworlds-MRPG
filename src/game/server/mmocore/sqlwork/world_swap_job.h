/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#ifndef GAME_SERVER_WORLDSWAPJOB_H
#define GAME_SERVER_WORLDSWAPJOB_H

#include "../component.h"

class WorldSwapJob : public CMmoComponent
{
	struct StructSwapWorld
	{
		int Level;
		int PositionX;
		int PositionY;
		int WorldID;
		int SwapID;
	};
	typedef std::map < int , StructSwapWorld > WorldSwapType;
	static WorldSwapType WorldSwap;

	struct StructPositionLogic
	{
		int BaseWorldID;
		int FindWorldID;
		vec2 Position;
	};
	typedef std::list < StructPositionLogic > WorldPositionLogicType;
	static WorldPositionLogicType WorldPositionLogic;

public:
	virtual void OnInitGlobal();
	virtual bool OnPlayerHandleTile(CCharacter* pChr, int IndexCollision);

	vec2 PositionQuestBot(int ClientID, int QuestID);
	int GetWorldType() const;

private:
	int CheckPosition(vec2 Pos);
	bool ChangingWorld(int ClientID, vec2 Pos);
};

#endif