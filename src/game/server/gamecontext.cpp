/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#include <base/math.h>
#include <thread>
#include <algorithm>

#include <engine/map.h>
#include <engine/shared/config.h>
#include <engine/shared/memheap.h>

#include <generated/server_data.h>
#include <game/version.h>
#include "gamecontext.h"

#include <game/collision.h>
#include <game/gamecore.h>
#include "gamemodes/main.h"
#include "gamemodes/dungeon.h"

#include "mmocore/GameEntities/Items/drop_bonuses.h"
#include "mmocore/GameEntities/Items/drop_items.h"
#include "mmocore/GameEntities/loltext.h"
#include "mmocore/CommandProcessor.h"
#include "mmocore/PathFinder.h"

#include <teeother/components/localization.h>

std::map < int , CGS::StructAttribut > CGS::AttributInfo;
std::map < int , std::map < std::string , int > > CGS::Effects;
int CGS::m_RaidExp = 100;

enum
{
	RESET,
	NO_RESET
};

void CGS::Construct(int Resetting)
{
	for (auto & apPlayer : m_apPlayers)
		apPlayer = nullptr;

	m_Resetting = false;
	m_pServer = nullptr;
	m_pController = nullptr;

	if(Resetting==NO_RESET)
		pMmoController = nullptr;
}

CGS::CGS(int Resetting)
{
	Construct(Resetting);
}

CGS::CGS()
{
	Construct(NO_RESET);
}

CGS::~CGS()
{
	for(auto & apPlayer : m_apPlayers)
		delete apPlayer;
	if(pMmoController)  
		delete pMmoController;
	if(m_pPathFinder)
		delete m_pPathFinder;
}

void CGS::Clear()
{
	CTuningParams Tuning = m_Tuning;         
	m_Resetting = true;
	this->~CGS();
	mem_zero(this, sizeof(*this));
	new (this) CGS(RESET);

	for(auto & BroadcastState : m_BroadcastStates)
	{
		BroadcastState.m_NoChangeTick = 0;
		BroadcastState.m_LifeSpanTick = 0;
		BroadcastState.m_Priority = 0;
		BroadcastState.m_PrevMessage[0] = 0;
		BroadcastState.m_NextMessage[0] = 0;
	}
	m_Tuning = Tuning;
}

class CCharacter *CGS::GetPlayerChar(int ClientID)
{
	if(ClientID < 0 || ClientID >= MAX_CLIENTS || !m_apPlayers[ClientID])
		return nullptr;
	return m_apPlayers[ClientID]->GetCharacter();
}

// Легкое получение игрока (ClientID, Авторизации, Чарактора)
CPlayer *CGS::GetPlayer(int ClientID, bool CheckAuthed, bool CheckCharacter)
{
	if(ClientID < 0 || ClientID >= MAX_CLIENTS || !m_apPlayers[ClientID])
		return nullptr;

	CPlayer *pPlayer = m_apPlayers[ClientID];
	if((CheckAuthed && pPlayer->IsAuthed()) || !CheckAuthed)
	{
		if(CheckCharacter && !pPlayer->GetCharacter())
			return nullptr;
		return pPlayer;	
	}
	return nullptr;
}

// Level String by Matodor (Progress Bar) создает что то рода прогресс бара
char* CGS::LevelString(int MaxValue, int CurrentValue, int Step, char toValue, char fromValue) 
{
    if (CurrentValue < 0) CurrentValue = 0;
    if (CurrentValue > MaxValue) CurrentValue = MaxValue;

    int Size = 3 + MaxValue / Step;
    char *Buf = new char[Size];
    Buf[0] = '[';
    Buf[Size - 2] = ']';
    Buf[Size - 1] = '\0';

    int a = CurrentValue / Step;
    int b = (MaxValue - CurrentValue) / Step;
    int i = 1;

	for (int ai = 0; ai < a; ai++, i++)
		Buf[i] = toValue;
	for (int bi = 0; bi < b || i < Size - 2; bi++, i++)
		Buf[i] = fromValue;
	return Buf;
}

const char* CGS::GetSymbolHandleMenu(int ClientID, bool HidenTabs, int ID) const
{
	if(CheckClient(ClientID))
	{
		if(HidenTabs)
			return ID >= NUM_TAB_MENU ? ("▵ ") : (ID < NUM_TAB_MENU_INTERACTIVES ? ("▼ :: ") : ("▲ :: "));
		return ID >= NUM_TAB_MENU ? ("▿ ") : (ID < NUM_TAB_MENU_INTERACTIVES ? ("▲ :: ") : ("▼ :: "));
	}
	if(HidenTabs)
		return ID >= NUM_TAB_MENU ? ("/\\ # ") : (ID < NUM_TAB_MENU_INTERACTIVES ? ("\\/ # ") : ("/\\ # "));
	return ID >= NUM_TAB_MENU ? ("\\/ # ") : (ID < NUM_TAB_MENU_INTERACTIVES ? ("/\\ # ") : ("\\/ # "));
}

// получить информацию предмета
ItemJob::ItemInformation &CGS::GetItemInfo(int ItemID) const { return ItemJob::ItemsInfo[ItemID]; }

/* #########################################################################
	EVENTS 
######################################################################### */
// Отправить запрос на рендер Урона
void CGS::CreateDamage(vec2 Pos, int ClientID, int Amount, bool OnlyVanilla)
{
	CNetEvent_Damage* pEventVanilla = (CNetEvent_Damage*)m_Events.Create(NETEVENTTYPE_DAMAGE, sizeof(CNetEvent_Damage));
	if(pEventVanilla)
	{
		int AmountDamageVanilla = Amount;
		if(ClientID >= 0 && ClientID < MAX_CLIENTS && m_apPlayers[ClientID])
		{
			CPlayer* pPlayer = m_apPlayers[ClientID];
			const int HealthStart = pPlayer->GetStartHealth();
			const float DamageTranslate = (float)Amount / (float)HealthStart * 6.0f;
			AmountDamageVanilla = clamp((int)DamageTranslate, 1, 6);
		}

		pEventVanilla->m_X = (int)Pos.x;
		pEventVanilla->m_Y = (int)Pos.y;
		pEventVanilla->m_ClientID = ClientID;
		pEventVanilla->m_Angle = 0;
		pEventVanilla->m_HealthAmount = AmountDamageVanilla;
		pEventVanilla->m_ArmorAmount = 0;
		pEventVanilla->m_Self = 0;
	}

	if(OnlyVanilla)
		return;

	CNetEvent_MmoDamage* pEventMmo = (CNetEvent_MmoDamage*)m_Events.Create(NETEVENTTYPE_MMODAMAGE, sizeof(CNetEvent_MmoDamage));
	if(pEventMmo)
	{
		pEventMmo->m_X = (int)Pos.x;
		pEventMmo->m_Y = (int)Pos.y;
		pEventMmo->m_ClientID = ClientID;
		pEventMmo->m_DamageCount = Amount;
	}
}

// Отправить запрос на рендер Удара молотка
void CGS::CreateHammerHit(vec2 Pos)
{
	CNetEvent_HammerHit *pEvent = (CNetEvent_HammerHit *)m_Events.Create(NETEVENTTYPE_HAMMERHIT, sizeof(CNetEvent_HammerHit));
	if(pEvent)
	{
		pEvent->m_X = (int)Pos.x;
		pEvent->m_Y = (int)Pos.y;
	}
}

// Отправить запрос на рендер Взрыв и Сделать урон в радиусе
void CGS::CreateExplosion(vec2 Pos, int Owner, int Weapon, int MaxDamage)
{
	// create the event
	CNetEvent_Explosion *pEvent = (CNetEvent_Explosion *)m_Events.Create(NETEVENTTYPE_EXPLOSION, sizeof(CNetEvent_Explosion));
	if(pEvent)
	{
		pEvent->m_X = (int)Pos.x;
		pEvent->m_Y = (int)Pos.y;
	}

	CCharacter *apEnts[MAX_CLIENTS];
	float Radius = g_pData->m_Explosion.m_Radius;
	float InnerRadius = 48.0f;
	float MaxForce = g_pData->m_Explosion.m_MaxForce;
	int Num = m_World.FindEntities(Pos, Radius, (CEntity**)apEnts, MAX_CLIENTS, CGameWorld::ENTTYPE_CHARACTER);
	for(int i = 0; i < Num; i++)
	{
		vec2 Diff = apEnts[i]->GetPos() - Pos;
		vec2 Force(0, MaxForce);
		float l = length(Diff);
		if(l)
			Force = normalize(Diff) * MaxForce;
		float Factor = 1 - clamp((l-InnerRadius)/(Radius-InnerRadius), 0.0f, 1.0f);
		if ((int)(Factor * MaxDamage))
		{
			apEnts[i]->TakeDamage(Force * Factor, (int)(Factor * MaxDamage), Owner, Weapon);
		}
	}
}

void CGS::CreatePlayerSpawn(vec2 Pos)
{
	CNetEvent_Spawn *ev = (CNetEvent_Spawn *)m_Events.Create(NETEVENTTYPE_SPAWN, sizeof(CNetEvent_Spawn));
	if(ev)
	{
		ev->m_X = (int)Pos.x;
		ev->m_Y = (int)Pos.y;
	}
}

void CGS::CreateDeath(vec2 Pos, int ClientID)
{
	CNetEvent_Death *pEvent = (CNetEvent_Death *)m_Events.Create(NETEVENTTYPE_DEATH, sizeof(CNetEvent_Death));
	if(pEvent)
	{
		pEvent->m_X = (int)Pos.x;
		pEvent->m_Y = (int)Pos.y;
		pEvent->m_ClientID = ClientID;
	}
}

void CGS::CreateSound(vec2 Pos, int Sound, int64 Mask)
{
	// fix for vanilla unterstand SoundID
	if(Sound < 0 || Sound > 40)
		return;

	CNetEvent_SoundWorld *pEvent = (CNetEvent_SoundWorld *)m_Events.Create(NETEVENTTYPE_SOUNDWORLD, sizeof(CNetEvent_SoundWorld), Mask);
	if(pEvent)
	{
		pEvent->m_X = (int)Pos.x;
		pEvent->m_Y = (int)Pos.y;
		pEvent->m_SoundID = Sound;
	}
}

void CGS::CreatePlayerSound(int ClientID, int Sound)
{
	// fix for vanilla unterstand SoundID
	if(!m_apPlayers[ClientID] || (!CheckClient(ClientID) && (Sound < 0 || Sound > 40)))
		return;

	CNetEvent_SoundWorld* pEvent = (CNetEvent_SoundWorld*)m_Events.Create(NETEVENTTYPE_SOUNDWORLD, sizeof(CNetEvent_SoundWorld), CmaskOne(ClientID));
	if(pEvent)
	{
		pEvent->m_X = (int)m_apPlayers[ClientID]->m_ViewPos.x;
		pEvent->m_Y = (int)m_apPlayers[ClientID]->m_ViewPos.y;
		pEvent->m_SoundID = Sound;
	}
}

void CGS::SendMmoEffect(vec2 Pos, int EffectID, int ClientID)
{
	CNetEvent_EffectMmo *pEvent = (CNetEvent_EffectMmo *)m_Events.Create(NETEVENTTYPE_EFFECTMMO, sizeof(CNetEvent_EffectMmo));
	if(pEvent)
	{
		pEvent->m_X = (int)Pos.x;
		pEvent->m_Y = (int)Pos.y;
		pEvent->m_EffectID = EffectID;
	}
}

void CGS::SendMmoPotion(vec2 Pos, const char *Potion, bool Added)
{
	CNetEvent_EffectPotion *pEvent = (CNetEvent_EffectPotion *)m_Events.Create(NETEVENTTYPE_EFFECTPOTION, sizeof(CNetEvent_EffectPotion));
	if(pEvent)
	{
		pEvent->m_X = (int)Pos.x;
		pEvent->m_Y = (int)Pos.y;
		pEvent->m_PotionAdded = Added;
		StrToInts(pEvent->m_Potion, 4, Potion);
	}
}

/* #########################################################################
	ЧАТ ФУНКЦИИ 
######################################################################### */
// Отправить сообщение
void CGS::SendChat(int ChatterClientID, int Mode, int To, const char *pText)
{
	char aBuf[256];
	if(ChatterClientID >= 0 && ChatterClientID < MAX_CLIENTS)
		str_format(aBuf, sizeof(aBuf), "%d:%d:%s: %s", ChatterClientID, Mode, Server()->ClientName(ChatterClientID), pText);
	else
		str_format(aBuf, sizeof(aBuf), "*** %s", pText);

	char aBufMode[32];
	if(Mode == CHAT_WHISPER)
		str_copy(aBufMode, "whisper", sizeof(aBufMode));
	else if(Mode == CHAT_TEAM)
		str_copy(aBufMode, "teamchat", sizeof(aBufMode));
	else
		str_copy(aBufMode, "chat", sizeof(aBufMode));
	Console()->Print(IConsole::OUTPUT_LEVEL_ADDINFO, aBufMode, aBuf);

	CNetMsg_Sv_Chat Msg;
	Msg.m_Mode = Mode;
	Msg.m_ClientID = ChatterClientID;
	Msg.m_pMessage = pText;
	Msg.m_TargetID = -1;

	if(Mode == CHAT_ALL)
	{
		Server()->SendPackMsg(&Msg, MSGFLAG_VITAL, -1);
		ChatDiscord(DC_SERVER_CHAT, Server()->ClientName(ChatterClientID), pText);
	}
	else if(Mode == CHAT_TEAM)
	{
		const int GuildID = m_apPlayers[ChatterClientID]->Acc().GuildID;
		if(GuildID <= 0)
		{
			Chat(ChatterClientID, "This chat is intended for team / guilds!");
			return;
		}

		// pack one for the recording only
		Server()->SendPackMsg(&Msg, MSGFLAG_VITAL|MSGFLAG_NOSEND, -1);
		ChatDiscord(DC_SERVER_CHAT, Server()->ClientName(ChatterClientID), pText);
		for(int i = 0; i < MAX_PLAYERS; i++)
		{
			CPlayer *pSearchPlayer = GetPlayer(i, true);
			if(pSearchPlayer && pSearchPlayer->Acc().GuildID == GuildID)
				Server()->SendPackMsg(&Msg, MSGFLAG_VITAL|MSGFLAG_NORECORD, i);
		}
	}
	else if(Mode == CHAT_WHISPER)
	{
		Msg.m_TargetID = To;
		Server()->SendPackMsg(&Msg, MSGFLAG_VITAL, ChatterClientID);
		Server()->SendPackMsg(&Msg, MSGFLAG_VITAL, To);
	}
}

// Отправить форматированное сообщение
void CGS::Chat(int ClientID, const char* pText, ...)
{
	int Start = (ClientID < 0 ? 0 : ClientID);
	int End = (ClientID < 0 ? MAX_CLIENTS : ClientID + 1);

	CNetMsg_Sv_Chat Msg;
	Msg.m_Mode = CHAT_ALL;
	Msg.m_ClientID = -1;

	va_list VarArgs;
	va_start(VarArgs, pText);

	dynamic_string Buffer;
	for (int i = Start; i < End; i++)
	{
		if (m_apPlayers[i])
		{
			Server()->Localization()->Format_VL(Buffer, m_apPlayers[i]->GetLanguage(), pText, VarArgs);

			Msg.m_TargetID = i;
			Msg.m_pMessage = Buffer.buffer();
			Server()->SendPackMsg(&Msg, MSGFLAG_VITAL, i);
			Buffer.clear();
		}
	}
	va_end(VarArgs);
}

void CGS::ChatFollow(int ClientID, const char* pText, ...)
{
	int Start = (ClientID < 0 ? 0 : ClientID);
	int End = (ClientID < 0 ? MAX_CLIENTS : ClientID + 1);

	CNetMsg_Sv_Chat Msg;
	Msg.m_Mode = CHAT_WHISPER;
	Msg.m_ClientID = ClientID;

	va_list VarArgs;
	va_start(VarArgs, pText);

	dynamic_string Buffer;
	for (int i = Start; i < End; i++)
	{
		if (m_apPlayers[i])
		{
			Server()->Localization()->Format_VL(Buffer, m_apPlayers[i]->GetLanguage(), pText, VarArgs);

			Msg.m_TargetID = i;
			Msg.m_pMessage = Buffer.buffer();
			Server()->SendPackMsg(&Msg, MSGFLAG_VITAL, i);
			Buffer.clear();
		}
	}
	va_end(VarArgs);
}

// Отправить авторизированному
void CGS::ChatAccountID(int AccountID, const char* pText, ...)
{
	const int ClientID = Mmo()->Account()->CheckOnlineAccount(AccountID);
	if(ClientID < 0 || ClientID >= MAX_PLAYERS)
		return;

	CNetMsg_Sv_Chat Msg;
	Msg.m_Mode = CHAT_ALL;
	Msg.m_ClientID = -1;

	va_list VarArgs;
	va_start(VarArgs, pText);
	
	dynamic_string Buffer;
	if(m_apPlayers[ClientID])
	{
		Server()->Localization()->Format_VL(Buffer, m_apPlayers[ClientID]->GetLanguage(), pText, VarArgs);
		
		Msg.m_TargetID = ClientID;
		Msg.m_pMessage = Buffer.buffer();
		Server()->SendPackMsg(&Msg, MSGFLAG_VITAL, ClientID);
		Buffer.clear();
	}
	va_end(VarArgs);
}

// Отправить организации сообщение
void CGS::ChatGuild(int GuildID, const char* pText, ...)
{
	if(GuildID <= 0)
		return;

	CNetMsg_Sv_Chat Msg;
	Msg.m_Mode = CHAT_ALL;
	Msg.m_ClientID = -1;

	va_list VarArgs;
	va_start(VarArgs, pText);
	
	dynamic_string Buffer;
	for(int i = 0 ; i < MAX_PLAYERS ; i ++)
	{
		CPlayer *pPlayer = GetPlayer(i, true);
		if(pPlayer && pPlayer->Acc().IsGuild() && pPlayer->Acc().GuildID == GuildID)
		{
			Buffer.append("[Guild]");
			Server()->Localization()->Format_VL(Buffer, m_apPlayers[i]->GetLanguage(), pText, VarArgs);
			
			Msg.m_TargetID = i;
			Msg.m_pMessage = Buffer.buffer();

			Server()->SendPackMsg(&Msg, MSGFLAG_VITAL, i);
			Buffer.clear();
		}
	}
	va_end(VarArgs);
}

// Отправить в данже сообщение
void CGS::ChatWorldID(int WorldID, const char* Suffix, const char* pText, ...)
{
	CNetMsg_Sv_Chat Msg;
	Msg.m_Mode = CHAT_ALL;
	Msg.m_ClientID = -1;

	va_list VarArgs;
	va_start(VarArgs, pText);

	dynamic_string Buffer;
	for (int i = 0; i < MAX_PLAYERS; i++)
	{
		CPlayer* pPlayer = GetPlayer(i, true);
		if(!pPlayer || !IsClientEqualWorldID(i, WorldID))
			continue;

		Buffer.append(Suffix);
		Server()->Localization()->Format_VL(Buffer, m_apPlayers[i]->GetLanguage(), pText, VarArgs);

		Msg.m_TargetID = i;
		Msg.m_pMessage = Buffer.buffer();

		Server()->SendPackMsg(&Msg, MSGFLAG_VITAL, i, WorldID);
		Buffer.clear();
	}
	va_end(VarArgs);
}

// Отправить дискорд сообщение
void CGS::ChatDiscord(const char *Color, const char *Title, const char* pText, ...)
{
#ifdef CONF_DISCORD
	va_list VarArgs;
	va_start(VarArgs, pText);
	
	dynamic_string Buffer;
	Server()->Localization()->Format_VL(Buffer, "en", pText, VarArgs);
	Server()->SendDiscordMessage(g_Config.m_SvDiscordChanal, Color, Title, Buffer.buffer());
	Buffer.clear();

	va_end(VarArgs);
#endif
}

// Отправить дискорд сообщение в канал
void CGS::ChatDiscordChannel(const char *pChanel, const char *Color, const char *Title, const char* pText, ...)
{
#ifdef CONF_DISCORD
	va_list VarArgs;
	va_start(VarArgs, pText);
	
	dynamic_string Buffer;
	Server()->Localization()->Format_VL(Buffer, "en", pText, VarArgs);
	Server()->SendDiscordMessage(pChanel, Color, Title, Buffer.buffer());
	Buffer.clear();
	
	va_end(VarArgs);
#endif
}


// Отправить мотд
void CGS::Motd(int ClientID, const char* Text, ...)
{
	if(ClientID < 0 || ClientID >= MAX_PLAYERS)
		return;

	CNetMsg_Sv_Motd Msg;

	va_list VarArgs;
	va_start(VarArgs, Text);
	
	if(m_apPlayers[ClientID])
	{
		dynamic_string Buffer;
		Server()->Localization()->Format_VL(Buffer, m_apPlayers[ClientID]->GetLanguage(), Text, VarArgs);
		
		Msg.m_pMessage = Buffer.buffer();

		Server()->SendPackMsg(&Msg, MSGFLAG_VITAL, ClientID);
		Buffer.clear();
	}
	va_end(VarArgs);
}

/* #########################################################################
	BROADCAST FUNCTIONS 
######################################################################### */
// Отправить броадкаст
void CGS::SendBroadcast(const char *pText, int ClientID, int Priority, int LifeSpan)
{
	int Start = (ClientID < 0 ? 0 : ClientID);
	int End = (ClientID < 0 ? MAX_PLAYERS : ClientID+1);
	
	for(int i = Start; i < End; i++)
	{
		if(m_apPlayers[i])
			AddBroadcast(i, pText, Priority, LifeSpan);
	}
}

// Добавить броадкаст
void CGS::AddBroadcast(int ClientID, const char* pText, int Priority, int LifeSpan)
{
	if (ClientID < 0 || ClientID >= MAX_PLAYERS)
		return;

	if(LifeSpan > 0)
	{
		if(m_BroadcastStates[ClientID].m_TimedPriority > Priority)
			return;
				
		str_copy(m_BroadcastStates[ClientID].m_TimedMessage, pText, sizeof(m_BroadcastStates[ClientID].m_TimedMessage));
		m_BroadcastStates[ClientID].m_LifeSpanTick = LifeSpan;
		m_BroadcastStates[ClientID].m_TimedPriority = Priority;
	}
	else
	{
		if(m_BroadcastStates[ClientID].m_Priority > Priority)
			return;
				
		str_copy(m_BroadcastStates[ClientID].m_NextMessage, pText, sizeof(m_BroadcastStates[ClientID].m_NextMessage));
		m_BroadcastStates[ClientID].m_Priority = Priority;
	}
}

// Форматированный броадкаст
void CGS::SBL(int ClientID, int Priority, int LifeSpan, const char *pText, ...)
{
	int Start = (ClientID < 0 ? 0 : ClientID);
	int End = (ClientID < 0 ? MAX_PLAYERS : ClientID+1);
	
	va_list VarArgs;
	va_start(VarArgs, pText);
	for(int i = Start; i < End; i++)
	{
		if(m_apPlayers[i])
		{
			dynamic_string Buffer;
			Server()->Localization()->Format_VL(Buffer, m_apPlayers[i]->GetLanguage(), pText, VarArgs);
			
			AddBroadcast(i, Buffer.buffer(), Priority, LifeSpan);
			Buffer.clear();
		}
	}
	va_end(VarArgs);	
}

// Форматированный броадкаст в данже
void CGS::BroadcastWorldID(int WorldID, int Priority, int LifeSpan, const char *pText, ...)
{
	va_list VarArgs;
	va_start(VarArgs, pText);
	for(int i = 0; i < MAX_PLAYERS; i++)
	{
		if(m_apPlayers[i] && IsClientEqualWorldID(i, WorldID))
		{
			dynamic_string Buffer;
			Server()->Localization()->Format_VL(Buffer, m_apPlayers[i]->GetLanguage(), pText, VarArgs);
			
			AddBroadcast(i, Buffer.buffer(), Priority, LifeSpan);
			Buffer.clear();
		}
	}
	va_end(VarArgs);	
}

// Тик броадкаса и его жизни
void CGS::BroadcastTick(int ClientID)
{
	if (ClientID < 0 || ClientID >= MAX_PLAYERS)
		return;

	if(m_apPlayers[ClientID] && IsClientEqualWorldID(ClientID))
	{
		if(m_BroadcastStates[ClientID].m_LifeSpanTick > 0 && m_BroadcastStates[ClientID].m_TimedPriority > m_BroadcastStates[ClientID].m_Priority)
			str_copy(m_BroadcastStates[ClientID].m_NextMessage, m_BroadcastStates[ClientID].m_TimedMessage, sizeof(m_BroadcastStates[ClientID].m_NextMessage));
		
		//Send broadcast only if the message is different, or to fight auto-fading
		if(str_comp(m_BroadcastStates[ClientID].m_PrevMessage, m_BroadcastStates[ClientID].m_NextMessage) != 0 ||
			m_BroadcastStates[ClientID].m_NoChangeTick > Server()->TickSpeed())
		{
			CNetMsg_Sv_Broadcast Msg;
			Msg.m_pMessage = m_BroadcastStates[ClientID].m_NextMessage;
			Server()->SendPackMsg(&Msg, MSGFLAG_VITAL, ClientID);
			str_copy(m_BroadcastStates[ClientID].m_PrevMessage, m_BroadcastStates[ClientID].m_NextMessage, sizeof(m_BroadcastStates[ClientID].m_PrevMessage));
			m_BroadcastStates[ClientID].m_NoChangeTick = 0;
		}
		else
			m_BroadcastStates[ClientID].m_NoChangeTick++;
		
		//Update broadcast state
		if(m_BroadcastStates[ClientID].m_LifeSpanTick > 0)
			m_BroadcastStates[ClientID].m_LifeSpanTick--;
		
		if(m_BroadcastStates[ClientID].m_LifeSpanTick <= 0)
		{
			m_BroadcastStates[ClientID].m_TimedMessage[0] = 0;
			m_BroadcastStates[ClientID].m_TimedPriority = 0;
		}
		m_BroadcastStates[ClientID].m_NextMessage[0] = 0;
		m_BroadcastStates[ClientID].m_Priority = 0;
	}
	else
	{
		m_BroadcastStates[ClientID].m_NoChangeTick = 0;
		m_BroadcastStates[ClientID].m_LifeSpanTick = 0;
		m_BroadcastStates[ClientID].m_Priority = 0;
		m_BroadcastStates[ClientID].m_TimedPriority = 0;
		m_BroadcastStates[ClientID].m_PrevMessage[0] = 0;
		m_BroadcastStates[ClientID].m_NextMessage[0] = 0;
		m_BroadcastStates[ClientID].m_TimedMessage[0] = 0;
	}
}

/* #########################################################################
	PACKET MESSAGE FUNCTIONS 
######################################################################### */
void CGS::SendEmoticon(int ClientID, int Emoticon, bool SenderClient)
{
	CPlayer* pPlayer = GetPlayer(ClientID, true, true);
	if (pPlayer && SenderClient)
		Mmo()->Skills()->ParseEmoticionSkill(pPlayer, Emoticon);

	CNetMsg_Sv_Emoticon Msg;
	Msg.m_ClientID = ClientID;
	Msg.m_Emoticon = Emoticon;
	Server()->SendPackMsg(&Msg, MSGFLAG_VITAL, -1, m_WorldID);
}

void CGS::SendWeaponPickup(int ClientID, int Weapon)
{
	CNetMsg_Sv_WeaponPickup Msg;
	Msg.m_Weapon = Weapon;
	Server()->SendPackMsg(&Msg, MSGFLAG_VITAL, ClientID);
}

void CGS::SendMotd(int ClientID)
{
	CNetMsg_Sv_Motd Msg;
	Msg.m_pMessage = g_Config.m_SvMotd;
	Server()->SendPackMsg(&Msg, MSGFLAG_VITAL, ClientID);
}

void CGS::SendSettings(int ClientID)
{
	CNetMsg_Sv_ServerSettings Msg;
	Msg.m_KickVote = 0;
	Msg.m_KickMin = 0;
	Msg.m_SpecVote = 0;
	Msg.m_TeamLock = 0;
	Msg.m_TeamBalance = 0;
	Msg.m_PlayerSlots = MAX_CLIENTS;
	Server()->SendPackMsg(&Msg, MSGFLAG_VITAL, ClientID);
}

// Отправить смену скинна
void CGS::SendSkinChange(int ClientID, int TargetID)
{
	CPlayer *pPlayer = GetPlayer(ClientID, true);
	if(!pPlayer)
		return;

	CNetMsg_Sv_SkinChange Msg;
	Msg.m_ClientID = ClientID;
	for(int p = 0; p < NUM_SKINPARTS; p++)
	{
		Msg.m_apSkinPartNames[p] = pPlayer->Acc().m_aaSkinPartNames[p];
		Msg.m_aUseCustomColors[p] = pPlayer->Acc().m_aUseCustomColors[p];
		Msg.m_aSkinPartColors[p] = pPlayer->Acc().m_aSkinPartColors[p];
	}
	Server()->SendPackMsg(&Msg, MSGFLAG_VITAL|MSGFLAG_NORECORD, TargetID);
}

// Отправить Equip Items
void CGS::SendEquipItem(int ClientID, int TargetID)
{
	CPlayer *pPlayer = GetPlayer(ClientID);
	if((TargetID != -1 && !CheckClient(TargetID)) || !pPlayer)
		return;

	// send players equiping global bots local on world
	const int WorldID = (pPlayer->IsBot() ? GetClientWorldID(ClientID) : -1);
	
	CNetMsg_Sv_EquipItems Msg;
	Msg.m_ClientID = ClientID;
	for(int k = 0; k < NUM_EQUIPS; k++)
	{
		const int EquipItem = pPlayer->GetEquippedItem(k);
		const bool EnchantItem = (pPlayer->IsBot() ? false : (bool)(pPlayer->GetItem(EquipItem).Enchant >= pPlayer->GetItem(EquipItem).Info().MaximalEnchant));
		Msg.m_EquipID[k] = EquipItem;
		Msg.m_EnchantItem[k] = EnchantItem;
	}
	Server()->SendPackMsg(&Msg, MSGFLAG_VITAL|MSGFLAG_NORECORD, TargetID, WorldID);
}

// Отправить снаряжение в радиусе
void CGS::SendRangeEquipItem(int TargetID, int StartSenderClientID, int EndSenderClientID)
{
	if (!CheckClient(TargetID) || StartSenderClientID < 0 || EndSenderClientID > MAX_CLIENTS || StartSenderClientID >= EndSenderClientID)
		return;

	for (int i = StartSenderClientID; i < EndSenderClientID; i++)
		SendEquipItem(i, TargetID);

	SendEquipItem(TargetID, TargetID);
	SendEquipItem(TargetID, -1);
}

// Отправить смену команды или команду
void CGS::SendTeam(int ClientID, int Team, bool DoChatMsg, int TargetID)
{
	CNetMsg_Sv_Team Msg;
	Msg.m_ClientID = ClientID;
	Msg.m_Team = Team;
	Msg.m_Silent = DoChatMsg ? 0 : 1;
	Msg.m_CooldownTick = 0;
	Server()->SendPackMsg(&Msg, MSGFLAG_VITAL, TargetID);	
}

// Отправить GameMsg
void CGS::SendGameMsg(int GameMsgID, int ClientID)
{
	CMsgPacker Msg(NETMSGTYPE_SV_GAMEMSG);
	Msg.AddInt(GameMsgID);
	Server()->SendMsg(&Msg, MSGFLAG_VITAL, ClientID);
}

// Отправить GameMsg
void CGS::SendGameMsg(int GameMsgID, int ParaI1, int ClientID)
{
	CMsgPacker Msg(NETMSGTYPE_SV_GAMEMSG);
	Msg.AddInt(GameMsgID);
	Msg.AddInt(ParaI1);
	Server()->SendMsg(&Msg, MSGFLAG_VITAL, ClientID);
}

// Отправить GameMsg
void CGS::SendGameMsg(int GameMsgID, int ParaI1, int ParaI2, int ParaI3, int ClientID)
{
	CMsgPacker Msg(NETMSGTYPE_SV_GAMEMSG);
	Msg.AddInt(GameMsgID);
	Msg.AddInt(ParaI1);
	Msg.AddInt(ParaI2);
	Msg.AddInt(ParaI3);
	Server()->SendMsg(&Msg, MSGFLAG_VITAL, ClientID);
}

void CGS::SendChatCommand(const CCommandManager::CCommand* pCommand, int ClientID)
{
	CNetMsg_Sv_CommandInfo Msg;
	Msg.m_Name = pCommand->m_aName;
	Msg.m_HelpText = pCommand->m_aHelpText;
	Msg.m_ArgsFormat = pCommand->m_aArgsFormat;

	Server()->SendPackMsg(&Msg, MSGFLAG_VITAL, ClientID);
}

void CGS::SendChatCommands(int ClientID)
{
	for (int i = 0; i < CommandManager()->CommandCount(); i++)
	{
		SendChatCommand(CommandManager()->GetCommand(i), ClientID);
	}
}

void CGS::SendRemoveChatCommand(const CCommandManager::CCommand* pCommand, int ClientID)
{
	CNetMsg_Sv_CommandInfoRemove Msg;
	Msg.m_Name = pCommand->m_aName;

	Server()->SendPackMsg(&Msg, MSGFLAG_VITAL, ClientID);
}

// Отправить тюннинг
void CGS::SendTuningParams(int ClientID)
{
	CMsgPacker Msg(NETMSGTYPE_SV_TUNEPARAMS);
	int *pParams = (int *)&m_Tuning;
	for(unsigned i = 0; i < sizeof(m_Tuning)/sizeof(int); i++)
		Msg.AddInt(pParams[i]);
	Server()->SendMsg(&Msg, MSGFLAG_VITAL, ClientID);
}

// Отправить пакет разговора с кем то
void CGS::SendTalkText(int ClientID, int TalkingID, bool PlayerTalked, const char *Message, int Style, int TalkingEmote)
{
	if (!CheckClient(ClientID))
		return;

	CNetMsg_Sv_TalkText Msg;
	Msg.m_PlayerTalked = PlayerTalked;
	Msg.m_pTalkClientID = TalkingID;
	Msg.m_pText = Message;
	Msg.m_TalkedEmote = TalkingEmote;
	Msg.m_Style = Style;
	Server()->SendPackMsg(&Msg, MSGFLAG_VITAL, ClientID);
}

void CGS::SendProgressBar(int ClientID, int Count, int Request, const char* Message)
{
	if (!CheckClient(ClientID))
		return;

	CNetMsg_Sv_ProgressBar Msg;
	Msg.m_pText = Message;
	Msg.m_pCount = Count;
	Msg.m_pRequires = Request;
	Server()->SendPackMsg(&Msg, MSGFLAG_VITAL, ClientID);
}

void CGS::ClearTalkText(int ClientID)
{
	if(!CheckClient(ClientID))
		return;

	CNetMsg_Sv_ClearTalkText Msg;
	Server()->SendPackMsg(&Msg, MSGFLAG_VITAL, ClientID);
}

// Помощь в поиске мира бота и отправки его
int CGS::GetClientWorldID(int ClientID) const
{
	if(ClientID < 0 || ClientID >= MAX_CLIENTS || !m_apPlayers[ClientID] || !Server()->ClientIngame(ClientID))
		return -1;

	return m_apPlayers[ClientID]->GetPlayerWorldID();
}

/* #########################################################################
	ENGINE GAMECONTEXT 
######################################################################### */
void CGS::UpdateDiscordStatus()
{
#ifdef CONF_DISCORD
	if(Server()->Tick() % (Server()->TickSpeed() * 10) != 0 || m_WorldID != LOCALWORLD)
		return;

	int Players = 0;
	for(int i = 0; i < MAX_PLAYERS; i++)
	{
		if(Server()->ClientIngame(i))
			Players++;
	}

	if(Players > 0)
	{
		char aBuf[64];
		str_format(aBuf, sizeof(aBuf), "%d player's play MRPG!", Players);
		Server()->SendDiscordStatus(aBuf, 3);
		return;
	}
	Server()->SendDiscordStatus("and expects players.", 3);
#endif
}

void CGS::NewCommandHook(const CCommandManager::CCommand* pCommand, void* pContext)
{
	CGS* pSelf = (CGS*)pContext;
	pSelf->SendChatCommand(pCommand, -1);
}

void CGS::RemoveCommandHook(const CCommandManager::CCommand* pCommand, void* pContext)
{
	CGS* pSelf = (CGS*)pContext;
	pSelf->SendRemoveChatCommand(pCommand, -1);
}

// инициализация GameContext данных
void CGS::OnInit(int WorldID)
{
	m_pServer = Kernel()->RequestInterface<IServer>();
	m_pConsole = Kernel()->RequestInterface<IConsole>();
	m_World.SetGameServer(this);
	m_Events.SetGameServer(this);
	m_CommandManager.Init(m_pConsole, this, NewCommandHook, RemoveCommandHook);
	m_WorldID = WorldID;
	m_RespawnWorld = -1;

	for(int i = 0; i < NUM_NETOBJTYPES; i++)
		Server()->SnapSetStaticsize(i, m_NetObjHandler.GetObjSize(i));

	// создаем контроллер
	m_Layers.Init(Kernel(), nullptr, WorldID);
	m_Collision.Init(&m_Layers);
	pMmoController = new MmoController(this);
	pMmoController->LoadLogicWorld();
	UpdateZoneDungeon();

	// создаем все гейм обьекты для сервера
	if(IsDungeon())
		m_pController = new CGameControllerDungeon(this);
	else
		m_pController = new CGameControllerMain(this);

	m_pController->RegisterChatCommands(CommandManager());

	// инициализируем слои
	CMapItemLayerTilemap *pTileMap = m_Layers.GameLayer();
	CTile *pTiles = (CTile *)Kernel()->RequestInterface<IMap>(WorldID)->GetData(pTileMap->m_Data);
	for(int y = 0; y < pTileMap->m_Height; y++) 
	{
		for(int x = 0; x < pTileMap->m_Width; x++) 
		{
			int Index = pTiles[y*pTileMap->m_Width+x].m_Index;
			if(Index >= ENTITY_OFFSET) 
			{
				vec2 Pos(x*32.0f+16.0f, y*32.0f+16.0f);
				m_pController->OnEntity(Index-ENTITY_OFFSET, Pos);
			}
		}
	}
	
	// инициализируем pathfinder
	m_pPathFinder = new CPathfinder(&m_Layers, &m_Collision);

	UpdateZonePVP();
	Console()->Chain("sv_motd", ConchainSpecialMotdupdate, this);
}

// Инициализация консоли
void CGS::OnConsoleInit()
{
	m_pServer = Kernel()->RequestInterface<IServer>();
	m_pConsole = Kernel()->RequestInterface<IConsole>();

	Console()->Register("parseskin", "i[cid]", CFGFLAG_SERVER, ConParseSkin, this, "Parse skin on console. Easy for devlop bots.");
	Console()->Register("giveitem", "i[cid]i[itemid]i[count]i[ench]i[mail]", CFGFLAG_SERVER, ConGiveItem, this, "Give item <clientid> <itemid> <count> <enchant> <mail 1=yes 0=no>");
	Console()->Register("tune", "s[tuning] i[value]", CFGFLAG_SERVER, ConTuneParam, this, "Tune variable to value");
	Console()->Register("tune_reset", "", CFGFLAG_SERVER, ConTuneReset, this, "Reset tuning");
	Console()->Register("tune_dump", "", CFGFLAG_SERVER, ConTuneDump, this, "Dump tuning");
	Console()->Register("say", "r[text]", CFGFLAG_SERVER, ConSay, this, "Say in chat");
	Console()->Register("addcharacter", "i[cid]r[botname]", CFGFLAG_SERVER, ConAddCharacter, this, "Add new bot on datatable <clientid> <nick name>");
}

// Отключение сервера
void CGS::OnShutdown()
{
	delete m_pController;
	m_pController = nullptr;

	delete pMmoController;
	pMmoController = nullptr;
	Clear();
}

// Таймер GameContext
void CGS::OnTick()
{
	// копируем тюннинг
	m_World.m_Core.m_Tuning = m_Tuning;
	m_World.Tick();

	// контроллер тик
	m_pController->Tick();
	for(int i = 0; i < MAX_CLIENTS; i++)
	{
		if(!m_apPlayers[i] || m_WorldID != GetClientWorldID(i))
			continue;

		m_apPlayers[i]->Tick();
		if(i < MAX_PLAYERS)
		{
			m_apPlayers[i]->PostTick();
			BroadcastTick(i);
		}
	}
	OnTickLocalWorld();
	Mmo()->OnTick();
}

// Таймер в OnTick-=
void CGS::OnTickLocalWorld()
{
	if(m_WorldID != LOCALWORLD) 
		return;

	// получить и отправить измененный день
	if(m_DayEnumType != Server()->GetEnumTypeDay())
	{
		m_DayEnumType = Server()->GetEnumTypeDay(); 
		if(m_DayEnumType == DayType::NIGHTTYPE)
			m_RaidExp = 100 + rand() % 200;
		else if(m_DayEnumType == DayType::MORNINGTYPE)
			m_RaidExp = 100;
	
		SendDayInfo(-1);
	}

	UpdateDiscordStatus();
}

// Рисование вывод всех объектов 
void CGS::OnSnap(int ClientID)
{
	if(GetClientWorldID(ClientID) != GetWorldID())
		return;

	m_World.Snap(ClientID);
	m_pController->Snap(ClientID);
	m_Events.Snap(ClientID);
	for(auto & apPlayer : m_apPlayers)
	{
		if(apPlayer)
			apPlayer->Snap(ClientID);
	}
}

void CGS::OnPreSnap() {}
void CGS::OnPostSnap()
{
	m_World.PostSnap();
	m_Events.Clear();
}

// Обработка пакетов
void CGS::OnMessage(int MsgID, CUnpacker *pUnpacker, int ClientID)
{
	void *pRawMsg = m_NetObjHandler.SecureUnpackMsg(MsgID, pUnpacker);
	CPlayer *pPlayer = m_apPlayers[ClientID];

	if(!pRawMsg)
	{
		if(g_Config.m_Debug)
		{
			char aBuf[256];
			str_format(aBuf, sizeof(aBuf), "dropped weird message '%s' (%d), failed on '%s'", m_NetObjHandler.GetMsgName(MsgID), MsgID, m_NetObjHandler.FailedMsgOn());
			Console()->Print(IConsole::OUTPUT_LEVEL_DEBUG, "server", aBuf);
		}
		return;
	}

	if(Server()->ClientIngame(ClientID))
	{
		if(MsgID == NETMSGTYPE_CL_SAY)
		{
			if(g_Config.m_SvSpamprotection && pPlayer->m_PlayerTick[TickState::LastChat] && pPlayer->m_PlayerTick[TickState::LastChat]+Server()->TickSpeed() > Server()->Tick())
				return;

			CNetMsg_Cl_Say *pMsg = (CNetMsg_Cl_Say *)pRawMsg;

			// trim right and set maximum length to 128 utf8-characters
			int Length = 0;
			const char *p = pMsg->m_pMessage;
			const char *pEnd = nullptr;
			while(*p)
			{
				const char *pStrOld = p;
				int Code = str_utf8_decode(&p);

				// check if unicode is not empty
				if(Code > 0x20 && Code != 0xA0 && Code != 0x034F && (Code < 0x2000 || Code > 0x200F) && (Code < 0x2028 || Code > 0x202F) &&
					(Code < 0x205F || Code > 0x2064) && (Code < 0x206A || Code > 0x206F) && (Code < 0xFE00 || Code > 0xFE0F) &&
					Code != 0xFEFF && (Code < 0xFFF9 || Code > 0xFFFC))
				{
					pEnd = nullptr;
				}
				else if(pEnd == nullptr)
					pEnd = pStrOld;

				if(++Length >= 127)
				{
					*(const_cast<char *>(p)) = 0;
					break;
				}
			}
			if(pEnd != nullptr)
				*(const_cast<char *>(pEnd)) = 0;

			pPlayer->m_PlayerTick[TickState::LastChat] = Server()->Tick();

			const int Mode = pMsg->m_Mode;
			if(Mode != CHAT_NONE)
			{
				// команды сервера
				if(pMsg->m_pMessage[0]=='/')
				{
					CommandProcessor pChat;
					pChat.ChatCmd(pMsg, this, pPlayer);
					return;
				}

				// отправить обычное сообщение в чат
				SendChat(ClientID, Mode, pMsg->m_Target, pMsg->m_pMessage);						
			}
		}
		else if (MsgID == NETMSGTYPE_CL_COMMAND)
		{
			CNetMsg_Cl_Command *pMsg = (CNetMsg_Cl_Command*)pRawMsg;
			CommandManager()->OnCommand(pMsg->m_Name, pMsg->m_Arguments, ClientID);
		}
		else if(MsgID == NETMSGTYPE_CL_CALLVOTE)
		{
			CNetMsg_Cl_CallVote *pMsg = (CNetMsg_Cl_CallVote *)pRawMsg;
			if (str_comp_nocase(pMsg->m_Type, "option") != 0 || Server()->Tick() < (pPlayer->m_PlayerTick[TickState::LastVoteTry] + (Server()->TickSpeed() / 2)))
				return;
			
			pPlayer->m_PlayerTick[TickState::LastVoteTry] = Server()->Tick();
			const auto& item = std::find_if(m_PlayerVotes[ClientID].begin(), m_PlayerVotes[ClientID].end(), [pMsg](const CVoteOptions& vote)
			{
				return (str_comp_nocase(pMsg->m_Value, vote.m_aDescription) == 0);
			});

			if (item != m_PlayerVotes[ClientID].end())
			{
				const int InteractiveCount = string_to_number(pMsg->m_Reason, 1, 10000000);
				ParseVote(ClientID, item->m_aCommand, item->m_TempID, item->m_TempID2, InteractiveCount, pMsg->m_Reason);
				return;
			}
			ResetVotes(ClientID, pPlayer->m_OpenVoteMenu);
		}
		else if(MsgID == NETMSGTYPE_CL_VOTE)
		{
			CNetMsg_Cl_Vote *pMsg = (CNetMsg_Cl_Vote *)pRawMsg;
			if(pPlayer->ParseItemsF3F4(pMsg->m_Vote))
				return;
		}
		else if(MsgID == NETMSGTYPE_CL_SETTEAM)
		{
			if(!pPlayer->IsAuthed())
			{
				SBL(pPlayer->GetCID(), BroadcastPriority::BROADCAST_MAIN_INFORMATION, 100, "Use /register <name> <pass>.");
				return;
			}
		}
		else if (MsgID == NETMSGTYPE_CL_SETSPECTATORMODE)
		{

		}
		else if (MsgID == NETMSGTYPE_CL_EMOTICON)
		{
			CNetMsg_Cl_Emoticon *pMsg = (CNetMsg_Cl_Emoticon *)pRawMsg;

			if(g_Config.m_SvSpamprotection && pPlayer->m_PlayerTick[TickState::LastEmote] && pPlayer->m_PlayerTick[TickState::LastEmote]+(Server()->TickSpeed() / 2) > Server()->Tick())
				return;

			pPlayer->m_PlayerTick[TickState::LastEmote] = Server()->Tick();
			SendEmoticon(ClientID, pMsg->m_Emoticon, true);
		}
		else if (MsgID == NETMSGTYPE_CL_KILL)
		{
			if(pPlayer->m_PlayerTick[TickState::LastKill] && pPlayer->m_PlayerTick[TickState::LastKill]+Server()->TickSpeed()*3 > Server()->Tick())
				return;

			//pPlayer->m_PlayerTick[TickState::LastKill] = Server()->Tick();
			//pPlayer->KillCharacter(WEAPON_SELF);
		}
		else if (MsgID == NETMSGTYPE_CL_READYCHANGE)
		{

		}
		else if(MsgID == NETMSGTYPE_CL_SKINCHANGE)
		{
			if(pPlayer->m_PlayerTick[TickState::LastChangeInfo] && pPlayer->m_PlayerTick[TickState::LastChangeInfo]+Server()->TickSpeed()*5 > Server()->Tick())
				return;

			pPlayer->m_PlayerTick[TickState::LastChangeInfo] = Server()->Tick();
			CNetMsg_Cl_SkinChange *pMsg = (CNetMsg_Cl_SkinChange *)pRawMsg;

			for(int p = 0; p < NUM_SKINPARTS; p++)
			{
				str_copy(pPlayer->Acc().m_aaSkinPartNames[p], pMsg->m_apSkinPartNames[p], 24);
				pPlayer->Acc().m_aUseCustomColors[p] = pMsg->m_aUseCustomColors[p];
				pPlayer->Acc().m_aSkinPartColors[p] = pMsg->m_aSkinPartColors[p];
			}

			// update all clients
			for(int i = 0; i < MAX_CLIENTS; ++i)
			{
				if(!m_apPlayers[i] || Server()->GetClientVersion(i) < MIN_SKINCHANGE_CLIENTVERSION)
					continue;

				SendSkinChange(pPlayer->GetCID(), i);
			}
		}

		//////////////////////////////////////////////////////////////////////////////////
		///////////// Если клиент прошел проверку можно альтернативу использовать клиента
		else if (MsgID == NETMSGTYPE_CL_ISMMOSERVER)
		{
			CNetMsg_Cl_IsMmoServer *pMsg = (CNetMsg_Cl_IsMmoServer *)pRawMsg;
			Server()->SetClientVersion(ClientID, pMsg->m_Version);

			/*
				получаем информацию о клиенте если старая версия пишем и запрещаем использование всех функций клиента
				дабы избежать крашей между размером пакетов
			*/
			if(!CheckClient(ClientID))
			{
				Server()->Kick(ClientID, "Update Mmo-Client use udapter or download in discord.");
				return;
			}

			// пишем клиент успешно прочекан 	
			SBL(ClientID, BroadcastPriority::BROADCAST_MAIN_INFORMATION, 100, "Successfully checks client.");

			// отправим что прошли проверку на стороне сервера
			CNetMsg_Sv_AfterIsMmoServer GoodCheck;
			Server()->SendPackMsg(&GoodCheck, MSGFLAG_VITAL|MSGFLAG_FLUSH|MSGFLAG_NORECORD, ClientID);

			// загрузка всех частей скинов игроков
			SendRangeEquipItem(ClientID, 0, MAX_CLIENTS);
		}
		else if(MsgID == NETMSGTYPE_CL_CLIENTAUTH)
		{
			CNetMsg_Cl_ClientAuth *pMsg = (CNetMsg_Cl_ClientAuth *)pRawMsg;
			
			// режим регистрации аккаунта
			if(pMsg->m_SelectRegister)
			{
				// TODO: добавить регу
				Mmo()->Account()->RegisterAccount(ClientID, pMsg->m_Login, pMsg->m_Password);
				return;
			}

			// режим авторизация клиента
			if(Mmo()->Account()->LoginAccount(ClientID, pMsg->m_Login, pMsg->m_Password) == AUTH_LOGIN_GOOD)
				Mmo()->Account()->LoadAccount(pPlayer, true);
		}
		else 
			Mmo()->OnMessage(MsgID, pRawMsg, ClientID);
	}
	else
	{
		if (MsgID == NETMSGTYPE_CL_STARTINFO)
		{
			if(pPlayer->m_PlayerTick[TickState::LastChangeInfo] != 0)
				return;

			CNetMsg_Cl_StartInfo *pMsg = (CNetMsg_Cl_StartInfo *)pRawMsg;
			pPlayer->m_PlayerTick[TickState::LastChangeInfo] = Server()->Tick();

			// set start infos
			Server()->SetClientName(ClientID, pMsg->m_pName);
			Server()->SetClientClan(ClientID, pMsg->m_pClan);
			Server()->SetClientCountry(ClientID, pMsg->m_Country);
			for(int p = 0; p < 6; p++)
			{
				str_copy(pPlayer->Acc().m_aaSkinPartNames[p], pMsg->m_apSkinPartNames[p], 24);
				pPlayer->Acc().m_aUseCustomColors[p] = pMsg->m_aUseCustomColors[p];
				pPlayer->Acc().m_aSkinPartColors[p] = pMsg->m_aSkinPartColors[p];
			}

			// send vote options
			CNetMsg_Sv_VoteClearOptions ClearMsg;
			Server()->SendPackMsg(&ClearMsg, MSGFLAG_VITAL, ClientID);

			// client is ready to enter
			CNetMsg_Sv_ReadyToEnter m;
			Server()->SendPackMsg(&m, MSGFLAG_VITAL|MSGFLAG_FLUSH, ClientID);
		}
	}
}

// Подключение клиента
void CGS::OnClientConnected(int ClientID)
{
	if(!m_apPlayers[ClientID])
	{
		const int savecidmem = ClientID+m_WorldID*MAX_CLIENTS;
		m_apPlayers[ClientID] = new(savecidmem) CPlayer(this, ClientID);
	}

	SendMotd(ClientID);
	SendSettings(ClientID);
	m_BroadcastStates[ClientID] = {};
}

// Вход на сервер игрока
void CGS::OnClientEnter(int ClientID)
{
	CPlayer* pPlayer = m_apPlayers[ClientID];
	if(!pPlayer || pPlayer->IsBot())
		return;

	m_pController->OnPlayerConnect(pPlayer);
	SendChatCommands(ClientID);
	pPlayer->SendClientInfo(-1);
	for(int i = 0; i < MAX_CLIENTS; ++i)
	{
		if(i == ClientID || !m_apPlayers[i] || !Server()->ClientIngame(i))
			continue;

		m_apPlayers[i]->SendClientInfo(ClientID);
	}
	pPlayer->SendClientInfo(ClientID);

	// another
	if(!pPlayer->IsAuthed())
	{
		// информируем о смене команды
		SendTeam(ClientID, TEAM_SPECTATORS, false, -1);
		// информация
		Chat(ClientID, "- - - - - - - [Welcome to MRPG] - - - - - - -");
		Chat(ClientID, "You need to create an account, or log in if you already have.");
		Chat(ClientID, "Register: \"/register <login> <pass>\"");
		Chat(ClientID, "Log in: \"/login <login> <pass>\"");
		SendDayInfo(ClientID);
		
		ChatDiscord(DC_JOIN_LEAVE, Server()->ClientName(ClientID), "connected and enter in MRPG");
		ResetVotesNewbieInformation(ClientID);
		return;
	}

	// если уже авторизован прогрузка аккаунта
	Mmo()->Account()->LoadAccount(pPlayer, false);
	ResetVotes(ClientID, MenuList::MAIN_MENU);
}

// Выход игрока
void CGS::OnClientDrop(int ClientID, const char *pReason, bool ChangeWorld)
{
	if(!m_apPlayers[ClientID] || m_apPlayers[ClientID]->IsBot())
		return;

	if(ChangeWorld)
	{
		m_apPlayers[ClientID]->KillCharacter();
		return;
	}
	m_pController->OnPlayerDisconnect(m_apPlayers[ClientID]);

	// update clients on drop
	if (Server()->ClientIngame(ClientID) && IsClientEqualWorldID(ClientID))
	{
		ChatDiscord(DC_JOIN_LEAVE, Server()->ClientName(ClientID), "leave game MRPG");

		CNetMsg_Sv_ClientDrop Msg;
		Msg.m_ClientID = ClientID;
		Msg.m_pReason = pReason;
		Msg.m_Silent = false;
		Server()->SendPackMsg(&Msg, MSGFLAG_VITAL | MSGFLAG_NORECORD, -1);
	}

	// очистка данных
	delete m_apPlayers[ClientID];
	m_apPlayers[ClientID] = nullptr;
	ClearClientData(ClientID);
}

// Input проверить и отправить
void CGS::OnClientDirectInput(int ClientID, void *pInput)
{
	int NumFailures = m_NetObjHandler.NumObjFailures();
	if(m_NetObjHandler.ValidateObj(NETOBJTYPE_PLAYERINPUT, pInput, sizeof(CNetObj_PlayerInput)) == -1)
	{
		if(g_Config.m_Debug && NumFailures != m_NetObjHandler.NumObjFailures())
		{
			char aBuf[128];
			str_format(aBuf, sizeof(aBuf), "NETOBJTYPE_PLAYERINPUT failed on '%s'", m_NetObjHandler.FailedObjOn());
			Console()->Print(IConsole::OUTPUT_LEVEL_DEBUG, "server", aBuf);
		}
	}
	else 
		m_apPlayers[ClientID]->OnDirectInput((CNetObj_PlayerInput *)pInput);
}

// Input проверить и отправить
void CGS::OnClientPredictedInput(int ClientID, void *pInput)
{
	int NumFailures = m_NetObjHandler.NumObjFailures();
	if(m_NetObjHandler.ValidateObj(NETOBJTYPE_PLAYERINPUT, pInput, sizeof(CNetObj_PlayerInput)) == -1)
	{
		if(g_Config.m_Debug && NumFailures != m_NetObjHandler.NumObjFailures())
		{
			char aBuf[128];
			str_format(aBuf, sizeof(aBuf), "NETOBJTYPE_PLAYERINPUT corrected on '%s'", m_NetObjHandler.FailedObjOn());
			Console()->Print(IConsole::OUTPUT_LEVEL_DEBUG, "server", aBuf);
		}
	}
	else 
		m_apPlayers[ClientID]->OnPredictedInput((CNetObj_PlayerInput *)pInput);
}

// Смена мира
void CGS::ChangeWorld(int ClientID)
{
	if (m_apPlayers[ClientID])
	{
		delete m_apPlayers[ClientID];
		m_apPlayers[ClientID] = nullptr;
	}
	const int savecidmem = ClientID+m_WorldID*MAX_CLIENTS;
	m_apPlayers[ClientID] = new(savecidmem) CPlayer(this, ClientID);
}

// Если игрок Готов к игре
bool CGS::IsClientReady(int ClientID) const
{
	return m_apPlayers[ClientID] && m_apPlayers[ClientID]->m_PlayerTick[TickState::LastChangeInfo] > 0;
}

// Если игрок в игре
bool CGS::IsClientPlayer(int ClientID) const
{
	return m_apPlayers[ClientID] && m_apPlayers[ClientID]->GetTeam() == TEAM_SPECTATORS ? false : true;
}
bool CGS::CheckClient(int ClientID) const
{
	return (bool)((Server()->GetClientVersion(ClientID) == CLIENT_VERSION_MMO) || (ClientID >= MAX_PLAYERS && ClientID < MAX_CLIENTS)); 
}
const char *CGS::GameType() const { return m_pController && m_pController->GetGameType() ? m_pController->GetGameType() : ""; }
const char *CGS::Version() const { return GAME_VERSION; }
const char *CGS::NetVersion() const { return GAME_NETVERSION; }

// Очистка всех данных при выходие клиента обезательно вызова на раз достаточно
void CGS::ClearClientData(int ClientID)
{
	Mmo()->ResetClientData(ClientID);
	if(Effects.find(ClientID) != Effects.end()) 
		Effects.erase(ClientID);

	// clear active snap bots for player
	for(auto& databot : BotJob::DataBot)
		databot.second.AlreadySnapQuestBot[ClientID] = false;
}

int CGS::GetRank(int AuthID)
{
	return Mmo()->Account()->GetRank(AuthID);
}

/* #########################################################################
	CONSOLE GAMECONTEXT 
######################################################################### */
// Парсинг скина
void CGS::ConParseSkin(IConsole::IResult *pResult, void *pUserData)
{
	CGS *pSelf = (CGS *)pUserData;
	int ClientID = clamp(pResult->GetInteger(0), 0, MAX_PLAYERS-1);
	CPlayer *pPlayer = pSelf->GetPlayer(ClientID, true);
	if(pPlayer)
	{
		char aBuf[256];
		str_format(aBuf, sizeof(aBuf), "%s %s %s %s %s %s", pPlayer->Acc().m_aaSkinPartNames[0], 
			pPlayer->Acc().m_aaSkinPartNames[1], pPlayer->Acc().m_aaSkinPartNames[2], 
			pPlayer->Acc().m_aaSkinPartNames[3], pPlayer->Acc().m_aaSkinPartNames[4],
			pPlayer->Acc().m_aaSkinPartNames[5]);
		pSelf->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "parseskin", aBuf);

		str_format(aBuf, sizeof(aBuf), "%d %d %d %d %d %d", pPlayer->Acc().m_aSkinPartColors[0], 
			pPlayer->Acc().m_aSkinPartColors[1], pPlayer->Acc().m_aSkinPartColors[2], 
			pPlayer->Acc().m_aSkinPartColors[3], pPlayer->Acc().m_aSkinPartColors[4],
			pPlayer->Acc().m_aSkinPartColors[5]);
		pSelf->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "parseskin", aBuf);
	}

}

// Выдать предмет игроку
void CGS::ConGiveItem(IConsole::IResult *pResult, void *pUserData)
{
	int ClientID = clamp(pResult->GetInteger(0), 0, MAX_PLAYERS - 1);
	int ItemID = pResult->GetInteger(1);
	int Count = pResult->GetInteger(2);
	int Enchant = pResult->GetInteger(3);
	int Mail = pResult->GetInteger(4);

	CGS *pSelf = (CGS *)pUserData;	
	CPlayer *pPlayer = pSelf->GetPlayer(ClientID, true);
	if(pPlayer)
	{
		if (Mail == 0)
		{
			pPlayer->GetItem(ItemID).Add(Count, 0, Enchant);
			return;
		}
		pSelf->SendInbox(ClientID, "The sender heavens", "Sent from console", ItemID, Count, Enchant);
	}
}

// Изменить параметр тюнинга
void CGS::ConTuneParam(IConsole::IResult *pResult, void *pUserData)
{
	CGS *pSelf = (CGS *)pUserData;
	const char *pParamName = pResult->GetString(0);
	float NewValue = pResult->GetFloat(1);

	if(pSelf->Tuning()->Set(pParamName, NewValue))
	{
		char aBuf[256];
		str_format(aBuf, sizeof(aBuf), "%s changed to %.2f", pParamName, NewValue);
		pSelf->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "tuning", aBuf);
		pSelf->SendTuningParams(-1);
	}
	else
		pSelf->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "tuning", "No such tuning parameter");
}

// Сбросить тюннинг
void CGS::ConTuneReset(IConsole::IResult *pResult, void *pUserData)
{
	CGS *pSelf = (CGS *)pUserData;
	CTuningParams TuningParams;
	*pSelf->Tuning() = TuningParams;
	pSelf->SendTuningParams(-1);
	pSelf->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "tuning", "Tuning reset");
}

// ДюмпТюннинга
void CGS::ConTuneDump(IConsole::IResult *pResult, void *pUserData)
{
	CGS *pSelf = (CGS *)pUserData;
	char aBuf[256];
	for(int i = 0; i < pSelf->Tuning()->Num(); i++)
	{
		float v;
		pSelf->Tuning()->Get(i, &v);
		str_format(aBuf, sizeof(aBuf), "%s %.2f", pSelf->Tuning()->m_apNames[i], v);
		pSelf->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "tuning", aBuf);
	}
}

// Отправить сообщение
void CGS::ConSay(IConsole::IResult *pResult, void *pUserData)
{
	CGS *pSelf = (CGS *)pUserData;
	pSelf->SendChat(-1, CHAT_ALL, -1, pResult->GetString(0));
}

// Добавить игрока бота нового в базу данных
void CGS::ConAddCharacter(IConsole::IResult *pResult, void *pUserData)
{
	CGS *pSelf = (CGS *)pUserData;

	// проверяем есть ли такой игрок
	int ClientID = pResult->GetInteger(0);
	if(ClientID < 0 || ClientID >= MAX_PLAYERS || !pSelf->m_apPlayers[ClientID])
		return;
	
	// добавляем новый вид бота
	pSelf->Mmo()->BotsData()->ConAddCharacterBot(ClientID, pResult->GetString(1));
}

void CGS::ConchainSpecialMotdupdate(IConsole::IResult *pResult, void *pUserData, IConsole::FCommandCallback pfnCallback, void *pCallbackUserData)
{
	pfnCallback(pResult, pCallbackUserData);
	if(pResult->NumArguments())
	{
		CGS *pSelf = (CGS *)pUserData;
		pSelf->SendMotd(-1);
	}
}
void CGS::ConchainSettingUpdate(IConsole::IResult *pResult, void *pUserData, IConsole::FCommandCallback pfnCallback, void *pCallbackUserData)
{
	pfnCallback(pResult, pCallbackUserData);
	if(pResult->NumArguments())
	{
		CGS *pSelf = (CGS *)pUserData;
		pSelf->SendSettings(-1);
	}
}
void CGS::ConchainGameinfoUpdate(IConsole::IResult *pResult, void *pUserData, IConsole::FCommandCallback pfnCallback, void *pCallbackUserData)
{
	pfnCallback(pResult, pCallbackUserData);
	if(pResult->NumArguments())
	{
	}
}

/* #########################################################################
	VOTING MMO GAMECONTEXT 
######################################################################### */
// очистить лист голосований
void CGS::ClearVotes(int ClientID)
{
	m_PlayerVotes[ClientID].clear();
	
	// send vote options
	CNetMsg_Sv_VoteClearOptions ClearMsg;
	Server()->SendPackMsg(&ClearMsg, MSGFLAG_VITAL, ClientID);
}

// добавить голосование
void CGS::AV(int To, const char *Cmd, const char *Desc, const int ID, const int ID2, const char *Icon)
{
	if(To < 0 || To > MAX_PLAYERS || !m_apPlayers[To])
		return;

	CVoteOptions Vote;	
	str_copy(Vote.m_aDescription, Desc, sizeof(Vote.m_aDescription));
	str_copy(Vote.m_aCommand, Cmd, sizeof(Vote.m_aCommand));
	Vote.m_TempID = ID;
	Vote.m_TempID2 = ID2;
	m_PlayerVotes[To].push_back(Vote);

	// отправить клиентам что имеют клиент ммо
	if(CheckClient(To))
	{
		if (str_length(Vote.m_aDescription) < 1)
			m_apPlayers[To]->m_Colored = { 0, 0, 0 };

		CNetMsg_Sv_VoteMmoOptionAdd OptionMsg;
		vec3 ToHexColor = m_apPlayers[To]->m_Colored;
		OptionMsg.m_pHexColor = ((int)ToHexColor.r << 16) + ((int)ToHexColor.g << 8) + (int)ToHexColor.b;
		OptionMsg.m_pDescription = Vote.m_aDescription;
		StrToInts(OptionMsg.m_pIcon, 4, Icon);
		Server()->SendPackMsg(&OptionMsg, MSGFLAG_VITAL|MSGFLAG_NORECORD, To);
		return;
	}

	// отправить ванильным клиентам
	CNetMsg_Sv_VoteOptionAdd OptionMsg;	
	OptionMsg.m_pDescription = Vote.m_aDescription;
	Server()->SendPackMsg(&OptionMsg, MSGFLAG_VITAL, To);
}

// Добавить форматированое голосование
void CGS::AVL(int To, const char* aCmd, const char* pText, ...)
{
	if(To >= 0 && To < MAX_PLAYERS && m_apPlayers[To])
	{
		va_list VarArgs;
		va_start(VarArgs, pText);
		
		dynamic_string Buffer;
		if(str_comp(aCmd, "null") != 0)
			Buffer.append("- ");
		
		Server()->Localization()->Format_VL(Buffer, m_apPlayers[To]->GetLanguage(), pText, VarArgs);
		AV(To, aCmd, Buffer.buffer());
		Buffer.clear();
		
		va_end(VarArgs);
	}
}

// Добавить форматированое голосование как Список с цветом
void CGS::AVH(int To, const int ID, vec3 Color, const char* pText, ...)
{
	if(To >= 0 && To < MAX_PLAYERS && m_apPlayers[To])
	{
		va_list VarArgs;
		va_start(VarArgs, pText);

		dynamic_string Buffer;
		bool HidenTabs = (ID >= TAB_STAT) ? m_apPlayers[To]->GetHidenMenu(ID) : false;
		Buffer.append(GetSymbolHandleMenu(To, HidenTabs, ID));

		Server()->Localization()->Format_VL(Buffer, m_apPlayers[To]->GetLanguage(), pText, VarArgs);
		if(ID > TAB_SETTINGS_MODULES && ID < NUM_TAB_MENU) { Buffer.append(" (Press me for help)"); }

		m_apPlayers[To]->m_Colored = { Color.r, Color.g, Color.b };
		AV(To, "HIDEN", Buffer.buffer(), ID);
		if(length(m_apPlayers[To]->m_Colored) > 1.0f)
			m_apPlayers[To]->m_Colored /= 4.0f;
		Buffer.clear();
		va_end(VarArgs);
	}
}

// Добавить форматированое голосование как Список с цветом и с иконкой
void CGS::AVHI(int To, const char *Icon, const int ID, vec3 Color, const char* pText, ...)
{
	if(To >= 0 && To < MAX_PLAYERS && m_apPlayers[To])
	{
		va_list VarArgs;
		va_start(VarArgs, pText);

		dynamic_string Buffer;
		bool HidenTabs = (bool)(ID >= TAB_STAT ? m_apPlayers[To]->GetHidenMenu(ID) : false);
		Buffer.append(GetSymbolHandleMenu(To, HidenTabs, ID));

		Server()->Localization()->Format_VL(Buffer, m_apPlayers[To]->GetLanguage(), pText, VarArgs);
		if(ID > TAB_SETTINGS_MODULES && ID < NUM_TAB_MENU)
			Buffer.append(" (Press me for help)");

		m_apPlayers[To]->m_Colored = Color;
		AV(To, "HIDEN", Buffer.buffer(), ID, -1, Icon);
		if(length(m_apPlayers[To]->m_Colored) > 1.0f)
			m_apPlayers[To]->m_Colored /= 4.0f;
			
		Buffer.clear();
		va_end(VarArgs);
	}
}

// Добавить форматированое голосование как Меню
void CGS::AVM(int To, const char* Type, const int ID, const int HideID, const char* pText, ...)
{
	if(To >= 0 && To < MAX_PLAYERS && m_apPlayers[To])
	{
		if((!m_apPlayers[To]->GetHidenMenu(HideID) && HideID > TAB_SETTINGS_MODULES) || 
			(m_apPlayers[To]->GetHidenMenu(HideID) && HideID <= TAB_SETTINGS_MODULES))
			return;

		va_list VarArgs;
		va_start(VarArgs, pText);

		dynamic_string Buffer;
		if(ID != NOPE) { Buffer.append("- "); }

		Server()->Localization()->Format_VL(Buffer, m_apPlayers[To]->GetLanguage(), pText, VarArgs);
		AV(To, Type, Buffer.buffer(), ID);
		Buffer.clear();
		va_end(VarArgs);
	}
}

// Добавить форматированое голосование как Меню с иконкой
void CGS::AVMI(int To, const char *Icon, const char* Type, const int ID, const int HideID, const char* pText, ...)
{
	if(To >= 0 && To < MAX_PLAYERS && m_apPlayers[To])
	{
		if((!m_apPlayers[To]->GetHidenMenu(HideID) && HideID > TAB_SETTINGS_MODULES) || 
			(m_apPlayers[To]->GetHidenMenu(HideID) && HideID <= TAB_SETTINGS_MODULES))
			return;

		va_list VarArgs;
		va_start(VarArgs, pText);

		dynamic_string Buffer;
		if(ID != NOPE) { Buffer.append("- "); }

		Server()->Localization()->Format_VL(Buffer, m_apPlayers[To]->GetLanguage(), pText, VarArgs);
		AV(To, Type, Buffer.buffer(), ID, -1, Icon);
		Buffer.clear();
		va_end(VarArgs);
	}
}

// Добавить форматированое голосование с двойным Айди
void CGS::AVD(int To, const char* Type, const int ID, const int ID2, const int HideID, const char* pText, ...)
{
	if(To >= 0 && To < MAX_PLAYERS && m_apPlayers[To])
	{
		if((!m_apPlayers[To]->GetHidenMenu(HideID) && HideID > TAB_SETTINGS_MODULES) || 
			(m_apPlayers[To]->GetHidenMenu(HideID) && HideID <= TAB_SETTINGS_MODULES))
			return;
			
		va_list VarArgs;
		va_start(VarArgs, pText);

		dynamic_string Buffer;
		if(ID != NOPE) { Buffer.append("- "); }

		Server()->Localization()->Format_VL(Buffer, m_apPlayers[To]->GetLanguage(), pText, VarArgs);
		AV(To, Type, Buffer.buffer(), ID, ID2);
		Buffer.clear();
		va_end(VarArgs);
	}
}

// Все основные меню для работы в сервере
void CGS::ResetVotes(int ClientID, int MenuList)
{	
	CPlayer *pPlayer = GetPlayer(ClientID, true);
	if(!pPlayer)
		return;

	kurosio::kpause(3);	
	pPlayer->m_OpenVoteMenu = MenuList;
	ClearVotes(ClientID);

	if (Mmo()->OnPlayerHandleMainMenu(ClientID, MenuList, true))
	{
		m_apPlayers[ClientID]->m_Colored = { 20, 7, 15 };
		AVL(ClientID, "null", "The main menu will return as soon as you leave this zone!");
		return;
	}
	
	if(MenuList == MenuList::MAIN_MENU)
	{
		pPlayer->m_LastVoteMenu = MenuList::MAIN_MENU;

		// меню статистики
		const int ExpForLevel = pPlayer->ExpNeed(pPlayer->Acc().Level);
		AVH(ClientID, TAB_STAT, GREEN_COLOR, "Hi, {STR} Last log in {STR}", Server()->ClientName(ClientID), pPlayer->Acc().LastLogin);
		AVM(ClientID, "null", NOPE, TAB_STAT, "Discord: \"{STR}\". Ideas, bugs, rewards", g_Config.m_SvDiscordInviteGroup);
		AVM(ClientID, "null", NOPE, TAB_STAT, "Level {INT} : Exp {INT}/{INT}", &pPlayer->Acc().Level, &pPlayer->Acc().Exp, &ExpForLevel);
		AVM(ClientID, "null", NOPE, TAB_STAT, "Skill Point {INT}SP", &pPlayer->GetItem(itSkillPoint).Count);
		AVM(ClientID, "null", NOPE, TAB_STAT, "Gold: {INT}", &pPlayer->GetItem(itGold).Count);
		AV(ClientID, "null", "");

		// меню персонал
		AVH(ClientID, TAB_PERSONAL, GRAY_COLOR, "☪ SUB MENU PERSONAL");
		AVM(ClientID, "MENU", MenuList::MENU_INVENTORY, TAB_PERSONAL, "Inventory"); 
		AVM(ClientID, "MENU", MenuList::MENU_EQUIPMENT, TAB_PERSONAL, "Equipment");
		AVM(ClientID, "MENU", MenuList::MENU_UPGRADE, TAB_PERSONAL, "Upgrades({INT}p)", &pPlayer->Acc().Upgrade);
		AVM(ClientID, "MENU", MenuList::MENU_DUNGEONS, TAB_PERSONAL, "Dungeons");
		AVM(ClientID, "MENU", MenuList::MENU_SETTINGS, TAB_PERSONAL, "Settings");
		AVM(ClientID, "MENU", MenuList::MENU_INBOX, TAB_PERSONAL, "Mailbox");
		AVM(ClientID, "MENU", MenuList::MENU_JOURNAL_MAIN, TAB_PERSONAL, "Journal");
		AVM(ClientID, "MENU", MenuList::MENU_GUILD, TAB_PERSONAL, "Guild");
		if(Mmo()->House()->PlayerHouseID(pPlayer) > 0)
			AVM(ClientID, "MENU", MenuList::MENU_HOUSE, TAB_PERSONAL, "House");
		AV(ClientID, "null", "");

		// меню информации
		AVH(ClientID, TAB_INFORMATION, BLUE_COLOR, "√ SUB MENU INFORMATION");
		AVM(ClientID, "MENU", MenuList::MENU_GUIDEDROP, TAB_INFORMATION, "Loots, mobs on your zone");
		AVM(ClientID, "MENU", MenuList::MENU_TOP_LIST, TAB_INFORMATION, "Ranking guilds and players");
		AV(ClientID, "null", "");

		// чекаем местонахождение
		CCharacter* pChar = pPlayer->GetCharacter();
		if(!pChar || !pChar->IsAlive())
			return;

		if (pChar->GetHelper()->BoolIndex(TILE_GUILD_HOUSE))
		{
			const int HouseID = Mmo()->Member()->GetPosHouseID(pChar->m_Core.m_Pos);
			Mmo()->Member()->ShowBuyHouse(pPlayer, HouseID);
		}
	}
	else if(MenuList == MenuList::MENU_JOURNAL_MAIN)
	{
		pPlayer->m_LastVoteMenu = MenuList::MAIN_MENU;

		Mmo()->Quest()->ShowFullQuestLift(pPlayer);
		AddBack(ClientID);
	}
	else if(MenuList == MenuList::MENU_INBOX) 
	{
		pPlayer->m_LastVoteMenu = MenuList::MAIN_MENU;

		AddBack(ClientID);
		AV(ClientID, "null", "");
		Mmo()->Inbox()->GetInformationInbox(pPlayer);
	}
	else if(MenuList == MenuList::MENU_UPGRADE) 
	{
		pPlayer->m_LastVoteMenu = MenuList::MAIN_MENU;

		AVH(ClientID, TAB_INFO_UPGR, GREEN_COLOR, "Upgrades Information");
		AVM(ClientID, "null", NOPE, TAB_INFO_UPGR, "Select upgrades type in Reason, write count.");
		AV(ClientID, "null", "");

		ShowPlayerStats(pPlayer);

		// Улучшения класса DPS дамаг
		int Range = pPlayer->GetLevelDisciple(AtributType::AtDps);
		AVH(ClientID, TAB_UPGR_DPS, RED_COLOR, "Disciple of War. Level Power {INT}", &Range);
		for(const auto& at : AttributInfo)
		{
			if(at.second.AtType != AtributType::AtDps || str_comp_nocase(at.second.FieldName, "unfield") == 0 || at.second.UpgradePrice <= 0) 
				continue;
			AVD(ClientID, "UPGRADE", at.first, at.second.UpgradePrice, TAB_UPGR_DPS, "{STR} {INT}P (Price {INT}P)", at.second.Name, &pPlayer->Acc().Stats[at.first], &at.second.UpgradePrice);
		}
		AV(ClientID, "null", "");

		// Улучшения класса TANK танк
		Range = pPlayer->GetLevelDisciple(AtributType::AtTank);
		AVH(ClientID, TAB_UPGR_TANK, BLUE_COLOR, "Disciple of Tank. Level Power {INT}", &Range);
		for(const auto& at : AttributInfo)
		{
			if(at.second.AtType != AtributType::AtTank || str_comp_nocase(at.second.FieldName, "unfield") == 0 || at.second.UpgradePrice <= 0) 
				continue;
			AVD(ClientID, "UPGRADE", at.first, at.second.UpgradePrice, TAB_UPGR_TANK, "{STR} {INT}P (Price {INT}P)", at.second.Name, &pPlayer->Acc().Stats[at.first], &at.second.UpgradePrice);
		}
		AV(ClientID, "null", "");

		// Улучшения класса HEALER хил
		Range = pPlayer->GetLevelDisciple(AtributType::AtHealer);
		AVH(ClientID, TAB_UPGR_HEALER, GREEN_COLOR, "Disciple of Healer. Level Power {INT}", &Range);
		for(const auto& at : AttributInfo)
		{
			if(at.second.AtType != AtributType::AtHealer || str_comp_nocase(at.second.FieldName, "unfield") == 0 || at.second.UpgradePrice <= 0) 
				continue;
			AVD(ClientID, "UPGRADE", at.first, at.second.UpgradePrice, TAB_UPGR_HEALER, "{STR} {INT}P (Price {INT}P)", at.second.Name, &pPlayer->Acc().Stats[at.first], &at.second.UpgradePrice);
		}
		AV(ClientID, "null", "");

		// Улучшения WEAPONS оружия
		AVH(ClientID, TAB_UPGR_WEAPON, GRAY_COLOR, "Upgrades Weapons / Ammo");
		for(const auto& at : AttributInfo)
		{
			if(at.second.AtType != AtributType::AtWeapon || str_comp_nocase(at.second.FieldName, "unfield") == 0 || at.second.UpgradePrice <= 0) 
				continue;
			AVD(ClientID, "UPGRADE", at.first, at.second.UpgradePrice, TAB_UPGR_WEAPON, "{STR} {INT}P (Price {INT}P)", at.second.Name, &pPlayer->Acc().Stats[at.first], &at.second.UpgradePrice);
		}

		AV(ClientID, "null", ""), 
		AVH(ClientID, TAB_UPGR_JOB, GOLDEN_COLOR, "Disciple of Jobs");
		Mmo()->PlantsAcc()->ShowMenu(ClientID);
		Mmo()->MinerAcc()->ShowMenu(pPlayer);
		AddBack(ClientID);
	}
	else if (MenuList == MenuList::MENU_TOP_LIST)
	{
		pPlayer->m_LastVoteMenu = MenuList::MAIN_MENU;
		AVH(ClientID, TAB_INFO_TOP, GREEN_COLOR, "Ranking Information");
		AVM(ClientID, "null", NOPE, TAB_INFO_TOP, "Here you can see top server Guilds, Players.");
		AV(ClientID, "null", "");

		m_apPlayers[ClientID]->m_Colored = { 20,7,15 };
		AVM(ClientID, "SELECTEDTOP", ToplistTypes::GUILDS_LEVELING, NOPE, "Top 10 guilds leveling");
		AVM(ClientID, "SELECTEDTOP", ToplistTypes::GUILDS_WEALTHY, NOPE, "Top 10 guilds wealthy");
		AVM(ClientID, "SELECTEDTOP", ToplistTypes::PLAYERS_LEVELING, NOPE, "Top 10 players leveling");
		AddBack(ClientID);
	}
	else if(MenuList == MenuList::MENU_GUIDEDROP) 
	{
		pPlayer->m_LastVoteMenu = MenuList::MAIN_MENU;
		AVH(ClientID, TAB_INFO_LOOT, GREEN_COLOR, "Chance & Loot Information");
		AVM(ClientID, "null", NOPE, TAB_INFO_LOOT, "Here you can see chance loot, mobs, on YOUR ZONE.");
		AV(ClientID, "null", "");

		char aBuf[128];
		bool FoundedBots = false;
		for(const auto& mobs : BotJob::MobBot)
		{
			if (!IsClientEqualWorldID(ClientID, mobs.second.WorldID))
				continue;

			const int HideID = (NUM_TAB_MENU+12500+mobs.first);
			const int PosX = mobs.second.PositionX/32, PosY = mobs.second.PositionY/32;
			AVH(ClientID, HideID, LIGHT_BLUE_COLOR, "{STR} {STR}(x{INT} y{INT})", mobs.second.GetName(), Server()->GetWorldName(mobs.second.WorldID), &PosX, &PosY);
	
			for(int i = 0; i < MAX_DROPPED_FROM_MOBS; i++)
			{
				if(mobs.second.DropItem[i] <= 0 || mobs.second.CountItem[i] <= 0)
					continue;
			
				double Chance = (double)mobs.second.RandomItem[i];
				ItemJob::ItemInformation &InfoDropItem = GetItemInfo(mobs.second.DropItem[i]);
				str_format(aBuf, sizeof(aBuf), "%sx%d - chance to loot %0.2f%%", InfoDropItem.GetName(pPlayer), mobs.second.CountItem[i], Chance);
				AVMI(ClientID, InfoDropItem.GetIcon(), "null", NOPE, HideID, "{STR}", aBuf);
				FoundedBots = true;
			}
		}

		if (!FoundedBots)
			AVL(ClientID, "null", "There are no active mobs in your zone!");

		AddBack(ClientID);
	}
		
	Mmo()->OnPlayerHandleMainMenu(ClientID, MenuList, false);
}

// Информация для неавтаризованных
void CGS::ResetVotesNewbieInformation(int ClientID)
{
	CPlayer *pPlayer = GetPlayer(ClientID);
	if(!pPlayer)
		return;
		
	AVL(ClientID, "null", "#### Hi, new adventurer! ####");
	AVL(ClientID, "null", "This server is a mmo server. You'll have to finish");
	AVL(ClientID, "null", "quests to continue the game. In these quests,");
	AVL(ClientID, "null", "you'll have to get items to give to quest npcs.");
	AVL(ClientID, "null", "To get a quest, you need to talk to NPCs.");
	AVL(ClientID, "null", "You talk to them by hammering them.");
	AVL(ClientID, "null", "You give these items by talking them again. ");
	AVL(ClientID, "null", "Hearts and Shields around you show the position");
	AVL(ClientID, "null", "quests' npcs. Hearts show Main quest, Shields show Others.");
	AVL(ClientID, "null", "Don't ask other people to give you the items,");
	AVL(ClientID, "null", "but you can ask for some help. Keep in mind that");
	AVL(ClientID, "null", "it is hard for everyone too. You can see that your shield");
	AVL(ClientID, "null", "(below your health bar) doesn't protect you,");
	AVL(ClientID, "null", "it's because it's not shield, it's mana.");
	AVL(ClientID, "null", "It is used for active skills, which you will need to buy");
	AVL(ClientID, "null", "in the future. Active skills use mana, but they use %% of mana.");
	AV(ClientID, "null", "");
	AVL(ClientID, "null", "#### Some Informations ####");
	AVL(ClientID, "null", "Brown Slimes drop Glue and Gel (lower chance to drop Gel)");
	AVL(ClientID, "null", "Green Slimes drop Glue");
	AVL(ClientID, "null", "Blue Slimes drop Gel");
	AVL(ClientID, "null", "Pink Slime drop 3 Gels and 3 Glue");
	AVL(ClientID, "null", "Goblins drop Goblin Ingots");
	AVL(ClientID, "null", "Orc Warrior drop Goblin Ingot and Torn Cloth Clothes of Orc");
	AVL(ClientID, "null", "- A more accurate drop can be found in the menu");
	AV(ClientID, "null", "");
	AVL(ClientID, "null", "#### The upgrades now ####");
	AVL(ClientID, "null", "- Strength : Damage");
	AVL(ClientID, "null", "- Dexterity : Shooting speed");
	AVL(ClientID, "null", "- Crit Dmg : Damage dealt by critical hits");
	AVL(ClientID, "null", "- Direct Crit Dmg : Critical chance");
	AVL(ClientID, "null", "- Hardness : Health");
	AVL(ClientID, "null", "- Lucky : Chance to avoid damage");
	AVL(ClientID, "null", "- Piety : Mana");
	AVL(ClientID, "null", "- Vampirism : Damage dealt converted into health");
	AVL(ClientID, "null", "All things you need is in Vote.");
	AVL(ClientID, "null", "Good game !");
}

// Созданно для апдейта меню если именно оно находится в открытых
void CGS::VResetVotes(int ClientID, int MenuID)
{
	if(m_apPlayers[ClientID] && m_apPlayers[ClientID]->m_OpenVoteMenu == MenuID)
		ResetVotes(ClientID, MenuID);
}

// Кнопка назад добавляет кнопку назад в меню (Но нужно не забывать указывать ID последнего меню)
void CGS::AddBack(int ClientID)
{	
	if(!m_apPlayers[ClientID]) 
		return;

	AV(ClientID, "null", "");
	m_apPlayers[ClientID]->m_Colored = RED_COLOR;
	AVL(ClientID, "BACK", "Backpage");
	m_apPlayers[ClientID]->m_Colored = {0,0,0};
}

// Вывести статистику игрока
void CGS::ShowPlayerStats(CPlayer *pPlayer)
{
	const int ClientID = pPlayer->GetCID();
	AVH(ClientID, TAB_INFO_STAT, BLUE_COLOR, "Player Stats {STR}", IsDungeon() ? "(Sync)" : "\0");
	for(const auto& at : AttributInfo)
	{
		if(str_comp_nocase(at.second.FieldName, "unfield") == 0) 
			continue;
	
		// если апгрейды стоят дешево то они имеют деление ральных статистик
		const int AttributeSize = pPlayer->GetAttributeCount(at.first);
		if(at.second.UpgradePrice < 10)
		{
			const int AttributeRealSize = pPlayer->GetAttributeCount(at.first, true);
			AVM(ClientID, "null", NOPE, TAB_INFO_STAT, "{INT} (+{INT}) - {STR}", &AttributeSize, &AttributeRealSize, AtributeName(at.first));
			continue;
		}

		// если апгрейды дорогие они имеют 1 статистики
		AVM(ClientID, "null", NOPE, TAB_INFO_STAT, "+{INT} - {STR}", &AttributeSize, AtributeName(at.first));
	}

	AVM(ClientID, "null", NOPE, NOPE, "Player Upgrade Point: {INT}P", &pPlayer->Acc().Upgrade);
	AV(ClientID, "null", "");
}

// Вывести информацию по голде
void CGS::ShowValueInformation(CPlayer *pPlayer, int ItemID)
{
	const int ClientID = pPlayer->GetCID();
	pPlayer->m_Colored = LIGHT_PURPLE_COLOR;
	AVMI(ClientID, GetItemInfo(ItemID).GetIcon(), "null", NOPE, NOPE, "You have {INT} {STR}", &pPlayer->GetItem(ItemID).Count, GetItemInfo(ItemID).GetName());
}

// Парсинг голосований всех функций методов действий
bool CGS::ParseVote(int ClientID, const char *CMD, const int VoteID, const int VoteID2, int Get, const char *Text)
{
	// проверка на игрока
	CPlayer *pPlayer = GetPlayer(ClientID, false, true);
	if(!pPlayer)
	{
		Chat(ClientID, "Use it when you're not dead!");
		return true;
	} 

	if(PPSTR(CMD, "null") == 0) 
		return true;
	else if(PPSTR(CMD, "BACK") == 0)
	{
		ResetVotes(ClientID, pPlayer->m_LastVoteMenu);
		return true;
	}
	else if(PPSTR(CMD, "MENU") == 0)
	{
		ResetVotes(ClientID, VoteID);
		return true;
	}
	else if (PPSTR(CMD, "SELECTEDTOP") == 0)
	{
		ResetVotes(ClientID, MenuList::MENU_TOP_LIST);
		AV(ClientID, "null", "\0");
		Mmo()->ShowTopList(pPlayer, VoteID);
		return true;
	}
	else if(pPlayer->ParseVoteUpgrades(CMD, VoteID, VoteID2, Get)) 
		return true;

	// парсинг всего остального
	sqlstr::CSqlString<64> FormatText = sqlstr::CSqlString<64>(Text);
	return (bool)(Mmo()->OnParseFullVote(pPlayer, CMD, VoteID, VoteID2, Get, FormatText.cstr()));
}

/* #########################################################################
	MMO GAMECONTEXT 
######################################################################### */
void CGS::CreateBot(short BotType, int BotID, int SubID)
{
	int BotClientID = MAX_PLAYERS;
	while(BotClientID < MAX_CLIENTS && m_apPlayers[BotClientID])
		BotClientID++;
	if (BotClientID >= MAX_CLIENTS)
		return;

	Server()->InitClientBot(BotClientID);
	const int savecidmem = BotClientID+m_WorldID*MAX_CLIENTS;
	m_apPlayers[BotClientID] = new(savecidmem) CPlayerBot(this, BotClientID, BotID, SubID, BotType);
}

// Удалить ботов что не активны у людей для квестов
void CGS::UpdateQuestsBot(int QuestID, int Step)
{
	for(auto& FindBot : BotJob::QuestBot)
	{
		if(QuestID != FindBot.second.QuestID || Step != FindBot.second.Progress)
			continue;

		// перекидываем recheck на мир моба
		if(FindBot.second.WorldID != GetWorldID())
		{
			Server()->QuestBotUpdateOnWorld(FindBot.second.WorldID, QuestID, Step);
			continue;
		}

		// ищем есть ли такой бот
		int QuestBotClientID = -1;
		for(int i = MAX_PLAYERS ; i < MAX_CLIENTS; i++)
		{
			if(!m_apPlayers[i] || m_apPlayers[i]->GetBotType() != BotsTypes::TYPE_BOT_QUEST || m_apPlayers[i]->GetBotSub() != FindBot.second.SubBotID) 
				continue;
			QuestBotClientID = i;
		}

		// ищем есть ли активный бот у всех игроков
		const bool ActiveBot = Mmo()->Quest()->IsActiveQuestBot(QuestID, Step);
		if(ActiveBot && QuestBotClientID <= -1)
			CreateBot(BotsTypes::TYPE_BOT_QUEST, FindBot.second.BotID, FindBot.second.SubBotID);

		// если бот не активен не у одного игрока, но игрок найден удаляем
		if (!ActiveBot && QuestBotClientID >= MAX_PLAYERS)
		{
			delete m_apPlayers[QuestBotClientID];
			m_apPlayers[QuestBotClientID] = nullptr;
		}
	}
}

// Создать Лол текст в мире
void CGS::CreateText(CEntity *pParent, bool Follow, vec2 Pos, vec2 Vel, int Lifespan, const char *pText)
{
	if(!CheckPlayersDistance(Pos, 800))
		return;

	CLoltext Text;
	Text.Create(&m_World, pParent, Pos, Vel, Lifespan, pText, true, Follow);
	return;
}

// Саздает бонус в позиции Типа и Количества и Их самих кол-ва
void CGS::CreateDropBonuses(vec2 Pos, int Type, int Count, int NumDrop, vec2 Force)
{
	for(int i = 0; i < NumDrop; i++) 
	{
		vec2 Vel = Force + vec2(frandom() * 15.0, frandom() * 15.0);
		const float Angle = Force.x * (0.15f + frandom() * 0.1f);
		new CDropBonuses(&m_World, Pos, Vel, Angle, Type, Count);
	}
}

// Саздает предметы в позиции Типа и Количества и Их самих кол-ва
void CGS::CreateDropItem(vec2 Pos, int ClientID, int ItemID, int Count, int Enchant, vec2 Force)
{
	ItemJob::InventoryItem DropItem(nullptr, ItemID);
	DropItem.Count = Count;
	DropItem.Enchant = Enchant;

	vec2 Vel = Force + vec2(frandom() * 15.0, frandom() * 15.0);
	const float Angle = Force.x * (0.15f + frandom() * 0.1f);
	new CDropItem(&m_World, Pos, Vel, Angle, DropItem, ClientID);
}

// Саздает предметы в позиции Типа и Количества и Их самих кол-ва
void CGS::CreateDropItem(vec2 Pos, int ClientID, ItemJob::InventoryItem &pPlayerItem, int Count, vec2 Force)
{
	ItemJob::InventoryItem CopyItem = pPlayerItem;
	CopyItem.Count = Count;

	if (pPlayerItem.Remove(Count))
	{
		const float Angle = Force.x * (0.15f + frandom() * 0.1f);
		new CDropItem(&m_World, Pos, Force, Angle, CopyItem, ClientID);
	}
}

// Проверить чекнуть и подобрать предмет Если он будет найден
bool CGS::TakeItemCharacter(int ClientID)
{
	CPlayer *pPlayer = GetPlayer(ClientID, true, true);
	if(!pPlayer)
		return false;

	CDropItem *pDrop = (CDropItem*)m_World.ClosestEntity(pPlayer->GetCharacter()->m_Core.m_Pos, 64, CGameWorld::ENTTYPE_DROPITEM, nullptr);
	if(pDrop) { return pDrop->TakeItem(ClientID);}
	return false;
}

// Отправить сообщение с предметом или без используя ClientID
void CGS::SendInbox(int ClientID, const char* Name, const char* Desc, int ItemID, int Count, int Enchant)
{
	CPlayer* pPlayer = GetPlayer(ClientID, true);
	if(!pPlayer) 
		return;

	Mmo()->Inbox()->SendInbox(pPlayer->Acc().AuthID, Name, Desc, ItemID, Count, Enchant);
} 

// отправить информацию о дне
void CGS::SendDayInfo(int ClientID)
{
	if(ClientID == -1)
		Chat(-1, "{STR} came! Good {STR}!", Server()->GetStringTypeDay(), Server()->GetStringTypeDay());
	if(m_DayEnumType == DayType::NIGHTTYPE)
		Chat(ClientID, "Night increase to experience {INT}%", &m_RaidExp);
	else if(m_DayEnumType == DayType::MORNINGTYPE)
		Chat(ClientID, "Day, experience was downgraded to 100%");
}

// Сменить Снаряжение автоматически найти тип по Предмету
void CGS::ChangeEquipSkin(int ClientID, int ItemID)
{
	CPlayer *pPlayer = GetPlayer(ClientID, true);
	if(!pPlayer || GetItemInfo(ItemID).Type != ItemType::TYPE_EQUIP || GetItemInfo(ItemID).Function == EQUIP_DISCORD || GetItemInfo(ItemID).Function == EQUIP_MINER)
		return;

	SendEquipItem(ClientID, -1);
}

// Повышем кол-во для определеннго значения как рейд
int CGS::IncreaseCountRaid(int IncreaseCount) const
{
	if(IsDungeon())
		return (int)kurosio::translate_to_procent_rest(IncreaseCount, 150);
	return (int)kurosio::translate_to_procent_rest(IncreaseCount, m_RaidExp);
}

void CGS::UpdateZonePVP()
{
	m_AllowedPVP = false;
	if(IsDungeon())
		return;
	
	for(int i = MAX_PLAYERS; i < MAX_CLIENTS; i++)
	{
		CPlayerBot* BotPlayer = static_cast<CPlayerBot*>(m_apPlayers[i]);
		if(BotPlayer && BotPlayer->GetBotType() == BotsTypes::TYPE_BOT_MOB && GetClientWorldID(i) == m_WorldID)
		{
			m_AllowedPVP = true;
			return;
		}
	}
}

void CGS::UpdateZoneDungeon()
{
	for(const auto& dd : DungeonJob::Dungeon)
	{
		if(m_WorldID == dd.second.WorldID)
		{
			m_DungeonID = dd.first;
			return;
		}
	}
	m_DungeonID = 0;
}

bool CGS::IsClientEqualWorldID(int ClientID, int WorldID) const
{
	if(!m_apPlayers[ClientID])
		return false;

	if (WorldID <= -1)
		return (bool)(m_apPlayers[ClientID]->GetPlayerWorldID() == m_WorldID);
	return (bool)(m_apPlayers[ClientID]->GetPlayerWorldID() == WorldID);
}

const char* CGS::AtributeName(int BonusID) const
{
	if(CGS::AttributInfo.find(BonusID) != CGS::AttributInfo.end())
		return CGS::AttributInfo[BonusID].Name;
	return "Has no stats";
}

bool CGS::CheckPlayersDistance(vec2 Pos, float Distance) const
{
	for(int i = 0; i < MAX_PLAYERS; i++)
	{
		if(m_apPlayers[i] && GetClientWorldID(i) == GetWorldID() && distance(Pos, m_apPlayers[i]->m_ViewPos) <= Distance)
			return true;
	}
	return false;
}

IGameServer *CreateGameServer() { return new CGS; }
