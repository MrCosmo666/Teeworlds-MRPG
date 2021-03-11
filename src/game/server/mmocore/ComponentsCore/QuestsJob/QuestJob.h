/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#ifndef GAME_SERVER_SQLQUEST_H
#define GAME_SERVER_SQLQUEST_H

#include <game/server/mmocore/MmoComponent.h>
#include "PlayerQuests.h"

enum QuestInteractive
{
	// INTERACTIVE_RANDOM_ACCEPT_ITEM = 1, // Todo this, it is necessary to lead to a fully understandable view of the conversation steps and refactoring it
	INTERACTIVE_DROP_AND_TAKE_IT = 2,
};

class QuestJob : public MmoComponent
{
	~QuestJob()
	{
		ms_aDataQuests.clear();
		ms_aPlayerQuests.clear();
	}

	void OnInit() override;
	void OnInitAccount(CPlayer* pPlayer) override;
	void OnResetClient(int ClientID) override;
	void OnMessage(int MsgID, void* pRawMsg, int ClientID) override;
	bool OnHandleMenulist(CPlayer* pPlayer, int Menulist, bool ReplaceMenu) override;
	bool OnHandleVoteCommands(CPlayer* pPlayer, const char* CMD, const int VoteID, const int VoteID2, int Get, const char* GetText) override;

public:
	static std::map < int, std::map <int , CPlayerQuest > > ms_aPlayerQuests;
	static std::map < int, CDataQuest > ms_aDataQuests;
	bool IsValidQuest(int QuestID, int ClientID = -1) const
	{
		if (ms_aDataQuests.find(QuestID) != ms_aDataQuests.end())
		{
			if (ClientID < 0 || ClientID >= MAX_PLAYERS)
				return true;
			if (ms_aPlayerQuests[ClientID].find(QuestID) != ms_aPlayerQuests[ClientID].end())
				return true;
		}
		return false;
	}

	void ShowQuestsMainList(CPlayer* pPlayer);
	void ShowQuestsActiveNPC(CPlayer* pPlayer, int QuestID);

private:
	void ShowQuestsTabList(CPlayer* pPlayer, int StateQuest);
	void ShowQuestID(CPlayer *pPlayer, int QuestID);

public:
	void QuestShowRequired(CPlayer* pPlayer, BotJob::QuestBotInfo& pBot, const char* TextTalk = "\0");
	void QuestTableAddItem(int ClientID, const char* pText, int Requires, int ItemID, bool Giving);
	void QuestTableAddInfo(int ClientID, const char* pText, int Requires, int Have);
	void QuestTableClear(int ClientID);

	bool InteractiveQuestNPC(CPlayer* pPlayer, BotJob::QuestBotInfo& pBot, bool FinalStepTalking);
	void AddMobProgressQuests(CPlayer* pPlayer, int BotID);
	void DoStepDropTakeItems(CPlayer* pPlayer, BotJob::QuestBotInfo& pBot);

	void UpdateArrowStep(CPlayer *pPlayer);
	void AcceptNextStoryQuestStep(CPlayer* pPlayer, int CheckQuestID);
	void AcceptNextStoryQuestStep(CPlayer* pPlayer);
	int GetUnfrozenItemCount(CPlayer* pPlayer, int ItemID) const;
};

#endif