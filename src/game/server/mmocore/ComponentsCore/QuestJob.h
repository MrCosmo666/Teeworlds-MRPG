/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#ifndef GAME_SERVER_SQLQUEST_H
#define GAME_SERVER_SQLQUEST_H

#include "../MmoComponent.h"

enum QuestInteractive
{
	INTERACTIVE_RANDOM_ACCEPT_ITEM = 1,
	INTERACTIVE_DROP_AND_TAKE_IT = 2,
};

class QuestJob : public MmoComponent
{
	struct StructQuestData
	{
		char Name[24];
		char StoryLine[24];

		int Money;
		int Exp;
		int ProgressSize;
	};
	struct StructQuest
	{
		int State;
		int MobProgress[2];
		int Progress;
	};
	static std::map < int, StructQuestData > QuestsData;

public:
	static std::map < int, std::map < int, StructQuest > > Quests;
	bool IsValidQuest(int QuestID, int ClientID = -1) const
	{
		if (QuestsData.find(QuestID) != QuestsData.end())
		{
			if (ClientID < 0 || ClientID >= MAX_PLAYERS)
				return true;
			if (Quests[ClientID].find(QuestID) != Quests[ClientID].end())
				return true;
		}
		return false;
	}
	bool IsCompletedQuest(int ClientID, int QuestID) const;
	int GetState(int ClientID, int QuestID) const;
	const char *GetQuestName(int QuestID) const;
	const char *GetStoryName(int QuestID) const;
	BotJob::QuestBotInfo *GetQuestBot(int QuestID, int Progress) const;
	bool IsActiveQuestBot(int QuestID, int Progress) const;

private:
	int GetStoryCount(const char* StoryName, int CountFromQuestID = -1) const;
	bool IsDefeatMobsComplete(int ClientID, int QuestID) const;

	void ShowQuestID(CPlayer *pPlayer, int QuestID);
	void FinishQuest(CPlayer *pPlayer, int QuestID);
	bool IsCollectItemComplete(CPlayer *pPlayer, BotJob::QuestBotInfo &BotData) const;
	void CollectItem(CPlayer *pPlayer, BotJob::QuestBotInfo &BotData);
	void AddProgress(CPlayer *pPlayer, int QuestID);
	bool ShowAdventureActiveNPC(CPlayer *pPlayer);
	void QuestTableShowRequired(CPlayer* pPlayer, BotJob::QuestBotInfo& BotData);
	void ShowQuestList(CPlayer* pPlayer, int StateQuest);
	void QuestTableAddItem(int ClientID, const char* pText, int Requires, int ItemID, bool Giving);
	void QuestTableAddInfo(int ClientID, const char* pText, int Requires, int Have);
	bool InteractiveTypeQuest(CPlayer* pPlayer, BotJob::QuestBotInfo& BotData);

public:
	bool AcceptQuest(int QuestID, CPlayer* pPlayer);
	void QuestTableShowRequired(CPlayer* pPlayer, BotJob::QuestBotInfo& BotData, const char* TextTalk);
	void AddMobProgress(CPlayer* pPlayer, int BotID);
	void UpdateArrowStep(int ClientID);
	bool CheckNewStories(CPlayer* pPlayer, int CheckQuestID = -1);
	bool InteractiveQuestNPC(CPlayer* pPlayer, BotJob::QuestBotInfo& BotData, bool LastDialog);
	void ShowFullQuestLift(CPlayer* pPlayer);
	void QuestTableClear(int ClientID);
	int QuestingAllowedItemsCount(CPlayer* pPlayer, int ItemID);
	void CreateQuestingItems(CPlayer* pPlayer, BotJob::QuestBotInfo& BotData);

	virtual void OnInit();
	virtual void OnInitAccount(CPlayer* pPlayer);
	virtual void OnResetClient(int ClientID);
	virtual void OnMessage(int MsgID, void* pRawMsg, int ClientID);
	virtual bool OnHandleMenulist(CPlayer* pPlayer, int Menulist, bool ReplaceMenu);
	virtual bool OnVotingMenu(CPlayer* pPlayer, const char* CMD, const int VoteID, const int VoteID2, int Get, const char* GetText);
};

#endif