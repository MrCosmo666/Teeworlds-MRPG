/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#include <base/stdafx.h>
#include "QuestStepDataInfo.h"

#include <game/server/gamecontext.h>
#include <teeother/system/string.h>

#include <game/server/mmocore/GameEntities/quest_path_finder.h>
#include <game/server/mmocore/GameEntities/Items/drop_quest_items.h>

#include <game/server/mmocore/Components/Inventory/InventoryCore.h>
#include "QuestCore.h"

// ##############################################################
// ################# GLOBAL STEP STRUCTURE ######################
void CQuestStepDataInfo::UpdateBot(CGS* pGS)
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
	const bool ActiveStepBot = IsActiveStep(pGS);
	if(ActiveStepBot && BotClientID <= -1)
	{
		//dbg_msg("quest sync", "quest to step bot active, but mob not found create");
		pBotGS->CreateBot(BotsTypes::TYPE_BOT_QUEST, m_Bot->m_BotID, m_Bot->m_SubBotID);
	}
	// if the bot is not active for more than one player
	if(!ActiveStepBot && BotClientID >= MAX_PLAYERS)
	{
		//dbg_msg("quest sync", "mob found, but quest to step not active on players");
		delete pBotGS->m_apPlayers[BotClientID];
		pBotGS->m_apPlayers[BotClientID] = nullptr;
	}
}

bool CQuestStepDataInfo::IsActiveStep(CGS* pGS) const
{
	const int QuestID = m_Bot->m_QuestID;
	const int SubBotID = m_Bot->m_SubBotID;
	for(int i = 0; i < MAX_PLAYERS; i++)
	{
		CPlayer* pPlayer = pGS->m_apPlayers[i];
		if(!pPlayer || !pPlayer->IsAuthed())
			continue;

		CQuestData& pPlayerQuest = pPlayer->GetQuest(QuestID);
		if(pPlayerQuest.GetState() != QuestState::QUEST_ACCEPT || m_Bot->m_Step != pPlayerQuest.m_Step)
			continue;

		// skip complete steps and players who come out to clear the world of bots
		if(pPlayerQuest.m_StepsQuestBot[SubBotID].m_StepComplete || pPlayerQuest.m_StepsQuestBot[SubBotID].m_ClientQuitting)
			continue;

		return true;
	}
	return false;
}

// ##############################################################
// ################# PLAYER STEP STRUCTURE ######################
int CPlayerQuestStepDataInfo::GetCountBlockedItem(CPlayer* pPlayer, int ItemID) const
{
	for(int i = 0; i < 2; i++)
	{
		const int BlockedItemID = m_Bot->m_aItemSearch[i];
		const int BlockedItemCount = m_Bot->m_aItemSearchCount[i];
		if(BlockedItemID <= 0 || BlockedItemCount <= 0 || ItemID != BlockedItemID)
			continue;
		return BlockedItemCount;
	}
	return 0;
}

bool CPlayerQuestStepDataInfo::IsCompleteItems(CPlayer* pPlayer) const
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

bool CPlayerQuestStepDataInfo::IsCompleteMobs(CPlayer* pPlayer) const
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

bool CPlayerQuestStepDataInfo::Finish(CPlayer* pPlayer, bool FinalStepTalking)
{
	CGS* pGS = pPlayer->GS();
	const int ClientID = pPlayer->GetCID();
	const int QuestID = m_Bot->m_QuestID;
	if(!IsCompleteItems(pPlayer) || !IsCompleteMobs(pPlayer))
	{
		pGS->Chat(ClientID, "Task has not been completed yet!");
		return false;
	}

	if(!FinalStepTalking)
	{
		pGS->CreatePlayerSound(ClientID, SOUND_CTF_CAPTURE);
		return true;
	}

	DoCollectItem(pPlayer);

	// update state complete
	m_StepComplete = true;
	DataBotInfo::ms_aDataBot[m_Bot->m_BotID].m_aAlreadyActiveQuestBot[ClientID] = false;
	CQuestData::ms_aPlayerQuests[ClientID][QuestID].SaveSteps();
	UpdateBot(pGS);

	CQuestData::ms_aPlayerQuests[ClientID][QuestID].CheckaAvailableNewStep();
	pGS->StrongUpdateVotes(ClientID, MenuList::MENU_JOURNAL_MAIN);
	return true;
}

void CPlayerQuestStepDataInfo::DoCollectItem(CPlayer* pPlayer)
{
	// anti stressing with double thread sql result what work one (item)
	CGS* pGS = pPlayer->GS();
	bool antiStressing = false;
	for(int i = 0; i < 2; i++)
	{
		const int ItemID = m_Bot->m_aItemSearch[i];
		const int Count = m_Bot->m_aItemSearchCount[i];
		if(ItemID > 0 && Count > 0)
		{
			pGS->Chat(pPlayer->GetCID(), "[Done] Give the {STR}x{INT} to the {STR}!", pPlayer->GetItem(ItemID).Info().GetName(pPlayer), Count, m_Bot->GetName());
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
				pGS->Mmo()->Item()->AddItemSleep(pPlayer->Acc().m_AccountID, ItemID, Count, 300);
				continue;
			}

			if(pPlayer->GetItem(ItemID).Info().IsEnchantable() && pPlayer->GetItem(ItemID).m_Count >= 1)
			{
				pGS->SendInbox(pPlayer, "No place for item", "You already have this item, but we can't put it in inventory", ItemID, 1);
				continue;
			}
			pPlayer->GetItem(ItemID).Add(Count);
		}
	}
}

void CPlayerQuestStepDataInfo::AddMobProgress(CPlayer* pPlayer, int BotID)
{
	const int QuestID = m_Bot->m_QuestID;
	if(!pPlayer || DataBotInfo::ms_aDataBot.find(BotID) == DataBotInfo::ms_aDataBot.end() || pPlayer->GetQuest(QuestID).GetState() != QuestState::QUEST_ACCEPT)
		return;

	CGS* pGS = pPlayer->GS();
	const int ClientID = pPlayer->GetCID();

	// check complecte mob
	for(int i = 0; i < 2; i++)
	{
		if(BotID != m_Bot->m_aNeedMob[i] || m_MobProgress[i] >= m_Bot->m_aNeedMobCount[i])
			continue;

		m_MobProgress[i]++;
		if(m_MobProgress[i] >= m_Bot->m_aNeedMobCount[i])
			pGS->Chat(ClientID, "[Done] Defeat the {STR}'s for the {STR}!", DataBotInfo::ms_aDataBot[BotID].m_aNameBot, m_Bot->GetName());

		CQuestData::ms_aPlayerQuests[ClientID][QuestID].SaveSteps();
		break;
	}
}

void CPlayerQuestStepDataInfo::CreateStepArrow(CPlayer* pPlayer)
{
	if(!pPlayer || !pPlayer->GetCharacter() || m_StepComplete)
		return;

	if(pPlayer->GetQuest(m_Bot->m_QuestID).GetState() == QuestState::QUEST_ACCEPT && pPlayer->GetQuest(m_Bot->m_QuestID).m_Step == m_Bot->m_Step)
	{
		CGS* pGS = pPlayer->GS();
		const int ClientID = pPlayer->GetCID();
		new CQuestPathFinder(&pGS->m_World, pPlayer->GetCharacter()->m_Core.m_Pos, ClientID, *m_Bot);
	}
}

void CPlayerQuestStepDataInfo::CreateStepDropTakeItems(CPlayer* pPlayer)
{
	if(!pPlayer || !pPlayer->GetCharacter() || m_Bot->m_InteractiveType != (int)QuestInteractive::INTERACTIVE_DROP_AND_TAKE_IT)
		return;

	CGS* pGS = pPlayer->GS();
	const int ClientID = pPlayer->GetCID();
	for(CDropQuestItem* pHh = (CDropQuestItem*)pGS->m_World.FindFirst(CGameWorld::ENTTYPE_DROPQUEST); pHh; pHh = (CDropQuestItem*)pHh->TypeNext())
	{
		if(pHh->m_ClientID == ClientID && pHh->m_QuestBot.m_QuestID == m_Bot->m_QuestID)
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


void CPlayerQuestStepDataInfo::ShowRequired(CPlayer* pPlayer, const char* TextTalk)
{
	CGS* pGS = pPlayer->GS();
	const int ClientID = pPlayer->GetCID();
	if(pGS->IsMmoClient(ClientID))
	{
		char aBuf[64];
		dynamic_string Buffer;

		// search item's
		for (int i = 0; i < 2; i++)
		{
			const int ItemID = m_Bot->m_aItemSearch[i];
			const int CountItem = m_Bot->m_aItemSearchCount[i];
			if(ItemID <= 0 || CountItem <= 0)
				continue;

			str_format(aBuf, sizeof(aBuf), "%s", pPlayer->GetItem(ItemID).Info().GetName(pPlayer));
			Buffer.append_at(Buffer.length(), aBuf);
			pGS->Mmo()->Quest()->QuestTableAddItem(ClientID, aBuf, CountItem, ItemID, false);
		}

		// search mob's
		for (int i = 0; i < 2; i++)
		{
			const int BotID = m_Bot->m_aNeedMob[i];
			const int CountMob = m_Bot->m_aNeedMobCount[i];
			if (BotID <= 0 || CountMob <= 0 || DataBotInfo::ms_aDataBot.find(BotID) == DataBotInfo::ms_aDataBot.end())
				continue;

			str_format(aBuf, sizeof(aBuf), "Defeat %s", DataBotInfo::ms_aDataBot[BotID].m_aNameBot);
			pGS->Mmo()->Quest()->QuestTableAddInfo(ClientID, aBuf, CountMob, m_MobProgress[i]);
		}

		// reward item's
		for (int i = 0; i < 2; i++)
		{
			const int ItemID = m_Bot->m_aItemGives[i];
			const int CountItem = m_Bot->m_aItemGivesCount[i];
			if (ItemID <= 0 || CountItem <= 0)
				continue;

			str_format(aBuf, sizeof(aBuf), "Receive %s", pPlayer->GetItem(ItemID).Info().GetName(pPlayer));
			pGS->Mmo()->Quest()->QuestTableAddItem(ClientID, aBuf, CountItem, ItemID, true);
		}
		return;
	}

	char aBuf[64];
	dynamic_string Buffer;
	bool IsActiveTask = false;

	// search item's and mob's
	for(int i = 0; i < 2; i++)
	{
		const int BotID = m_Bot->m_aNeedMob[i];
		const int CountMob = m_Bot->m_aNeedMobCount[i];
		if(BotID > 0 && CountMob > 0 && DataBotInfo::ms_aDataBot.find(BotID) != DataBotInfo::ms_aDataBot.end())
		{
			str_format(aBuf, sizeof(aBuf), "\n- Defeat %s [%d/%d]", DataBotInfo::ms_aDataBot[BotID].m_aNameBot, m_MobProgress[i], CountMob);
			Buffer.append_at(Buffer.length(), aBuf);
			IsActiveTask = true;
		}

		const int ItemID = m_Bot->m_aItemSearch[i];
		const int CountItem = m_Bot->m_aItemSearchCount[i];
		if(ItemID > 0 && CountItem > 0)
		{
			InventoryItem PlayerQuestItem = pPlayer->GetItem(ItemID);
			str_format(aBuf, sizeof(aBuf), "\n- Need %s [%d/%d]", PlayerQuestItem.Info().GetName(pPlayer), PlayerQuestItem.m_Count, CountItem);
			Buffer.append_at(Buffer.length(), aBuf);
			IsActiveTask = true;
		}
	}

	// reward item's
	for(int i = 0; i < 2; i++)
	{
		const int ItemID = m_Bot->m_aItemGives[i];
		const int CountItem = m_Bot->m_aItemGivesCount[i];
		if(ItemID > 0 && CountItem > 0)
		{
			str_format(aBuf, sizeof(aBuf), "\n- Receive %s [%d]", pPlayer->GetItem(ItemID).Info().GetName(pPlayer), CountItem);
			Buffer.append_at(Buffer.length(), aBuf);
		}
	}

	pGS->Motd(ClientID, "{STR}\n\n{STR}{STR}\n\n", TextTalk, (IsActiveTask ? "### Task" : "\0"), Buffer.buffer());
	pPlayer->ClearDialogText();
	Buffer.clear();
}
