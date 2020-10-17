/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#ifndef GAME_SERVER_SQLQUEST_H
#define GAME_SERVER_SQLQUEST_H

#include <game/server/mmocore/MmoComponent.h>

enum QuestInteractive
{
	INTERACTIVE_RANDOM_ACCEPT_ITEM = 1,
	INTERACTIVE_DROP_AND_TAKE_IT = 2,
};

class QuestJob : public MmoComponent
{
	struct StructQuestData
	{
		char m_aName[24];
		char m_aStoryLine[24];

		int m_Gold;
		int m_Exp;
		int m_NumberSteps;
	};
	struct StructQuest
	{
		int m_State;
		int m_aMobProgress[2];
		int m_Step;
	};
	static std::map < int, StructQuestData > ms_aQuestsData;

public:
	static std::map < int, std::map < int, StructQuest > > ms_aQuests;
	bool IsValidQuest(int QuestID, int ClientID = -1) const
	{
		if (ms_aQuestsData.find(QuestID) != ms_aQuestsData.end())
		{
			if (ClientID < 0 || ClientID >= MAX_PLAYERS)
				return true;
			if (ms_aQuests[ClientID].find(QuestID) != ms_aQuests[ClientID].end())
				return true;
		}
		return false;
	}
	bool IsCompletedQuest(int ClientID, int QuestID) const;
	int GetState(int ClientID, int QuestID) const;
	const char *GetQuestName(int QuestID) const;
	const char *GetStoryName(int QuestID) const;
	BotJob::QuestBotInfo *GetQuestBot(int QuestID, int Step) const;
	bool IsActiveQuestBot(int QuestID, int Step) const;

private:
	int GetStoryCount(const char* StoryName, int CountFromQuestID = -1) const;
	bool IsProgressMobsComplete(int ClientID, int QuestID) const;

	void ShowQuestID(CPlayer *pPlayer, int QuestID);
	void FinishQuest(CPlayer *pPlayer, int QuestID);
	bool IsCollectItemComplete(CPlayer *pPlayer, BotJob::QuestBotInfo &BotData) const;
	void CollectItem(CPlayer *pPlayer, BotJob::QuestBotInfo &BotData);
	void AddStep(CPlayer *pPlayer, int QuestID);
	bool ShowQuestsActiveNPC(CPlayer *pPlayer);
	void QuestTableShowRequired(CPlayer* pPlayer, BotJob::QuestBotInfo& BotData);
	void ShowQuestsTabList(CPlayer* pPlayer, int StateQuest);
	void QuestTableAddItem(int ClientID, const char* pText, int Requires, int ItemID, bool Giving);
	void QuestTableAddInfo(int ClientID, const char* pText, int Requires, int Have);
	bool InteractiveTypeQuest(CPlayer* pPlayer, BotJob::QuestBotInfo& BotData);

public:
	bool AcceptQuest(int QuestID, CPlayer* pPlayer);
	void QuestTableShowRequired(CPlayer* pPlayer, BotJob::QuestBotInfo& BotData, const char* TextTalk);
	void AddMobProgress(CPlayer* pPlayer, int BotID);
	void UpdateArrowStep(int ClientID);
	void AcceptNextStoryQuestStep(CPlayer* pPlayer, int CheckQuestID);
	void AcceptNextStoryQuestStep(CPlayer* pPlayer);
	bool InteractiveQuestNPC(CPlayer* pPlayer, BotJob::QuestBotInfo& pBotData, bool LastDialog);
	void ShowQuestsMainList(CPlayer* pPlayer);
	void QuestTableClear(int ClientID);
	int QuestingAllowedItemsCount(CPlayer* pPlayer, int ItemID);
	void CreateQuestingItems(CPlayer* pPlayer, BotJob::QuestBotInfo& BotData);

	virtual void OnInit();
	virtual void OnInitAccount(CPlayer* pPlayer);
	virtual void OnResetClient(int ClientID);
	virtual void OnMessage(int MsgID, void* pRawMsg, int ClientID);
	virtual bool OnHandleMenulist(CPlayer* pPlayer, int Menulist, bool ReplaceMenu);
	virtual bool OnParsingVoteCommands(CPlayer* pPlayer, const char* CMD, const int VoteID, const int VoteID2, int Get, const char* GetText);
};

#endif