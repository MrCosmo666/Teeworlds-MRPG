/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#include <stdarg.h>
#include <thread>
#include <algorithm>

#include <base/math.h>
#include <engine/shared/config.h>

#include <generated/server_data.h>
#include <game/version.h>
#include "gamecontext.h"

#include <game/collision.h>
#include <game/gamecore.h>
#include "gamemodes/main.h"
#include "gamemodes/dungeon.h"

#include "mmocore/GameEntities/Items/drop_bonuses.h"
#include "mmocore/GameEntities/Items/drop_items.h"
#include "mmocore/GameEntities/Items/flying_experience.h"
#include "mmocore/GameEntities/loltext.h"
#include "mmocore/CommandProcessor.h"
#include "mmocore/PathFinder.h"

#include <teeother/components/localization.h>

// static data that have the same value in different objects
std::map < int, CGS::StructAttribut > CGS::ms_aAttributsInfo;
std::map < std::string, int > CGS::ms_aEffects[MAX_PLAYERS];
int CGS::m_MultiplierExp = 100;

CGS::CGS()
{
	for(auto& pBroadcastState : m_aBroadcastStates)
	{
		pBroadcastState.m_NoChangeTick = 0;
		pBroadcastState.m_LifeSpanTick = 0;
		pBroadcastState.m_Priority = 0;
		pBroadcastState.m_aPrevMessage[0] = 0;
		pBroadcastState.m_aNextMessage[0] = 0;
	}

	for(auto& apPlayer : m_apPlayers)
		apPlayer = nullptr;

	m_pServer = nullptr;
	m_pController = nullptr;
	m_pMmoController = nullptr;
	m_pCommandProcessor = nullptr;
	m_pPathFinder = nullptr;
}

CGS::~CGS()
{
	m_Events.Clear();
	ms_aAttributsInfo.clear();
	m_CommandManager.ClearCommands();
	for(auto& pEffects : ms_aEffects)
		pEffects.clear();
	for(auto* apPlayer : m_apPlayers)
		delete apPlayer;

	delete m_pController;
	delete m_pMmoController;
	delete m_pCommandProcessor;
	delete m_pPathFinder;
}

class CCharacter *CGS::GetPlayerChar(int ClientID)
{
	if(ClientID < 0 || ClientID >= MAX_CLIENTS || !m_apPlayers[ClientID])
		return nullptr;
	return m_apPlayers[ClientID]->GetCharacter();
}

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

CPlayer* CGS::GetPlayerFromAuthID(int AuthID)
{
	for(int i = 0; i < MAX_PLAYERS; i++)
	{
		CPlayer* pPlayer = GetPlayer(i, true);
		if(pPlayer && pPlayer->Acc().m_AuthID == AuthID)
			return pPlayer;
	}
	return nullptr;
}

// Level String by Matodor (Progress Bar) creates some sort of bar progress
std::unique_ptr<char[]> CGS::LevelString(int MaxValue, int CurrentValue, int Step, char toValue, char fromValue)
{
	CurrentValue = clamp(CurrentValue, 0, MaxValue);
	 
	int Size = 3 + MaxValue / Step;
	std::unique_ptr<char[]> Buf(new char[Size]);
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
	// mrpg client
	if(IsMmoClient(ClientID))
	{
		if(HidenTabs)
			return ID >= NUM_TAB_MENU ? ("▵ ") : (ID < NUM_TAB_MENU_INTERACTIVES ? ("▼ :: ") : ("▲ :: "));
		return ID >= NUM_TAB_MENU ? ("▿ ") : (ID < NUM_TAB_MENU_INTERACTIVES ? ("▲ :: ") : ("▼ :: "));
	}

	// vanilla
	if(HidenTabs)
		return ID >= NUM_TAB_MENU ? ("/\\ # ") : (ID < NUM_TAB_MENU_INTERACTIVES ? ("\\/ # ") : ("/\\ # "));
	return ID >= NUM_TAB_MENU ? ("\\/ # ") : (ID < NUM_TAB_MENU_INTERACTIVES ? ("/\\ # ") : ("\\/ # "));
}

ItemInformation &CGS::GetItemInfo(int ItemID) const { return InventoryJob::ms_aItemsInfo[ItemID]; }
CDataQuest &CGS::GetQuestInfo(int QuestID) const { return QuestJob::ms_aDataQuests[QuestID]; }

/* #########################################################################
	EVENTS 
######################################################################### */
void CGS::CreateDamage(vec2 Pos, int ClientID, int Amount, bool CritDamage, bool OnlyVanilla)
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
		pEventMmo->m_DamageCount = Amount;
		pEventMmo->m_CritDamage = CritDamage;
	}
}

void CGS::CreateHammerHit(vec2 Pos)
{
	CNetEvent_HammerHit *pEvent = (CNetEvent_HammerHit *)m_Events.Create(NETEVENTTYPE_HAMMERHIT, sizeof(CNetEvent_HammerHit));
	if(pEvent)
	{
		pEvent->m_X = (int)Pos.x;
		pEvent->m_Y = (int)Pos.y;
	}
}

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
	const float Radius = g_pData->m_Explosion.m_Radius;
	const float InnerRadius = 48.0f;
	const float MaxForce = g_pData->m_Explosion.m_MaxForce;
	int Num = m_World.FindEntities(Pos, Radius, (CEntity**)apEnts, MAX_CLIENTS, CGameWorld::ENTTYPE_CHARACTER);
	for(int i = 0; i < Num; i++)
	{
		vec2 Diff = apEnts[i]->GetPos() - Pos;
		vec2 Force(0, MaxForce);
		const float l = length(Diff);
		if(l)
			Force = normalize(Diff) * MaxForce;
		const float Factor = 1 - clamp((l-InnerRadius)/(Radius-InnerRadius), 0.0f, 1.0f);
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

void CGS::SendWorldMusic(int ClientID, int MusicID)
{
	CNetMsg_Sv_WorldMusic Msg;
	Msg.m_pSoundID = (MusicID != 0 ? MusicID : m_MusicID);
	Msg.m_pVolume = (IsDungeon() ? 8 : 2);
	Server()->SendPackMsg(&Msg, MSGFLAG_VITAL|MSGFLAG_NORECORD, ClientID, m_WorldID);
}

void CGS::CreatePlayerSound(int ClientID, int Sound)
{
	// fix for vanilla unterstand SoundID
	if(!m_apPlayers[ClientID] || (!IsMmoClient(ClientID) && (Sound < 0 || Sound > 40)))
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
	CHAT FUNCTIONS 
######################################################################### */
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
		// send discord chat only from players
		if(ChatterClientID < MAX_PLAYERS)
			ChatDiscord(DC_SERVER_CHAT, Server()->ClientName(ChatterClientID), pText);

		Server()->SendPackMsg(&Msg, MSGFLAG_VITAL, -1);
	}
	else if(Mode == CHAT_TEAM)
	{
		CPlayer* pChatterPlayer = GetPlayer(ChatterClientID, true);
		if(!pChatterPlayer || pChatterPlayer->Acc().m_GuildID <= 0)
		{
			Chat(ChatterClientID, "This chat is intended for team / guilds!");
			return;
		}

		// send discord chat only from players
		if(pChatterPlayer)
			ChatDiscord(DC_SERVER_CHAT, Server()->ClientName(ChatterClientID), pText);

		// pack one for the recording only
		Server()->SendPackMsg(&Msg, MSGFLAG_VITAL|MSGFLAG_NOSEND, -1);

		// send chat to guild team
		const int GuildID = pChatterPlayer->Acc().m_GuildID;
		for(int i = 0; i < MAX_PLAYERS; i++)
		{
			CPlayer *pSearchPlayer = GetPlayer(i, true);
			if(pSearchPlayer && pSearchPlayer->Acc().m_GuildID == GuildID)
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

// Fake chat player in game
void CGS::FakeChat(const char *pName, const char *pText)
{
	const int FakeClientID = CreateBot(BotsTypes::TYPE_BOT_FAKE, 1, 1);
	if(FakeClientID < 0 || FakeClientID > MAX_CLIENTS || !m_apPlayers[FakeClientID])
		return;

	// kick off
	CNetMsg_Sv_ClientDrop LeaveMsg;
	LeaveMsg.m_ClientID = FakeClientID;
	LeaveMsg.m_pReason = "\0";
	LeaveMsg.m_Silent = true;
	Server()->SendPackMsg(&LeaveMsg, MSGFLAG_VITAL|MSGFLAG_NORECORD, -1);

	// update info
	CNetMsg_Sv_ClientInfo ClientInfoMsg;
	ClientInfoMsg.m_ClientID = FakeClientID;
	ClientInfoMsg.m_Local = false;
	ClientInfoMsg.m_Team = TEAM_BLUE;
	ClientInfoMsg.m_pName = pName;
	ClientInfoMsg.m_pClan = "::Bots::";
	ClientInfoMsg.m_Country = 137;
	ClientInfoMsg.m_Silent = true;
	for (int p = 0; p < 6; p++)
	{
		ClientInfoMsg.m_apSkinPartNames[p] = "standard";
		ClientInfoMsg.m_aUseCustomColors[p] = true;
		ClientInfoMsg.m_aSkinPartColors[p] = 0;
	}
	Server()->SendPackMsg(&ClientInfoMsg, MSGFLAG_VITAL|MSGFLAG_NORECORD, -1);
	
	// send a chat and delete a player and throw a player away
	SendChat(FakeClientID, CHAT_ALL, -1, pText);
	delete m_apPlayers[FakeClientID];
	m_apPlayers[FakeClientID] = nullptr;
	Server()->SendPackMsg(&LeaveMsg, MSGFLAG_VITAL|MSGFLAG_NORECORD, -1);
	Server()->BackInformationFakeClient(FakeClientID);
}

// send a formatted message
void CGS::Chat(int ClientID, const char* pText, ...)
{
	const int Start = (ClientID < 0 ? 0 : ClientID);
	const int End = (ClientID < 0 ? MAX_CLIENTS : ClientID + 1);

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

// send to an authorized player
void CGS::ChatAccountID(int AccountID, const char* pText, ...)
{
	CPlayer *pPlayer = GetPlayerFromAuthID(AccountID);
	if(!pPlayer)
		return;

	CNetMsg_Sv_Chat Msg;
	Msg.m_Mode = CHAT_ALL;
	Msg.m_ClientID = -1;

	va_list VarArgs;
	va_start(VarArgs, pText);
	
	dynamic_string Buffer;
	Server()->Localization()->Format_VL(Buffer, pPlayer->GetLanguage(), pText, VarArgs);

	Msg.m_TargetID = pPlayer->GetCID();
	Msg.m_pMessage = Buffer.buffer();
	Server()->SendPackMsg(&Msg, MSGFLAG_VITAL, pPlayer->GetCID());
	Buffer.clear();
	va_end(VarArgs);
}

// Send a guild a message
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
		if(pPlayer && pPlayer->Acc().IsGuild() && pPlayer->Acc().m_GuildID == GuildID)
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

// Send a message in world
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
		if(!pPlayer || !IsPlayerEqualWorldID(i, WorldID))
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

// Send discord message
void CGS::ChatDiscord(const char *Color, const char *Title, const char* pText, ...)
{
#ifdef CONF_DISCORD
	va_list VarArgs;
	va_start(VarArgs, pText);
	
	dynamic_string Buffer;
	Server()->Localization()->Format_VL(Buffer, "en", pText, VarArgs);
	Server()->SendDiscordMessage(g_Config.m_SvDiscordServerChatChannel, Color, Title, Buffer.buffer());
	Buffer.clear();

	va_end(VarArgs);
#endif
}

// Send a discord message to the channel
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

// Send Motd
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
void CGS::AddBroadcast(int ClientID, const char* pText, int Priority, int LifeSpan)
{
	if (ClientID < 0 || ClientID >= MAX_PLAYERS)
		return;

	if(LifeSpan > 0)
	{
		if(m_aBroadcastStates[ClientID].m_TimedPriority > Priority)
			return;

		str_copy(m_aBroadcastStates[ClientID].m_aTimedMessage, pText, sizeof(m_aBroadcastStates[ClientID].m_aTimedMessage));
		m_aBroadcastStates[ClientID].m_LifeSpanTick = LifeSpan;
		m_aBroadcastStates[ClientID].m_TimedPriority = Priority;
	}
	else
	{
		if(m_aBroadcastStates[ClientID].m_Priority > Priority)
			return;

		str_copy(m_aBroadcastStates[ClientID].m_aNextMessage, pText, sizeof(m_aBroadcastStates[ClientID].m_aNextMessage));
		m_aBroadcastStates[ClientID].m_Priority = Priority;
	}
}

// formatted broadcast
void CGS::Broadcast(int ClientID, int Priority, int LifeSpan, const char *pText, ...)
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

// formatted world broadcast
void CGS::BroadcastWorldID(int WorldID, int Priority, int LifeSpan, const char *pText, ...)
{
	va_list VarArgs;
	va_start(VarArgs, pText);
	for(int i = 0; i < MAX_PLAYERS; i++)
	{
		if(m_apPlayers[i] && IsPlayerEqualWorldID(i, WorldID))
		{
			dynamic_string Buffer;
			Server()->Localization()->Format_VL(Buffer, m_apPlayers[i]->GetLanguage(), pText, VarArgs);
			AddBroadcast(i, Buffer.buffer(), Priority, LifeSpan);
			Buffer.clear();
		}
	}
	va_end(VarArgs);	
}

// the tick of the broadcast and his life
void CGS::BroadcastTick(int ClientID)
{
	if (ClientID < 0 || ClientID >= MAX_PLAYERS)
		return;

	if(m_apPlayers[ClientID] && IsPlayerEqualWorldID(ClientID))
	{
		if(m_aBroadcastStates[ClientID].m_LifeSpanTick > 0 && m_aBroadcastStates[ClientID].m_TimedPriority > m_aBroadcastStates[ClientID].m_Priority)
			str_copy(m_aBroadcastStates[ClientID].m_aNextMessage, m_aBroadcastStates[ClientID].m_aTimedMessage, sizeof(m_aBroadcastStates[ClientID].m_aNextMessage));
		
		// send broadcast only if the message is different, or to fight auto-fading
		if(str_comp(m_aBroadcastStates[ClientID].m_aPrevMessage, m_aBroadcastStates[ClientID].m_aNextMessage) != 0 ||
			m_aBroadcastStates[ClientID].m_NoChangeTick > Server()->TickSpeed())
		{
			CNetMsg_Sv_Broadcast Msg;
			Msg.m_pMessage = m_aBroadcastStates[ClientID].m_aNextMessage;
			Server()->SendPackMsg(&Msg, MSGFLAG_VITAL, ClientID);
			str_copy(m_aBroadcastStates[ClientID].m_aPrevMessage, m_aBroadcastStates[ClientID].m_aNextMessage, sizeof(m_aBroadcastStates[ClientID].m_aPrevMessage));
			m_aBroadcastStates[ClientID].m_NoChangeTick = 0;
		}
		else
			m_aBroadcastStates[ClientID].m_NoChangeTick++;
		
		// update broadcast state
		if(m_aBroadcastStates[ClientID].m_LifeSpanTick > 0)
			m_aBroadcastStates[ClientID].m_LifeSpanTick--;
		
		if(m_aBroadcastStates[ClientID].m_LifeSpanTick <= 0)
		{
			m_aBroadcastStates[ClientID].m_aTimedMessage[0] = 0;
			m_aBroadcastStates[ClientID].m_TimedPriority = 0;
		}
		m_aBroadcastStates[ClientID].m_aNextMessage[0] = 0;
		m_aBroadcastStates[ClientID].m_Priority = 0;
	}
	else
	{
		m_aBroadcastStates[ClientID].m_NoChangeTick = 0;
		m_aBroadcastStates[ClientID].m_LifeSpanTick = 0;
		m_aBroadcastStates[ClientID].m_Priority = 0;
		m_aBroadcastStates[ClientID].m_TimedPriority = 0;
		m_aBroadcastStates[ClientID].m_aPrevMessage[0] = 0;
		m_aBroadcastStates[ClientID].m_aNextMessage[0] = 0;
		m_aBroadcastStates[ClientID].m_aTimedMessage[0] = 0;
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

// Send a change of skin
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

// Send equipments
void CGS::SendEquipments(int ClientID, int TargetID)
{
	CPlayer *pPlayer = GetPlayer(ClientID);
	if(!pPlayer)
		return;

	CNetMsg_Sv_EquipItems Msg;
	Msg.m_ClientID = ClientID;
	for(int k = 0; k < NUM_EQUIPS; k++)
	{
		const int EquipItem = pPlayer->GetEquippedItemID(k);
		const bool EnchantItem = pPlayer->IsBot() ? false : pPlayer->GetItem(EquipItem).IsEnchantMaxLevel();
		Msg.m_EquipID[k] = EquipItem;
		Msg.m_EnchantItem[k] = EnchantItem;
	}

	// send players equipping global bots local on world
	const int MsgWorldID = (pPlayer->IsBot() ? pPlayer->GetPlayerWorldID() : -1);
	Server()->SendPackMsg(&Msg, MSGFLAG_VITAL|MSGFLAG_NORECORD, TargetID, MsgWorldID);
}

// Send fully players equipments
void CGS::SendFullyEquipments(int TargetID)
{
	for (int i = 0; i < MAX_CLIENTS; i++)
			SendEquipments(i, TargetID);

	SendEquipments(TargetID, TargetID);
	SendEquipments(TargetID, -1);
}

void CGS::SendTeam(int ClientID, int Team, bool DoChatMsg, int TargetID)
{
	CNetMsg_Sv_Team Msg;
	Msg.m_ClientID = ClientID;
	Msg.m_Team = Team;
	Msg.m_Silent = DoChatMsg ? 0 : 1;
	Msg.m_CooldownTick = 0;
	Server()->SendPackMsg(&Msg, MSGFLAG_VITAL, TargetID);	
}

void CGS::SendGameMsg(int GameMsgID, int ClientID)
{
	CMsgPacker Msg(NETMSGTYPE_SV_GAMEMSG);
	Msg.AddInt(GameMsgID);
	Server()->SendMsg(&Msg, MSGFLAG_VITAL, ClientID);
}

void CGS::SendGameMsg(int GameMsgID, int ParaI1, int ClientID)
{
	CMsgPacker Msg(NETMSGTYPE_SV_GAMEMSG);
	Msg.AddInt(GameMsgID);
	Msg.AddInt(ParaI1);
	Server()->SendMsg(&Msg, MSGFLAG_VITAL, ClientID);
}

void CGS::SendGameMsg(int GameMsgID, int ParaI1, int ParaI2, int ParaI3, int ClientID)
{
	CMsgPacker Msg(NETMSGTYPE_SV_GAMEMSG);
	Msg.AddInt(GameMsgID);
	Msg.AddInt(ParaI1);
	Msg.AddInt(ParaI2);
	Msg.AddInt(ParaI3);
	Server()->SendMsg(&Msg, MSGFLAG_VITAL, ClientID);
}

// Send client information
void CGS::UpdateClientInformation(int ClientID)
{
	CPlayer *pPlayer = GetPlayer(ClientID, false, false);
	if(!pPlayer)
		return;
	
	pPlayer->SendClientInfo(-1);
	SendEquipments(ClientID, -1);
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
	// remove default commands from client
	SendRemoveChatCommand("all", ClientID);
	SendRemoveChatCommand("friend", ClientID);
	SendRemoveChatCommand("m", ClientID);
	SendRemoveChatCommand("mute", ClientID);
	SendRemoveChatCommand("r", ClientID);
	SendRemoveChatCommand("team", ClientID);
	SendRemoveChatCommand("w", ClientID);
	SendRemoveChatCommand("whisper", ClientID);

	// send our commands
	for (int i = 0; i < CommandManager()->CommandCount(); i++)
		SendChatCommand(CommandManager()->GetCommand(i), ClientID);
}

void CGS::SendRemoveChatCommand(const CCommandManager::CCommand* pCommand, int ClientID)
{
	CNetMsg_Sv_CommandInfoRemove Msg;
	Msg.m_Name = pCommand->m_aName;

	Server()->SendPackMsg(&Msg, MSGFLAG_VITAL, ClientID);
}

void CGS::SendRemoveChatCommand(const char* pCommand, int ClientID)
{
	CNetMsg_Sv_CommandInfoRemove Msg;
	Msg.m_Name = pCommand;

	Server()->SendPackMsg(&Msg, MSGFLAG_VITAL, ClientID);
}

void CGS::SendTuningParams(int ClientID)
{
	CMsgPacker Msg(NETMSGTYPE_SV_TUNEPARAMS);
	int *pParams = (int *)&m_Tuning;
	for(unsigned i = 0; i < sizeof(m_Tuning)/sizeof(int); i++)
		Msg.AddInt(pParams[i]);
	Server()->SendMsg(&Msg, MSGFLAG_VITAL, ClientID);
}

// Send a conversation package with someone
void CGS::SendTalkText(int ClientID, int TalkingID, bool PlayerTalked, const char *Message, int Style, int TalkingEmote)
{
	CNetMsg_Sv_TalkText Msg;
	Msg.m_PlayerTalked = PlayerTalked;
	Msg.m_pTalkClientID = TalkingID;
	Msg.m_pText = Message;
	Msg.m_TalkedEmote = TalkingEmote;
	Msg.m_Style = Style;
	Server()->SendPackMsg(&Msg, MSGFLAG_VITAL, ClientID);
}

void CGS::ClearTalkText(int ClientID)
{
	if(!IsMmoClient(ClientID))
		return;

	CNetMsg_Sv_ClearTalkText Msg;
	Server()->SendPackMsg(&Msg, MSGFLAG_VITAL, ClientID);
}

void CGS::SendProgressBar(int ClientID, int Count, int Request, const char* Message)
{
	CNetMsg_Sv_ProgressBar Msg;
	Msg.m_pText = Message;
	Msg.m_pCount = Count;
	Msg.m_pRequires = Request;
	Server()->SendPackMsg(&Msg, MSGFLAG_VITAL, ClientID);
}


/* #########################################################################
	ENGINE GAMECONTEXT 
######################################################################### */
void CGS::UpdateDiscordStatus()
{
#ifdef CONF_DISCORD
	if(Server()->Tick() % (Server()->TickSpeed() * 10) != 0 || m_WorldID != MAIN_WORLD_ID)
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
		Server()->UpdateDiscordStatus(aBuf);
		return;
	}
	Server()->UpdateDiscordStatus("and expects players.");
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

void CGS::OnInit(int WorldID)
{
	m_pServer = Kernel()->RequestInterface<IServer>();
	m_pConsole = Kernel()->RequestInterface<IConsole>();
	m_World.SetGameServer(this);
	m_Events.SetGameServer(this);
	m_CommandManager.Init(m_pConsole, this, NewCommandHook, RemoveCommandHook);
	m_WorldID = WorldID;
	m_RespawnWorldID = -1;
	m_MusicID = -1;

	for(int i = 0; i < NUM_NETOBJTYPES; i++)
		Server()->SnapSetStaticsize(i, m_NetObjHandler.GetObjSize(i));

	// create controller
	m_Layers.Init(Kernel(), nullptr, WorldID);
	m_Collision.Init(&m_Layers);
	m_pMmoController = new MmoController(this);
	m_pMmoController->LoadLogicWorld();
	UpdateZoneDungeon();
	UpdateZonePVP();

	// create all game objects for the server
	if(IsDungeon())
		m_pController = new CGameControllerDungeon(this);
	else
		m_pController = new CGameControllerMain(this);

	m_pCommandProcessor = new CCommandProcessor(this);
	m_pController->RegisterChatCommands(CommandManager());

	// initialize layers
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
	
	// initialize pathfinder
	m_pPathFinder = new CPathfinder(&m_Layers, &m_Collision);
	Console()->Chain("sv_motd", ConchainSpecialMotdupdate, this);
}

void CGS::OnConsoleInit()
{
	m_pServer = Kernel()->RequestInterface<IServer>();
	m_pConsole = Kernel()->RequestInterface<IConsole>();

	Console()->Register("set_world_time", "i[hour]", CFGFLAG_SERVER, ConSetWorldTime, m_pServer, "Set worlds time.");
	Console()->Register("parseskin", "i[cid]", CFGFLAG_SERVER, ConParseSkin, m_pServer, "Parse skin on console. Easy for devlop bots.");
	Console()->Register("giveitem", "i[cid]i[itemid]i[count]i[enchant]i[mail]", CFGFLAG_SERVER, ConGiveItem, m_pServer, "Give item <clientid> <itemid> <count> <enchant> <mail 1=yes 0=no>");
	Console()->Register("removeitem", "i[cid]i[itemid]i[count]", CFGFLAG_SERVER, ConRemItem, m_pServer, "Remove item <clientid> <itemid> <count>");
	Console()->Register("disband_guild", "r[guildname]", CFGFLAG_SERVER, ConDisbandGuild, m_pServer, "Disband the guild with the name");
	Console()->Register("say", "r[text]", CFGFLAG_SERVER, ConSay, m_pServer, "Say in chat");
	Console()->Register("addcharacter", "i[cid]r[botname]", CFGFLAG_SERVER, ConAddCharacter, m_pServer, "(Warning) Add new bot on database or update if finding <clientid> <bot name>");
}

void CGS::OnTick()
{
	m_World.m_Core.m_Tuning = m_Tuning;
	m_World.Tick();

	m_pController->Tick();
	for(int i = 0; i < MAX_CLIENTS; i++)
	{
		if(!m_apPlayers[i] || m_apPlayers[i]->GetPlayerWorldID() != m_WorldID)
			continue;

		m_apPlayers[i]->Tick();
		m_apPlayers[i]->PostTick();
		if(i < MAX_PLAYERS)
		{
			BroadcastTick(i);
		}
	}

	Mmo()->OnTick();
}

// Here we use functions that can have static data or functions that don't need to be called in all worlds
void CGS::OnTickMainWorld()
{
	if(m_DayEnumType != Server()->GetEnumTypeDay())
	{
		m_DayEnumType = Server()->GetEnumTypeDay(); 
		if(m_DayEnumType == DayType::NIGHT_TYPE)
			m_MultiplierExp = 100 + random_int() % 200;
		else if(m_DayEnumType == DayType::MORNING_TYPE)
			m_MultiplierExp = 100;
	
		SendDayInfo(-1);
	}

	UpdateDiscordStatus();
}

// output of all objects
void CGS::OnSnap(int ClientID)
{
	CPlayer* pPlayer = m_apPlayers[ClientID];
	if(!pPlayer || pPlayer->GetPlayerWorldID() != GetWorldID())
		return;

	m_World.Snap(ClientID);
	m_pController->Snap();
	m_Events.Snap(ClientID);
	for(auto& arpPlayer : m_apPlayers)
	{
		if(arpPlayer)
			arpPlayer->Snap(ClientID);
	}
}

void CGS::OnPreSnap() {}
void CGS::OnPostSnap()
{
	m_World.PostSnap();
	m_Events.Clear();
}

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
			if(g_Config.m_SvSpamprotection && pPlayer->m_aPlayerTick[TickState::LastChat] && pPlayer->m_aPlayerTick[TickState::LastChat]+Server()->TickSpeed() > Server()->Tick())
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

			pPlayer->m_aPlayerTick[TickState::LastChat] = Server()->Tick();

			const int Mode = pMsg->m_Mode;
			if(Mode != CHAT_NONE)
			{
				if(pMsg->m_pMessage[0] == '/')
				{
					CommandProcessor()->ChatCmd(pMsg, pPlayer);
					return;
				}

				SendChat(ClientID, Mode, pMsg->m_Target, pMsg->m_pMessage);						
			}
		}

		else if (MsgID == NETMSGTYPE_CL_COMMAND)
		{
			if (g_Config.m_SvSpamprotection && pPlayer->m_aPlayerTick[TickState::LastChat] && pPlayer->m_aPlayerTick[TickState::LastChat] + Server()->TickSpeed() > Server()->Tick())
				return;

			CNetMsg_Cl_Command *pMsg = (CNetMsg_Cl_Command*)pRawMsg;

			if (!pMsg->m_Name[0])
				return;

			char aFullMsg[256];
			str_format(aFullMsg, sizeof(aFullMsg), "/%s %s", pMsg->m_Name, pMsg->m_Arguments);

			if (pMsg->m_Arguments[0])
			{
				// trim right and set maximum length to 128 utf8-characters
				int Length = 0;
				const char* p = pMsg->m_Arguments;
				const char* pEnd = nullptr;
				while (*p)
				{
					const char* pStrOld = p;
					int Code = str_utf8_decode(&p);

					// check if unicode is not empty
					if (Code > 0x20 && Code != 0xA0 && Code != 0x034F && (Code < 0x2000 || Code > 0x200F) && (Code < 0x2028 || Code > 0x202F) &&
						(Code < 0x205F || Code > 0x2064) && (Code < 0x206A || Code > 0x206F) && (Code < 0xFE00 || Code > 0xFE0F) &&
						Code != 0xFEFF && (Code < 0xFFF9 || Code > 0xFFFC))
					{
						pEnd = nullptr;
					}
					else if (pEnd == nullptr)
						pEnd = pStrOld;

					if (++Length >= 127)
					{
						*(const_cast<char*>(p)) = 0;
						break;
					}
				}
				if (pEnd != nullptr)
					*(const_cast<char*>(pEnd)) = 0;

				// drop empty and autocreated spam messages (more than 20 characters per second)
				if (Length == 0)
					return;
			}

			pPlayer->m_aPlayerTick[TickState::LastChat] = Server()->Tick();

			if (Console()->IsCommand(pMsg->m_Name, CFGFLAG_CHAT))
			{
				Console()->ExecuteLineFlag(aFullMsg + 1, CFGFLAG_CHAT, ClientID, false);
				return;
			}

			ChatFollow(ClientID, "Command {STR} not found!", pMsg->m_Name);
			// CommandManager()->OnCommand(pMsg->m_Name, pMsg->m_Arguments, ClientID);
		}

		else if(MsgID == NETMSGTYPE_CL_CALLVOTE)
		{
			CNetMsg_Cl_CallVote *pMsg = (CNetMsg_Cl_CallVote *)pRawMsg;
			if (str_comp_nocase(pMsg->m_Type, "option") != 0 || Server()->Tick() < (pPlayer->m_aPlayerTick[TickState::LastVoteTry] + (Server()->TickSpeed() / 2)))
				return;
			
			pPlayer->m_aPlayerTick[TickState::LastVoteTry] = Server()->Tick();
			const auto& item = std::find_if(m_aPlayerVotes[ClientID].begin(), m_aPlayerVotes[ClientID].end(), [pMsg](const CVoteOptions& vote)
			{
				return (str_comp_nocase(pMsg->m_Value, vote.m_aDescription) == 0);
			});

			if(item != m_aPlayerVotes[ClientID].end())
			{
				const int InteractiveCount = string_to_number(pMsg->m_Reason, 1, 10000000);
				ParsingVoteCommands(ClientID, item->m_aCommand, item->m_TempID, item->m_TempID2, InteractiveCount, pMsg->m_Reason, item->m_Callback);
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
				Broadcast(pPlayer->GetCID(), BroadcastPriority::BROADCAST_MAIN_INFORMATION, 100, "Use /register <name> <pass>.");
				return;
			}

			Broadcast(ClientID, BroadcastPriority::BROADCAST_MAIN_INFORMATION, 100, "Team change is not allowed.");
		}

		else if (MsgID == NETMSGTYPE_CL_SETSPECTATORMODE)
		{
			return;
		}

		else if (MsgID == NETMSGTYPE_CL_EMOTICON)
		{
			CNetMsg_Cl_Emoticon *pMsg = (CNetMsg_Cl_Emoticon *)pRawMsg;

			if(g_Config.m_SvSpamprotection && pPlayer->m_aPlayerTick[TickState::LastEmote] && pPlayer->m_aPlayerTick[TickState::LastEmote]+(Server()->TickSpeed() / 2) > Server()->Tick())
				return;

			pPlayer->m_aPlayerTick[TickState::LastEmote] = Server()->Tick();
			SendEmoticon(ClientID, pMsg->m_Emoticon, true);
		}

		else if (MsgID == NETMSGTYPE_CL_KILL)
		{
			if(pPlayer->m_aPlayerTick[TickState::LastKill] && pPlayer->m_aPlayerTick[TickState::LastKill]+Server()->TickSpeed()*3 > Server()->Tick())
				return;

			Broadcast(ClientID, BroadcastPriority::BROADCAST_MAIN_INFORMATION, 100, "Self kill is not allowed.");
			//pPlayer->m_PlayerTick[TickState::LastKill] = Server()->Tick();
			//pPlayer->KillCharacter(WEAPON_SELF);
		}

		else if (MsgID == NETMSGTYPE_CL_READYCHANGE)
		{
			return;
		}

		else if(MsgID == NETMSGTYPE_CL_SKINCHANGE)
		{
			if(pPlayer->m_aPlayerTick[TickState::LastChangeInfo] && pPlayer->m_aPlayerTick[TickState::LastChangeInfo]+Server()->TickSpeed()*5 > Server()->Tick())
				return;

			pPlayer->m_aPlayerTick[TickState::LastChangeInfo] = Server()->Tick();
			CNetMsg_Cl_SkinChange *pMsg = (CNetMsg_Cl_SkinChange *)pRawMsg;

			for(int p = 0; p < NUM_SKINPARTS; p++)
			{
				str_utf8_copy_num(pPlayer->Acc().m_aaSkinPartNames[p], pMsg->m_apSkinPartNames[p], sizeof(pPlayer->Acc().m_aaSkinPartNames[p]), MAX_SKIN_LENGTH);
				pPlayer->Acc().m_aUseCustomColors[p] = pMsg->m_aUseCustomColors[p];
				pPlayer->Acc().m_aSkinPartColors[p] = pMsg->m_aSkinPartColors[p];
			}

			// update all clients
			for(int i = 0; i < MAX_CLIENTS; ++i)
			{
				if(!m_apPlayers[i] || Server()->GetClientProtocolVersion(i) < MIN_SKINCHANGE_CLIENTVERSION)
					continue;

				SendSkinChange(pPlayer->GetCID(), i);
			}
		}

		//////////////////////////////////////////////////////////////////////////////////
		///////////// If the client has passed the test, the alternative is to use the client
		else if (MsgID == NETMSGTYPE_CL_ISMMOSERVER)
		{
			CNetMsg_Cl_IsMmoServer *pMsg = (CNetMsg_Cl_IsMmoServer *)pRawMsg;
			Server()->SetClientProtocolVersion(ClientID, pMsg->m_Version);

			/*
				receive information about the client if the old version is written and prohibit the use of all client functions
				to avoid crashes between package sizes
			*/
			if(!IsMmoClient(ClientID))
			{
				Server()->Kick(ClientID, "Update client use updater or download in discord.");
				return;
			}

			// send check success message
			CNetMsg_Sv_AfterIsMmoServer GoodCheck;
			Server()->SendPackMsg(&GoodCheck, MSGFLAG_VITAL|MSGFLAG_FLUSH|MSGFLAG_NORECORD, ClientID);

			// loading of all parts of players' data
			SendFullyEquipments(ClientID);
		}
		else 
			Mmo()->OnMessage(MsgID, pRawMsg, ClientID);
	}
	else
	{
		if (MsgID == NETMSGTYPE_CL_STARTINFO)
		{
			if(pPlayer->m_aPlayerTick[TickState::LastChangeInfo] != 0)
				return;

			CNetMsg_Cl_StartInfo *pMsg = (CNetMsg_Cl_StartInfo *)pRawMsg;
			pPlayer->m_aPlayerTick[TickState::LastChangeInfo] = Server()->Tick();

			// set start infos
			Server()->SetClientName(ClientID, pMsg->m_pName);
			Server()->SetClientClan(ClientID, pMsg->m_pClan);
			Server()->SetClientCountry(ClientID, pMsg->m_Country);
			for(int p = 0; p < 6; p++)
			{
				str_utf8_copy_num(pPlayer->Acc().m_aaSkinPartNames[p], pMsg->m_apSkinPartNames[p], sizeof(pPlayer->Acc().m_aaSkinPartNames[p]), MAX_SKIN_LENGTH);
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

void CGS::OnClientConnected(int ClientID)
{
	if(!m_apPlayers[ClientID])
	{
		const int AllocMemoryCell = ClientID+m_WorldID*MAX_CLIENTS;
		m_apPlayers[ClientID] = new(AllocMemoryCell) CPlayer(this, ClientID);
	}

	SendMotd(ClientID);
	SendSettings(ClientID);
	m_aBroadcastStates[ClientID] = {};
}

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
		SendTeam(ClientID, TEAM_SPECTATORS, false, -1);
		Chat(ClientID, "- - - - - - - [Welcome to MRPG] - - - - - - -");
		Chat(ClientID, "You need to create an account, or log in if you already have.");
		Chat(ClientID, "Register: \"/register <login> <pass>\"");
		Chat(ClientID, "Log in: \"/login <login> <pass>\"");
		SendDayInfo(ClientID);
		
		ChatDiscord(DC_JOIN_LEAVE, Server()->ClientName(ClientID), "connected and enter in MRPG");
		ShowVotesNewbieInformation(ClientID);
		return;
	}

	Mmo()->Account()->LoadAccount(pPlayer, false);
	ResetVotes(ClientID, MenuList::MAIN_MENU);
}

void CGS::OnClientDrop(int ClientID, const char *pReason)
{
	if(!m_apPlayers[ClientID] || m_apPlayers[ClientID]->IsBot())
		return;

	// update clients on drop
	m_pController->OnPlayerDisconnect(m_apPlayers[ClientID]);
	if (Server()->ClientIngame(ClientID) && IsPlayerEqualWorldID(ClientID))
	{
		ChatDiscord(DC_JOIN_LEAVE, Server()->ClientName(ClientID), "leave game MRPG");

		CNetMsg_Sv_ClientDrop Msg;
		Msg.m_ClientID = ClientID;
		Msg.m_pReason = pReason;
		Msg.m_Silent = false;
		Server()->SendPackMsg(&Msg, MSGFLAG_VITAL | MSGFLAG_NORECORD, -1);
	}

	delete m_apPlayers[ClientID];
	m_apPlayers[ClientID] = nullptr;
}

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

// change the world
void CGS::PrepareClientChangeWorld(int ClientID)
{
	if (m_apPlayers[ClientID])
	{
		m_apPlayers[ClientID]->KillCharacter(WEAPON_WORLD);
		delete m_apPlayers[ClientID];
		m_apPlayers[ClientID] = nullptr;
	}
	const int AllocMemoryCell = ClientID+m_WorldID*MAX_CLIENTS;
	m_apPlayers[ClientID] = new(AllocMemoryCell) CPlayer(this, ClientID);
}

bool CGS::IsClientReady(int ClientID) const
{
	return m_apPlayers[ClientID] && m_apPlayers[ClientID]->m_aPlayerTick[TickState::LastChangeInfo] > 0;
}

bool CGS::IsClientPlayer(int ClientID) const
{
	return m_apPlayers[ClientID] && m_apPlayers[ClientID]->GetTeam() == TEAM_SPECTATORS ? false : true;
}
bool CGS::IsMmoClient(int ClientID) const
{
	return (bool)((Server()->GetClientProtocolVersion(ClientID) == PROTOCOL_VERSION_MMO) || (ClientID >= MAX_PLAYERS && ClientID < MAX_CLIENTS)); 
}
const char *CGS::GameType() const { return m_pController && m_pController->GetGameType() ? m_pController->GetGameType() : ""; }
const char *CGS::Version() const { return GAME_VERSION; }
const char *CGS::NetVersion() const { return GAME_NETVERSION; }

// clearing all data at the exit of the client necessarily call once enough
void CGS::ClearClientData(int ClientID)
{
	Mmo()->ResetClientData(ClientID);
	m_aPlayerVotes[ClientID].clear();
	ms_aEffects[ClientID].clear();

	// clear active snap bots for player
	for(auto& pActiveSnap : BotJob::ms_aDataBot)
		pActiveSnap.second.m_aAlreadyActiveQuestBot[ClientID] = false;
}

int CGS::GetRank(int AuthID)
{
	return Mmo()->Account()->GetRank(AuthID);
}

/* #########################################################################
	CONSOLE GAMECONTEXT 
######################################################################### */
void CGS::ConSetWorldTime(IConsole::IResult* pResult, void* pUserData)
{
	int Hour = pResult->GetInteger(0);
	IServer* pServer = (IServer*)pUserData;
	pServer->SetOffsetWorldTime(Hour);
}

void CGS::ConParseSkin(IConsole::IResult *pResult, void *pUserData)
{
	int ClientID = clamp(pResult->GetInteger(0), 0, MAX_PLAYERS-1);
	IServer* pServer = (IServer*)pUserData;
	CGS* pSelf = (CGS*)pServer->GameServer(pServer->GetClientWorldID(ClientID));

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

// give the item to the player
void CGS::ConGiveItem(IConsole::IResult *pResult, void *pUserData)
{
	const int ClientID = clamp(pResult->GetInteger(0), 0, MAX_PLAYERS - 1);
	const int ItemID = pResult->GetInteger(1);
	const int Count = pResult->GetInteger(2);
	const int Enchant = pResult->GetInteger(3);
	const int Mail = pResult->GetInteger(4);

	IServer* pServer = (IServer*)pUserData;
	CGS* pSelf = (CGS*)pServer->GameServer(pServer->GetClientWorldID(ClientID));

	CPlayer *pPlayer = pSelf->GetPlayer(ClientID, true);
	if(pPlayer)
	{
		if (Mail == 0)
		{
			pPlayer->GetItem(ItemID).Add(Count, 0, Enchant);
			return;
		}
		pSelf->SendInbox(pPlayer, "The sender heavens", "Sent from console", ItemID, Count, Enchant);
	}
}

void CGS::ConDisbandGuild(IConsole::IResult* pResult, void* pUserData)
{
	IServer* pServer = (IServer*)pUserData;
	CGS* pSelf = (CGS*)pServer->GameServer(MAIN_WORLD_ID);
	const char* pGuildName = pResult->GetString(0);
	const int GuildID = pSelf->Mmo()->Member()->SearchGuildByName(pGuildName);

	char aBuf[256];
	if(GuildID <= 0)
	{
		str_format(aBuf, sizeof(aBuf), "\"%s\", no such guild has been found.", pGuildName);
		pSelf->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "disbandguild", aBuf);
		return;
	}

	str_format(aBuf, sizeof(aBuf), "Guild with identifier %d and by the name of %s has been disbanded.", GuildID, pGuildName);
	pSelf->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "disbandguild", aBuf);
	pSelf->Mmo()->Member()->DisbandGuild(GuildID);
}

void CGS::ConRemItem(IConsole::IResult* pResult, void* pUserData)
{
	const int ClientID = clamp(pResult->GetInteger(0), 0, MAX_PLAYERS - 1);
	const int ItemID = pResult->GetInteger(1);
	const int Count = pResult->GetInteger(2);

	IServer* pServer = (IServer*)pUserData;
	CGS* pSelf = (CGS*)pServer->GameServer(pServer->GetClientWorldID(ClientID));

	CPlayer* pPlayer = pSelf->GetPlayer(ClientID, true);
	if (pPlayer)
	{
		pPlayer->GetItem(ItemID).Remove(Count, 0);
	}
}

void CGS::ConSay(IConsole::IResult *pResult, void *pUserData)
{
	IServer* pServer = (IServer*)pUserData;
	CGS* pSelf = (CGS*)pServer->GameServer(MAIN_WORLD_ID);
	pSelf->SendChat(-1, CHAT_ALL, -1, pResult->GetString(0));
}

// add a new bot player to the database
void CGS::ConAddCharacter(IConsole::IResult *pResult, void *pUserData)
{
	const int ClientID = clamp(pResult->GetInteger(0), 0, MAX_PLAYERS - 1);
	IServer* pServer = (IServer*)pUserData;
	CGS* pSelf = (CGS*)pServer->GameServer(pServer->GetClientWorldID(ClientID));

	// we check if there is a player
	if(ClientID < 0 || ClientID >= MAX_PLAYERS || !pSelf->m_apPlayers[ClientID])
		return;
	
	// add a new kind of bot
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
		return;
	}
}

/* #########################################################################
	VOTING MMO GAMECONTEXT 
######################################################################### */
void CGS::ClearVotes(int ClientID)
{
	m_aPlayerVotes[ClientID].clear();
	
	// send vote options
	CNetMsg_Sv_VoteClearOptions ClearMsg;
	Server()->SendPackMsg(&ClearMsg, MSGFLAG_VITAL, ClientID);
}

// add a vote
void CGS::AV(int ClientID, const char *pCmd, const char *pDesc, const int TempInt, const int TempInt2, const char *pIcon, VoteCallBack Callback)
{
	if(ClientID < 0 || ClientID >= MAX_PLAYERS || !m_apPlayers[ClientID])
		return;

	char aBufDesc[128]; // buffer x2 with unicode
	str_copy(aBufDesc, pDesc, sizeof(aBufDesc));
	if(str_comp(m_apPlayers[ClientID]->GetLanguage(), "ru") == 0 || str_comp(m_apPlayers[ClientID]->GetLanguage(), "uk") == 0)
		str_translation_utf8_to_cp(aBufDesc);
	
	CVoteOptions Vote;	
	str_copy(Vote.m_aDescription, aBufDesc, sizeof(Vote.m_aDescription));
	str_copy(Vote.m_aCommand, pCmd, sizeof(Vote.m_aCommand));
	str_copy(Vote.m_aIcon, pIcon, sizeof(Vote.m_aIcon));
	Vote.m_TempID = TempInt;
	Vote.m_TempID2 = TempInt2;
	Vote.m_Callback = Callback;

	// trim right and set maximum length to 64 utf8-characters
	int Length = 0;
	const char *p = Vote.m_aDescription;
	const char *pEnd = nullptr;
	while(*p)
	{
		const char *pStrOld = p;
		const int Code = str_utf8_decode(&p);

		// check if unicode is not empty
		if(Code > 0x20 && Code != 0xA0 && Code != 0x034F && (Code < 0x2000 || Code > 0x200F) && (Code < 0x2028 || Code > 0x202F) &&
			(Code < 0x205F || Code > 0x2064) && (Code < 0x206A || Code > 0x206F) && (Code < 0xFE00 || Code > 0xFE0F) &&
			Code != 0xFEFF && (Code < 0xFFF9 || Code > 0xFFFC))
		{
			pEnd = nullptr;
		}
		else if(pEnd == nullptr)
			pEnd = pStrOld;

		if(++Length >= 63)
		{
			*(const_cast<char *>(p)) = 0;
			break;
		}
	}
	if(pEnd != nullptr)
		*(const_cast<char *>(pEnd)) = 0;
	
	m_aPlayerVotes[ClientID].push_back(Vote);

	// send to customers that have a mmo client
	if(IsMmoClient(ClientID))
	{
		if(Vote.m_aDescription[0] == '\0')
			m_apPlayers[ClientID]->m_Colored = { 0, 0, 0 };

		CNetMsg_Sv_VoteMmoOptionAdd OptionMsg;
		const vec3 ToHexColor = m_apPlayers[ClientID]->m_Colored;
		OptionMsg.m_pHexColor = ((int)ToHexColor.r << 16) + ((int)ToHexColor.g << 8) + (int)ToHexColor.b;
		OptionMsg.m_pDescription = Vote.m_aDescription;
		StrToInts(OptionMsg.m_pIcon, 4, Vote.m_aIcon);
		Server()->SendPackMsg(&OptionMsg, MSGFLAG_VITAL|MSGFLAG_NORECORD, ClientID);
		return;
	}

	if(Vote.m_aDescription[0] == '\0')
		str_copy(Vote.m_aDescription, "———————————", sizeof(Vote.m_aDescription));

	// send to vanilla clients
	CNetMsg_Sv_VoteOptionAdd OptionMsg;	
	OptionMsg.m_pDescription = Vote.m_aDescription;
	Server()->SendPackMsg(&OptionMsg, MSGFLAG_VITAL, ClientID);
}

// add formatted vote
void CGS::AVL(int ClientID, const char *pCmd, const char *pText, ...)
{
	if(ClientID >= 0 && ClientID < MAX_PLAYERS && m_apPlayers[ClientID])
	{
		va_list VarArgs;
		va_start(VarArgs, pText);
		
		dynamic_string Buffer;
		if(str_comp(pCmd, "null") != 0)
			Buffer.append("- ");
		
		Server()->Localization()->Format_VL(Buffer, m_apPlayers[ClientID]->GetLanguage(), pText, VarArgs);
		AV(ClientID, pCmd, Buffer.buffer());
		Buffer.clear();
		
		va_end(VarArgs);
	}
}

// add formatted vote with color
void CGS::AVH(int ClientID, const int HideID, vec3 Color, const char *pText, ...)
{
	if(ClientID >= 0 && ClientID < MAX_PLAYERS && m_apPlayers[ClientID])
	{
		va_list VarArgs;
		va_start(VarArgs, pText);

		dynamic_string Buffer;
		const bool HidenTabs = (HideID >= TAB_STAT) ? m_apPlayers[ClientID]->GetHidenMenu(HideID) : false;
		Buffer.append(GetSymbolHandleMenu(ClientID, HidenTabs, HideID));

		Server()->Localization()->Format_VL(Buffer, m_apPlayers[ClientID]->GetLanguage(), pText, VarArgs);
		if(HideID > TAB_SETTINGS_MODULES && HideID < NUM_TAB_MENU) { Buffer.append(" (Press me for help)"); }

		m_apPlayers[ClientID]->m_Colored = { Color.r, Color.g, Color.b };
		AV(ClientID, "HIDEN", Buffer.buffer(), HideID, -1, "unused");
		if(length(m_apPlayers[ClientID]->m_Colored) > 1.0f)
			m_apPlayers[ClientID]->m_Colored /= 4.0f;
		Buffer.clear();
		va_end(VarArgs);
	}
}

// add formatted vote with color and icon
void CGS::AVHI(int ClientID, const char *pIcon, const int HideID, vec3 Color, const char* pText, ...)
{
	if(ClientID >= 0 && ClientID < MAX_PLAYERS && m_apPlayers[ClientID])
	{
		va_list VarArgs;
		va_start(VarArgs, pText);

		dynamic_string Buffer;
		const bool HidenTabs = (bool)(HideID >= TAB_STAT ? m_apPlayers[ClientID]->GetHidenMenu(HideID) : false);
		Buffer.append(GetSymbolHandleMenu(ClientID, HidenTabs, HideID));

		Server()->Localization()->Format_VL(Buffer, m_apPlayers[ClientID]->GetLanguage(), pText, VarArgs);
		if(HideID > TAB_SETTINGS_MODULES && HideID < NUM_TAB_MENU)
			Buffer.append(" (Press me for help)");

		m_apPlayers[ClientID]->m_Colored = Color;
		AV(ClientID, "HIDEN", Buffer.buffer(), HideID, -1, pIcon);
		if(length(m_apPlayers[ClientID]->m_Colored) > 1.0f)
			m_apPlayers[ClientID]->m_Colored /= 4.0f;
			
		Buffer.clear();
		va_end(VarArgs);
	}
}

// add formatted vote as menu
void CGS::AVM(int ClientID, const char *pCmd, const int TempInt, const int HideID, const char* pText, ...)
{
	if(ClientID >= 0 && ClientID < MAX_PLAYERS && m_apPlayers[ClientID])
	{
		if((!m_apPlayers[ClientID]->GetHidenMenu(HideID) && HideID > TAB_SETTINGS_MODULES) ||
			(m_apPlayers[ClientID]->GetHidenMenu(HideID) && HideID <= TAB_SETTINGS_MODULES))
			return;

		va_list VarArgs;
		va_start(VarArgs, pText);

		dynamic_string Buffer;
		if(TempInt != NOPE) { Buffer.append("- "); }

		Server()->Localization()->Format_VL(Buffer, m_apPlayers[ClientID]->GetLanguage(), pText, VarArgs);
		AV(ClientID, pCmd, Buffer.buffer(), TempInt);
		Buffer.clear();
		va_end(VarArgs);
	}
}

// add formatted vote as menu with icon
void CGS::AVMI(int ClientID, const char *pIcon, const char *pCmd, const int TempInt, const int HideID, const char *pText, ...)
{
	if(ClientID >= 0 && ClientID < MAX_PLAYERS && m_apPlayers[ClientID])
	{
		if((!m_apPlayers[ClientID]->GetHidenMenu(HideID) && HideID > TAB_SETTINGS_MODULES) ||
			(m_apPlayers[ClientID]->GetHidenMenu(HideID) && HideID <= TAB_SETTINGS_MODULES))
			return;

		va_list VarArgs;
		va_start(VarArgs, pText);

		dynamic_string Buffer;
		if(TempInt != NOPE) { Buffer.append("- "); }

		Server()->Localization()->Format_VL(Buffer, m_apPlayers[ClientID]->GetLanguage(), pText, VarArgs);
		AV(ClientID, pCmd, Buffer.buffer(), TempInt, -1, pIcon);
		Buffer.clear();
		va_end(VarArgs);
	}
}

// add formatted vote with multiple id's
void CGS::AVD(int ClientID, const char *pCmd, const int TempInt, const int TempInt2, const int HideID, const char *pText, ...)
{
	if(ClientID >= 0 && ClientID < MAX_PLAYERS && m_apPlayers[ClientID])
	{
		if((!m_apPlayers[ClientID]->GetHidenMenu(HideID) && HideID > TAB_SETTINGS_MODULES) ||
			(m_apPlayers[ClientID]->GetHidenMenu(HideID) && HideID <= TAB_SETTINGS_MODULES))
			return;
			
		va_list VarArgs;
		va_start(VarArgs, pText);

		dynamic_string Buffer;
		if(TempInt != NOPE) { Buffer.append("- "); }

		Server()->Localization()->Format_VL(Buffer, m_apPlayers[ClientID]->GetLanguage(), pText, VarArgs);
		AV(ClientID, pCmd, Buffer.buffer(), TempInt, TempInt2);
		Buffer.clear();
		va_end(VarArgs);
	}
}

// add formatted callback vote with multiple id's (need disallow used it for menu)
void CGS::AVCALLBACK(int ClientID, const char *pCmd, const char *pIcon, const int TempInt, const int TempInt2, const int HideID, VoteCallBack Callback, const char* pText, ...)
{
	if(ClientID >= 0 && ClientID < MAX_PLAYERS && m_apPlayers[ClientID])
	{
		if((!m_apPlayers[ClientID]->GetHidenMenu(HideID) && HideID > TAB_SETTINGS_MODULES) ||
			(m_apPlayers[ClientID]->GetHidenMenu(HideID) && HideID <= TAB_SETTINGS_MODULES))
			return;

		va_list VarArgs;
		va_start(VarArgs, pText);

		dynamic_string Buffer;
		Buffer.append("- ");

		Server()->Localization()->Format_VL(Buffer, m_apPlayers[ClientID]->GetLanguage(), pText, VarArgs);
		AV(ClientID, pCmd, Buffer.buffer(), TempInt, TempInt2, pIcon, Callback);

		Buffer.clear();
		va_end(VarArgs);
	}
}

void CGS::ResetVotes(int ClientID, int MenuList)
{
	CPlayer *pPlayer = GetPlayer(ClientID, true);
	if(!pPlayer)
		return;

	if(pPlayer && pPlayer->m_ActiveMenuRegisteredCallback)
	{
		pPlayer->m_ActiveMenuRegisteredCallback = nullptr;
		pPlayer->m_ActiveMenuOptionCallback = { 0 };
	}

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

		// statistics menu
		const int ExpForLevel = pPlayer->ExpNeed(pPlayer->Acc().m_Level);
		AVH(ClientID, TAB_STAT, GREEN_COLOR, "Hi, {STR} Last log in {STR}", Server()->ClientName(ClientID), pPlayer->Acc().m_aLastLogin);
		AVM(ClientID, "null", NOPE, TAB_STAT, "Discord: \"{STR}\"", g_Config.m_SvDiscordInviteLink);
		AVM(ClientID, "null", NOPE, TAB_STAT, "Level {INT} : Exp {INT}/{INT}", &pPlayer->Acc().m_Level, &pPlayer->Acc().m_Exp, &ExpForLevel);
		AVM(ClientID, "null", NOPE, TAB_STAT, "Skill Point {INT}SP", &pPlayer->GetItem(itSkillPoint).m_Count);
		AVM(ClientID, "null", NOPE, TAB_STAT, "Gold: {INT}", &pPlayer->GetItem(itGold).m_Count);
		AV(ClientID, "null");

		// personal menu
		AVH(ClientID, TAB_PERSONAL, GRAY_COLOR, "☪ SUB MENU PERSONAL");
		AVM(ClientID, "MENU", MenuList::MENU_INVENTORY, TAB_PERSONAL, "Inventory"); 
		AVM(ClientID, "MENU", MenuList::MENU_EQUIPMENT, TAB_PERSONAL, "Equipment");
		AVM(ClientID, "MENU", MenuList::MENU_UPGRADE, TAB_PERSONAL, "Upgrades({INT}p)", &pPlayer->Acc().m_Upgrade);
		AVM(ClientID, "MENU", MenuList::MENU_DUNGEONS, TAB_PERSONAL, "Dungeons");
		AVM(ClientID, "MENU", MenuList::MENU_SETTINGS, TAB_PERSONAL, "Settings");
		AVM(ClientID, "MENU", MenuList::MENU_INBOX, TAB_PERSONAL, "Mailbox");
		AVM(ClientID, "MENU", MenuList::MENU_JOURNAL_MAIN, TAB_PERSONAL, "Journal");
		if(Mmo()->House()->PlayerHouseID(pPlayer) > 0)
			AVM(ClientID, "MENU", MenuList::MENU_HOUSE, TAB_PERSONAL, "House");
		AVM(ClientID, "MENU", MenuList::MENU_GUILD_FINDER, TAB_PERSONAL, "Guild finder");
		if(pPlayer->Acc().IsGuild())
			AVM(ClientID, "MENU", MenuList::MENU_GUILD, TAB_PERSONAL, "Guild");
		AV(ClientID, "null");

		// info menu
		AVH(ClientID, TAB_INFORMATION, BLUE_COLOR, "√ SUB MENU INFORMATION");
		AVM(ClientID, "MENU", MenuList::MENU_GUIDEDROP, TAB_INFORMATION, "Loots, mobs on your zone");
		AVM(ClientID, "MENU", MenuList::MENU_TOP_LIST, TAB_INFORMATION, "Ranking guilds and players");
	}
	else if(MenuList == MenuList::MENU_JOURNAL_MAIN)
	{
		pPlayer->m_LastVoteMenu = MenuList::MAIN_MENU;
		
		Mmo()->Quest()->ShowQuestsMainList(pPlayer);
		AddVotesBackpage(ClientID);
	}
	else if(MenuList == MenuList::MENU_INBOX) 
	{
		pPlayer->m_LastVoteMenu = MenuList::MAIN_MENU;

		AddVotesBackpage(ClientID);
		AV(ClientID, "null");
		Mmo()->Inbox()->GetInformationInbox(pPlayer);
	}
	else if(MenuList == MenuList::MENU_UPGRADE) 
	{
		pPlayer->m_LastVoteMenu = MenuList::MAIN_MENU;

		AVH(ClientID, TAB_INFO_UPGR, GREEN_COLOR, "Upgrades Information");
		AVM(ClientID, "null", NOPE, TAB_INFO_UPGR, "Select upgrades type in Reason, write count.");
		AV(ClientID, "null");

		ShowVotesPlayerStats(pPlayer);

		// DPS UPGRADES
		int Range = pPlayer->GetLevelTypeAttribute(AtributType::AtDps);
		AVH(ClientID, TAB_UPGR_DPS, RED_COLOR, "Disciple of War. Level Power {INT}", &Range);
		for(const auto& at : ms_aAttributsInfo)
		{
			if(at.second.m_Type != AtributType::AtDps || str_comp_nocase(at.second.m_aFieldName, "unfield") == 0 || at.second.m_UpgradePrice <= 0) 
				continue;
			AVD(ClientID, "UPGRADE", at.first, at.second.m_UpgradePrice, TAB_UPGR_DPS, "{STR} {INT}P (Price {INT}P)", at.second.m_aName, &pPlayer->Acc().m_aStats[at.first], &at.second.m_UpgradePrice);
		}
		AV(ClientID, "null");

		// TANK UPGRADES
		Range = pPlayer->GetLevelTypeAttribute(AtributType::AtTank);
		AVH(ClientID, TAB_UPGR_TANK, BLUE_COLOR, "Disciple of Tank. Level Power {INT}", &Range);
		for(const auto& at : ms_aAttributsInfo)
		{
			if(at.second.m_Type != AtributType::AtTank || str_comp_nocase(at.second.m_aFieldName, "unfield") == 0 || at.second.m_UpgradePrice <= 0) 
				continue;
			AVD(ClientID, "UPGRADE", at.first, at.second.m_UpgradePrice, TAB_UPGR_TANK, "{STR} {INT}P (Price {INT}P)", at.second.m_aName, &pPlayer->Acc().m_aStats[at.first], &at.second.m_UpgradePrice);
		}
		AV(ClientID, "null");

		// HEALER UPGRADES
		Range = pPlayer->GetLevelTypeAttribute(AtributType::AtHealer);
		AVH(ClientID, TAB_UPGR_HEALER, GREEN_COLOR, "Disciple of Healer. Level Power {INT}", &Range);
		for(const auto& at : ms_aAttributsInfo)
		{
			if(at.second.m_Type != AtributType::AtHealer || str_comp_nocase(at.second.m_aFieldName, "unfield") == 0 || at.second.m_UpgradePrice <= 0) 
				continue;
			AVD(ClientID, "UPGRADE", at.first, at.second.m_UpgradePrice, TAB_UPGR_HEALER, "{STR} {INT}P (Price {INT}P)", at.second.m_aName, &pPlayer->Acc().m_aStats[at.first], &at.second.m_UpgradePrice);
		}
		AV(ClientID, "null");

		// WEAPONS UPGRADES
		AVH(ClientID, TAB_UPGR_WEAPON, GRAY_COLOR, "Upgrades Weapons / Ammo");
		for(const auto& at : ms_aAttributsInfo)
		{
			if(at.second.m_Type != AtributType::AtWeapon || str_comp_nocase(at.second.m_aFieldName, "unfield") == 0 || at.second.m_UpgradePrice <= 0) 
				continue;
			AVD(ClientID, "UPGRADE", at.first, at.second.m_UpgradePrice, TAB_UPGR_WEAPON, "{STR} {INT}P (Price {INT}P)", at.second.m_aName, &pPlayer->Acc().m_aStats[at.first], &at.second.m_UpgradePrice);
		}

		AV(ClientID, "null"), 
		AVH(ClientID, TAB_UPGR_JOB, GOLDEN_COLOR, "Disciple of Jobs");
		Mmo()->PlantsAcc()->ShowMenu(ClientID);
		Mmo()->MinerAcc()->ShowMenu(pPlayer);
		AddVotesBackpage(ClientID);
	}
	else if (MenuList == MenuList::MENU_TOP_LIST)
	{
		pPlayer->m_LastVoteMenu = MenuList::MAIN_MENU;
		AVH(ClientID, TAB_INFO_TOP, GREEN_COLOR, "Ranking Information");
		AVM(ClientID, "null", NOPE, TAB_INFO_TOP, "Here you can see top server Guilds, Players.");
		AV(ClientID, "null");

		m_apPlayers[ClientID]->m_Colored = { 20,7,15 };
		AVM(ClientID, "SELECTEDTOP", ToplistTypes::GUILDS_LEVELING, NOPE, "Top 10 guilds leveling");
		AVM(ClientID, "SELECTEDTOP", ToplistTypes::GUILDS_WEALTHY, NOPE, "Top 10 guilds wealthy");
		AVM(ClientID, "SELECTEDTOP", ToplistTypes::PLAYERS_LEVELING, NOPE, "Top 10 players leveling");
		AVM(ClientID, "SELECTEDTOP", ToplistTypes::PLAYERS_WEALTHY, NOPE, "Top 10 players wealthy");
		AddVotesBackpage(ClientID);
	}
	else if(MenuList == MenuList::MENU_GUIDEDROP) 
	{
		pPlayer->m_LastVoteMenu = MenuList::MAIN_MENU;
		AVH(ClientID, TAB_INFO_LOOT, GREEN_COLOR, "Chance & Loot Information");
		AVM(ClientID, "null", NOPE, TAB_INFO_LOOT, "Here you can see chance loot, mobs, on YOUR ZONE.");
		AV(ClientID, "null");

		char aBuf[128];
		bool FoundedBots = false;
		const float AddedChanceDrop = clamp((float)pPlayer->GetAttributeCount(Stats::StLuckyDropItem, true) / 100.0f, 0.01f, 10.0f);
		for(const auto& mobs : BotJob::ms_aMobBot)
		{
			if (!IsPlayerEqualWorldID(ClientID, mobs.second.m_WorldID))
				continue;

			const int HideID = (NUM_TAB_MENU+12500+mobs.first);
			const int PosX = mobs.second.m_PositionX/32, PosY = mobs.second.m_PositionY/32;
			AVH(ClientID, HideID, LIGHT_BLUE_COLOR, "{STR} [x{INT} y{INT}]", mobs.second.GetName(), &PosX, &PosY);
	
			for(int i = 0; i < MAX_DROPPED_FROM_MOBS; i++)
			{
				if(mobs.second.m_aDropItem[i] <= 0 || mobs.second.m_aCountItem[i] <= 0)
					continue;
			
				const float Chance = mobs.second.m_aRandomItem[i];
				ItemInformation &InfoDropItem = GetItemInfo(mobs.second.m_aDropItem[i]);
				str_format(aBuf, sizeof(aBuf), "%sx%d - chance to loot %0.2f%%(+%0.2f%%)", InfoDropItem.GetName(pPlayer), mobs.second.m_aCountItem[i], Chance, AddedChanceDrop);
				AVMI(ClientID, InfoDropItem.GetIcon(), "null", NOPE, HideID, "{STR}", aBuf);
				FoundedBots = true;
			}
		}

		if (!FoundedBots)
			AVL(ClientID, "null", "There are no active mobs in your zone!");

		AddVotesBackpage(ClientID);
	}
		
	Mmo()->OnPlayerHandleMainMenu(ClientID, MenuList, false);
}

// information for unauthorized players
void CGS::ShowVotesNewbieInformation(int ClientID)
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
	AV(ClientID, "null");
	AVL(ClientID, "null", "#### Some Informations ####");
	AVL(ClientID, "null", "Brown Slimes drop Glue and Gel (lower chance to drop Gel)");
	AVL(ClientID, "null", "Green Slimes drop Glue");
	AVL(ClientID, "null", "Blue Slimes drop Gel");
	AVL(ClientID, "null", "Pink Slime drop 3 Gels and 3 Glue");
	AVL(ClientID, "null", "Goblins drop Goblin Ingots");
	AVL(ClientID, "null", "Orc Warrior drop Goblin Ingot and Torn Cloth Clothes of Orc");
	AVL(ClientID, "null", "- A more accurate drop can be found in the menu");
	AV(ClientID, "null");
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

// strong update votes variability of the data
void CGS::StrongUpdateVotes(int ClientID, int MenuList)
{
	if(m_apPlayers[ClientID] && m_apPlayers[ClientID]->m_OpenVoteMenu == MenuList)
		ResetVotes(ClientID, MenuList);
}

// strong update votes variability of the data
void CGS::StrongUpdateVotesForAll(int MenuList)
{
	for(int i = 0; i < MAX_PLAYERS; i++)
	{
		if(m_apPlayers[i] && m_apPlayers[i]->m_OpenVoteMenu == MenuList)
			ResetVotes(i, MenuList);
	}
}

// the back button adds a back button to the menu (But remember to specify the last menu ID).
void CGS::AddVotesBackpage(int ClientID)
{	
	if(!m_apPlayers[ClientID]) 
		return;

	AV(ClientID, "null");
	m_apPlayers[ClientID]->m_Colored = RED_COLOR;
	AVL(ClientID, "BACK", "Backpage");
	m_apPlayers[ClientID]->m_Colored = {0,0,0};
}

// print player statistics
void CGS::ShowVotesPlayerStats(CPlayer *pPlayer)
{
	const int ClientID = pPlayer->GetCID();
	AVH(ClientID, TAB_INFO_STAT, BLUE_COLOR, "Player Stats {STR}", IsDungeon() ? "(Sync)" : "\0");
	for(const auto& pAtt : ms_aAttributsInfo)
	{
		if(str_comp_nocase(pAtt.second.m_aFieldName, "unfield") == 0)
			continue;
	
		// if upgrades are cheap, they have a division of statistics
		const int AttributeSize = pPlayer->GetAttributeCount(pAtt.first);
		if(pAtt.second.m_Devide <= 1)
		{
			const int AttributeRealSize = pPlayer->GetAttributeCount(pAtt.first, true);
			AVM(ClientID, "null", NOPE, TAB_INFO_STAT, "{INT} (+{INT}) - {STR}", &AttributeSize, &AttributeRealSize, pAtt.second.m_aName);
			continue;
		}

		AVM(ClientID, "null", NOPE, TAB_INFO_STAT, "+{INT} - {STR}", &AttributeSize, pAtt.second.m_aName);
	}

	AVM(ClientID, "null", NOPE, NOPE, "Player Upgrade Point: {INT}P", &pPlayer->Acc().m_Upgrade);
	AV(ClientID, "null");
}

// display information by currency
void CGS::ShowVotesItemValueInformation(CPlayer *pPlayer, int ItemID)
{
	const int ClientID = pPlayer->GetCID();
	pPlayer->m_Colored = LIGHT_PURPLE_COLOR;
	AVMI(ClientID, GetItemInfo(ItemID).GetIcon(), "null", NOPE, NOPE, "You have {INT} {STR}", &pPlayer->GetItem(ItemID).m_Count, GetItemInfo(ItemID).GetName());
}

// vote parsing of all functions of action methods
bool CGS::ParsingVoteCommands(int ClientID, const char *CMD, const int VoteID, const int VoteID2, int Get, const char *Text, VoteCallBack Callback)
{
	CPlayer *pPlayer = GetPlayer(ClientID, false, true);
	if(!pPlayer)
	{
		Chat(ClientID, "Use it when you're not dead!");
		return true;
	}

	const sqlstr::CSqlString<64> FormatText = sqlstr::CSqlString<64>(Text);
	if(Callback)
	{
		CVoteOptionsCallback InstanceCallback;
		InstanceCallback.pPlayer = pPlayer;
		InstanceCallback.Get = Get;
		InstanceCallback.VoteID = VoteID;
		InstanceCallback.VoteID2 = VoteID2;
		str_copy(InstanceCallback.Text, FormatText.cstr(), sizeof(InstanceCallback.Text));
		str_copy(InstanceCallback.Command, CMD, sizeof(InstanceCallback.Command));

		// TODO: fixme. improve the system using the ID method, as well as the ability to implement Backpage
		if(PPSTR(CMD, "MENU") == 0)
		{
			pPlayer->m_ActiveMenuOptionCallback = InstanceCallback;
			pPlayer->m_ActiveMenuRegisteredCallback = Callback;
		}
		Callback(InstanceCallback);
		return true;
	}


	if(PPSTR(CMD, "null") == 0) 
		return true;
	else if(PPSTR(CMD, "BACK") == 0)
	{
		CreatePlayerSound(ClientID, SOUND_BOOK_FLIP);
		ResetVotes(ClientID, pPlayer->m_LastVoteMenu);
		return true;
	}
	else if(PPSTR(CMD, "MENU") == 0)
	{
		CreatePlayerSound(ClientID, SOUND_BOOK_FLIP);
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

	// parsing everything else
	return (bool)(Mmo()->OnParsingVoteCommands(pPlayer, CMD, VoteID, VoteID2, Get, FormatText.cstr()));
}

/* #########################################################################
	MMO GAMECONTEXT 
######################################################################### */
int CGS::CreateBot(short BotType, int BotID, int SubID)
{
	int BotClientID = MAX_PLAYERS;
	while(m_apPlayers[BotClientID])
	{
		BotClientID++;
		if(BotClientID >= MAX_CLIENTS)
			return -1;
	}

	Server()->InitClientBot(BotClientID);
	const int AllocMemoryCell = BotClientID+m_WorldID*MAX_CLIENTS;
	m_apPlayers[BotClientID] = new(AllocMemoryCell) CPlayerBot(this, BotClientID, BotID, SubID, BotType);
	return BotClientID;
}

// create lol text in the world
void CGS::CreateText(CEntity* pParent, bool Follow, vec2 Pos, vec2 Vel, int Lifespan, const char* pText)
{
	if(!CheckingPlayersDistance(Pos, 800))
		return;

	CLoltext Text;
	Text.Create(&m_World, pParent, Pos, Vel, Lifespan, pText, true, Follow);
}

// creates a particle of experience that follows the player
void CGS::CreateParticleExperience(vec2 Pos, int ClientID, int Experience, vec2 Force)
{
	new CFlyingExperience(&m_World, Pos, ClientID, Experience, Force);
}

// gives a bonus in the position type and quantity and the number of them.
void CGS::CreateDropBonuses(vec2 Pos, int Type, int Count, int NumDrop, vec2 Force)
{
	for(int i = 0; i < NumDrop; i++) 
	{
		vec2 Vel = Force + vec2(frandom() * 15.0, frandom() * 15.0);
		const float Angle = Force.x * (0.15f + frandom() * 0.1f);
		new CDropBonuses(&m_World, Pos, Vel, Angle, Type, Count);
	}
}

// lands items in the position type and quantity and their number themselves
void CGS::CreateDropItem(vec2 Pos, int ClientID, InventoryItem DropItem, vec2 Force)
{
	if(DropItem.m_ItemID <= 0 || DropItem.m_Count <= 0)
		return;

	const float Angle = GetAngle(normalize(Force));
	new CDropItem(&m_World, Pos, Force, Angle, DropItem, ClientID);
}

// random drop of the item with percentage
void CGS::CreateRandomDropItem(vec2 Pos, int ClientID, float Random, InventoryItem DropItem, vec2 Force)
{
	const float RandomDrop = frandom() * 100.0f;
	if(RandomDrop < Random)
		CreateDropItem(Pos, ClientID, DropItem, Force);
}

bool CGS::TakeItemCharacter(int ClientID)
{
	CPlayer *pPlayer = GetPlayer(ClientID, true, true);
	if(!pPlayer)
		return false;

	CDropItem *pDrop = (CDropItem*)m_World.ClosestEntity(pPlayer->GetCharacter()->m_Core.m_Pos, 64, CGameWorld::ENTTYPE_DROPITEM, nullptr);
	if(pDrop) { return pDrop->TakeItem(ClientID);}
	return false;
}

// send a message with or without the object using ClientID
void CGS::SendInbox(CPlayer* pPlayer, const char* Name, const char* Desc, int ItemID, int Count, int Enchant)
{
	if(!pPlayer || !pPlayer->IsAuthed())
		return;

	SendInbox(pPlayer->Acc().m_AuthID, Name, Desc, ItemID, Count, Enchant);
} 

// send a message with or without the object using AuthID
void CGS::SendInbox(int AuthID, const char* Name, const char* Desc, int ItemID, int Count, int Enchant)
{
	Mmo()->Inbox()->SendInbox(AuthID, Name, Desc, ItemID, Count, Enchant);
} 

// send day information
void CGS::SendDayInfo(int ClientID)
{
	if(ClientID == -1)
		Chat(-1, "{STR} came! Good {STR}!", Server()->GetStringTypeDay(), Server()->GetStringTypeDay());
	if(m_DayEnumType == DayType::NIGHT_TYPE)
		Chat(ClientID, "Nighttime experience was increase to {INT}%", &m_MultiplierExp);
	else if(m_DayEnumType == DayType::MORNING_TYPE)
		Chat(ClientID, "Daytime experience was downgraded to 100%");
}

void CGS::ChangeEquipSkin(int ClientID, int ItemID)
{
	CPlayer *pPlayer = GetPlayer(ClientID, true);
	if(!pPlayer || GetItemInfo(ItemID).m_Type != ItemType::TYPE_EQUIP || GetItemInfo(ItemID).m_Function == EQUIP_DISCORD || GetItemInfo(ItemID).m_Function == EQUIP_MINER)
		return;

	SendEquipments(ClientID, -1);
}

int CGS::GetExperienceMultiplier(int Experience) const
{
	if(IsDungeon())
		return (int)kurosio::translate_to_procent_rest(Experience, g_Config.m_SvMultiplierExpRaidDungeon);
	return (int)kurosio::translate_to_procent_rest(Experience, m_MultiplierExp);
}

void CGS::UpdateZonePVP()
{
	m_AllowedPVP = false;
	if(IsDungeon())
		return;
	
	for(int i = MAX_PLAYERS; i < MAX_CLIENTS; i++)
	{
		if(m_apPlayers[i] && m_apPlayers[i]->GetBotType() == BotsTypes::TYPE_BOT_MOB && m_apPlayers[i]->GetPlayerWorldID() == m_WorldID)
		{
			m_AllowedPVP = true;
			return;
		}
	}
}

void CGS::UpdateZoneDungeon()
{
	m_DungeonID = 0;
	for(const auto& dd : DungeonJob::ms_aDungeon)
	{
		if(m_WorldID != dd.second.m_WorldID)
			continue;
		
		m_DungeonID = dd.first;
		return;
	}
}

bool CGS::IsPlayerEqualWorldID(int ClientID, int WorldID) const
{
	if(ClientID < 0 || ClientID >= MAX_CLIENTS || !m_apPlayers[ClientID])
		return false;

	if (WorldID <= -1)
		return m_apPlayers[ClientID]->GetPlayerWorldID() == m_WorldID;
	return m_apPlayers[ClientID]->GetPlayerWorldID() == WorldID;
}

bool CGS::CheckingPlayersDistance(vec2 Pos, float Distance) const
{
	for(int i = 0; i < MAX_PLAYERS; i++)
	{
		if(m_apPlayers[i] && IsPlayerEqualWorldID(i) && distance(Pos, m_apPlayers[i]->m_ViewPos) <= Distance)
			return true;
	}
	return false;
}

IGameServer *CreateGameServer() { return new CGS; }
