/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#ifndef GAME_SERVER_WORLDSWAPJOB_H
#define GAME_SERVER_WORLDSWAPJOB_H

#include "../component.h"

class WorldSwapJob : public CMmoComponent
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

	void UpdateWorldsList();

public:
	virtual void OnInitGlobal();
	virtual bool OnPlayerHandleTile(CCharacter* pChr, int IndexCollision);

	vec2 GetPositionQuestBot(int ClientID, int QuestID);
	int GetWorldType() const;
	int GetNecessaryQuest(int WorldID = -1) const;
	bool ChangeWorld(int ClientID, vec2 Pos);

private:
	int GetSwapID(vec2 Pos);
};

#endif