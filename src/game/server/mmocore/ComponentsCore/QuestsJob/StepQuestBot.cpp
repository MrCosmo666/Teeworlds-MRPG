/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#include <engine/shared/config.h>
#include <teeother/system/string.h>

#include <game/server/gamecontext.h>
#include "StepQuestBot.h"

#include "DataQuests.h"
#include "QuestJob.h"

#include <game/server/mmocore/GameEntities/Items/drop_quest_items.h>

// ##############################################################
// ################# GLOBAL STEP STRUCTURE ######################
void CStepQuestBot::UpdateBot(CGS* pGS)
{
	CGS* pBotGS = (CGS*)pGS->Server()->GameServer(m_Bot->m_WorldID);
	if(!pBotGS)
		return;

	// check it's if there's a active bot
	int BotClientID = -1;
	for(int i = MAX_PLAYERS; i < MAX_CLIENTS; i++)
	{
		if(!pBotGS->m_apPlayers[i] || pBotGS->m_apPlayers[i]->GetBotType() != BotsTypes::TYPE_BOT_QUEST || pBotGS->m_apPlayers[i]->GetBotSub() != m_Bot->m_SubBotID)
			continue;

		BotClientID = i;
	}

	// seek if all players have an active bot
	const bool ActiveBot = IsActiveStep(pGS);
	if(ActiveBot && BotClientID <= -1)
	{
		dbg_msg("quest test", "quest to step bot active, but mob not found create");
		pBotGS->CreateBot(BotsTypes::TYPE_BOT_QUEST, m_Bot->m_BotID, m_Bot->m_SubBotID);
	}
	// if the bot is not active for more than one player
	if(!ActiveBot && BotClientID >= MAX_PLAYERS)
	{
		dbg_msg("quest test", "mob found, but quest to step not active on players");
		delete pBotGS->m_apPlayers[BotClientID];
		pBotGS->m_apPlayers[BotClientID] = nullptr;
	}
}

bool CStepQuestBot::IsActiveStep(CGS* pGS) const
{
	const int QuestID = m_Bot->m_QuestID;
	const int SubBotID = m_Bot->m_SubBotID;
	for(int i = 0; i < MAX_PLAYERS; i++)
	{
		CPlayer* pPlayer = pGS->m_apPlayers[i];
		if(!pPlayer || !pPlayer->IsAuthed() || pPlayer->GetQuest(QuestID).GetState() != QuestState::QUEST_ACCEPT)
			continue;

		if(m_Bot->m_Step != QuestJob::ms_aPlayerQuests[i][QuestID].m_Step || QuestJob::ms_aPlayerQuests[i][QuestID].m_StepsQuestBot[SubBotID].m_StepComplete ||
			QuestJob::ms_aPlayerQuests[i][QuestID].m_StepsQuestBot[SubBotID].m_ClientQuitting)
			continue;

		return true;
	}
	return false;
}

// ##############################################################
// ################# PLAYER STEP STRUCTURE ######################
bool CPlayerStepQuestBot::IsCompleteItems(CPlayer* pPlayer) const
{
	for(int i = 0; i < 2; i++)
	{
		const int ItemID = m_Bot->m_aItemSearch[i];
		const int Count = m_Bot->m_aItemSearchCount[i];
		if(ItemID <= 0 || Count <= 0)
			continue;

		if(pPlayer->GetItem(ItemID).m_Count < Count)
			return false;
	}
	return true;
}

bool CPlayerStepQuestBot::IsCompleteMobs(CPlayer* pPlayer) const
{
	for(int i = 0; i < 2; i++)
	{
		const int MobID = m_Bot->m_aNeedMob[i];
		const int Count = m_Bot->m_aNeedMobCount[i];
		if(MobID <= 0 || Count <= 0)
			continue;

		if(m_MobProgress[i] < Count)
			return false;
	}
	return true;
}

bool CPlayerStepQuestBot::Finish(CPlayer* pPlayer, bool LastDialog)
{
	CGS* pGS = pPlayer->GS();
	//if(m_Bot->m_DesignBot)
	//	return true;

	const int ClientID = pPlayer->GetCID();
	const int QuestID = m_Bot->m_QuestID;
	if(!IsCompleteItems(pPlayer) || !IsCompleteMobs(pPlayer))
	{
		pGS->Chat(ClientID, "Task has not been completed yet!");
		return false;
	}

	if(!LastDialog)
	{
		pGS->CreatePlayerSound(ClientID, SOUND_CTF_CAPTURE);
		return true;
	}

	DoCollectItem(pPlayer);

	// update state complete
	m_StepComplete = true;
	SJK.UD("tw_accounts_quests_bots_step", "Completed = '1' WHERE SubBotID = '%d' AND OwnerID = '%d'", m_Bot->m_SubBotID, pPlayer->Acc().m_AuthID);
	BotJob::ms_aDataBot[m_Bot->m_BotID].m_aAlreadySnapQuestBot[ClientID] = false;
	UpdateBot(pGS);

	QuestJob::ms_aPlayerQuests[ClientID][QuestID].CheckaAvailableNewStep();
	pGS->StrongUpdateVotes(ClientID, MenuList::MENU_JOURNAL_MAIN);
	return true;
}

void CPlayerStepQuestBot::DoCollectItem(CPlayer* pPlayer)
{
	/* // actions with what is required
	if(m_Bot->m_InteractiveType == (int)QuestInteractive::INTERACTIVE_RANDOM_ACCEPT_ITEM && m_Bot->m_InteractiveTemp > 0)
	{
		const bool Succesful = random_int() % m_Bot->m_InteractiveTemp == 0;
		for(int i = 0; i < 2; i++)
		{
			const int ItemID = m_Bot->m_aItemSearch[i];
			const int Count = m_Bot->m_aItemSearchCount[i];
			if(ItemID > 0 && Count > 0)
				pPlayer->GetItem(ItemID).Remove(Count);
		}

		if(!Succesful)
			pGS->Chat(ClientID, "{STR} don't like the item.", m_Bot->GetName());
		return Succesful;
	}*/

	// anti stressing with double thread sql result what work one (item)
	CGS* pGS = pPlayer->GS();
	bool antiStressing = false;
	for(int i = 0; i < 2; i++)
	{
		const int ItemID = m_Bot->m_aItemSearch[i];
		const int Count = m_Bot->m_aItemSearchCount[i];
		if(ItemID > 0 && Count > 0)
		{
			pGS->Chat(pPlayer->GetCID(), "You used quest item {STR}x{INT}!", pPlayer->GetItem(ItemID).Info().GetName(pPlayer), &Count);
			antiStressing = (bool)(ItemID == m_Bot->m_aItemGives[0] || ItemID == m_Bot->m_aItemGives[1]);
			pPlayer->GetItem(ItemID).Remove(Count);
		}
	}

	for(int i = 0; i < 2; i++)
	{
		const int ItemID = m_Bot->m_aItemGives[i];
		const int Count = m_Bot->m_aItemGivesCount[i];
		if(ItemID > 0 && Count > 0)
		{
			if(antiStressing)
			{
				pGS->Mmo()->Item()->AddItemSleep(pPlayer->Acc().m_AuthID, ItemID, Count, 300);
				continue;
			}

			if(pPlayer->GetItem(ItemID).Info().IsEnchantable() && pPlayer->GetItem(ItemID).m_Count >= 1)
			{
				pGS->SendInbox(pPlayer->GetCID(), "No place for item", "You already have this item, but we can't put it in inventory", ItemID, 1);
				continue;
			}
			pPlayer->GetItem(ItemID).Add(Count);
		}
	}
}

void CPlayerStepQuestBot::CreateQuestingItems(CPlayer* pPlayer)
{
	if(!pPlayer || !pPlayer->GetCharacter() || m_Bot->m_InteractiveType != (int)QuestInteractive::INTERACTIVE_DROP_AND_TAKE_IT)
		return;

	CGS* pGS = pPlayer->GS();
	const int ClientID = pPlayer->GetCID();
	for(CDropQuestItem* pHh = (CDropQuestItem*)pGS->m_World.FindFirst(CGameWorld::ENTTYPE_DROPQUEST); pHh; pHh = (CDropQuestItem*)pHh->TypeNext())
	{
		if(pHh->m_OwnerID == ClientID && pHh->m_QuestBot.m_QuestID == m_Bot->m_QuestID)
			return;
	}

	const int Count = 3 + m_Bot->m_aItemSearchCount[0];
	const vec2 Pos = vec2(m_Bot->m_PositionX, m_Bot->m_PositionY);
	for(int i = 0; i < Count; i++)
	{
		const vec2 Vel = vec2(frandom() * 40.0f - frandom() * 80.0f, frandom() * 40.0f - frandom() * 80.0f);
		const float AngleForce = Vel.x * (0.15f + frandom() * 0.1f);
		new CDropQuestItem(&pGS->m_World, Pos, Vel, AngleForce, *m_Bot, ClientID);
	}
}

void CPlayerStepQuestBot::AddMobProgress(CPlayer* pPlayer, int BotID)
{
	if(!pPlayer || !pPlayer->GS()->Mmo()->BotsData()->IsDataBotValid(BotID))
		return;

	CGS* pGS = pPlayer->GS();
	const int ClientID = pPlayer->GetCID();
	const int QuestID = m_Bot->m_QuestID;

	// if the quest is accepted
	if(QuestJob::ms_aPlayerQuests[ClientID][QuestID].m_State != QuestState::QUEST_ACCEPT)
		return;

	// check complecte mob
	for(int i = 0; i < 2; i++)
	{
		const int SubBotID = m_Bot->m_SubBotID;
		if(BotID != m_Bot->m_aNeedMob[i] || m_MobProgress[i] >= m_Bot->m_aNeedMobCount[i])
			continue;

		m_MobProgress[i]++;
		if(m_MobProgress[i] >= m_Bot->m_aNeedMobCount[i])
			pGS->Chat(ClientID, "You killed {STR} required amount for NPC {STR}", BotJob::ms_aDataBot[BotID].m_aNameBot, m_Bot->GetName());

		SJK.UD("tw_accounts_quests_bots_step", "Mob1Progress = '%d', Mob2Progress = '%d' WHERE SubBotID = '%d' AND OwnerID = '%d'", m_MobProgress[0], m_MobProgress[1], SubBotID, pPlayer->Acc().m_AuthID);
		break;
	}
}