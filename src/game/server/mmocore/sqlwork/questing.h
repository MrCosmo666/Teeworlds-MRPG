/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#ifndef GAME_SERVER_SQLQUEST_H
#define GAME_SERVER_SQLQUEST_H

#include "../component.h"

class QuestBase : public CMmoComponent
{
	/* #########################################################################
		VAR AND OBJECTS QUESTING 
	######################################################################### */
	struct StructQuestData
	{
		char Name[32];
		char Location[64];
		char StoryLine[32];

		int Level;
		int Money;
		int Exp;
		int ProgressSize;

		int ItemRewardID[3];
		int ItemRewardCount[3];
	};
	typedef std::map < int , StructQuestData > QuestDataType;
	static QuestDataType QuestsData;

	struct StructQuest
	{
		int Type;
		int MobProgress[2];
		int Progress;
	};
	typedef std::map < int , std::map < int , StructQuest > > QuestType;

public:
	static QuestType Quests;

	virtual void OnInitGlobal();
	virtual void OnInitAccount(CPlayer *pPlayer);
	bool IsComplecte(int ClientID, int QuestID) const;

private:
	/* #########################################################################
		GET CHECK QUESTING 
	######################################################################### */


	int GetStoryCountQuest(const char *StoryName, int QuestID = -1) const;
	const char *QuestState(int Type) const;
	bool IsDefeatMobComplete(int ClientID, int QuestID);

public:
	int GetQuestState(int ClientID, int QuestID) const;
	bool IsValidQuest(int QuestID, int ClientID = -1) const
	{
		if(QuestsData.find(QuestID) != QuestsData.end())
		{
			if(ClientID < 0 || ClientID >= MAX_PLAYERS)
				return true;
			
			if(Quests[ClientID].find(QuestID) != Quests[ClientID].end())
				return true;
		}
		return false;
	}

	const char *GetStoryName(int QuestID) const;
	ContextBots::QuestBotInfo &GetQuestBot(int QuestID, int Progress);
	bool IsActiveQuestBot(int QuestID, int Progress);

private:
	/* #########################################################################
		FUNCTIONS QUESTING 
	######################################################################### */
	void ShowQuestID(CPlayer *pPlayer, int QuestID, bool Passive = false);
	void FinishQuest(CPlayer *pPlayer, int QuestID);
	bool IsCollectItemComplete(CPlayer *pPlayer, ContextBots::QuestBotInfo &BotData, bool Gived, bool Interactive = false);
	void TalkProgress(CPlayer *pPlayer, int QuestID);

	bool ShowAdventureActiveNPC(CPlayer *pPlayer);

public:

	void ShowQuestList(CPlayer *pPlayer, int StateQuest);
	void ShowFullQuestLift(CPlayer *pPlayer);
	void ShowQuestInformation(CPlayer* pPlayer, ContextBots::QuestBotInfo& BotData, const char* TextTalk);

	void CheckQuest(CPlayer *pPlayer);
	bool AcceptQuest(int QuestID, CPlayer *pPlayer);
	bool InteractiveQuestNPC(CPlayer *pPlayer, ContextBots::QuestBotInfo &BotData, bool LastDialog);
	void AutoStartNextQuest(CPlayer *pPlayer, int QuestID);

	void AddMobProgress(CPlayer *pPlayer, int BotID);

	virtual bool OnMessage(int MsgID, void *pRawMsg, int ClientID);
	virtual bool OnParseVotingMenu(CPlayer *pPlayer, const char *CMD, const int VoteID, const int VoteID2, int Get, const char *GetText);

	void QuestTableAddItem(int ClientID, const char* pText, int Requires, int ItemID);
	void QuestTableAddInfo(int ClientID, const char* pText, int Requires, int Have);
	void QuestTableClear(int ClientID);
	void QuestTableShowInformation(CPlayer* pPlayer, ContextBots::QuestBotInfo& BotData);
	int QuestingAllowedItemsCount(CPlayer* pPlayer, int ItemID);
};

#endif