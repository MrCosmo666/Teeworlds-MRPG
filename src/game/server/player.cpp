/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#include <engine/shared/config.h>

#include "gamecontext.h"
#include "player.h"

MACRO_ALLOC_POOL_ID_IMPL(CPlayer, MAX_CLIENTS*COUNT_WORLD+MAX_CLIENTS)

CPlayer::CPlayer(CGS *pGS, int ClientID) : m_pGS(pGS), m_ClientID(ClientID)
{
	for(short & SortTab : m_SortTabs)
		SortTab = 0;

	m_PlayerTick[TickState::Respawn] = Server()->Tick() + Server()->TickSpeed();
	m_PlayerTick[TickState::CheckClient] = Server()->Tick();
	m_PlayerTick[TickState::Die] = Server()->Tick();

	m_Spawned = true;
	m_LastVoteMenu = NOPE;
	m_OpenVoteMenu = MenuList::MAIN_MENU;
	m_PrevTuningParams = *pGS->Tuning();
	m_NextTuningParams = m_PrevTuningParams;

	ClearParsing();
	Acc().Team = GetStartTeam();
	if (Acc().AuthID > 0)
	{
		GS()->Mmo()->Account()->LoadAccount(this, false);
		m_SyncDuneon = GS()->Mmo()->Dungeon()->SyncFactor();
	}

	SetLanguage(Server()->GetClientLanguage(ClientID));
	GS()->SendTuningParams(ClientID);
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

	Server()->SetClientScore(m_ClientID, Acc().Level);

	{ // вычисление пинга и установка данных клиенту
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

	if (!m_pCharacter && GetTeam() == TEAM_SPECTATORS)
		m_ViewPos -= vec2(clamp(m_ViewPos.x - m_LatestActivity.m_TargetX, -500.0f, 500.0f), clamp(m_ViewPos.y - m_LatestActivity.m_TargetY, -400.0f, 400.0f));

	// # # # # # # # # # # # # # # # # # # # # # #
	// # # # # # ДАЛЬШЕ АВТОРИЗОВАННЫМ # # # # # #
	// # # # # # # # # # # # # # # # # # # # # # #
	if (!IsAuthed())
		return;

	if (m_pCharacter && !m_pCharacter->IsAlive())
	{
		delete m_pCharacter;
		m_pCharacter = nullptr;
	}

	if (m_pCharacter)
	{
		if (m_pCharacter->IsAlive())
			m_ViewPos = m_pCharacter->GetPos();
	}
	else if (m_Spawned && m_PlayerTick[TickState::Respawn] + Server()->TickSpeed() * 3 <= Server()->Tick())
		TryRespawn();

	if (m_pCharacter && m_pCharacter->IsAlive())
		PotionsTick();

	HandleTuningParams();
	TickOnlinePlayer();
}

void CPlayer::PotionsTick()
{
	if (Server()->Tick() % Server()->TickSpeed() != 0)
		return;

	// TODO: change it
	for (auto ieffect = CGS::Effects[m_ClientID].begin(); ieffect != CGS::Effects[m_ClientID].end();)
	{
		if (str_comp(ieffect->first.c_str(), "Poison") == 0)
			m_pCharacter->TakeDamage(vec2(0, 0), vec2(0, 0), 1, m_ClientID, WEAPON_SELF);

		if (str_comp(ieffect->first.c_str(), "RegenHealth") == 0)
			m_pCharacter->IncreaseHealth(15);

		ieffect->second--;
		if (ieffect->second <= 0)
		{
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
		Acc().TempLatencyPing = m_Latency.m_Min;
}

// Тик авторизированного в ::Tick
void CPlayer::TickOnlinePlayer()
{
	// интерактив принятия F3 или F4
	if(CGS::Interactive[m_ClientID].ParsingLifeTick > 0)
	{
		CGS::Interactive[m_ClientID].ParsingLifeTick--;
		if(!CGS::Interactive[m_ClientID].ParsingLifeTick)
		{
			int SaveCID = CGS::Interactive[m_ClientID].ParsingClientID;
			if(SaveCID >= 0 && SaveCID < MAX_PLAYERS && GS()->m_apPlayers[SaveCID])
			{
				GS()->Chat(SaveCID, "Your invite {STR} refused {STR}.", CGS::Interactive[m_ClientID].ParsingType, Server()->ClientName(m_ClientID));
			}
			GS()->Chat(m_ClientID, "Action {STR} timed out.", CGS::Interactive[m_ClientID].ParsingType);
			ClearParsing(true, false);
		}
	}

	TickSystemTalk();
}

void CPlayer::TickSystemTalk()
{
	if(m_TalkingNPC.m_TalkedID == -1 || m_TalkingNPC.m_TalkedID == m_ClientID)
		return;

	int TalkedID = m_TalkingNPC.m_TalkedID;
	if(TalkedID < MAX_PLAYERS || !GS()->m_apPlayers[TalkedID] || distance(m_ViewPos, GS()->m_apPlayers[TalkedID]->m_ViewPos) > 180.0f)
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

	pPlayerInfo->m_Latency = (SnappingClient == -1 ? m_Latency.m_Min : Acc().TempLatencyPing);
	pPlayerInfo->m_Score = Acc().Level;

	// --------------------- CUSTOM ----------------------
	// ---------------------------------------------------
	if(!GS()->CheckClient(SnappingClient) || GetTeam() == TEAM_SPECTATORS || !IsAuthed())
		return;

	CNetObj_Mmo_ClientInfo *pClientInfo = static_cast<CNetObj_Mmo_ClientInfo *>(Server()->SnapNewItem(NETOBJTYPE_MMO_CLIENTINFO, m_ClientID, sizeof(CNetObj_Mmo_ClientInfo)));
	if(!pClientInfo)
		return;

	bool local_ClientID = (m_ClientID == SnappingClient);
	m_MoodState = GetMoodState();
	pClientInfo->m_Local = local_ClientID;
	pClientInfo->m_WorldType = GS()->Mmo()->WorldSwap()->GetWorldType();
	pClientInfo->m_MoodType = m_MoodState;
	pClientInfo->m_Level = Acc().Level;
	pClientInfo->m_Exp = Acc().Exp;
	pClientInfo->m_Health = GetHealth();
	pClientInfo->m_HealthStart = GetStartHealth();
	pClientInfo->m_Armor = 0;

	for(int p = 0; p < 6; p++)
	{
		StrToInts(pClientInfo->m_aaSkinPartNames[p], 6, Acc().m_aaSkinPartNames[p]);
		pClientInfo->m_aUseCustomColors[p] = Acc().m_aUseCustomColors[p];
		pClientInfo->m_aSkinPartColors[p] = Acc().m_aSkinPartColors[p];
	}

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

	Server()->Localization()->Format(Buffer, GetLanguage(), "{INT}", &GetItem(itMoney).Count);
	StrToInts(pClientInfo->m_Gold, 6, Buffer.buffer());
	Buffer.clear();

	if(Acc().GuildID > 0)
		StrToInts(pClientInfo->m_StateName, 6, GS()->Mmo()->Member()->GuildName(Acc().GuildID));
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
	if(Acc().TempActiveSafeSpawn)
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

	const int CheckHouseSpawn = GS()->Mmo()->House()->PlayerHouseID(this);
	if(CheckHouseSpawn > 0)
	{
		const int HouseWorldID = GS()->Mmo()->House()->GetWorldID(CheckHouseSpawn);
		if (GS()->IsClientEqualWorldID(m_ClientID, HouseWorldID))
		{
			int HouseID = GS()->Mmo()->House()->PlayerHouseID(this);
			SpawnPos = GS()->Mmo()->House()->GetPositionHouse(HouseID);
		}
	}

	if(Acc().TempTeleportX > 1 || Acc().TempTeleportY > 1)
	{
		SpawnPos = vec2(Acc().TempTeleportX, Acc().TempTeleportY);
		Acc().TempTeleportX = Acc().TempTeleportY = -1;
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

	int NeedXp = ExpNeed;
	float getlv = (MyExp * 100.0) / NeedXp;
	float getexp = (GivedExp * 100.0) / NeedXp;
	char *Level = GS()->LevelString(100, (int)getlv, 10, ':', ' ');
	char BufferInBroadcast[128];
	str_format(BufferInBroadcast, sizeof(BufferInBroadcast), "^235Lv%d %s%s %0.2f%%+%0.3f%%(%d)XP\n", MyLevel, Name, Level, getlv, getexp, GivedExp);
	GS()->SBL(m_ClientID, BroadcastPriority::BROADCAST_GAME_INFORMATION, 100, BufferInBroadcast);
	mem_zero(Level, sizeof(Level));
}

bool CPlayer::Upgrade(int Count, int *Upgrade, int *Useless, int Price, int MaximalUpgrade, const char *UpgradeName)
{
	int UpgradeNeed = Price*Count;
	if((*Upgrade + Count) > MaximalUpgrade)
	{
		GS()->SBL(m_ClientID, BroadcastPriority::BROADCAST_GAME_WARNING, 100, "This upgrades maximal.");
		return false;		
	}
	if(*Useless < UpgradeNeed)
	{
		GS()->SBL(m_ClientID, BroadcastPriority::BROADCAST_GAME_WARNING, 100, "Have no upgrade points for upgrade +{INT}. Need {INT}.", &Count, &UpgradeNeed);
		return false;
	}
	*Useless -= UpgradeNeed;
	*Upgrade += Count;
	return true;
}

// Именна бонусов статистик

/* #########################################################################
	FUNCTIONS PLAYER ACCOUNT 
######################################################################### */
bool CPlayer::CheckFailMoney(int Price, int ItemID, bool CheckOnly)
{
	if (ItemID < 0)
		return true;
	if (Price < 0)
		return false;

	ItemJob::InventoryItem &pPlayerItem = GetItem(ItemID);
	if(pPlayerItem.Count < Price)
	{
		GS()->Chat(m_ClientID,"Sorry, need {INT} but you have only {INT} {STR}!", &Price, &pPlayerItem.Count, pPlayerItem.Info().GetName(this), NULL);
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
		CGS::Effects[m_ClientID][Potion] = Sec;
		GS()->SendMmoPotion(m_pCharacter->m_Core.m_Pos, Potion, true);
	}
}

void CPlayer::SetLanguage(const char* pLanguage)
{
	str_copy(m_aLanguage, pLanguage, sizeof(m_aLanguage));
}

void CPlayer::UpdateTempData(int Health, int Mana)
{
	Acc().TempHealth = Health;
	Acc().TempMana = Mana;
}

void CPlayer::AddExp(int Exp)
{
	Acc().Exp += Exp;
	for( ; Acc().Exp >= ExpNeed(Acc().Level); ) 
	{
		Acc().Exp -= ExpNeed(Acc().Level), Acc().Level++;
		Acc().Upgrade += 8;

		GS()->CreateDeath(m_pCharacter->m_Core.m_Pos, m_ClientID);
		GS()->CreateSound(m_pCharacter->m_Core.m_Pos, 4);
		GS()->CreateText(m_pCharacter, false, vec2(0, -40), vec2(0, -1), 30, "level", Server()->GetWorldID(m_ClientID));
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
	GetItem(itMoney).Add(Money); 
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
		if(it.second.Count <= 0 || it.second.Settings <= 0) 
			continue;
		
		int BonusCount = it.second.Info().GetStatsBonus(BonusID);
		if (BonusCount > 0)
		{
			int PlayerBonusCount = BonusCount * (it.second.Enchant + 1);
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

int CPlayer::ExpNeed(int Level)
{
	return (int)kurosio::computeExperience(Level);
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

const char* CPlayer::GetLanguage()
{
	return m_aLanguage;
}

void CPlayer::ShowInformationStats()
{
	if (m_ClientID < 0 || m_ClientID >= MAX_PLAYERS)
		return;

	int Health = GetHealth();
	int StartHealth = GetStartHealth();
	int Mana = m_pCharacter->Mana();
	int StartMana = GetStartMana();
	GS()->SBL(m_ClientID, BroadcastPriority::BROADCAST_BASIC_STATS, 100, "H: {INT}/{INT} M: {INT}/{INT}", &Health, &StartHealth, &Mana, &StartMana);
}

/* #########################################################################
	FUNCTIONS PLAYER PARSING 
######################################################################### */
// Создание нового действия приглашения
bool CPlayer::SetParsing(int Sec, int InviteID, int SaveInt, const char* Interact)
{
	if(CGS::Interactive[m_ClientID].ParsingLifeTick >= 0 || InviteID < 0 || InviteID > MAX_PLAYERS || !GS()->m_apPlayers[InviteID])
		return false;

	CGS::Interactive[m_ClientID].ParsingLifeTick = Sec*m_pGS->Server()->TickSpeed();
	CGS::Interactive[m_ClientID].ParsingClientID = InviteID;
	CGS::Interactive[m_ClientID].ParsingSaveInt = SaveInt;
	str_copy(CGS::Interactive[m_ClientID].ParsingType, Interact, sizeof(CGS::Interactive[m_ClientID].ParsingType));

	char aBuf[64];
	str_format(aBuf, sizeof(aBuf), "%s %s", Server()->ClientName(InviteID), Interact);

	CNetMsg_Sv_VoteSet Msg;
	Msg.m_Type = VOTE_UNKNOWN;
	Msg.m_Timeout = Sec;
	Msg.m_ClientID = m_ClientID;
	Msg.m_pDescription = "";
	Msg.m_pReason = aBuf;
	Server()->SendPackMsg(&Msg, MSGFLAG_VITAL, m_ClientID);	
	return true;
}
// Очиста парсинга приглашений
void CPlayer::ClearParsing(bool ClearVote, bool VotePass)
{
	CGS::Interactive[m_ClientID].ParsingLifeTick = -1;
	CGS::Interactive[m_ClientID].ParsingClientID = -1;
	CGS::Interactive[m_ClientID].ParsingSaveInt = -1;
	str_copy(CGS::Interactive[m_ClientID].ParsingType, "null", sizeof(CGS::Interactive[m_ClientID].ParsingType));
	if(ClearVote)
	{
		CNetMsg_Sv_VoteSet Msg;
		Msg.m_Type = VotePass ? (int)VOTE_END_PASS : (int)VOTE_END_FAIL;
		Msg.m_Timeout = 0;
		Msg.m_ClientID = m_ClientID;
		Msg.m_pDescription = "";
		Msg.m_pReason = "";
		Server()->SendPackMsg(&Msg, MSGFLAG_VITAL, m_ClientID);	
	}
}
// Парсинг F3 или F4 всех Приглашений
bool CPlayer::ParseInteractive(int Vote)
{
	if(ParseItemsF3F4(Vote) || CGS::Interactive[m_ClientID].ParsingLifeTick < 0)
		return false;

	const int SaveCID = CGS::Interactive[m_ClientID].ParsingClientID;
	CPlayer *pSavePlayer = GS()->m_apPlayers[SaveCID];
	if(!pSavePlayer)
		return false;

	// проверяем если нажата F3
	if(Vote == 1)
	{
		if(!str_comp(CGS::Interactive[m_ClientID].ParsingType, "Deal"))
		{
			const int Price = CGS::Interactive[m_ClientID].ParsingSaveInt;
			GS()->Mmo()->House()->SellToHouse(SaveCID, m_ClientID, Price);
		}
		else if(!str_comp(CGS::Interactive[m_ClientID].ParsingType, "Member"))
		{
			const int GuildID = CGS::Interactive[m_ClientID].ParsingSaveInt;
			GS()->Mmo()->Member()->JoinGuild(Acc().AuthID, GuildID);
		}
		ClearParsing(true);
		return true;
	}
	else // если игрок нажал F4
	{
		GS()->Chat(m_ClientID, "You refused the request player {STR}", Server()->ClientName(SaveCID));
		GS()->Chat(SaveCID, "Player {STR} refused the request", Server()->ClientName(m_ClientID));
		ClearParsing(true);
		return true;
	}
	return false;
}

bool CPlayer::ParseItemsF3F4(int Vote)
{
	if (!m_pCharacter)
	{
		GS()->Chat(m_ClientID, "Use it when you're not dead!");
		return true;
	}

	// - - - - - - - - - - - - -
	// - - - - - F3- - - - - - -
	if (Vote == 1)
	{
		return false;
	}

	// - - - - - - - - - - - - -
	// - - - - - F4- - - - - - -
	// смена режима полета
	if(m_PlayerFlags&PLAYERFLAG_SCOREBOARD && GetItemEquip(EQUIP_WINGS) > 0)
	{
		m_Flymode ^= true;
		GS()->Chat(m_ClientID, "You {STR} fly mode, your hook changes!", m_Flymode ? "Enable" : "Disable");
		return true;
	}

	// общение на диалогах для ванильных клиентов
	if (GetTalkedID() > 0 && !GS()->CheckClient(m_ClientID))
	{
		if (m_PlayerTick[TickState::LastDialog] && m_PlayerTick[TickState::LastDialog] > GS()->Server()->Tick())
			return true;

		m_PlayerTick[TickState::LastDialog] = GS()->Server()->Tick() + (GS()->Server()->TickSpeed() / 4);
		SetTalking(GetTalkedID(), true);
		return true;
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
	ItemJob::Items[m_ClientID][ItemID].SetBasic(this, ItemID);
	return ItemJob::Items[m_ClientID][ItemID]; 
}

// Получить одетый предмет
int CPlayer::GetItemEquip(int EquipID, int SkipItemID) const
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
int CPlayer::GetAttributeCount(int BonusID, bool Really)
{
	if (CGS::AttributInfo.find(BonusID) == CGS::AttributInfo.end()) 
		return 0;

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
	if (GS()->IsDungeon() && CGS::AttributInfo[BonusID].UpgradePrice < 10)
	{
		int NewStat = 0;
		if (AttributEx > 0)
		{
			NewStat = (int)((float)m_SyncDuneon / 25.0f) + ((float)AttributEx / 25.0f);
			if (m_MoodState == MOOD_PLAYER_TANK && BonusID == Stats::StHardness)
				NewStat *= 2;
		}
		if(AttributEx > NewStat)
			AttributEx = NewStat;
	}
	return AttributEx;
}

// Получить уровень Классов по атрибутам
int CPlayer::GetLevelDisciple(int Class)
{
	int Atributs = 0;
	for (const auto& at : CGS::AttributInfo)
	{
		if (at.second.AtType == Class)
			Atributs += GetAttributeCount(at.first, true);
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
	int MobID = BotPlayer->GetBotSub();
	if (BotPlayer->GetBotType() == BotsTypes::TYPE_BOT_NPC)
	{
		int sizeTalking = BotJob::NpcBot[MobID].m_Talk.size();
		if (m_TalkingNPC.m_TalkedProgress >= sizeTalking)
		{
			ClearTalking();
			GS()->ClearTalkText(m_ClientID);
			return;
		}

		int GivingQuestID = GS()->Mmo()->BotsData()->IsGiveQuestNPC(MobID);
		if (GS()->Mmo()->Quest()->GetState(m_ClientID, GivingQuestID) >= QuestState::QUEST_ACCEPT)
		{
			const char* pTalking[2] = { "[Player], do you have any questions? I'm sorry I can't help you.", 
										"What a beautiful [Time], we already talked. I don't have anything for you [Player]." };
			GS()->Mmo()->BotsData()->TalkingBotNPC(this, MobID, m_TalkingNPC.m_TalkedProgress, TalkedID, pTalking[random_int()%2]);
			m_TalkingNPC.m_TalkedProgress = 999;
			return;
		}

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
		int sizeTalking = BotJob::QuestBot[MobID].m_Talk.size();
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
void CPlayer::FormatTextQuest(int DataBotID, const char *pText)
{
	if(!GS()->Mmo()->BotsData()->IsDataBotValid(DataBotID) || m_FormatTalkQuest[0] != '\0') 
		return;

	str_copy(m_FormatTalkQuest, pText, sizeof(m_FormatTalkQuest));
	str_replace(m_FormatTalkQuest, "[Player]", GS()->Server()->ClientName(m_ClientID));
	str_replace(m_FormatTalkQuest, "[Talked]", BotJob::DataBot[DataBotID].NameBot);
	str_replace(m_FormatTalkQuest, "[Time]", GS()->Server()->GetStringTypeDay());
}
void CPlayer::ClearFormatQuestText()
{
	mem_zero(m_FormatTalkQuest, sizeof(m_FormatTalkQuest));
}

// another need optimize
int CPlayer::GetMoodState()
{
	if (!GS()->IsDungeon())
		return MOOD_NORMAL;

	int MaximalHealth = GetAttributeCount(Stats::StHardness, true);
	for (int i = 0; i < MAX_PLAYERS; i++)
	{
		CPlayer* pPlayer = GS()->m_apPlayers[i];
		if (!pPlayer || Server()->GetWorldID(m_ClientID) != Server()->GetWorldID(i))
			continue;

		int FinderHardness = pPlayer->GetAttributeCount(Stats::StHardness, true);
		if (FinderHardness > MaximalHealth)
			return MOOD_NORMAL;
	}
	return MOOD_PLAYER_TANK;
}

void CPlayer::ChangeWorld(int WorldID)
{
	if(m_pCharacter)
	{
		GS()->m_World.DestroyEntity(m_pCharacter);
		GS()->m_World.m_Core.m_apCharacters[m_ClientID] = 0;
	}
	Server()->ChangeWorld(m_ClientID, WorldID);
}