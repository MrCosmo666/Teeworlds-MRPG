/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#ifndef GAME_SERVER_COMPONENT_BOT_CORE_H
#define GAME_SERVER_COMPONENT_BOT_CORE_H
#include <game/server/mmocore/MmoComponent.h>

#include "BotData.h"

class CBotCore : public MmoComponent
{
	~CBotCore() override
	{
		DataBotInfo::ms_aDataBot.clear();
		QuestBotInfo::ms_aQuestBot.clear();
		NpcBotInfo::ms_aNpcBot.clear();
		MobBotInfo::ms_aMobBot.clear();
	};

	void OnInitWorld(const char* pWhereLocalWorld) override;

	void LoadMainInformationBots();
	void LoadQuestBots(const char* pWhereLocalWorld);
	void LoadNpcBots(const char* pWhereLocalWorld);
	void LoadMobsBots(const char* pWhereLocalWorld);

public:
	void ProcessingTalkingNPC(int OwnID, int TalkingID, bool PlayerTalked, const char *Message, int Style, int TalkingEmote);
	void TalkingBotNPC(CPlayer* pPlayer, int MobID, int Progress, int TalkedID, const char *pText = "empty");
	void TalkingBotQuest(CPlayer* pPlayer, int MobID, int Progress, int TalkedID);
	void ShowBotQuestTaskInfo(CPlayer* pPlayer, int MobID, int Progress);
	int GetQuestNPC(int MobID) const;
	const char *GetMeaninglessDialog();

	void ConDumpDialogsForTranslate();
	void ConAddCharacterBot(int ClientID, const char* pCharacter);
};

#endif