/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#include <engine/shared/config.h>
#include <teeother/components/localization.h>

#include "gamemodes/dungeon.h"
#include "gamecontext.h"
#include "player.h"

MACRO_ALLOC_POOL_ID_IMPL(CPlayer, MAX_CLIENTS*COUNT_WORLD+MAX_CLIENTS)

CPlayer::CPlayer(CGS *pGS, int ClientID) : m_pGS(pGS), m_ClientID(ClientID)
{
	for(short & SortTab : m_SortTabs)
		SortTab = 0;

	m_PlayerTick[TickState::Respawn] = Server()->Tick() + Server()->TickSpeed();
	m_PlayerTick[TickState::Die] = Server()->Tick();
	m_Spawned = true;
	m_LastVoteMenu = NOPE;
	m_OpenVoteMenu = MenuList::MAIN_MENU;
	m_PrevTuningParams = *pGS->Tuning();
	m_NextTuningParams = m_PrevTuningParams;
	m_MoodState = MOOD_NORMAL;
	GS()->SendTuningParams(ClientID);

	if(!IsBot())
	{
		Acc().Team = GetStartTeam();
	}
}

CPlayer::~CPlayer()
{
	delete m_pCharacter;
	m_pCharacter = nullptr;
}

/* #########################################################################
	FUNCTIONS PLAYER ENGINE 
######################################################################### */
// Тик игрока
void CPlayer::Tick()
{
	if (!Server()->ClientIngame(m_ClientID))
		return;

	if(!m_pCharacter && GetTeam() == TEAM_SPECTATORS)
		m_ViewPos -= vec2(clamp(m_ViewPos.x - m_LatestActivity.m_TargetX, -500.0f, 500.0f), clamp(m_ViewPos.y - m_LatestActivity.m_TargetY, -400.0f, 400.0f));

	// # # # # # ДАЛЬШЕ АВТОРИЗОВАННЫМ # # # # # #
	if(!IsAuthed())
		return;

	Server()->SetClientScore(m_ClientID, Acc().Level);
	{
		IServer::CClientInfo Info;
		if (Server()->GetClientInfo(m_ClientID, &Info))
		{
			m_Latency.m_AccumMax = max(m_Latency.m_AccumMax, Info.m_Latency);
			m_Latency.m_AccumMin = min(m_Latency.m_AccumMin, Info.m_Latency);
		}

		if (Server()->Tick() % Server()->TickSpeed() == 0)
		{
			m_Latency.m_Max = m_Latency.m_AccumMax;
			m_Latency.m_Min = m_Latency.m_AccumMin;
			m_Latency.m_AccumMin = 1000;
			m_Latency.m_AccumMax = 0;
		}
	}

	if (m_pCharacter && !m_pCharacter->IsAlive())
	{
		delete m_pCharacter;
		m_pCharacter = nullptr;
	}

	if (m_pCharacter)
	{
		if(m_pCharacter->IsAlive())
		{
			m_ViewPos = m_pCharacter->GetPos();
			PotionsTick();
		}
	}
	else if (m_Spawned && m_PlayerTick[TickState::Respawn] + Server()->TickSpeed() * 3 <= Server()->Tick())
		TryRespawn();

	TickOnlinePlayer();
	HandleTuningParams();
}

void CPlayer::PotionsTick()
{
	if (Server()->Tick() % Server()->TickSpeed() != 0)
		return;

	// TODO: change it
	for (auto ieffect = CGS::Effects[m_ClientID].begin(); ieffect != CGS::Effects[m_ClientID].end();)
	{
		ieffect->second--;
		if (ieffect->second <= 0)
		{
			GS()->Chat(m_ClientID, "You lost the effect {STR}.", ieffect->first.c_str());
			GS()->SendMmoPotion(m_pCharacter->m_Core.m_Pos, ieffect->first.c_str(), false);
			ieffect = CGS::Effects[m_ClientID].erase(ieffect);
			continue;
		}
		ieffect++;
	}	
}

// Пост тик
void CPlayer::PostTick()
{
	// update latency value
	if (Server()->ClientIngame(m_ClientID) && GS()->IsClientEqualWorldID(m_ClientID) && IsAuthed())
		GetTempData().TempLatencyPing = (short)m_Latency.m_Min;
}

// Тик авторизированного в ::Tick
void CPlayer::TickOnlinePlayer()
{
	TickSystemTalk();
}

void CPlayer::TickSystemTalk()
{
	if(m_TalkingNPC.m_TalkedID == -1 || m_TalkingNPC.m_TalkedID == m_ClientID)
		return;

	const int TalkedID = m_TalkingNPC.m_TalkedID;
	if(!m_pCharacter || TalkedID < MAX_PLAYERS || !GS()->m_apPlayers[TalkedID] || distance(m_ViewPos, GS()->m_apPlayers[TalkedID]->m_ViewPos) > 180.0f)
		ClearTalking();
}

// Персональный тюннинг игрока
void CPlayer::HandleTuningParams()
{
	if(!(m_PrevTuningParams == m_NextTuningParams))
	{
		if(GetCharacter())
		{
			CMsgPacker Msg(NETMSGTYPE_SV_TUNEPARAMS);
			int *pParams = (int *)&m_NextTuningParams;
			for(unsigned i = 0; i < sizeof(m_NextTuningParams)/sizeof(int); i++)
				Msg.AddInt(pParams[i]);
			Server()->SendMsg(&Msg, MSGFLAG_VITAL, m_ClientID);
		}
		m_PrevTuningParams = m_NextTuningParams;
	}
	m_NextTuningParams = *GS()->Tuning();
}

// Рисовка игрока
void CPlayer::Snap(int SnappingClient)
{
	if(!Server()->ClientIngame(m_ClientID))
		return;

	CNetObj_PlayerInfo *pPlayerInfo = static_cast<CNetObj_PlayerInfo *>(Server()->SnapNewItem(NETOBJTYPE_PLAYERINFO, m_ClientID, sizeof(CNetObj_PlayerInfo)));
	if(!pPlayerInfo)
		return;

	pPlayerInfo->m_PlayerFlags = m_PlayerFlags&PLAYERFLAG_CHATTING;
	pPlayerInfo->m_PlayerFlags |= PLAYERFLAG_READY;
	if(Server()->IsAuthed(m_ClientID))
		pPlayerInfo->m_PlayerFlags |= PLAYERFLAG_ADMIN;

	pPlayerInfo->m_Latency = (SnappingClient == -1 ? m_Latency.m_Min : GetTempData().TempLatencyPing);
	pPlayerInfo->m_Score = Acc().Level;

	// --------------------- CUSTOM ----------------------
	if(!GS()->CheckClient(SnappingClient) || GetTeam() == TEAM_SPECTATORS || !IsAuthed())
		return;

	CNetObj_Mmo_ClientInfo *pClientInfo = static_cast<CNetObj_Mmo_ClientInfo *>(Server()->SnapNewItem(NETOBJTYPE_MMO_CLIENTINFO, m_ClientID, sizeof(CNetObj_Mmo_ClientInfo)));
	if(!pClientInfo)
		return;

	bool local_ClientID = (m_ClientID == SnappingClient);
	pClientInfo->m_Local = local_ClientID;
	pClientInfo->m_WorldType = GS()->Mmo()->WorldSwap()->GetWorldType();
	pClientInfo->m_MoodType = m_MoodState;
	pClientInfo->m_Level = Acc().Level;
	pClientInfo->m_Exp = Acc().Exp;
	pClientInfo->m_Health = GetHealth();
	pClientInfo->m_HealthStart = GetStartHealth();
	pClientInfo->m_Armor = GetMana();

	dynamic_string Buffer;
	for (auto& eff : CGS::Effects[m_ClientID])
	{
		char aBuf[32];
		bool Minutes = eff.second >= 60;
		str_format(aBuf, sizeof(aBuf), "%s %d%s ", eff.first.c_str(), Minutes ? eff.second / 60 : eff.second, Minutes ? "m" : "");
		Buffer.append_at(Buffer.length(), aBuf);
	}
	StrToInts(pClientInfo->m_Potions, 12, Buffer.buffer());
	Buffer.clear();

	Server()->Localization()->Format(Buffer, GetLanguage(), "{INT}", &GetItem(itGold).Count);
	StrToInts(pClientInfo->m_Gold, 6, Buffer.buffer());
	Buffer.clear();

	if(Acc().GuildID > 0)
	{
		const int GuildID = Acc().GuildID;

		char aBuf[24];
		str_format(aBuf, sizeof(aBuf), "%s %s", GS()->Mmo()->Member()->GetGuildRank(GuildID, Acc().GuildRank), GS()->Mmo()->Member()->GuildName(GuildID));
		StrToInts(pClientInfo->m_StateName, 6, aBuf);
	}
	else
		StrToInts(pClientInfo->m_StateName, 6, "\0");
}

// Получить черактера игрока
CCharacter *CPlayer::GetCharacter()
{
	if(m_pCharacter && m_pCharacter->IsAlive())
		return m_pCharacter;
	return nullptr;
}

// Спавн игрока
void CPlayer::TryRespawn()
{
	vec2 SpawnPos;
	int SpawnType = SPAWN_HUMAN;

	if(GS()->IsDungeon() && GetTempData().TempActiveSafeSpawn)
		GetTempData().TempActiveSafeSpawn = false;
	else if(GetTempData().TempActiveSafeSpawn)
	{
		const int SafezoneWorldID = GS()->GetRespawnWorld();
		if(SafezoneWorldID >= 0 && !GS()->IsClientEqualWorldID(m_ClientID, SafezoneWorldID))
		{
			ChangeWorld(SafezoneWorldID);
			return;
		}
		SpawnType = SPAWN_HUMAN_SAFE;
	}

	if(!GS()->m_pController->CanSpawn(SpawnType, &SpawnPos, vec2(-1, -1)))
		return;

	if(!GS()->IsDungeon() && (GetTempData().TempTeleportX > 1 || GetTempData().TempTeleportY > 1))
	{
		SpawnPos = vec2(GetTempData().TempTeleportX, GetTempData().TempTeleportY);
		GetTempData().TempTeleportX = GetTempData().TempTeleportY = -1;
	}
	int savecidmem = MAX_CLIENTS*GS()->GetWorldID()+m_ClientID;
	m_pCharacter = new(savecidmem) CCharacter(&GS()->m_World);
	m_pCharacter->Spawn(this, SpawnPos);
	GS()->CreatePlayerSpawn(SpawnPos);
	m_Spawned = false;
}

void CPlayer::KillCharacter(int Weapon)
{
	if(m_pCharacter)
	{
		m_pCharacter->Die(m_ClientID, Weapon);
		delete m_pCharacter;
		m_pCharacter = nullptr;
	}
}

void CPlayer::OnDisconnect()
{
	KillCharacter();
}

void CPlayer::OnDirectInput(CNetObj_PlayerInput *NewInput)
{
	if(NewInput->m_PlayerFlags&PLAYERFLAG_CHATTING)
	{
		// skip the input if chat is active
		if(m_PlayerFlags&PLAYERFLAG_CHATTING)
			return;

		// reset input
		if(m_pCharacter)
			m_pCharacter->ResetInput();

		m_PlayerFlags = NewInput->m_PlayerFlags;
		return;
	}
	
	m_PlayerFlags = NewInput->m_PlayerFlags;

	if(m_pCharacter)
		m_pCharacter->OnDirectInput(NewInput);

	// check for activity
	if(NewInput->m_Direction || m_LatestActivity.m_TargetX != NewInput->m_TargetX ||
		m_LatestActivity.m_TargetY != NewInput->m_TargetY || NewInput->m_Jump ||
		NewInput->m_Fire&1 || NewInput->m_Hook)
	{
		m_LatestActivity.m_TargetX = NewInput->m_TargetX;
		m_LatestActivity.m_TargetY = NewInput->m_TargetY;
	}
}

void CPlayer::OnPredictedInput(CNetObj_PlayerInput *NewInput)
{
	// skip the input if chat is active
	if((m_PlayerFlags&PLAYERFLAG_CHATTING) && (NewInput->m_PlayerFlags&PLAYERFLAG_CHATTING))
		return;

	if(m_pCharacter)
		m_pCharacter->OnPredictedInput(NewInput);
}

// Получить команду игрока
int CPlayer::GetTeam()
{
	if(GS()->Mmo()->Account()->IsActive(m_ClientID)) 
		return Acc().Team;
	return TEAM_SPECTATORS;
}

/* #########################################################################
	FUNCTIONS PLAYER HELPER 
######################################################################### */
void CPlayer::ProgressBar(const char *Name, int MyLevel, int MyExp, int ExpNeed, int GivedExp)
{
	if (GS()->CheckClient(m_ClientID))
	{
		GS()->SendProgressBar(m_ClientID, MyExp, ExpNeed, Name);
		return;
	}

	const float GetLevelProgress = (float)(MyExp * 100.0) / (float)ExpNeed;
	const float GetExpProgress = (float)(GivedExp * 100.0) / (float)ExpNeed;
	char *Level = GS()->LevelString(100, (int)GetLevelProgress, 10, ':', ' ');
	char BufferInBroadcast[128];
	str_format(BufferInBroadcast, sizeof(BufferInBroadcast), "^235Lv%d %s%s %0.2f%%+%0.3f%%(%d)XP\n", MyLevel, Name, Level, GetLevelProgress, GetExpProgress, GivedExp);
	GS()->SBL(m_ClientID, BroadcastPriority::BROADCAST_GAME_INFORMATION, 100, BufferInBroadcast);
	mem_zero(Level, sizeof(Level));
}

bool CPlayer::Upgrade(int Count, int *Upgrade, int *Useless, int Price, int MaximalUpgrade, const char *UpgradeName)
{
	int UpgradeNeed = Price*Count;
	if((*Upgrade + Count) > MaximalUpgrade)
	{
		GS()->SBL(m_ClientID, BroadcastPriority::BROADCAST_GAME_WARNING, 100, "Upgrade has a maximum level.");
		return false;		
	}
	if(*Useless < UpgradeNeed)
	{
		GS()->SBL(m_ClientID, BroadcastPriority::BROADCAST_GAME_WARNING, 100, "Not upgrade points for +{INT}. Required {INT}.", &Count, &UpgradeNeed);
		return false;
	}
	*Useless -= UpgradeNeed;
	*Upgrade += Count;
	return true;
}

/* #########################################################################
	FUNCTIONS PLAYER ACCOUNT 
######################################################################### */
bool CPlayer::CheckFailMoney(int Price, int ItemID, bool CheckOnly)
{
	if (ItemID < 0)
		return true;
	if (Price <= 0)
		return false;

	ItemJob::InventoryItem &pPlayerItem = GetItem(ItemID);
	if(pPlayerItem.Count < Price)
	{
		GS()->Chat(m_ClientID,"Required {INT}, but you have only {INT} {STR}!", &Price, &pPlayerItem.Count, pPlayerItem.Info().GetName(this), NULL);
		return true;
	}

	if (CheckOnly)
		return false;
	if (!pPlayerItem.Remove(Price))
		return true;
	return false;
}

void CPlayer::GiveEffect(const char* Potion, int Sec, int Random)
{
	if(!m_pCharacter || !m_pCharacter->IsAlive())
		return;

	if((Random && rand()%Random == 0) || !Random)
	{
		GS()->Chat(m_ClientID, "You got the effect {STR} time {INT}sec.", Potion, &Sec);
		CGS::Effects[m_ClientID][Potion] = Sec;
		GS()->SendMmoPotion(m_pCharacter->m_Core.m_Pos, Potion, true);
	}
}

void CPlayer::SetLanguage(const char* pLanguage)
{
	Server()->SetClientLanguage(m_ClientID, pLanguage);
}

const char *CPlayer::GetLanguage() const
{
	return Server()->GetClientLanguage(m_ClientID);
}

void CPlayer::UpdateTempData(int Health, int Mana)
{
	GetTempData().TempHealth = Health;
	GetTempData().TempMana = Mana;
}

void CPlayer::AddExp(int Exp)
{
	Acc().Exp += Exp;
	for( ; Acc().Exp >= ExpNeed(Acc().Level); ) 
	{
		Acc().Exp -= ExpNeed(Acc().Level), Acc().Level++;
		Acc().Upgrade += 10;

		GS()->CreateDeath(m_pCharacter->m_Core.m_Pos, m_ClientID);
		GS()->CreateSound(m_pCharacter->m_Core.m_Pos, 4);
		GS()->CreateText(m_pCharacter, false, vec2(0, -40), vec2(0, -1), 30, "level");
		GS()->ChatFollow(m_ClientID, "Level UP. Now Level {INT}!", &Acc().Level);
		if(Acc().Exp < ExpNeed(Acc().Level))
		{
			GS()->VResetVotes(m_ClientID, MenuList::MAIN_MENU);
			GS()->Mmo()->SaveAccount(this, SaveType::SAVE_STATS);
			GS()->Mmo()->SaveAccount(this, SaveType::SAVE_UPGRADES);
		}
	}
	ProgressBar("Account", Acc().Level, Acc().Exp, ExpNeed(Acc().Level), Exp);

	if (rand() % 5 == 0)
		GS()->Mmo()->SaveAccount(this, SaveType::SAVE_STATS);

	if (Acc().IsGuild())
		GS()->Mmo()->Member()->AddExperience(Acc().GuildID);
}

void CPlayer::AddMoney(int Money) 
{ 
	GetItem(itGold).Add(Money); 
}

bool CPlayer::CheckEffect(const char* Potion)
{
	if(CGS::Effects[m_ClientID].find(Potion) != CGS::Effects[m_ClientID].end())
		return true;
	return false;
}

bool CPlayer::GetHidenMenu(int HideID) const
{
	if(m_HidenMenu.find(HideID) != m_HidenMenu.end())
		return m_HidenMenu.at(HideID);
	return false;
}

bool CPlayer::IsAuthed()
{ 
	if(GS()->Mmo()->Account()->IsActive(m_ClientID))
		return Acc().AuthID; 
	return false; 
}

int CPlayer::EnchantAttributes(int BonusID) const
{
	int BonusAttributes = 0;
	for (const auto& it : ItemJob::Items[m_ClientID])
	{
		if(!it.second.IsEquipped()) 
			continue;
		
		const int BonusCount = it.second.Info().GetStatsBonus(BonusID);
		if (BonusCount > 0)
		{
			const int PlayerBonusCount = BonusCount * (it.second.Enchant + 1);
			BonusAttributes += PlayerBonusCount;
		}
	}
	return BonusAttributes;
}

int CPlayer::GetStartTeam()
{
	if(Acc().AuthID)
		return TEAM_RED;

	return TEAM_SPECTATORS;
}

int CPlayer::ExpNeed(int Level) const
{
	return kurosio::computeExperience(Level);
}

int CPlayer::GetStartHealth()
{
	return 10 + GetAttributeCount(Stats::StHardness, true);
}

int CPlayer::GetStartMana()
{
	int EnchantBonus = GetAttributeCount(Stats::StPiety, true);
	return 10 + EnchantBonus;
}

void CPlayer::ShowInformationStats()
{
	if (!m_pCharacter)
		return;

	const int Health = GetHealth();
	const int StartHealth = GetStartHealth();
	const int Mana = GetMana();
	const int StartMana = GetStartMana();
	GS()->SBL(m_ClientID, BroadcastPriority::BROADCAST_BASIC_STATS, 100, "H: {INT}/{INT} M: {INT}/{INT}", &Health, &StartHealth, &Mana, &StartMana);
}

/* #########################################################################
	FUNCTIONS PLAYER PARSING 
######################################################################### */
bool CPlayer::ParseItemsF3F4(int Vote)
{
	if (!m_pCharacter)
	{
		GS()->Chat(m_ClientID, "Use it when you're not dead!");
		return true;
	}

	// - - - - - F3- - - - - - -
	if (Vote == 1)
	{
	}
	// - - - - - F4- - - - - - -
	else
	{
		// смена режима полета
		if(m_PlayerFlags & PLAYERFLAG_SCOREBOARD && GetEquippedItem(EQUIP_WINGS) > 0)
		{
			m_Flymode ^= true;
			GS()->Chat(m_ClientID, "You {STR} fly mode, your hook changes!", m_Flymode ? "Enable" : "Disable");
			return true;
		}

		// общение на диалогах для ванильных клиентов
		if(GetTalkedID() > 0 && !GS()->CheckClient(m_ClientID))
		{
			if(m_PlayerTick[TickState::LastDialog] && m_PlayerTick[TickState::LastDialog] > GS()->Server()->Tick())
				return true;

			m_PlayerTick[TickState::LastDialog] = GS()->Server()->Tick() + (GS()->Server()->TickSpeed() / 4);
			SetTalking(GetTalkedID(), true);
			return true;
		}
	}
	return false;
}
// Парсинг голосований и улучшение статистик
bool CPlayer::ParseVoteUpgrades(const char *CMD, const int VoteID, const int VoteID2, int Get)
{
	if(PPSTR(CMD, "UPGRADE") == 0)
	{
		if(Upgrade(Get, &Acc().Stats[VoteID], &Acc().Upgrade, VoteID2, 1000, GS()->AtributeName(VoteID))) 
		{
			GS()->Mmo()->SaveAccount(this, SaveType::SAVE_UPGRADES);
			GS()->ResetVotes(m_ClientID, MenuList::MENU_UPGRADE);
		}
		return true;
	}

	if(PPSTR(CMD, "HIDEN") == 0)
	{
		if(VoteID < TAB_STAT)
			return true;

		for(auto& x : m_HidenMenu)
		{
			if((x.first > NUM_TAB_MENU && x.first != VoteID))
				x.second = false;
		}
		m_HidenMenu[VoteID] ^= true;
		if(m_HidenMenu[VoteID] == false)
			m_HidenMenu.erase(VoteID);

		GS()->ResetVotes(m_ClientID, m_OpenVoteMenu);
		return true;
	}
	return false;
}

ItemJob::InventoryItem &CPlayer::GetItem(int ItemID) 
{
	if(ItemJob::Items[m_ClientID].find(ItemID) == ItemJob::Items[m_ClientID].end())
		ItemJob::Items[m_ClientID][ItemID] = ItemJob::InventoryItem(this, ItemID);

	ItemJob::Items[m_ClientID][ItemID].SetPlayer(this);
	return ItemJob::Items[m_ClientID][ItemID];
}

// Получить одетый предмет
int CPlayer::GetEquippedItem(int EquipID, int SkipItemID) const
{
	for(const auto& it : ItemJob::Items[m_ClientID])
	{
		if(!it.second.Count || !it.second.Settings || it.second.Info().Function != EquipID || it.first == SkipItemID) 
			continue;
		return it.first;
	}
	return -1;
}

// Общий уровень атрибутов Реальный и Обычный
int CPlayer::GetAttributeCount(int BonusID, bool Really, bool SearchClass)
{
	// обычная передача если нет сохранения и нет процентов
	int AttributEx = EnchantAttributes(BonusID);
	const bool SaveData = (str_comp_nocase(CGS::AttributInfo[BonusID].FieldName, "unfield") != 0);
	if (SaveData)
		AttributEx += Acc().Stats[BonusID];

	// если реальная стата то делим
	if (Really && CGS::AttributInfo[BonusID].UpgradePrice < 10) 
	{ 
		if (BonusID == Stats::StStrength || CGS::AttributInfo[BonusID].AtType == AtHardtype)
			AttributEx /= 10;
		else
			AttributEx /= 5; 
	}

	// если тип мира данж
	if(GS()->IsDungeon() && !SearchClass && CGS::AttributInfo[BonusID].UpgradePrice < 10)
		AttributEx = static_cast<CGameControllerDungeon*>(GS()->m_pController)->GetDungeonSync(this, BonusID);
	return AttributEx;
}

// Получить уровень Классов по атрибутам
int CPlayer::GetLevelDisciple(int Class, bool SearchClass)
{
	int Atributs = 0;
	for (const auto& at : CGS::AttributInfo)
	{
		if (at.second.AtType == Class)
			Atributs += GetAttributeCount(at.first, true, SearchClass);
	}
	return Atributs;
}

// - - - - - - T A L K I N G - - - - B O T S - - - - - - - - - 
void CPlayer::SetTalking(int TalkedID, bool ToProgress)
{
	if (TalkedID < MAX_PLAYERS || !GS()->m_apPlayers[TalkedID] || (!ToProgress && m_TalkingNPC.m_TalkedID != -1) || (ToProgress && m_TalkingNPC.m_TalkedID == -1))
		return;

	m_TalkingNPC.m_TalkedID = TalkedID;
	GS()->Mmo()->Quest()->QuestTableClear(m_ClientID);
	CPlayerBot* BotPlayer = static_cast<CPlayerBot*>(GS()->m_apPlayers[TalkedID]);
	const int MobID = BotPlayer->GetBotSub();
	if (BotPlayer->GetBotType() == BotsTypes::TYPE_BOT_NPC)
	{
		// Очистка конца диалогов или диалога который был бесмысленный
		const int sizeTalking = BotJob::NpcBot[MobID].m_Talk.size();
		const bool isTalkingEmpty = BotJob::NpcBot[MobID].m_Talk.empty();
		if ((isTalkingEmpty && m_TalkingNPC.m_TalkedProgress == 999) || (!isTalkingEmpty && m_TalkingNPC.m_TalkedProgress >= sizeTalking))
		{
			ClearTalking();
			GS()->ClearTalkText(m_ClientID);
			return;
		}

		// Узнать вообщем получен если квест выдавать рандомный бесмысленный диалог
		int GivingQuestID = GS()->Mmo()->BotsData()->GetQuestNPC(MobID);
		if (isTalkingEmpty || GS()->Mmo()->Quest()->GetState(m_ClientID, GivingQuestID) >= QuestState::QUEST_ACCEPT)
		{
			const char* MeaninglessDialog = GS()->Mmo()->BotsData()->GetMeaninglessDialog();
			GS()->Mmo()->BotsData()->TalkingBotNPC(this, MobID, -1, TalkedID, MeaninglessDialog);
			m_TalkingNPC.m_TalkedProgress = 999;
			return;
		}

		// Получить квест по прогрессу диалога если он есть в данном прогрессе то принимаем квест
		GivingQuestID = BotJob::NpcBot[MobID].m_Talk[m_TalkingNPC.m_TalkedProgress].m_GivingQuest;
		if (GivingQuestID >= 1)
		{
			if (!m_TalkingNPC.m_FreezedProgress)
			{
				GS()->Mmo()->BotsData()->TalkingBotNPC(this, MobID, m_TalkingNPC.m_TalkedProgress, TalkedID);
				m_TalkingNPC.m_FreezedProgress = true;
				return;
			}

			// принимаем квест
			GS()->Mmo()->Quest()->AcceptQuest(GivingQuestID, this);
			m_TalkingNPC.m_TalkedProgress++;
		}

		GS()->Mmo()->BotsData()->TalkingBotNPC(this, MobID, m_TalkingNPC.m_TalkedProgress, TalkedID);
	}

	else if (BotPlayer->GetBotType() == BotsTypes::TYPE_BOT_QUEST)
	{
		const int sizeTalking = BotJob::QuestBot[MobID].m_Talk.size();
		if (m_TalkingNPC.m_TalkedProgress >= sizeTalking)
		{
			ClearTalking();
			GS()->ClearTalkText(m_ClientID);
			GS()->Mmo()->Quest()->InteractiveQuestNPC(this, BotJob::QuestBot[MobID], true);
			return;
		}

		bool RequiestQuestTask = BotJob::QuestBot[MobID].m_Talk[m_TalkingNPC.m_TalkedProgress].m_RequestComplete;
		if (RequiestQuestTask)
		{
			if (!m_TalkingNPC.m_FreezedProgress)
			{
				GS()->Mmo()->Quest()->CreateQuestingItems(this, BotJob::QuestBot[MobID]);
				GS()->Mmo()->BotsData()->TalkingBotQuest(this, MobID, m_TalkingNPC.m_TalkedProgress, TalkedID);
				GS()->Mmo()->BotsData()->ShowBotQuestTaskInfo(this, MobID, m_TalkingNPC.m_TalkedProgress);
				m_TalkingNPC.m_FreezedProgress = true;
				return;
			}

			// skip non complete dialog quest
			if (!GS()->Mmo()->Quest()->InteractiveQuestNPC(this, BotJob::QuestBot[MobID], false))
			{
				GS()->Mmo()->BotsData()->TalkingBotQuest(this, MobID, m_TalkingNPC.m_TalkedProgress, TalkedID);
				GS()->Mmo()->BotsData()->ShowBotQuestTaskInfo(this, MobID, m_TalkingNPC.m_TalkedProgress);
				return;
			}
			else
			{
				m_TalkingNPC.m_TalkedProgress++;
			}
		}
		GS()->Mmo()->BotsData()->TalkingBotQuest(this, MobID, m_TalkingNPC.m_TalkedProgress, TalkedID);
	}
	m_TalkingNPC.m_TalkedProgress++;
}

void CPlayer::ClearTalking()
{
	GS()->ClearTalkText(m_ClientID);
	m_TalkingNPC.m_TalkedID = -1;
	m_TalkingNPC.m_TalkedProgress = 0;
	m_TalkingNPC.m_FreezedProgress = false;
}

// - - - - - - F O R M A T - - - - - T E X T - - - - - - - - - 
const char *CPlayer::FormatedTalkedText() 
{ 
	return GS()->Server()->Localization()->Localize(GetLanguage(), m_FormatTalkQuest); 
}
void CPlayer::FormatTextQuest(int DataBotID, const char *pText)
{
	if(!GS()->Mmo()->BotsData()->IsDataBotValid(DataBotID) || m_FormatTalkQuest[0] != '\0') 
		return;

	str_copy(m_FormatTalkQuest, pText, sizeof(m_FormatTalkQuest));
	str_replace(m_FormatTalkQuest, "[Player]", GS()->Server()->ClientName(m_ClientID));
	str_replace(m_FormatTalkQuest, "[Talked]", BotJob::DataBot[DataBotID].NameBot);
	str_replace(m_FormatTalkQuest, "[Time]", GS()->Server()->GetStringTypeDay());
	str_replace(m_FormatTalkQuest, "[Here]", GS()->Server()->GetWorldName(GS()->GetWorldID()));
}
void CPlayer::ClearFormatQuestText()
{
	mem_zero(m_FormatTalkQuest, sizeof(m_FormatTalkQuest));
}

void CPlayer::ChangeWorld(int WorldID)
{
	// reset dungeon temp data
	GetTempData().TempAlreadyVotedDungeon = false;
	GetTempData().TempTankVotingDungeon = 0;
	GetTempData().TempTimeDungeon = 0;

	if(m_pCharacter)
	{
		GS()->m_World.DestroyEntity(m_pCharacter);
		GS()->m_World.m_Core.m_apCharacters[m_ClientID] = 0;
	}
	Server()->ChangeWorld(m_ClientID, WorldID);
}

void CPlayer::SendClientInfo(int TargetID)
{	
	if(TargetID != -1 && (TargetID < 0 || TargetID >= MAX_PLAYERS || !Server()->ClientIngame(TargetID)))
		return;
		
	CNetMsg_Sv_ClientInfo ClientInfoMsg;
	ClientInfoMsg.m_ClientID = m_ClientID;
	ClientInfoMsg.m_Local = (bool)(m_ClientID == TargetID);
	ClientInfoMsg.m_Team = GetTeam();
	ClientInfoMsg.m_pName = Server()->ClientName(m_ClientID);
	ClientInfoMsg.m_pClan = Server()->ClientClan(m_ClientID);
	ClientInfoMsg.m_Country = Server()->ClientCountry(m_ClientID);
	ClientInfoMsg.m_Silent = (bool)(IsAuthed());
	for (int p = 0; p < 6; p++)
	{
		ClientInfoMsg.m_apSkinPartNames[p] = Acc().m_aaSkinPartNames[p];
		ClientInfoMsg.m_aUseCustomColors[p] = Acc().m_aUseCustomColors[p];
		ClientInfoMsg.m_aSkinPartColors[p] = Acc().m_aSkinPartColors[p];
	}

	// player data it static have accept it all worlds
	Server()->SendPackMsg(&ClientInfoMsg, MSGFLAG_VITAL|MSGFLAG_NORECORD, TargetID);
}

int CPlayer::GetPlayerWorldID() const
{
	return Server()->GetWorldID(m_ClientID);
}