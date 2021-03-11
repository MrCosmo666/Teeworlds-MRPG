/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#ifndef GAME_SERVER_WORLDSWAPJOB_H
#define GAME_SERVER_WORLDSWAPJOB_H

#include "../MmoComponent.h"

class WorldSwapJob : public MmoComponent
{
	~WorldSwapJob()
	{
		ms_aWorldSwap.clear();
		ms_aWorldPositionLogic.clear();
	};

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

	void OnInit() override;
	void OnInitWorld(const char* pWhereLocalWorld) override;
	bool OnHandleTile(CCharacter* pChr, int IndexCollision) override;

public:
	int GetWorldType() const;
	int GetNecessaryQuest(int WorldID = -1) const;
	vec2 GetPositionQuestBot(int ClientID, BotJob::QuestBotInfo QuestBot) const;
	void CheckQuestingOpened(CPlayer* pPlayer, int QuestID) const;

private:
	bool ChangeWorld(CPlayer* pPlayer, vec2 Pos);
	int GetID(vec2 Pos) const;
};

#endif