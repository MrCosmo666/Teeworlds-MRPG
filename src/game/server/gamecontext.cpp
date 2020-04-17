/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#include <base/math.h>
#include <thread>
#include <algorithm>

#include <engine/shared/config.h>
#include <engine/shared/memheap.h>
#include <engine/map.h>

#include <generated/server_data.h>
#include <game/version.h>

#include "gamecontext.h"
#include <game/collision.h>
#include <game/gamecore.h>
#include "gamemodes/mod.h"
#include "gamemodes/dungeon.h"

// object entities
#include "mmocore/items/dropingbonuses.h"
#include "mmocore/items/dropingitem.h"
#include "mmocore/mmocontroller/loltext.h"

// информационные данные
#include "mmocore/cmds.h"

// Безопасные структуры хоть и прожорливо но работает (Прежде всего всегда их при выходе игрока Отрезаем)
std::map < int , CGS::StructAttribut > CGS::AttributInfo;

// Структуры игроков

std::map < int , CGS::StructParsing > CGS::Interactive;
std::map < int , CGS::StructInteractiveSub > CGS::InteractiveSub;
std::map < int , std::map < std::string , int > > CGS::Effects;
int CGS::m_RaidExp = 100;

enum
{
	RESET,
	NO_RESET
};

// Конструктор сервера
void CGS::Construct(int Resetting)
{
	for (int i = 0; i < MAX_CLIENTS; i++)
		m_apPlayers[i] = 0;

	m_Resetting = 0;
	m_pServer = 0;

	m_pController = 0;
	if(Resetting==NO_RESET)
		pMmoController = 0;
}

// Конструктор сервера
CGS::CGS(int Resetting)
{
	Construct(Resetting);
}

// Конструктор сервера
CGS::CGS()
{
	Construct(NO_RESET);
}

// Дискруктор сервера
CGS::~CGS()
{
	for(int i = 0; i < MAX_CLIENTS; i++)
		delete m_apPlayers[i];

	if(pMmoController)  
		delete pMmoController;
}

// Очистка сервера  
void CGS::Clear()
{
	CTuningParams Tuning = m_Tuning;         
	m_Resetting = true;
	this->~CGS();
	mem_zero(this, sizeof(*this));
	new (this) CGS(RESET);

	for(int i = 0; i < MAX_PLAYERS; i++)
	{
		m_BroadcastStates[i].m_NoChangeTick = 0;
		m_BroadcastStates[i].m_LifeSpanTick = 0;
		m_BroadcastStates[i].m_Priority = 0;
		m_BroadcastStates[i].m_PrevMessage[0] = 0;
		m_BroadcastStates[i].m_NextMessage[0] = 0;
	}
	m_Tuning = Tuning;
}

/* #########################################################################
	HELPER PLAYER FUNCTION 
######################################################################### */
// Получить Character по ClientID
class CCharacter *CGS::GetPlayerChar(int ClientID)
{
	if(ClientID < 0 || ClientID >= MAX_CLIENTS || !m_apPlayers[ClientID])
		return 0;
	return m_apPlayers[ClientID]->GetCharacter();
}

// Легкое получение игрока (ClientID, Авторизации, Чарактора)
CPlayer *CGS::GetPlayer(int ClientID, bool CheckAuthed, bool CheckCharacter)
{
	if(ClientID >= 0 && ClientID < MAX_CLIENTS && m_apPlayers[ClientID]) 
	{
		CPlayer *pPlayer = m_apPlayers[ClientID];
		if(!CheckCharacter && !CheckAuthed)
			return pPlayer;
		else if((CheckAuthed && pPlayer->IsAuthed()) || !CheckAuthed)
		{
			if(CheckCharacter && !pPlayer->GetCharacter())
				return NULL;
			return pPlayer;
		}
	}
	return NULL;
}

// Level String by Matodor (Progress Bar) создает что то рода прогресс бара
const char* CGS::LevelString(int MaxValue, int CurrentValue, int Step, char toValue, char fromValue) 
{
    if (CurrentValue < 0) CurrentValue = 0;
    if (CurrentValue > MaxValue) CurrentValue = MaxValue;

    int Size = 2 + MaxValue / Step + 1;
    char *Buf = new char[Size];
    Buf[0] = '[';
    Buf[Size - 2] = ']';
    Buf[Size - 1] = '\0';

    int a = CurrentValue / Step;
    int b = (MaxValue - CurrentValue) / Step;
    int i = 1;

    for (int ai = 0; ai < a; ai++, i++) 
    {
        Buf[i] = toValue;
    }

    for (int bi = 0; bi < b || i < Size - 2; bi++, i++) 
    {
        Buf[i] = fromValue;
    }
    return Buf;
}
// получить объект предмета
ItemSql::ItemInformation &CGS::GetItemInfo(int ItemID) const { return ItemSql::ItemsInfo[ItemID]; }

/* #########################################################################
	EVENTS 
######################################################################### */
// Отправить запрос на рендер Урона
void CGS::CreateDamage(vec2 Pos, int Id, vec2 Source, int HealthAmount, int ArmorAmount, bool Self)
{
	CreateDamageTranslate(Pos, Id, Source, HealthAmount, ArmorAmount, Self);
}

// Транслированный урон // TODO: исправить упростить и не отправлять в обработку по 2000 инвентов за дамаг
void CGS::CreateDamageTranslate(vec2 Pos, int Id, vec2 Source, int HealthAmount, int ArmorAmount, bool Self)
{
	for(int i = 0 ; i < MAX_PLAYERS ; i ++)
	{
		if(!Server()->ClientIngame(i))
			continue;
		
		if(!CheckClient(i))
		{
			CNetEvent_Damage *pEventVanilla = (CNetEvent_Damage *)m_Events.Create(NETEVENTTYPE_DAMAGE, sizeof(CNetEvent_Damage), CmaskOne(i));
			if(pEventVanilla)
			{
				float f = angle(Source);
				pEventVanilla->m_X = (int)Pos.x;
				pEventVanilla->m_Y = (int)Pos.y;
				pEventVanilla->m_ClientID = Id;
				pEventVanilla->m_Angle = (int)(f*256.0f);
				pEventVanilla->m_HealthAmount = clamp(HealthAmount, 1, 9);
				pEventVanilla->m_ArmorAmount = clamp(ArmorAmount, 1, 9);
				pEventVanilla->m_Self = Self;
			}
			continue;
		}

		CNetEvent_MmoDamage *pEventMmo = (CNetEvent_MmoDamage *)m_Events.Create(NETEVENTTYPE_MMODAMAGE, sizeof(CNetEvent_MmoDamage), CmaskOne(i));
		if(pEventMmo)
		{
			pEventMmo->m_X = (int)Pos.x;
			pEventMmo->m_Y = (int)Pos.y;
			pEventMmo->m_ClientID = Id;
			pEventMmo->m_DamageCount = HealthAmount + ArmorAmount;
		}
	}
}

// Отправить запрос на рендер Удара молотка
void CGS::CreateHammerHit(vec2 Pos)
{
	// create the event
	CNetEvent_HammerHit *pEvent = (CNetEvent_HammerHit *)m_Events.Create(NETEVENTTYPE_HAMMERHIT, sizeof(CNetEvent_HammerHit), MaskWorldID());
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
	CNetEvent_Explosion *pEvent = (CNetEvent_Explosion *)m_Events.Create(NETEVENTTYPE_EXPLOSION, sizeof(CNetEvent_Explosion), MaskWorldID());
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
		if((int)(Factor * MaxDamage))
			apEnts[i]->TakeDamage(Force * Factor, Diff*-1, (int)(Factor * MaxDamage), Owner, Weapon);
	}
}

// Отправить запрос на рендер Спавна игрока
void CGS::CreatePlayerSpawn(vec2 Pos)
{
	// create the event
	CNetEvent_Spawn *ev = (CNetEvent_Spawn *)m_Events.Create(NETEVENTTYPE_SPAWN, sizeof(CNetEvent_Spawn), MaskWorldID());
	if(ev)
	{
		ev->m_X = (int)Pos.x;
		ev->m_Y = (int)Pos.y;
	}
}

// Отправить запрос на рендер Смерти игрока
void CGS::CreateDeath(vec2 Pos, int ClientID)
{
	// create the event
	CNetEvent_Death *pEvent = (CNetEvent_Death *)m_Events.Create(NETEVENTTYPE_DEATH, sizeof(CNetEvent_Death), MaskWorldID());
	if(pEvent)
	{
		pEvent->m_X = (int)Pos.x;
		pEvent->m_Y = (int)Pos.y;
		pEvent->m_ClientID = ClientID;
	}
}

// Отправить запрос на звук, по маске
void CGS::CreateSound(vec2 Pos, int Sound, int64 Mask)
{
	// create a sound
	CNetEvent_SoundWorld *pEvent = (CNetEvent_SoundWorld *)m_Events.Create(NETEVENTTYPE_SOUNDWORLD, sizeof(CNetEvent_SoundWorld), Mask);
	if(pEvent)
	{
		pEvent->m_X = (int)Pos.x;
		pEvent->m_Y = (int)Pos.y;
		pEvent->m_SoundID = Sound;
	}
}

// Отправить запрос на звук, по маске
void CGS::CreatePlayerSound(int ClientID, int Sound)
{
	int64 Mask = CmaskOne(ClientID);
	CreateSound(m_apPlayers[ClientID]->m_ViewPos, Sound, Mask);
}

// Отправить ммо эффекты
void CGS::SendMmoEffect(vec2 Pos, int EffectID)
{
	CNetEvent_EffectMmo *pEvent = (CNetEvent_EffectMmo *)m_Events.Create(NETEVENTTYPE_EFFECTMMO, sizeof(CNetEvent_EffectMmo), MaskWorldID());
	if(pEvent)
	{
		pEvent->m_X = (int)Pos.x;
		pEvent->m_Y = (int)Pos.y;
		pEvent->m_EffectID = EffectID;
	}
}

// Отправить эффект Зелья
void CGS::SendMmoPotion(vec2 Pos, const char *Potion, bool Added)
{
	CNetEvent_EffectPotion *pEvent = (CNetEvent_EffectPotion *)m_Events.Create(NETEVENTTYPE_EFFECTPOTION, sizeof(CNetEvent_EffectPotion), MaskWorldID());
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
		Server()->SendPackMsg(&Msg, MSGFLAG_VITAL, -1);
	else if(Mode == CHAT_TEAM)
	{
		// pack one for the recording only
		Server()->SendPackMsg(&Msg, MSGFLAG_VITAL|MSGFLAG_NOSEND, -1);

		To = m_apPlayers[ChatterClientID]->GetTeam();

		// send to the clients
		for(int i = 0; i < MAX_CLIENTS; i++)
		{
			if(m_apPlayers[i] && m_apPlayers[i]->GetTeam() == To)
				Server()->SendPackMsg(&Msg, MSGFLAG_VITAL|MSGFLAG_NORECORD, i);
		}
	}
	else // Mode == CHAT_WHISPER
	{
		// send to the clients
		Msg.m_TargetID = To;
		Server()->SendPackMsg(&Msg, MSGFLAG_VITAL, ChatterClientID);
		Server()->SendPackMsg(&Msg, MSGFLAG_VITAL, To);
	}
}

// Отправить форматированное сообщение
void CGS::Chat(int ClientID, const char* pText, ... )
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
		if (pPlayer && Server()->GetWorldID(i) == WorldID)
		{
			Buffer.append(Suffix);
			Server()->Localization()->Format_VL(Buffer, m_apPlayers[i]->GetLanguage(), pText, VarArgs);

			Msg.m_TargetID = i;
			Msg.m_pMessage = Buffer.buffer();

			Server()->SendPackMsg(&Msg, MSGFLAG_VITAL, i);
			Buffer.clear();
		}
	}
	va_end(VarArgs);
}

// Отправить дискорд сообщение
void CGS::ChatDiscord(bool Icon, const char *Color, const char *Title, const char* pText, ...)
{
#ifdef CONF_DISCORD
	va_list VarArgs;
	va_start(VarArgs, pText);
	
	dynamic_string Buffer;
	Server()->Localization()->Format_VL(Buffer, "en", pText, VarArgs);
	Server()->SendDiscordMessage(g_Config.m_SvDiscordChanal, Color, Title, Buffer.buffer(), Icon);
	Buffer.clear();

	va_end(VarArgs);
#endif
}

// Отправить дискорд сообщение в канал
void CGS::ChatDiscordChannel(bool Icon, const char *pChanel, const char *Color, const char *Title, const char* pText, ...)
{
#ifdef CONF_DISCORD
	va_list VarArgs;
	va_start(VarArgs, pText);
	
	dynamic_string Buffer;
	Server()->Localization()->Format_VL(Buffer, "en", pText, VarArgs);
	Server()->SendDiscordMessage(pChanel, Color, Title, Buffer.buffer(), Icon);
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
	int End = (ClientID < 0 ? MAX_CLIENTS : ClientID+1);
	
	for(int i = Start; i < End; i++)
	{
		if(m_apPlayers[i])
			AddBroadcast(i, pText, Priority, LifeSpan);
	}
}

// Добавить броадкаст
void CGS::AddBroadcast(int ClientID, const char* pText, int Priority, int LifeSpan)
{
	if(ClientID >= 0 && ClientID < MAX_PLAYERS)
	{
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
}

// Форматированный броадкаст
void CGS::SBL(int ClientID, int Priority, int LifeSpan, const char *pText, ...)
{
	int Start = (ClientID < 0 ? 0 : ClientID);
	int End = (ClientID < 0 ? MAX_CLIENTS : ClientID+1);
	
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
		if(m_apPlayers[i] && Server()->GetWorldID(i) == WorldID)
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
	if(m_apPlayers[ClientID] && GetWorldID() == Server()->GetWorldID(ClientID))
	{
		if(m_BroadcastStates[ClientID].m_LifeSpanTick > 0 && m_BroadcastStates[ClientID].m_TimedPriority > m_BroadcastStates[ClientID].m_Priority)
		{
			str_copy(m_BroadcastStates[ClientID].m_NextMessage, m_BroadcastStates[ClientID].m_TimedMessage, sizeof(m_BroadcastStates[ClientID].m_NextMessage));
		}
		
		//Send broadcast only if the message is different, or to fight auto-fading
		if(str_comp(m_BroadcastStates[ClientID].m_PrevMessage, m_BroadcastStates[ClientID].m_NextMessage) != 0 ||
			m_BroadcastStates[ClientID].m_NoChangeTick > Server()->TickSpeed()
		)
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
// Отправить эмоции что на шифт
void CGS::SendEmoticon(int ClientID, int Emoticon)
{
	CNetMsg_Sv_Emoticon Msg;
	Msg.m_ClientID = ClientID;
	Msg.m_Emoticon = Emoticon;
	Server()->SendPackMsg(&Msg, MSGFLAG_VITAL, -1, m_WorldID);
}

// Отправить оружие подбора
void CGS::SendWeaponPickup(int ClientID, int Weapon)
{
	CNetMsg_Sv_WeaponPickup Msg;
	Msg.m_Weapon = Weapon;
	Server()->SendPackMsg(&Msg, MSGFLAG_VITAL, ClientID);
}

// Отправить мотд
void CGS::SendMotd(int ClientID)
{
	CNetMsg_Sv_Motd Msg;
	Msg.m_pMessage = g_Config.m_SvMotd;
	Server()->SendPackMsg(&Msg, MSGFLAG_VITAL, ClientID);
}

// Отправить настройки сервера
void CGS::SendSettings(int ClientID)
{
	CNetMsg_Sv_ServerSettings Msg;
	Msg.m_KickVote = g_Config.m_SvVoteKick;
	Msg.m_KickMin = g_Config.m_SvVoteKickMin;
	Msg.m_SpecVote = g_Config.m_SvVoteSpectate;
	Msg.m_TeamLock = 0;
	Msg.m_TeamBalance = 0;
	Msg.m_PlayerSlots = MAX_CLIENTS;
	Server()->SendPackMsg(&Msg, MSGFLAG_VITAL, ClientID);
}

// Отправить смену скинна
void CGS::SendSkinChange(int ClientID, int TargetID)
{
	if(!m_apPlayers[ClientID])
		return;

	CNetMsg_Sv_SkinChange Msg;
	Msg.m_ClientID = ClientID;
	for(int p = 0; p < NUM_SKINPARTS; p++)
	{
		Msg.m_apSkinPartNames[p] = m_apPlayers[ClientID]->Acc().m_aaSkinPartNames[p];
		Msg.m_aUseCustomColors[p] = m_apPlayers[ClientID]->Acc().m_aUseCustomColors[p];
		Msg.m_aSkinPartColors[p] = m_apPlayers[ClientID]->Acc().m_aSkinPartColors[p];
	}
	Server()->SendPackMsg(&Msg, MSGFLAG_VITAL|MSGFLAG_NORECORD, TargetID);
}

// Отправить Equip Items
void CGS::SendEquipItem(int ClientID, int TargetID)
{
	if(!m_apPlayers[ClientID] || !m_apPlayers[ClientID]->IsAuthed())
		return;

	CNetMsg_Sv_EquipItems Msg;
	Msg.m_ClientID = ClientID;
	for(int k = 0; k < NUM_EQUIPS; k++)
	{
		int EquipItem = m_apPlayers[ClientID]->GetItemEquip(k);
		Msg.m_EquipID[k] = EquipItem;
		Msg.m_EnchantItem[k] = m_apPlayers[ClientID]->GetItem(EquipItem).Enchant;
	}
	Server()->SendPackMsg(&Msg, MSGFLAG_VITAL|MSGFLAG_NORECORD, TargetID);
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
void CGS::SendTalkText(int OwnID, int TalkingID, bool PlayerTalked, const char *Message, int Style, int TalkingEmote)
{
	if(!CheckClient(OwnID))
		return;

	CNetMsg_Sv_TalkText Msg;
	Msg.m_PlayerTalked = PlayerTalked;
	Msg.m_pTalkClientID = TalkingID;
	Msg.m_pText = Message;
	Msg.m_TalkedEmote = TalkingEmote;
	Msg.m_Style = Style;
	Server()->SendPackMsg(&Msg, MSGFLAG_VITAL, OwnID);
}

void CGS::ClearTalkText(int OwnID)
{
	CNetMsg_Sv_ClearTalkText Msg;
	Server()->SendPackMsg(&Msg, MSGFLAG_VITAL, OwnID);	
}

// Помощь в поиске мира бота и отправки его
int CGS::CheckPlayerMessageWorldID(int ClientID)
{
	if(ClientID < 0 || ClientID >= MAX_CLIENTS || !m_apPlayers[ClientID])
		return -1;

	if(ClientID >= MAX_PLAYERS)
	{
		CPlayerBot* pPlayer = static_cast<CPlayerBot *>(m_apPlayers[ClientID]);
		int SubBotID = pPlayer->GetBotSub();
		if(pPlayer->GetSpawnBot() == SPAWNMOBS) 
			return ContextBots::MobBot[SubBotID].WorldID;
		else if(pPlayer->GetSpawnBot() == SPAWNNPC) 
			return ContextBots::NpcBot[SubBotID].WorldID;
		else 
			return ContextBots::QuestBot[SubBotID].WorldID;
	}
	else 
		return Server()->GetWorldID(ClientID);
}

// Маска игроков с одного мира и сервера
int64 CGS::MaskWorldID()
{
	int64 Mask = -1;
	for(int i = 0; i < MAX_PLAYERS; i++)
	{
		if(Server()->GetWorldID(i) == GetWorldID())
			Mask |= CmaskOne(i);
	}
	return Mask;
}

/* #########################################################################
	ENGINE GAMECONTEXT 
######################################################################### */
// инициализация GameContext данных
void CGS::OnInit(int WorldID)
{
	m_pServer = Kernel()->RequestInterface<IServer>();
	m_pConsole = Kernel()->RequestInterface<IConsole>();
	m_World.SetGameServer(this);
	m_Events.SetGameServer(this);
	m_DungeonID = ItDungeon(WorldID);
	m_WorldID = WorldID;

	for(int i = 0; i < NUM_NETOBJTYPES; i++)
		Server()->SnapSetStaticsize(i, m_NetObjHandler.GetObjSize(i));

	// создаем все гейм обьекты для сервера
	if(m_DungeonID >= 1)
	{
		m_pController = new CGameControllerDungeon(this);
	}
	else
	{
		m_pController = new CGameControllerMOD(this);
	}

	// создаем контроллер
	m_Layers.Init(Kernel(), NULL, WorldID);
	m_Collision.Init(&m_Layers);
	pMmoController = new SqlController(this);
	pMmoController->LoadLogicWorld();

	// инициализируем слои
	CMapItemLayerTilemap *pTileMap = m_Layers.GameLayer();
	CTile *pTiles = (CTile *)Kernel()->RequestInterface<IMap>(WorldID)->GetData(pTileMap->m_Data);
	for(int y = 0; y < pTileMap->m_Height; y++) {
		for(int x = 0; x < pTileMap->m_Width; x++) {
			int Index = pTiles[y*pTileMap->m_Width+x].m_Index;
			if(Index >= ENTITY_OFFSET) {
				vec2 Pos(x*32.0f+16.0f, y*32.0f+16.0f);
				m_pController->OnEntity(Index-ENTITY_OFFSET, Pos);
			}
		}
	}

	// ставим на постоянную обработку
	Console()->Chain("sv_motd", ConchainSpecialMotdupdate, this);
	Console()->Chain("sv_vote_kick", ConchainSettingUpdate, this);
	Console()->Chain("sv_vote_kick_min", ConchainSettingUpdate, this);
	Console()->Chain("sv_vote_spectate", ConchainSettingUpdate, this);
	Console()->Chain("sv_scorelimit", ConchainGameinfoUpdate, this);
	Console()->Chain("sv_timelimit", ConchainGameinfoUpdate, this);
	Console()->Chain("sv_matches_per_map", ConchainGameinfoUpdate, this);
}

// Инициализация консоли
void CGS::OnConsoleInit()
{
	m_pServer = Kernel()->RequestInterface<IServer>();
	m_pConsole = Kernel()->RequestInterface<IConsole>();

	Console()->Register("parseskin", "i", CFGFLAG_SERVER, ConParseSkin, this, "Parse skin on console. Easy for devlop bots.");
	Console()->Register("giveitem", "iiiii", CFGFLAG_SERVER, ConGiveItem, this, "Give item <clientid> <itemid> <count> <enchant> <mail 1=yes 0=no>");
	Console()->Register("tune", "si", CFGFLAG_SERVER, ConTuneParam, this, "Tune variable to value");
	Console()->Register("tune_reset", "", CFGFLAG_SERVER, ConTuneReset, this, "Reset tuning");
	Console()->Register("tune_dump", "", CFGFLAG_SERVER, ConTuneDump, this, "Dump tuning");
	Console()->Register("say", "r", CFGFLAG_SERVER, ConSay, this, "Say in chat");
	Console()->Register("addcharacter", "ir", CFGFLAG_SERVER, ConAddCharacter, this, "Add new bot on datatable <clientid> <nick name>");
}

// Отключение сервера
void CGS::OnShutdown()
{
	delete m_pController;
	m_pController = 0;

	delete pMmoController;
	pMmoController = 0;
	Clear();
}

// Таймер GameContext
void CGS::OnTick()
{
	// копируем тюннинг
	m_World.m_Core.m_Tuning = m_Tuning;
	m_World.Tick();

	//if(Server()->Tick() % Server()->TickSpeed() / 2 == 0)
	//	SJK.UD("tw_devnotepad", "nDesc = '1.3.5 pre-relased' WHERE ID = '1'");

	// контроллер тик
	m_pController->Tick();
	for(int i = 0; i < MAX_CLIENTS; i++)
	{
		if(m_apPlayers[i] && m_WorldID == CheckPlayerMessageWorldID(i))
		{
			m_apPlayers[i]->Tick();
			if(i < MAX_PLAYERS)
			{
				m_apPlayers[i]->PostTick();
				BroadcastTick(i);
			}
		}
	}

	OnTickLocalWorld();
	Mmo()->OnTick();

	// обновить точки Click Click
	if (Server()->CheckWorldTime(6, 59))
		m_pController->RespawnedClickEvent();
}

// Таймер в OnTick-=
void CGS::OnTickLocalWorld()
{
	if(m_WorldID != LOCALWORLD) 
		return;

	// получить и отправить измененный день
	if(m_DayEnumType != Server()->GetEnumTypeDay())
	{
		int Hour = Server()->GetHourWorld();
		Chat(-1, "{STR} came! Good {STR}! Time {INT}:00", Server()->GetStringTypeDay(), Server()->GetStringTypeDay(), &Hour);

		m_DayEnumType = Server()->GetEnumTypeDay();
		if (m_DayEnumType == DayType::NIGHTTYPE)
			m_RaidExp = 100 + rand() % 200;
	
		SendDayInfo(-1);
	}
}

// Рисование вывод всех объектов 
void CGS::OnSnap(int ClientID)
{
	if(CheckPlayerMessageWorldID(ClientID) != GetWorldID())
		return;

	m_World.Snap(ClientID);
	m_pController->Snap(ClientID);
	m_Events.Snap(ClientID);
	for(int i = 0; i < MAX_CLIENTS; i++)
	{
		if(m_apPlayers[i])
			m_apPlayers[i]->Snap(ClientID);
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
			const char *pEnd = 0;
			while(*p)
			{
				const char *pStrOld = p;
				int Code = str_utf8_decode(&p);

				// check if unicode is not empty
				if(Code > 0x20 && Code != 0xA0 && Code != 0x034F && (Code < 0x2000 || Code > 0x200F) && (Code < 0x2028 || Code > 0x202F) &&
					(Code < 0x205F || Code > 0x2064) && (Code < 0x206A || Code > 0x206F) && (Code < 0xFE00 || Code > 0xFE0F) &&
					Code != 0xFEFF && (Code < 0xFFF9 || Code > 0xFFFC))
				{
					pEnd = 0;
				}
				else if(pEnd == 0)
					pEnd = pStrOld;

				if(++Length >= 127)
				{
					*(const_cast<char *>(p)) = 0;
					break;
				}
			}
			if(pEnd != 0)
				*(const_cast<char *>(pEnd)) = 0;

			// drop empty and autocreated spam messages (more than 20 characters per second)
			if(Length == 0 || (g_Config.m_SvSpamprotection && pPlayer->m_PlayerTick[TickState::LastChat] && pPlayer->m_PlayerTick[TickState::LastChat] + Server()->TickSpeed()*(Length/20) > Server()->Tick()))
				return;

			pPlayer->m_PlayerTick[TickState::LastChat] = Server()->Tick();

			// don't allow spectators to disturb players during a running game in tournament mode
			int Mode = pMsg->m_Mode;
			if((g_Config.m_SvTournamentMode == 2) &&
				pPlayer->GetTeam() == TEAM_SPECTATORS &&
				!Server()->IsAuthed(ClientID))
			{
				if(Mode != CHAT_WHISPER)
					Mode = CHAT_TEAM;
				else if(m_apPlayers[pMsg->m_Target] && m_apPlayers[pMsg->m_Target]->GetTeam() != TEAM_SPECTATORS)
					Mode = CHAT_NONE;
			}

			if(Mode != CHAT_NONE)
			{
				// команды сервера
				if(pMsg->m_pMessage[0]=='/')
				{
					CCmd pChat;
					pChat.ChatCmd(pMsg, this, pPlayer);
					return;
				}

				// общение чат для организаций
				if(pPlayer->Acc().IsGuild())
				{
					char aBuf[1024];
					str_format(aBuf, sizeof(aBuf), "[%s:%s] %s", Mmo()->Member()->GuildName(pPlayer->Acc().GuildID),
						Mmo()->Member()->GetGuildRank(pPlayer->Acc().GuildID, pPlayer->Acc().GuildRank), pMsg->m_pMessage);
				
					ChatDiscord(false, DC_SERVER_CHAT, Server()->ClientName(ClientID), aBuf);
					SendChat(ClientID, Mode, pMsg->m_Target, aBuf);
					return;
				}

				// отправить обычное сообщение в чат
				ChatDiscord(false, DC_SERVER_CHAT, Server()->ClientName(ClientID), pMsg->m_pMessage);
				SendChat(ClientID, Mode, pMsg->m_Target, pMsg->m_pMessage);						
			}
		}
		else if (MsgID == NETMSGTYPE_CL_COMMAND)
		{
			CNetMsg_Cl_Command *pMsg = (CNetMsg_Cl_Command*)pRawMsg;
			m_pController->OnPlayerCommand(pPlayer, pMsg->m_Name, pMsg->m_Arguments);
		}
		else if(MsgID == NETMSGTYPE_CL_CALLVOTE)
		{
			CNetMsg_Cl_CallVote *pMsg = (CNetMsg_Cl_CallVote *)pRawMsg;
			if (str_comp_nocase(pMsg->m_Type, "option") != 0 || pMsg->m_Force
				|| Server()->Tick() < (pPlayer->m_PlayerTick[TickState::LastVoteTry] + Server()->TickSpeed()) / 2)
				return;
			
			int InteractiveCount = string_to_number(pMsg->m_Reason, 1, 10000000);
			const auto& item = std::find_if(m_PlayerVotes[ClientID].begin(), m_PlayerVotes[ClientID].end(), [&](const CVoteOptions& vote)
			{
				return (str_comp_nocase(pMsg->m_Value, vote.m_aDescription) == 0);
			});

			if(item != m_PlayerVotes[ClientID].end() 
				&& ParseVote(ClientID, item->m_aCommand, item->m_TempID, item->m_TempID2, InteractiveCount, pMsg->m_Reason))
				pPlayer->m_PlayerTick[TickState::LastVoteTry] = Server()->Tick();
		}
		else if(MsgID == NETMSGTYPE_CL_VOTE)
		{
			CNetMsg_Cl_Vote *pMsg = (CNetMsg_Cl_Vote *)pRawMsg;
			if(pPlayer->ParseInteractive(pMsg->m_Vote))
				return;
		}
		else if(MsgID == NETMSGTYPE_CL_SETTEAM)
		{
			if(!pPlayer->IsAuthed())
				return SBL(pPlayer->GetCID(), PRENORMAL, 100, "Use /register <name> <pass>.");

		}
		else if (MsgID == NETMSGTYPE_CL_SETSPECTATORMODE)
		{

		}
		else if (MsgID == NETMSGTYPE_CL_EMOTICON)
		{
			CNetMsg_Cl_Emoticon *pMsg = (CNetMsg_Cl_Emoticon *)pRawMsg;

			if(g_Config.m_SvSpamprotection && pPlayer->m_PlayerTick[TickState::LastEmote] && pPlayer->m_PlayerTick[TickState::LastEmote]+Server()->TickSpeed()*3 > Server()->Tick())
				return;

			pPlayer->m_PlayerTick[TickState::LastEmote] = Server()->Tick();
			SendEmoticon(ClientID, pMsg->m_Emoticon);
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
			SBL(ClientID, 1000, 100, "Successfully checks client.");
			pPlayer->m_PlayerTick[TickState::CheckClient] = 0;

			// загрузка всех частей скинов игроков
			for(int i = 0; i < MAX_CLIENTS; ++i)
			{
				if(i == ClientID || !m_apPlayers[i])
					continue;

				SendEquipItem(i, ClientID);
			}

			// отправим что прошли проверку на стороне сервера
			CNetMsg_Sv_AfterIsMmoServer GoodCheck;
			Server()->SendPackMsg(&GoodCheck, MSGFLAG_VITAL|MSGFLAG_NORECORD, ClientID);
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
			if(Mmo()->Account()->LoginAccount(ClientID, pMsg->m_Login, pMsg->m_Password) == AUTH_ALL_GOOD)
				Mmo()->Account()->LoadAccount(pPlayer, true);
		}
		else if(Mmo()->OnMessage(MsgID, pRawMsg, ClientID)) { }
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

			// send tuning parameters to client
			SendTuningParams(ClientID);

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
		int savecidmem = ClientID+m_WorldID*MAX_CLIENTS;
		m_apPlayers[ClientID] = new(savecidmem) CPlayer(this, ClientID);
	}

	SendMotd(ClientID);
	SendSettings(ClientID);

	m_BroadcastStates[ClientID] = {};
}

// Вход на сервер игрока
void CGS::OnClientEnter(int ClientID)
{
	CPlayer *pPlayer = m_apPlayers[ClientID];
	m_pController->OnPlayerConnect(pPlayer);

	// update client infos (others before local)
	CNetMsg_Sv_ClientInfo NewClientInfoMsg;
	NewClientInfoMsg.m_ClientID = ClientID;
	NewClientInfoMsg.m_Local = 0;
	NewClientInfoMsg.m_Team = pPlayer->GetTeam();
	NewClientInfoMsg.m_pName = Server()->ClientName(ClientID);
	NewClientInfoMsg.m_pClan = Server()->ClientClan(ClientID);
	NewClientInfoMsg.m_Country = Server()->ClientCountry(ClientID);
	NewClientInfoMsg.m_Silent = pPlayer->IsAuthed();
	for(int p = 0; p < 6; p++)
	{
		NewClientInfoMsg.m_apSkinPartNames[p] = pPlayer->Acc().m_aaSkinPartNames[p];
		NewClientInfoMsg.m_aUseCustomColors[p] = pPlayer->Acc().m_aUseCustomColors[p];
		NewClientInfoMsg.m_aSkinPartColors[p] = pPlayer->Acc().m_aSkinPartColors[p];
	}

	for(int i = 0; i < MAX_CLIENTS; ++i)
	{
		if(i == ClientID || !m_apPlayers[i] || !Server()->ClientIngame(i))
			continue;

		// new info for others
		if(i < MAX_PLAYERS)
			Server()->SendPackMsg(&NewClientInfoMsg, MSGFLAG_VITAL|MSGFLAG_NORECORD, i);

		// existing infos for new player
		bool Bot = m_apPlayers[i]->IsBot();
		int BotID = m_apPlayers[i]->GetBotID();

		CNetMsg_Sv_ClientInfo ClientInfoMsg;
		ClientInfoMsg.m_ClientID = i;
		ClientInfoMsg.m_Local = 0;
		ClientInfoMsg.m_Team = m_apPlayers[i]->GetTeam();

		// имя
		ClientInfoMsg.m_pName = ContextBots::DataBot[BotID].Name(m_apPlayers[i]);
		ClientInfoMsg.m_pClan = Server()->ClientClan(i);
		ClientInfoMsg.m_Country = Server()->ClientCountry(i);
		ClientInfoMsg.m_Silent = false;
		for(int p = 0; p < 6; p++)
		{
			ClientInfoMsg.m_apSkinPartNames[p] = Bot ? ContextBots::DataBot[BotID].SkinNameBot[p] : m_apPlayers[i]->Acc().m_aaSkinPartNames[p];
			ClientInfoMsg.m_aUseCustomColors[p] = Bot ? ContextBots::DataBot[BotID].UseCustomBot[p] : m_apPlayers[i]->Acc().m_aUseCustomColors[p];
			ClientInfoMsg.m_aSkinPartColors[p] = Bot ? ContextBots::DataBot[BotID].SkinColorBot[p] : m_apPlayers[i]->Acc().m_aSkinPartColors[p];
		}
		Server()->SendPackMsg(&ClientInfoMsg, MSGFLAG_VITAL|MSGFLAG_NORECORD, ClientID);
	}
	// local info
	NewClientInfoMsg.m_Local = 1;
	Server()->SendPackMsg(&NewClientInfoMsg, MSGFLAG_VITAL|MSGFLAG_NORECORD, ClientID);

	// another
	ResetVotes(ClientID, MAINMENU);
	if(!pPlayer->IsAuthed())
	{
		// информируем о смене команды
		SendTeam(ClientID, TEAM_SPECTATORS, false, -1);

		// информация
		Chat(ClientID, "Welcome to MmoTee");
		Chat(ClientID, "You need to register '/register <login> <pass> or");
		Chat(ClientID, "Enter '/login <login> <pass> !");
		
		SendDayInfo(ClientID);
		ChatDiscord(false, DC_JOIN_LEAVE, Server()->ClientName(ClientID), "connected and enter in Mmo 0.7");
	}

	// fail check client
	if(!CheckClient(ClientID))
	{
		SBL(ClientID, 1000, 1000, "Vanilla client.\nSpecial client for MmoTee.\n\"{STR}\"", g_Config.m_SvDiscordInviteGroup);
	}
}

// Выход игрока
void CGS::OnClientDrop(int ClientID, const char *pReason, bool ChangeWorld)
{
	if(!m_apPlayers[ClientID])
		return;

	if(ChangeWorld)
	{
		m_apPlayers[ClientID]->KillCharacter();
		return;
	}
	m_pController->OnPlayerDisconnect(m_apPlayers[ClientID]);

	// update clients on drop
	if (Server()->ClientIngame(ClientID) && m_WorldID == Server()->GetWorldID(ClientID))
	{
		ChatDiscord(false, DC_JOIN_LEAVE, Server()->ClientName(ClientID), "leave game Mmo 0.7");

		CNetMsg_Sv_ClientDrop Msg;
		Msg.m_ClientID = ClientID;
		Msg.m_pReason = pReason;
		Msg.m_Silent = false;
		Server()->SendPackMsg(&Msg, MSGFLAG_VITAL | MSGFLAG_NORECORD, -1);
	}

	// очистка данных
	delete m_apPlayers[ClientID];
	m_apPlayers[ClientID] = NULL;

	if (m_apPlayers[ClientID])
		dbg_msg("test", "yes i security");

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
		m_apPlayers[ClientID] = NULL;
	}
	int savecidmem = ClientID+m_WorldID*MAX_CLIENTS;
	m_apPlayers[ClientID] = new(savecidmem) CPlayer(this, ClientID);
	
	// если клиент проверен отправим свой скин
	if(CheckClient(ClientID))
		SendEquipItem(ClientID, ClientID);
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
bool CGS::CheckClient(int ClientID) const{ return (Server()->GetClientVersion(ClientID) == CLIENT_VERSION_MMO); }
const char *CGS::GameType() const { return m_pController && m_pController->GetGameType() ? m_pController->GetGameType() : ""; }
const char *CGS::Version() const { return GAME_VERSION; }
const char *CGS::NetVersion() const { return GAME_NETVERSION; }

// Очистка всех данных при выходие клиента обезательно вызова на раз достаточно
void CGS::ClearClientData(int ClientID)
{
	if(AccountMainSql::Data.find(ClientID) != AccountMainSql::Data.end()) 
		AccountMainSql::Data.erase(ClientID);

	if(Interactive.find(ClientID) != Interactive.end()) 
		Interactive.erase(ClientID);
		
	if(ItemSql::Items.find(ClientID) != ItemSql::Items.end()) 
		ItemSql::Items.erase(ClientID);

	if(SkillJob::Skill.find(ClientID) != SkillJob::Skill.end())
		SkillJob::Skill.erase(ClientID);
	
	if(Effects.find(ClientID) != Effects.end()) 
		Effects.erase(ClientID);

	if(InteractiveSub.find(ClientID) != InteractiveSub.end())
		InteractiveSub.erase(ClientID);
		
	if(QuestBase::Quests.find(ClientID) != QuestBase::Quests.end())
	{ 
		// пробрасываем мобов что требуется удалить
		std::map < int, int > m_talkcheck;
		for(const auto& qp : QuestBase::Quests[ClientID])
		{
			if(qp.second.Type == QUESTFINISHED) 
				continue;
			m_talkcheck[qp.first] = qp.second.Progress;
		}

		// очищаем квесты игрока
		QuestBase::Quests.erase(ClientID); 

		// очищаем ботов что требуется проверить для удаления
		for(const auto& qst : m_talkcheck)
			ClearQuestsBot(qst.first, qst.second);
	}
}

// Обновить мир
void CGS::UpdateWorld()
{
	Mmo()->BotsData()->LoadGlobalBots();
	Mmo()->LoadFullSystems();
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
	CGS *pSelf = (CGS *)pUserData;
	int ClientID = clamp(pResult->GetInteger(0), 0, MAX_PLAYERS-1), ItemID = pResult->GetInteger(1), Count = pResult->GetInteger(2);
	int Enchant = pResult->GetInteger(3), Mail = pResult->GetInteger(4);
	
	CPlayer *pPlayer = pSelf->GetPlayer(ClientID, true);
	if(pPlayer)
	{
		if(Mail == 0) pPlayer->GetItem(ItemID).Add(Count, 0, Enchant);
		else pSelf->SendInbox(ClientID, "The sender heavens", "Sent from console", ItemID, Count, Enchant);
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
		//CGS *pSelf = (CGS *)pUserData;
		//if(pSelf->m_pController)
		//	pSelf->m_pController->CheckGameInfo();
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
		CNetMsg_Sv_VoteMmoOptionAdd OptionMsg;	
		OptionMsg.m_pDescription = Vote.m_aDescription;

		if(str_length(Vote.m_aDescription) < 1) 
			m_apPlayers[To]->m_Colored = { 0, 0, 0 };

		OptionMsg.m_pColored[0] = m_apPlayers[To]->m_Colored.r;
		OptionMsg.m_pColored[1] = m_apPlayers[To]->m_Colored.g;
		OptionMsg.m_pColored[2] = m_apPlayers[To]->m_Colored.b;
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
			Buffer.append("▻ ");
		
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
		bool HidenTabs = (ID >= HSTAT) ? m_apPlayers[To]->GetHidenMenu(ID) : false;
		if(HidenTabs) {	Buffer.append(ID >= NUMHIDEMENU ? ("◈ ") : (ID < HQUESTITEM ? ("△ ") : ("▽ ")));	}
		else {	Buffer.append(ID >= NUMHIDEMENU ? ("◇ ") : (ID < HQUESTITEM ? ("▽ ") : ("△ ")));	}

		Server()->Localization()->Format_VL(Buffer, m_apPlayers[To]->GetLanguage(), pText, VarArgs);
		if(ID > HQUESTITEM && ID < NUMHIDEMENU) { Buffer.append(" (Press me for help)"); }

		m_apPlayers[To]->m_Colored = { Color.r/2, Color.g/2, Color.b/2 };
		AV(To, "HIDEN", Buffer.buffer(), ID);
		m_apPlayers[To]->m_Colored = { Color.r/5, Color.g/5, Color.b/5 };
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
		bool HidenTabs = (ID >= HSTAT) ? m_apPlayers[To]->GetHidenMenu(ID) : false;
		if(HidenTabs) {	Buffer.append(ID >= NUMHIDEMENU ? ("◈ ") : (ID < HQUESTITEM ? ("△ ") : ("▽ ")));	}
		else {	Buffer.append(ID >= NUMHIDEMENU ? ("◇ ") : (ID < HQUESTITEM ? ("▽ ") : ("△ ")));	}

		Server()->Localization()->Format_VL(Buffer, m_apPlayers[To]->GetLanguage(), pText, VarArgs);
		if(ID > HQUESTITEM && ID < NUMHIDEMENU) { Buffer.append(" (Press me for help)"); }

		m_apPlayers[To]->m_Colored = { Color.r/2, Color.g/2, Color.b/2 };
		AV(To, "HIDEN", Buffer.buffer(), ID, -1, Icon);
		m_apPlayers[To]->m_Colored = { Color.r/5, Color.g/5, Color.b/5 };
		Buffer.clear();
		va_end(VarArgs);
	}
}

// Добавить форматированое голосование как Меню
void CGS::AVM(int To, const char* Type, const int ID, const int HideID, const char* pText, ...)
{
	if(To >= 0 && To < MAX_PLAYERS && m_apPlayers[To])
	{
		if((!m_apPlayers[To]->GetHidenMenu(HideID) && HideID >= HQUESTITEM) || 
			(m_apPlayers[To]->GetHidenMenu(HideID) && HideID < HQUESTITEM))
			return;

		va_list VarArgs;
		va_start(VarArgs, pText);

		dynamic_string Buffer;
		if(ID != NOPE) { Buffer.append("▻ "); }

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
		if((!m_apPlayers[To]->GetHidenMenu(HideID) && HideID >= HQUESTITEM) || 
			(m_apPlayers[To]->GetHidenMenu(HideID) && HideID < HQUESTITEM))
			return;

		va_list VarArgs;
		va_start(VarArgs, pText);

		dynamic_string Buffer;
		if(ID != NOPE) { Buffer.append("▻ "); }

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
		if((!m_apPlayers[To]->GetHidenMenu(HideID) && HideID >= HQUESTITEM) || 
			(m_apPlayers[To]->GetHidenMenu(HideID) && HideID < HQUESTITEM))
			return;
			
		va_list VarArgs;
		va_start(VarArgs, pText);

		dynamic_string Buffer;
		if(ID != NOPE) { Buffer.append("▸ "); }

		Server()->Localization()->Format_VL(Buffer, m_apPlayers[To]->GetLanguage(), pText, VarArgs);
		AV(To, Type, Buffer.buffer(), ID, ID2);
		Buffer.clear();
		va_end(VarArgs);
	}
}

// Все основные меню для работы в сервере
void CGS::ResetVotes(int ClientID, int MenuList)
{	
	if(!GetPlayer(ClientID, true))
		return;

	// таймер для синхронизации с SQL
	kurosio::kpause(3);
	
	CPlayer *pPlayer = m_apPlayers[ClientID];
	pPlayer->m_OpenVoteMenu = MenuList;
	ClearVotes(ClientID);
	if(MenuList == MAINMENU)
	{
		pPlayer->m_LastVoteMenu = MAINMENU;

		// меню статистики
		int NeedExp = pPlayer->ExpNeed(pPlayer->Acc().Level);
		AVH(ClientID, HSTAT, vec3(35,80,40), "Hi, {STR} Last log in {STR}", Server()->ClientName(ClientID), pPlayer->Acc().LastLogin);
		AVM(ClientID, "null", NOPE, HSTAT, "Discord: \"{STR}\". Ideas, bugs, rewards", g_Config.m_SvDiscordInviteGroup);
		AVM(ClientID, "null", NOPE, HSTAT, "Level {INT} : Exp {INT}/{INT}", &pPlayer->Acc().Level, &pPlayer->Acc().Exp, &NeedExp);
		AVM(ClientID, "null", NOPE, HSTAT, "Money {INT} gold", &pPlayer->GetItem(itMoney).Count);
		AVM(ClientID, "null", NOPE, HSTAT, "Skill Point {INT}SP", &pPlayer->GetItem(itSkillPoint).Count);
		AV(ClientID, "null", "");

		// меню персонал
		AVH(ClientID, HPERSONAL, vec3(50,52,57), "☪ SUB MENU PERSONAL");
		AVM(ClientID, "MENU", INVENTORY, HPERSONAL, "⁂ Inventory"); 
		AVM(ClientID, "MENU", EQUIPMENU, HPERSONAL, "★ Equipment");
		AVM(ClientID, "MENU", INBOXLIST, HPERSONAL, "✉ Mailbox");
		AVM(ClientID, "MENU", UPGRADES, HPERSONAL, "◒ Upgrades");
		AVM(ClientID, "MENU", SETTINGS, HPERSONAL, "☑ Settings");

		CCharacter *pChar = pPlayer->GetCharacter();
		if(pChar && pChar->IsAlive() && pChar->GetHelper()->BoolIndex(TILE_CRAFT))
			AVM(ClientID, "MENU", CRAFTING, HPERSONAL, "☭ Crafting");
		
		AVM(ClientID, "MENU", ADVENTUREJOURNAL, HPERSONAL, "ღ Adventure Journal");
		AVM(ClientID, "MENU", DUNGEONSMENU, HPERSONAL, "◎ Dungeons");
		AVM(ClientID, "MENU", MEMBERMENU, HPERSONAL, "☃ Your Guild");
		AVM(ClientID, "MENU", HOUSEMENU, HPERSONAL, "❖ Your House");
		AV(ClientID, "null", "");

		// меню информации
		AVH(ClientID, HINFORMATION, vec3(15,40,80), "# SUB MENU INFORMATION");
		AVM(ClientID, "MENU", TOPLISTMENU, HINFORMATION, "♛ Top list");
		AVM(ClientID, "MENU", GUIDEDROP, HINFORMATION, "♣ Loot mobs");
		AV(ClientID, "null", "");

		// чекаем местонахождение
		if(!pChar || !pChar->IsAlive())
			return;

		if (pChar->GetHelper()->BoolIndex(TILE_AUCTION))
			Mmo()->Auction()->ShowAuction(pPlayer);
		else if (pChar->GetHelper()->BoolIndex(TILE_HOUSE))
		{
			const int HouseID = Mmo()->House()->GetHouse(pChar->m_Core.m_Pos);
			if (HouseID > 0) Mmo()->House()->ShowHouseMenu(pPlayer, HouseID);
		}
		else if (pChar->GetHelper()->BoolIndex(TILE_MEMBERHOUSE))
		{
			const int HouseID = Mmo()->Member()->GetPosHouseID(pChar->m_Core.m_Pos);
			Mmo()->Member()->ShowBuyHouse(pPlayer, HouseID);
		}
		else if (pChar->GetHelper()->BoolIndex(TILE_SELLHOUSE))
			Mmo()->ShowBussinesHousesSell(pPlayer);
		else if (pChar->GetHelper()->BoolIndex(TILE_STORAGE))
		{
			const int StorageID = Mmo()->Storage()->GetLoadStorage(pChar->m_Core.m_Pos);
			Mmo()->Storage()->ShowStorageMenu(ClientID, StorageID);
			Mmo()->Auction()->ShowMailShop(pPlayer, StorageID);
		}
		else if (pChar->GetHelper()->BoolIndex(TILE_LEARNSKILL))
			Mmo()->Skills()->ShowMailSkillList(pPlayer);
	}  
	else if(MenuList == INVENTORY) 
	{
		pPlayer->m_LastVoteMenu = MAINMENU;
		AVH(ClientID, HINVINFO, vec3(35,80,40), "Inventory Information");
		AVM(ClientID, "null", NOPE, HINVINFO, "Choose the type of items you want to show");
		AVM(ClientID, "null", NOPE, HINVINFO, "After, need select item to interact");
		AV(ClientID, "null", "");

		ShowPlayerStats(pPlayer);

		AVH(ClientID, HINVSELECT, vec3(40, 10, 5), "Inventory Select List");
		AVM(ClientID, "SORTEDINVENTORY", ITEMUSED, HINVSELECT, "Used Items");
		AVM(ClientID, "SORTEDINVENTORY", ITEMCRAFT, HINVSELECT, "Craft Items");
		AVM(ClientID, "SORTEDINVENTORY", ITEMQUEST, HINVSELECT, "Quest Items");
		AVM(ClientID, "SORTEDINVENTORY", ITEMUPGRADE, HINVSELECT, "Modules Items");
		AVM(ClientID, "SORTEDINVENTORY", ITEMEQUIP, HINVSELECT, "Equiping Items");
		AVM(ClientID, "SORTEDINVENTORY", ITEMPOTION, HINVSELECT, "Potion Items");
		AVM(ClientID, "SORTEDINVENTORY", ITEMOTHER, HINVSELECT, "Other Items");
		if(pPlayer->m_SortTabs[SORTINVENTORY])
			Mmo()->Item()->ListInventory(pPlayer, pPlayer->m_SortTabs[SORTINVENTORY]);

		AddBack(ClientID);
	}
	else if(MenuList == ADVENTUREJOURNAL) 
	{
		pPlayer->m_LastVoteMenu = MAINMENU;

		// чекаем местонахождение
		AV(ClientID, "null", "");
		Mmo()->Quest()->ShowFullQuestLift(pPlayer);

		AddBack(ClientID);
	}
	else if(MenuList == INBOXLIST) 
	{
		pPlayer->m_LastVoteMenu = MAINMENU;

		AddBack(ClientID);
		AV(ClientID, "null", "");
		Mmo()->Inbox()->GetInformationInbox(pPlayer);
	}
	else if(MenuList == MEMBERMENU) 
	{ 
		pPlayer->m_LastVoteMenu = MAINMENU;
		Mmo()->Member()->ShowMenuGuild(pPlayer);
	}
	else if(MenuList == GuildRank) 
	{
		pPlayer->m_LastVoteMenu = MEMBERMENU;
		Mmo()->Member()->ShowMenuRank(pPlayer);
	}
	else if(MenuList == MEMBERINVITES) 
	{
		pPlayer->m_LastVoteMenu = MEMBERMENU;
		Mmo()->Member()->ShowInvitesGuilds(ClientID, pPlayer->Acc().GuildID);
	}
	else if(MenuList == MEMBERHISTORY) 
	{
		pPlayer->m_LastVoteMenu = MEMBERMENU;		
		Mmo()->Member()->ShowHistoryGuild(ClientID, pPlayer->Acc().GuildID);
	}	
	else if(MenuList == SETTINGS) 
	{
		pPlayer->m_LastVoteMenu = MAINMENU;
		
		// обычные настройки
		bool FoundSettings = false;
		AVH(ClientID, HSETTINGSS, vec3(50,30,40), "Some of the settings becomes valid after death");
		for(const auto& it : ItemSql::Items[ClientID])
		{
			if(it.second.Count <= 0 || it.second.Info().Type != ITEMSETTINGS)
				continue;
			
			AVM(ClientID, "ISETTINGS", it.first, HSETTINGSS, "[{STR}] {STR}", 
				it.second.Settings ? "Enable" : "Disable", it.second.Info().GetName(pPlayer));
			FoundSettings = true;
		}
		if(!FoundSettings) { AVM(ClientID, "null", NOPE, HSETTINGSS, "The list of equipment sub upgrades is empty"); }

		// поиск всех предметов этого типа
		FoundSettings = false;
		AV(ClientID, "null", "");
		AVH(ClientID, HSETTINGSU, vec3(30,50,40), "Sub items settings.");
		for(const auto& it : ItemSql::Items[ClientID])
		{
			if(it.second.Count <= 0 || it.second.Info().Type != ITEMUPGRADE) 
				continue;
			
			int BonusCount = it.second.Info().BonusCount*(it.second.Enchant+1);
			AVMI(ClientID, it.second.Info().GetIcon(), "ISETTINGS", it.first, HSETTINGSU, "[{STR}] {STR}({STR} +{INT})"), 
				(it.second.Settings ? "Dress" : "Not dressed", it.second.Info().GetName(pPlayer),
				pPlayer->AtributeName(it.second.Info().BonusID), &BonusCount);
			FoundSettings = true;
		}
		if(!FoundSettings) { AVM(ClientID, "null", NOPE, HSETTINGSU, "The list of equipment sub upgrades is empty"); }
		AddBack(ClientID);
	}
	else if(MenuList == CRAFTING) 
	{
		pPlayer->m_LastVoteMenu = MAINMENU;
		CCharacter *pChar = pPlayer->GetCharacter();

		if(pChar && pChar->GetHelper()->BoolIndex(TILE_CRAFT))
		{
			AVH(ClientID, HCRAFTINFO, vec3(35,80,40), "Crafting Information");
			AVM(ClientID, "null", NOPE, HCRAFTINFO, "Choose the type of crafts you want to show");
			AVM(ClientID, "null", NOPE, HCRAFTINFO, "If you will not have enough items for crafting");
			AVM(ClientID, "null", NOPE, HCRAFTINFO, "You will write those and the amount that is still required");
			AV(ClientID, "null", "");
			
			AVH(ClientID, HCRAFTSELECT, vec3(40, 10, 5), "Crafting Select List");
			AVM(ClientID, "SORTEDCRAFT", CRAFTBASIC, HCRAFTSELECT, "Basic Items");
			AVM(ClientID, "SORTEDCRAFT", CRAFTARTIFACT, HCRAFTSELECT, "Artifacts");
			AVM(ClientID, "SORTEDCRAFT", CRAFTWEAPON, HCRAFTSELECT, "Modules & Weapons");
			AVM(ClientID, "SORTEDCRAFT", CRAFTEAT, HCRAFTSELECT, "Buffs & Eat");
			AVM(ClientID, "SORTEDCRAFT", CRAFTWORK, HCRAFTSELECT, "Work & Job");
			AVM(ClientID, "SORTEDCRAFT", CRAFTQUEST, HCRAFTSELECT, "Quests");
			AV(ClientID, "null", "");
			if(pPlayer->m_SortTabs[SORTCRAFT])
				Mmo()->Craft()->ShowCraftList(pPlayer, pPlayer->m_SortTabs[SORTCRAFT]);
		}
		else
			AVL(ClientID, "null", "You need find craft room.");

		AddBack(ClientID);
	}
	else if(MenuList == HOUSEMENU) 
	{
		pPlayer->m_LastVoteMenu = MAINMENU;
		
		Mmo()->House()->ShowPersonalHouse(pPlayer);
		AddBack(ClientID);
	}
	else if(MenuList == HOUSEDECORATION) 
	{
		pPlayer->m_LastVoteMenu = HOUSEMENU;

		AVH(ClientID, HDECORATION, vec3(35,80,40), "Decorations Information");
		AVM(ClientID, "null", NOPE, HDECORATION, "Add: Select your item in list. Select (Add to house),");
		AVM(ClientID, "null", NOPE, HDECORATION, "later press (ESC) and mouse select position");
		AVM(ClientID, "null", NOPE, HDECORATION, "Return in inventory: Select down your decorations");
		AVM(ClientID, "null", NOPE, HDECORATION, "and press (Back to inventory).");

		Mmo()->Item()->ListInventory(pPlayer, ITEMDECORATION);
		AV(ClientID, "null", "");

		Mmo()->House()->ShowDecorationList(pPlayer);

		AddBack(ClientID);	
	}
	else if(MenuList == HOUSEPLANTS) 
	{
		const int HouseID = Mmo()->House()->OwnerHouseID(pPlayer->Acc().AuthID);
		const int PlantItemID = Mmo()->House()->GetPlantsID(HouseID);
		pPlayer->m_LastVoteMenu = HOUSEMENU;

		AVH(ClientID, HPLANTS, vec3(35,80,40), "Plants Information");
		AVM(ClientID, "null", NOPE, HPLANTS, "Select item and in tab select 'Change Plants'");
		AV(ClientID, "null", "");

		AVM(ClientID, "null", NOPE, NOPE, "Housing Active Plants: {STR}", GetItemInfo(PlantItemID).GetName(pPlayer));

		Mmo()->Item()->ListInventory(pPlayer, ITPLANTS, true);
		AddBack(ClientID);	
	}
	else if(MenuList == UPGRADES) 
	{
		pPlayer->m_LastVoteMenu = MAINMENU;

		AVH(ClientID, HUPGRINFO, vec3(35,80,40), "Upgrades Information");
		AVM(ClientID, "null", NOPE, HUPGRINFO, "Select upgrades type in Reason, write count.");
		AV(ClientID, "null", "");

		ShowPlayerStats(pPlayer);

		// Улучшения класса DPS дамаг
		int Range = pPlayer->GetLevelDisciple(AtributType::AtDps);
		AVH(ClientID, HUPGDPS, vec3(80,30,30), "Disciple of War. Level Range {INT}", &Range);
		for(const auto& at : AttributInfo)
		{
			if(at.second.AtType != AtributType::AtDps || str_comp_nocase(at.second.FieldName, "unfield") == 0 || at.second.UpgradePrice <= 0) 
				continue;
	
			AVD(ClientID, "UPGRADE", at.first, at.second.UpgradePrice, HUPGDPS, "[Price {INT}] {INT}P {STR}", &at.second.UpgradePrice, &pPlayer->Acc().Stats[at.first], pPlayer->AtributeName(at.first));
		}
		AV(ClientID, "null", "");

		// Улучшения класса TANK танк
		Range = pPlayer->GetLevelDisciple(AtributType::AtTank);
		AVH(ClientID, HUPGTANK, vec3(30,30,80), "Disciple of Tank. Level Range {INT}", "lvl", &Range);
		for(const auto& at : AttributInfo)
		{
			if(at.second.AtType != AtributType::AtTank || str_comp_nocase(at.second.FieldName, "unfield") == 0 || at.second.UpgradePrice <= 0) 
				continue;
	
			AVD(ClientID, "UPGRADE", at.first, at.second.UpgradePrice, HUPGTANK, "[Price {INT}] {INT}P {STR}", &at.second.UpgradePrice, &pPlayer->Acc().Stats[at.first], pPlayer->AtributeName(at.first));
		}
		AV(ClientID, "null", "");

		// Улучшения класса HEALER хил
		Range = pPlayer->GetLevelDisciple(AtributType::AtHealer);
		AVH(ClientID, HUPGHEALER, vec3(30,80,30), "Disciple of Healer. Level Range {INT}", "lvl", &Range);
		for(const auto& at : AttributInfo)
		{
			if(at.second.AtType != AtributType::AtHealer || str_comp_nocase(at.second.FieldName, "unfield") == 0 || at.second.UpgradePrice <= 0) 
				continue;
	
			AVD(ClientID, "UPGRADE", at.first, at.second.UpgradePrice, HUPGHEALER, "[Price {INT}] {INT}P {STR}", &at.second.UpgradePrice, &pPlayer->Acc().Stats[at.first], pPlayer->AtributeName(at.first));
		}
		AV(ClientID, "null", "");

		// Улучшения WEAPONS оружия
		AVH(ClientID, HUPGWEAPON, vec3(30,30,30), "Upgrades Weapons / Ammo");
		for(const auto& at : AttributInfo)
		{
			if(at.second.AtType != AtributType::AtWeapon || str_comp_nocase(at.second.FieldName, "unfield") == 0 || at.second.UpgradePrice <= 0) 
				continue;
	
			AVD(ClientID, "UPGRADE", at.first, at.second.UpgradePrice, HUPGWEAPON, "[Price {INT}] {INT}P {STR}", &at.second.UpgradePrice, &pPlayer->Acc().Stats[at.first], pPlayer->AtributeName(at.first));
		}

		AV(ClientID, "null", ""), 
		AVH(ClientID, HJOBUPGRADE, vec3(80,56,10), "Disciple of Jobs");
		Mmo()->SpaAcc()->ShowMenu(ClientID);
		AVM(ClientID, "null", NOPE, HJOBUPGRADE, "═══════════════════════════════");
		Mmo()->PlantsAcc()->ShowMenu(ClientID);
		AVM(ClientID, "null", NOPE, HJOBUPGRADE, "═══════════════════════════════");
		Mmo()->MinerAcc()->ShowMenu(ClientID);
		AddBack(ClientID);
	}
	else if (MenuList == TOPLISTMENU)
	{
		pPlayer->m_LastVoteMenu = MAINMENU;

		AVH(ClientID, HTOPMENUINFO, vec3(35, 80, 40), "Top list Information");
		AVM(ClientID, "null", NOPE, HTOPMENUINFO, "Here you can see top server Guilds, Players.");
		AV(ClientID, "null", "");

		m_apPlayers[ClientID]->m_Colored = { 20,7,15 };
		AVM(ClientID, "SELECTEDTOP", ToplistTypes::GUILDS_LEVELING, NOPE, "Top 10 guilds leveling");
		AVM(ClientID, "SELECTEDTOP", ToplistTypes::GUILDS_WEALTHY, NOPE, "Top 10 guilds wealthy");
		AVM(ClientID, "SELECTEDTOP", ToplistTypes::PLAYERS_LEVELING, NOPE, "Top 10 players leveling");
		AddBack(ClientID);
	}
	else if(MenuList == GUIDEDROP) 
	{
		pPlayer->m_LastVoteMenu = MAINMENU;

		// информация
		AVH(ClientID, HCHANCELOOTINFO, vec3(35,80,40), "Chance & Loot Information");
		AVM(ClientID, "null", NOPE, HCHANCELOOTINFO, "Here you can see chance loot, mobs, positions, world.");
		AV(ClientID, "null", "");

		// сортируем всех ботов и получаем их данные по дропу
		char aBuf[128];
		for(const auto& mobs : ContextBots::MobBot)
		{
			const int HideID = (NUMHIDEMENU+12500+mobs.first);
			int PosX = mobs.second.PositionX/32, PosY = mobs.second.PositionY/32;

			AVH(ClientID, HideID, vec3(20, 7, 15), "{STR} [{STR}] {STR}(x: {INT} y: {INT})", mobs.second.Boss ? "Raid" : "Mob", mobs.second.Name,
				Server()->GetWorldName(mobs.second.WorldID), &PosX, &PosY);
	
			for(int i = 0; i < 6; i++)
			{
				if(mobs.second.DropItem[i] <= 0 || mobs.second.CountItem[i] <= 0) 
					continue;
			
				double Chance = mobs.second.RandomItem[i] <= 0 ? 100.0f : (1.0f / (double)mobs.second.RandomItem[i]) * 100;
				ItemSql::ItemInformation &InfoDropItem = GetItemInfo(mobs.second.DropItem[i]);
	
				str_format(aBuf, sizeof(aBuf), "%sx%d - chance to loot %0.2f%%", InfoDropItem.GetName(pPlayer), mobs.second.CountItem[i], Chance);
				AVMI(ClientID, InfoDropItem.GetIcon(), "null", NOPE, HideID, "{STR}", aBuf);		
			}
		}
		AddBack(ClientID);
	}
	else if(MenuList == EQUIPMENU) 
	{
		pPlayer->m_LastVoteMenu = MAINMENU;
		AVH(ClientID, HEQUIPINFO, vec3(35,80,40), "Equip / Armor Information");
		AVM(ClientID, "null", NOPE, HEQUIPINFO, "Select tab and select armor.");
		AV(ClientID, "null", "");
		ShowPlayerStats(pPlayer);

		AVH(ClientID, HEQUIPSELECT, vec3(40, 10, 5), "Equip Select List");
		const char* pType[NUM_EQUIPS] = { "Wings", "Stabilized", "Hammer", "Gun", "Shotgun", "Grenade", "Rifle", "Discord", "Pickaxe" };
		for(int i = EQUIP_WINGS; i < NUM_EQUIPS; i++) 
		{
			const int ItemID = pPlayer->GetItemEquip(i);
			if(ItemID <= 0 || !pPlayer->GetItem(ItemID).Settings) 
			{
				AVM(ClientID, "SORTEDEQUIP", i, HEQUIPSELECT, "{STR} Not equipped", pType[i]);
				continue;
			}

			const int BonusItem = GetItemInfo(ItemID).BonusID;
			int BonusCount = GetItemInfo(ItemID).BonusCount*(pPlayer->GetItem(ItemID).Enchant+1);
			AVM(ClientID, "SORTEDEQUIP", i, HEQUIPSELECT, "{STR} {STR} | {STR} +{INT}", pType[i], GetItemInfo(ItemID).GetName(pPlayer), pPlayer->AtributeName(BonusItem), &BonusCount);
		}

		// все Equip слоты предемтов
		for(int i = EQUIP_WINGS; i < NUM_EQUIPS; i++) 
		{
			if(pPlayer->m_SortTabs[SORTEQUIP] != i) 
				continue;

			bool FindItem = false;
			AV(ClientID, "null", "");
			for(const auto& it : ItemSql::Items[ClientID]) 
			{
				if(!it.second.Count || it.second.Info().Function != i) 
					continue;
		
				Mmo()->Item()->ItemSelected(pPlayer, it.second, true);			
				FindItem = true;
			}

			if(!FindItem) 
				AVL(ClientID, "null", "There are no items in this tab");
		}
		AddBack(ClientID);
	}
	else if(MenuList == FINISHQUESTMENU) 
	{
		pPlayer->m_LastVoteMenu = ADVENTUREJOURNAL;
		Mmo()->Quest()->ShowQuestList(pPlayer, QUESTFINISHED);
		AddBack(ClientID);
	}
	
	Mmo()->OnPlayerHandleMainMenu(ClientID, MenuList);
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
	m_apPlayers[ClientID]->m_Colored = {40,0,0};
	AVL(ClientID, "BACK", "Backpage");
	m_apPlayers[ClientID]->m_Colored = {0,0,0};
}

// Вывести статистику игрока
void CGS::ShowPlayerStats(CPlayer *pPlayer)
{
	const int ClientID = pPlayer->GetCID();
	AVH(ClientID, HUPGRADESTATS, vec3(22,49,80), "Player Stats");
	for(const auto& at : AttributInfo)
	{
		if(str_comp_nocase(at.second.FieldName, "unfield") == 0) 
			continue;
	
		// если апгрейды стоят дешево то они имеют деление ральных статистик
		if(at.second.UpgradePrice < 10)
		{
			int SumingAt = pPlayer->GetAttributeCount(at.first), RealSum = pPlayer->GetAttributeCount(at.first, true);
			AVM(ClientID, "null", NOPE, HUPGRADESTATS, "{INT} [+{INT}] - {STR}", &SumingAt, &RealSum, pPlayer->AtributeName(at.first));
			continue;
		}

		// если апгрейды дорогие они имеют 1 статистики
		int RealSum = pPlayer->GetAttributeCount(at.first);
		AVM(ClientID, "null", NOPE, HUPGRADESTATS, "[+{INT}] - {STR}", &RealSum, pPlayer->AtributeName(at.first));
	}

	AVM(ClientID, "null", NOPE, NOPE, "!!! Player Upgrade Point: [{INT}P] !!!", &pPlayer->Acc().Upgrade);
	AV(ClientID, "null", "");
}

// Парсинг голосований всех функций методов действий
bool CGS::ParseVote(int ClientID, const char *CMD, const int VoteID, const int VoteID2, int Get, const char *Text)
{
	// проверка на игрока
	CPlayer *pPlayer = m_apPlayers[ClientID];
	if(!pPlayer || !pPlayer->GetCharacter())
	{
		Chat(ClientID, "Use it when you're not dead!");
		return true;
	} 

	// базовый парсинг
	if(PPSTR(CMD, "null") == 0) 
		return true;

	if(PPSTR(CMD, "BACK") == 0)
	{
		ResetVotes(ClientID, pPlayer->m_LastVoteMenu);
		return true;
	}

	if(PPSTR(CMD, "MENU") == 0)
	{
		ResetVotes(ClientID, VoteID);
		return true;
	}

	if (PPSTR(CMD, "SELECTEDTOP") == 0)
	{
		ResetVotes(ClientID, TOPLISTMENU);
	
		AV(ClientID, "null", "\0");
		Mmo()->ShowTopList(pPlayer, VoteID);
		return true;
	}

	if(pPlayer->ParseVoteUpgrades(CMD, VoteID, VoteID2, Get)) 
		return true;

	// парсинг всего остального
	sqlstr::CSqlString<64> FormatText = sqlstr::CSqlString<64>(Text);
	if(Mmo()->OnParseFullVote(pPlayer, CMD, VoteID, VoteID2, Get, FormatText.cstr())) return true;
	return false;
}

/* #########################################################################
	MMO GAMECONTEXT 
######################################################################### */
// Создать бота
void CGS::CreateBot(short SpawnPoint, int BotID, int SubID)
{
	int BotClientID = MAX_PLAYERS;
	while(BotClientID < MAX_CLIENTS && m_apPlayers[BotClientID])
		BotClientID++;

	if (BotClientID >= MAX_CLIENTS)
		return;

	int savecidmem = BotClientID+m_WorldID*MAX_CLIENTS;
	m_apPlayers[BotClientID] = new(savecidmem) CPlayerBot(this, BotClientID, BotID, SubID, SpawnPoint);
	Server()->InitClientBot(BotClientID);
}

// Удалить ботов что не активны у людей для квестов
void CGS::ClearQuestsBot(int QuestID, int Step)
{
	// собираем все данные о ботах
	ContextBots::QuestBotInfo FindBot = Mmo()->Quest()->GetQuestBot(QuestID, Step);
	if(!FindBot.IsActive())
		return;

	// перекидываем recheck на мир моба
	if(FindBot.WorldID != GetWorldID())
	{
		Server()->QuestBotRecheck(FindBot.WorldID, QuestID, Step);
		return;
	}

	// ищем есть ли такой бот
	int QuestBotClientID = -1;
	for(int i = MAX_PLAYERS ; i < MAX_CLIENTS; i++)
	{
		if(!m_apPlayers[i] || m_apPlayers[i]->GetSpawnBot() != SPAWNQUESTNPC || m_apPlayers[i]->GetBotSub() != FindBot.SubBotID) 
			continue;
		
		QuestBotClientID = i;
	}

	// ищем есть ли активный бот у всех игроков
	bool ActiveBot = Mmo()->Quest()->IsActiveQuestBot(QuestID, Step);

	// если активен у кого то бот но его нету создаем
	dbg_msg("test", "%d %d", ActiveBot, QuestBotClientID);
	if(ActiveBot && QuestBotClientID <= -1)
	{
		dbg_msg("test", "я создаю бота для квеста %d", QuestID);
		CreateBot(SPAWNQUESTNPC, FindBot.BotID, FindBot.SubBotID);
	}
	
	// если бот не активен не у одного игрока, но игрок найден удаляем
	if(!ActiveBot && QuestBotClientID >= MAX_PLAYERS)
	{
		dbg_msg("test", "я удаляю бота для квеста %d", QuestID);
		delete m_apPlayers[QuestBotClientID];
		m_apPlayers[QuestBotClientID] = NULL;
	}
}

// Создать Лол текст в мире
void CGS::CreateText(CEntity *pParent, bool Follow, vec2 Pos, vec2 Vel, int Lifespan, const char *pText, int WorldID)
{
	if(!GetPlayerCliped(Pos, 1200)) 
		return;
	
	CLoltext Text;
	Text.Create(this, &m_World, pParent, Pos, Vel, Lifespan, pText, true, Follow, WorldID);
}

// Саздает бонус в позиции Типа и Количества и Их самих кол-ва
void CGS::CreateDropBonuses(vec2 Pos, int Type, int Count, int BonusCount)
{
	for(int i = 0; i < BonusCount; i++) {
		vec2 Dir = normalize(vec2(1200-rand()%2400, -1500-rand()%2000));
		vec2 Projdrop = (Dir * max(0.001f, 2.6f));
		new CDropingBonuses(&m_World, Pos, Projdrop/15, Type, Count);	
	}
}

// Саздает предметы в позиции Типа и Количества и Их самих кол-ва
void CGS::CreateDropItem(vec2 Pos, int ClientID, int ItemID, int Count, int Enchant)
{
	ItemSql::ItemPlayer DropItem;
	DropItem.SetBasic(NULL, ItemID);
	DropItem.Count = Count;
	DropItem.Enchant = Enchant;
	
	vec2 Dir = normalize(vec2(0+rand()%20-rand()%20, -15-rand()%30));
	vec2 Projdrop = (Dir * max(0.001f, 0.2f));
	new CDropingItem(&m_World, Pos, Projdrop, DropItem, ClientID);
}

// Саздает предметы в позиции Типа и Количества и Их самих кол-ва
void CGS::CreateDropItem(vec2 Pos, int ClientID, ItemSql::ItemPlayer &PlayerItem, int Count)
{
	ItemSql::ItemPlayer CopyItem;
	CopyItem.Copy(PlayerItem);
	CopyItem.Count = Count;

	if(PlayerItem.Remove(Count))
	{
		vec2 Dir = normalize(vec2(0+rand()%20-rand()%20, -15-rand()%30));
		vec2 Projdrop = (Dir * max(0.001f, 0.2f));
		new CDropingItem(&m_World, Pos, Projdrop, CopyItem, ClientID);
	}
}

// Проверить чекнуть и подобрать предмет Если он будет найден
bool CGS::TakeItemCharacter(int ClientID)
{
	// check for use
	CPlayer *pPlayer = GetPlayer(ClientID, true);
	if(!pPlayer || !pPlayer->GetCharacter() /*|| pChar->m_ReloadTimer*/)
		return false;

	CDropingItem *pDrop = (CDropingItem*)m_World.ClosestEntity(pPlayer->GetCharacter()->m_Core.m_Pos, 64, CGameWorld::ENTTYPE_DROPITEM, 0);
	if(pDrop) { return pDrop->TakeItem(ClientID);}
	return false;
}

// Отправить сообщение с предметом или без используя ClientID
void CGS::SendInbox(int ClientID, const char* Name, const char* Desc, int ItemID, int Count, int Enchant)
{
	CPlayer *pPlayer = GetPlayer(ClientID, true);
	if(!pPlayer)
		return;

	Mmo()->Inbox()->SendInbox(pPlayer->Acc().AuthID, Name, Desc, ItemID, Count, Enchant);
} 

// отправить информацию о дне
void CGS::SendDayInfo(int ClientID)
{
	if(m_DayEnumType == DayType::NIGHTTYPE)
	{
		if(ClientID == -1) { m_RaidExp = 100+rand()%200; } // для всех значит глобально
		Chat(ClientID, "Night, Mobs more aggressive!");
		Chat(ClientID, "Raid, increase to experience [{INT}%]", &m_RaidExp);
	}
	else if(m_DayEnumType == DayType::MORNINGTYPE)
	{
		if(ClientID == -1) { m_RaidExp = 100; } // для всех значит глобально
		Chat(ClientID, "Morning, Mobs not particularly aggressive!");
		Chat(ClientID, "Raid, experience was downgraded to [100%]!");
	}
}

// Сменить Снаряжение автоматически найти тип по Предмету
void CGS::ChangeEquipSkin(int ClientID, int ItemID)
{
	CPlayer *pPlayer = GetPlayer(ClientID, true);
	if(!pPlayer || !pPlayer->IsAuthed())
		return;
	
	if (GetItemInfo(ItemID).Type == ITEMEQUIP &&
		(GetItemInfo(ItemID).Function == EQUIP_WINGS || GetItemInfo(ItemID).Function == EQUIP_HAMMER))
	{
		CNetMsg_Sv_EquipItems Msg;
		Msg.m_ClientID = ClientID;
		for (int p = 0; p < NUM_EQUIPS; p++)
		{
			int EquipItem = pPlayer->GetItemEquip(p);
			Msg.m_EquipID[p] = EquipItem;
			Msg.m_EnchantItem[p] = pPlayer->GetItem(EquipItem).Enchant;
		}
		Server()->SendPackMsg(&Msg, MSGFLAG_VITAL | MSGFLAG_NORECORD, -1);
	}
}

// Очистить интерактивы сюб
void CGS::ClearInteractiveSub(int ClientID)
{
	if(InteractiveSub.find(ClientID) != InteractiveSub.end()) 
		InteractiveSub.erase(ClientID);
}

// Повышем кол-во для определеннго значения как рейд
int CGS::IncreaseCountRaid(int IncreaseCount) const
{
	return (int)kurosio::translate_to_procent_rest(IncreaseCount, m_RaidExp);
}

// Узнать есть ли рядом игроки
bool CGS::GetPlayerCliped(vec2 Pos, float Distance) const
{
	for(int i = 0; i < MAX_PLAYERS; i++)
	{
		if(m_apPlayers[i] && distance(m_apPlayers[i]->m_ViewPos, Pos) < Distance)
			return true;
	}
	return false;
}

// Проверяем данж ли этот мир или нет
int CGS::ItDungeon(int WorldID) const
{
	for(const auto& dd : DungeonJob::Dungeon)
	{
		if(WorldID == dd.second.WorldID)
			return dd.first;
	}
	return -1;
}

IGameServer *CreateGameServer() { return new CGS; }
