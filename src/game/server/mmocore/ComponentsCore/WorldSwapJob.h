/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#ifndef GAME_SERVER_WORLDSWAPJOB_H
#define GAME_SERVER_WORLDSWAPJOB_H

#include "../MmoComponent.h"

class WorldSwapJob : public MmoComponent
{
	struct StructSwapWorld
	{
		int m_OpenQuestID;
		int m_PositionX;
		int m_PositionY;
		int m_WorldID;
		int m_TwoPositionX;
		int m_TwoPositionY;
		int m_TwoWorldID;
	};
	struct StructPositionLogic
	{
		int m_BaseWorldID;
		int m_FindWorldID;
		vec2 m_Position;
	};

	static std::map < int, StructSwapWorld > ms_aWorldSwap;
	static std::list < StructPositionLogic > ms_aWorldPositionLogic;
public:
	virtual void OnInit();
	virtual void OnInitWorld(const char* pWhereLocalWorld);
	virtual bool OnHandleTile(CCharacter* pChr, int IndexCollision);

	vec2 GetPositionQuestBot(int ClientID, BotJob::QuestBotInfo QuestBot);
	int GetWorldType() const;
	int GetNecessaryQuest(int WorldID = -1) const;
	void CheckQuestingOpened(CPlayer* pPlayer, int QuestID);

private:
	bool ChangeWorld(CPlayer *pPlayer, vec2 Pos);
	int GetID(vec2 Pos);
};

#endif