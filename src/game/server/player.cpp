/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#include <engine/shared/config.h>

#include "gamecontext.h"
#include "player.h"

MACRO_ALLOC_POOL_ID_IMPL(CPlayer, MAX_CLIENTS*COUNT_WORLD+MAX_CLIENTS)

CPlayer::CPlayer(CGS *pGS, int ClientID) : m_pGS(pGS), m_ClientID(ClientID)
{
	for(int i = 0 ; i < NUMTABSORT ; i ++ )
		m_SortTabs[ i ] = 0;

	m_PlayerTick[TickState::CheckClient] = Server()->Tick();
	m_PlayerTick[TickState::Respawn] = Server()->Tick();
	m_PlayerTick[TickState::Die] = Server()->Tick();
	m_ShowHealthBroadcast = false;

	m_Spawned = true;
	m_LastVoteMenu = NOPE;
	m_OpenVoteMenu = MAINMENU;

	m_PrevTuningParams = *pGS->Tuning();
	m_NextTuningParams = m_PrevTuningParams;

	ClearParsing();

	Acc().Team = GetStartTeam();
	if(Acc().AuthID > 0)
		GS()->Mmo()->Account()->LoadAccount(this, false);
	
	SetLanguage(Server()->GetClientLanguage(ClientID));
}

CPlayer::~CPlayer()
{
	delete m_pCharacter;
	m_pCharacter = NULL;
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
		m_pCharacter = NULL;
	}

	if (m_pCharacter)
	{
		if (m_pCharacter->IsAlive())
			m_ViewPos = m_pCharacter->GetPos();
	}
	else if (m_Spawned && m_PlayerTick[TickState::Respawn] + Server()->TickSpeed() * 3 <= Server()->Tick())
		TryRespawn();

	if (m_pCharacter && m_pCharacter->IsAlive())
	{
		// проверка всех зелий и действия раз в секунду
		if (Server()->Tick() % Server()->TickSpeed() == 0)
		{
			for (auto ieffect = CGS::Effects[m_ClientID].begin(); ieffect != CGS::Effects[m_ClientID].end();)
			{
				if (!str_comp(ieffect->first.c_str(), "Poison"))
					m_pCharacter->TakeDamage(vec2(0, 0), vec2(0, 0), 1, m_ClientID, WEAPON_SELF);

				if (!str_comp(ieffect->first.c_str(), "RegenHealth"))
					m_pCharacter->IncreaseHealth(15);

				ieffect->second--;
				if (ieffect->second <= 0)
				{
					GS()->SendMmoPotion(m_pCharacter->m_Core.m_Pos, ieffect->first.c_str(), false);
					ieffect = CGS::Effects[m_ClientID].erase(ieffect);
				}
				else ieffect++;
			}
		}
	}

	HandleTuningParams();
	TickOnlinePlayer();
}

// Пост тик
void CPlayer::PostTick()
{
	// update latency value
	if (Server()->ClientIngame(m_ClientID) && GS()->IsClientEqualWorldID(m_ClientID) && IsAuthed())
		Acc().LatencyPing = m_Latency.m_Min;
}

// Тик авторизированного в ::Tick
void CPlayer::TickOnlinePlayer()
{
	// показать лист броадкаст
	if(pValueBroadcast > 0 && m_pCharacter && m_pCharacter->IsAlive())
	{
		m_ShowHealthBroadcast = false;
		GS()->SBL(m_ClientID, PRENORMAL+pValueBroadcast, 80, Broadcast.buffer());
		pValueBroadcast = 0;
		Broadcast.clear();
	}

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
	if(TalkedID >= MAX_CLIENTS || TalkedID < MAX_PLAYERS || !GS()->m_apPlayers[TalkedID] ||
		distance(m_ViewPos, GS()->m_apPlayers[TalkedID]->m_ViewPos) > 90.0f)
	{
		ClearTalking();
	}
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

	pPlayerInfo->m_Latency = (SnappingClient == -1 ? m_Latency.m_Min : Acc().LatencyPing);
	pPlayerInfo->m_Score = Acc().Level;

	// --------------------- CUSTOM ----------------------
	// ---------------------------------------------------
	if(!GS()->CheckClient(SnappingClient) || GetTeam() == TEAM_SPECTATORS || !IsAuthed())
		return;

	CNetObj_Mmo_ClientInfo *pClientInfo = static_cast<CNetObj_Mmo_ClientInfo *>(Server()->SnapNewItem(NETOBJTYPE_MMO_CLIENTINFO, m_ClientID, sizeof(CNetObj_Mmo_ClientInfo)));
	if(!pClientInfo)
		return;

	bool local_ClientID = (m_ClientID == SnappingClient);
	pClientInfo->m_Local = local_ClientID;
	pClientInfo->m_WorldType = GS()->Mmo()->WorldSwap()->GetWorldType();
	pClientInfo->m_MoodType = MOOD_NORMAL;
	pClientInfo->m_Level = Acc().Level;
	pClientInfo->m_Exp = Acc().Exp;
	pClientInfo->m_ExpNeed = ExpNeed(Acc().Level);
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
		StrToInts(pClientInfo->m_Guildname, 12, GS()->Mmo()->Member()->GuildName(Acc().GuildID));
	else
		StrToInts(pClientInfo->m_Guildname, 12, "\0");
}

// Получить черактера игрока
CCharacter *CPlayer::GetCharacter()
{
	if(m_pCharacter && m_pCharacter->IsAlive())
		return m_pCharacter;
	return 0;
}

// Установить команду игроку
void CPlayer::SetTeam(int Team, bool DoChatMsg)
{
	KillCharacter();

	Acc().Team = Team;
	m_PlayerTick[TickState::Respawn] = Server()->Tick()+Server()->TickSpeed()*2;
}

// Спавн игрока
void CPlayer::TryRespawn()
{
	// сам спавн
	vec2 SpawnPos;
	if(!GS()->m_pController->CanSpawn(TEAM_RED, &SpawnPos, vec2(-1, -1)))
		return;

	// спавн в организации
	const int CheckGuildSpawn = GS()->Mmo()->Member()->GetGuildHouseID(Acc().GuildID);
	if(CheckGuildSpawn > 0)
	{
		const int WorldID = GS()->Mmo()->Member()->GetHouseWorldID(CheckGuildSpawn);
		if(GS()->IsClientEqualWorldID(m_ClientID, WorldID))
			SpawnPos = GS()->Mmo()->Member()->GetPositionHouse(Acc().GuildID);
	}

	// спавн в доме
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

	// спавн в точках что указаны
	if(Acc().TeleportX > 1 || Acc().TeleportY > 1)
		SpawnPos = vec2(Acc().TeleportX, Acc().TeleportY);

	int savecidmem = MAX_CLIENTS*GS()->GetWorldID()+m_ClientID;
	m_pCharacter = new(savecidmem) CCharacter(&GS()->m_World);
	m_pCharacter->Spawn(this, SpawnPos);
	GS()->CreatePlayerSpawn(SpawnPos);
	m_Spawned = false;
}

// Убить Character игрока и очистить
void CPlayer::KillCharacter(int Weapon)
{
	if(m_pCharacter)
	{
		m_pCharacter->Die(m_ClientID, Weapon);
		delete m_pCharacter;
		m_pCharacter = NULL;
	}
}

// Отключение игрока
void CPlayer::OnDisconnect()
{
	KillCharacter();
}

// Отправка и установка Input
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
// Отправка и установка пред Input
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
	FUNCTIONS PLAYER BOTS 
######################################################################### */


/* #########################################################################
	FUNCTIONS PLAYER HELPER 
######################################################################### */
// Добавить броадкаст в общий вывод
void CPlayer::AddInBroadcast(const char *pBuffer)
{
	const char *pSaveBuf = Broadcast.buffer();
	const unsigned sizes = sizeof(pBuffer) + sizeof(pSaveBuf);
	if(sizes < 256 && pValueBroadcast < 3)
	{
		pValueBroadcast++;
		Broadcast.append(pBuffer);
	}
	return;
}
// Прогресс бар информация о уровнях
void CPlayer::ProgressBar(const char *Name, int MyLevel, int MyExp, int ExpNeed, int GivedExp)
{
	int NeedXp = ExpNeed;
	float getlv = ( MyExp * 100.0 ) / NeedXp;
	float getexp = ( GivedExp * 100.0 ) / NeedXp;

	const char *Level = GS()->LevelString(100, (int)getlv, 10, ':', ' ');
	char BufferInBroadcast[128];
	str_format(BufferInBroadcast, sizeof(BufferInBroadcast), "^235Lv%d %s%s %0.2f%%+%0.3f%%(%d)XP\n", MyLevel, Name, Level, getlv, getexp, GivedExp);
	AddInBroadcast(BufferInBroadcast);
	delete [] Level;
}
// Улучшения апгрейдов любых не зависимо от структуры класса или переменных
bool CPlayer::Upgrade(int Count, int *Upgrade, int *Useless, int Price, int MaximalUpgrade, const char *UpgradeName)
{
	int UpgradeNeed = Price*Count;
	if((*Upgrade + Count) > MaximalUpgrade)
	{
		GS()->SBL(m_ClientID, PRELEGENDARY, 100, "This upgrades maximal.");
		return false;		
	}

	if(*Useless < UpgradeNeed)
	{
		GS()->SBL(m_ClientID, PRELEGENDARY, 100, "Have no upgrade points for upgrade +{INT}. Need {INT}.", &Count, &UpgradeNeed);
		return false;
	}

	*Useless -= UpgradeNeed;
	*Upgrade += Count;
	return true;
}

// Именна бонусов статистик
const char *CPlayer::AtributeName(int BonusID) const
{
	for(const auto& at : CGS::AttributInfo)
	{
		if(at.first != BonusID) 
			continue;
			
		return at.second.Name;
	}
	return "Has no stats";
}

/* #########################################################################
	FUNCTIONS PLAYER ACCOUNT 
######################################################################### */
// Проверить и снять деньги
bool CPlayer::CheckFailMoney(int Price, int ItemID)
{
	ItemSql::ItemPlayer &CoinItem = GetItem(ItemID);
	if(CoinItem.Count < Price)
	{
		GS()->Chat(m_ClientID,"Sorry, need {INT} but you have in your pocket/backpack only {INT} {STR}!", &Price, &CoinItem.Count, CoinItem.Info().GetName(this), NULL);
		return true;
	}

	if(!CoinItem.Remove(Price)) 
		return true;

	return false;
}
// Наложить на игрока 'Эффект зелий'
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
// Установить язык
void CPlayer::SetLanguage(const char* pLanguage)
{
	str_copy(m_aLanguage, pLanguage, sizeof(m_aLanguage));
}
// Установка здоровья и маны
void CPlayer::SetStandart(int Health, int Mana)
{
	Acc().PlayerHealth = Health;
	Acc().PlayerMana = Mana;
}
// Дать опыт игроку
void CPlayer::AddExp(int Exp)
{
	Acc().Exp += Exp;
	for( ; Acc().Exp >= ExpNeed(Acc().Level); ) 
	{
		Acc().Exp -= ExpNeed(Acc().Level), Acc().Level++;
		Acc().Upgrade += 6;

		GS()->CreateDeath(m_pCharacter->m_Core.m_Pos, m_ClientID);
		GS()->CreateSound(m_pCharacter->m_Core.m_Pos, 4);
		GS()->CreateText(m_pCharacter, false, vec2(0, -40), vec2(0, -1), 30, "level up", Server()->GetWorldID(m_ClientID));
		GS()->ChatFollow(m_ClientID, "Level UP. Now Level {INT}!", &Acc().Level);

		// если это последний уровень повышения
		if(Acc().Exp < ExpNeed(Acc().Level))
		{
			GS()->VResetVotes(m_ClientID, MAINMENU);
			GS()->Mmo()->SaveAccount(this, SAVESTATS);
			GS()->Mmo()->SaveAccount(this, SAVEUPGRADES);
		}
	}
	ProgressBar("Account", Acc().Level, Acc().Exp, ExpNeed(Acc().Level), Exp);
	if(rand()%5 == 0) GS()->Mmo()->SaveAccount(this, SAVESTATS);

	if(Acc().IsGuild())
		GS()->Mmo()->Member()->AddExperience(Acc().GuildID);
}

// Дать деньги игроку
void CPlayer::AddMoney(int Money) 
{ 
	GetItem(itMoney).Add(Money); 
}

// Проверить эффект зелья имеется или нет
bool CPlayer::CheckEffect(const char* Potion)
{
	if(CGS::Effects[m_ClientID].find(Potion) != CGS::Effects[m_ClientID].end())
		return true;
	return false;
}
// Вернуть скрытое меню
bool CPlayer::GetHidenMenu(int HideID) const
{
	if(m_HidenMenu.find(HideID) != m_HidenMenu.end())
		return m_HidenMenu.at(HideID);
	return false;
}
// Если авторизован
bool CPlayer::IsAuthed()
{ 
	if(GS()->Mmo()->Account()->IsActive(m_ClientID))
		return Acc().AuthID; 
	return false; 
}

// Уровень зачарованных вещей Атрибутов
int CPlayer::EnchantAttributes(int BonusID) const
{
	int BonusAttributes = 0;
	for (const auto& it : ItemSql::Items[m_ClientID])
	{
		if(it.second.Info().BonusID != BonusID || it.second.Count <= 0 || it.second.Settings <= 0) continue;
		
		// если предмет поврежден
		int BonusCount = it.second.Info().BonusCount*(it.second.Enchant+1);
		BonusAttributes += BonusCount;
	}
	return BonusAttributes;
}

// Начальная команда
int CPlayer::GetStartTeam()
{
	if(Acc().AuthID)
		return TEAM_RED;

	return TEAM_SPECTATORS;
}
// Нужно опыта для повышения уровня
int CPlayer::ExpNeed(int Level)
{
	return (g_Config.m_SvExpForLevel+Level*2)*(Level*Level);
}
// стартовое здоровье
int CPlayer::GetStartHealth()
{
	return 10 + GetAttributeCount(Stats::StHardness, true);
}
// стартовая мана
int CPlayer::GetStartMana()
{
	int EnchantBonus = GetAttributeCount(Stats::StPiety, true);
	return 10 + EnchantBonus;
}
// язык игрока
const char* CPlayer::GetLanguage()
{
	return m_aLanguage;
}
// Добавить статистику в broadcast
void CPlayer::AddInformationStats()
{
	if(m_ShowHealthBroadcast)
		return;

	char aBuf[64];
	str_format(aBuf, sizeof(aBuf), "^151H: %d/%d ^115M: %d/%d\n", GetHealth(), GetStartHealth(), m_pCharacter->Mana(), GetStartMana());
	AddInBroadcast(aBuf);
	m_ShowHealthBroadcast = true;
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
		Msg.m_Type = VotePass ? VOTE_END_PASS : VOTE_END_FAIL;
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
		if(Upgrade(Get, &Acc().Stats[VoteID], &Acc().Upgrade, VoteID2, 1000, AtributeName(VoteID))) 
		{
			GS()->Mmo()->SaveAccount(this, SAVEUPGRADES);
			GS()->ResetVotes(m_ClientID, UPGRADES);
		}
		return true;
	}

	if(PPSTR(CMD, "HIDEN") == 0)
	{
		if(VoteID < HSTAT)
			return true;

		for(auto& x : m_HidenMenu) {
			if(m_HidenMenu.find(x.first) != m_HidenMenu.end() && (x.first > NUMHIDEMENU && x.first != VoteID))
				m_HidenMenu[x.first] = false; 
			}

		m_HidenMenu[VoteID] ^= true;
		if(m_HidenMenu[VoteID] == false)
			m_HidenMenu.erase(VoteID);

		GS()->ResetVotes(m_ClientID, m_OpenVoteMenu);
		return true;
	}
	return false;
}

/* #########################################################################
	FUNCTIONS PLAYER ITEMS 
######################################################################### */

int CPlayer::GetItemDurability(int Item) const { return ItemSql::Items[m_ClientID][Item].Durability; }

ItemSql::ItemPlayer &CPlayer::GetItem(int ItemID) 
{
	ItemSql::Items[m_ClientID][ItemID].SetBasic(this, ItemID);
	return ItemSql::Items[m_ClientID][ItemID]; 
}

// Получить одетый предмет
int CPlayer::GetItemEquip(int EquipID, int SkipItemID) const
{
	for(const auto& it : ItemSql::Items[m_ClientID])
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
	// если бот то возращаем в зависимости от обьема установленного здоровья
	if (CGS::AttributInfo.find(BonusID) == CGS::AttributInfo.end()) return 0;

	// обычная передача если нет сохранения и нет процентов
	int AttributEx = EnchantAttributes(BonusID);
	const int ProcentID = CGS::AttributInfo[BonusID].ProcentID;
	const bool SaveData = (str_comp_nocase(CGS::AttributInfo[BonusID].FieldName, "unfield") != 0);

	// если есть процент прибавляем
	if (ProcentID > 0)
	{
		const int ProcentBonus = EnchantAttributes(ProcentID);
		const int ProcentSize = round_to_int(kurosio::translate_to_procent_rest(AttributEx, ProcentBonus));
		AttributEx += ProcentSize;
	}
	// если есть локальные данные то добавляем
	if (SaveData) { AttributEx += Acc().Stats[BonusID]; }

	// если реальная стата то делил
	if (Really && CGS::AttributInfo[BonusID].UpgradePrice < 10) { AttributEx /= (int)(BonusID == Stats::StStrength ? 10 : 5); }
	return AttributEx;
}

// Получить уровень Классов по атрибутам
int CPlayer::GetLevelDisciple(int Class)
{
	int Atributs = 0;
	for (const auto& at : CGS::AttributInfo)
	{
		if (at.second.AtType != Class) continue;
		Atributs += GetAttributeCount(at.first);
	}
	return Atributs;
}



// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
// - - - - - - V V V V V V V - - - - V V V V - - - - - - - - - 
// - - - - - - T A L K I N G - - - - B O T S - - - - - - - - - 
// - - - - - - V V V V V V V - - - - V V V V - - - - - - - - - 
void CPlayer::SetTalking(int TalkedID, bool ToProgress)
{
	if (TalkedID < MAX_PLAYERS || !GS()->m_apPlayers[TalkedID] || (!ToProgress && m_TalkingNPC.m_TalkedID != -1) || (ToProgress && m_TalkingNPC.m_TalkedID == -1))
		return;

	m_TalkingNPC.m_TalkedID = TalkedID;
	CPlayerBot* BotPlayer = static_cast<CPlayerBot*>(GS()->m_apPlayers[TalkedID]);
	int MobID = BotPlayer->GetBotSub();
	if (BotPlayer->GetSpawnBot() == SPAWNNPC)
	{
		int sizeTalking = ContextBots::NpcBot[MobID].m_Talk.size();
		if (m_TalkingNPC.m_TalkedProgress >= sizeTalking)
		{
			GS()->ClearTalkText(m_ClientID);
			return;
		}

		// если прогресс диалога равен какому то квесту
		int GivingQuestID = ContextBots::NpcBot[MobID].m_Talk[m_TalkingNPC.m_TalkedProgress].m_GivingQuest;
		if (GivingQuestID >= 1)
		{
			// замораживаем при информации и принятии квеста
			if (!m_TalkingNPC.m_FreezedProgress)
			{
				if (GS()->Mmo()->Quest()->GetQuestState(m_ClientID, GivingQuestID) >= QUESTACCEPT)
					GS()->Mmo()->BotsData()->TalkingBotNPC(this, MobID, m_TalkingNPC.m_TalkedProgress, TalkedID, "I'm sorry, don't have more new stories for you!");
				else
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

	else if (BotPlayer->GetSpawnBot() == SPAWNQUESTNPC)
	{
		int sizeTalking = ContextBots::QuestBot[MobID].m_Talk.size();
		if (m_TalkingNPC.m_TalkedProgress >= sizeTalking)
		{
			GS()->Mmo()->Quest()->InteractiveQuestNPC(this, ContextBots::QuestBot[MobID], true);
			GS()->ClearTalkText(m_ClientID);
			return;
		}

		GS()->Mmo()->Quest()->QuestTableClear(m_ClientID);
		bool RequiestQuestTask = ContextBots::QuestBot[MobID].m_Talk[m_TalkingNPC.m_TalkedProgress].m_RequestComplete;
		if (RequiestQuestTask)
		{
			GS()->Mmo()->Quest()->CreateQuestingItems(this, ContextBots::QuestBot[MobID]);
			if (!m_TalkingNPC.m_FreezedProgress)
			{
				GS()->Mmo()->BotsData()->TalkingBotQuest(this, MobID, m_TalkingNPC.m_TalkedProgress, TalkedID);
				GS()->Mmo()->BotsData()->ShowBotQuestTaskInfo(this, MobID, m_TalkingNPC.m_TalkedProgress);
				m_TalkingNPC.m_FreezedProgress = true;
				return;
			}

			// skip non complete dialog quest
			if (!GS()->Mmo()->Quest()->InteractiveQuestNPC(this, ContextBots::QuestBot[MobID], false))
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
	m_TalkingNPC.m_TalkedID = -1;
	m_TalkingNPC.m_TalkedProgress = 0;
	m_TalkingNPC.m_FreezedProgress = false;
	GS()->SendTalkText(m_ClientID, -1, 0, "\0");
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
// - - - - - - V V V V V V - - - - - V V V V - - - - - - - - - 
// - - - - - - F O R M A T - - - - - T E X T - - - - - - - - - 
// - - - - - - V V V V V V - - - - - V V V V - - - - - - - - - 
void CPlayer::FormatTextQuest(int DataBotID, const char *pText)
{
	if(!GS()->Mmo()->BotsData()->IsDataBotValid(DataBotID) || m_FormatTalkQuest[0] != '\0') 
		return;

	// формат текста под вид квестов
	str_copy(m_FormatTalkQuest, pText, sizeof(m_FormatTalkQuest));
	str_replace(m_FormatTalkQuest, "[Player]", GS()->Server()->ClientName(m_ClientID));
	str_replace(m_FormatTalkQuest, "[Talked]", ContextBots::DataBot[DataBotID].NameBot);
	str_replace(m_FormatTalkQuest, "[Time]", GS()->Server()->GetStringTypeDay());
}
void CPlayer::ClearFormatQuestText()
{
	mem_zero(m_FormatTalkQuest, sizeof(m_FormatTalkQuest));
}
