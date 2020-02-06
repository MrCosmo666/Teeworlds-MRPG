/* (c) Alexandre Díaz. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#include <new>
#include <engine/shared/config.h>
#include <game/server/gamecontext.h>
#include <game/mapitems.h>
#include "botaijob.h"

MACRO_ALLOC_POOL_ID_IMPL(BotAI, MAX_CLIENTS*COUNT_WORLD+MAX_CLIENTS)

BotAI::BotAI(CGameWorld *pWorld) : CCharacter(pWorld) {}

BotAI::~BotAI() 
{
	RemoveSnapProj(100, GetSnapFullID());
	RemoveSnapProj(100, GetSnapFullID(), true);
}

// Получить оформление для SnapFull
int BotAI::GetSnapFullID() const 
{
	return GetPlayer()->GetCID() * SNAPBOTS;
}

// Найти самаго толстого танка
void BotAI::FindHardHealth()
{
	for (auto listDmg = m_ListDmgPlayers.begin(); listDmg != m_ListDmgPlayers.end(); )
	{
		int PlayerID = listDmg->first;

		// проверяем на дистанцию игрока
		CPlayer *pFinderHard = GS()->GetPlayer(PlayerID, true, true);
		if(!pFinderHard || distance(pFinderHard->GetCharacter()->m_Core.m_Pos, m_Core.m_Pos) > 800.0f)
		{
			m_BotTargetID = GetPlayer()->GetCID();
			listDmg = m_ListDmgPlayers.erase(listDmg);
			continue;	
		}

		// проверяем есть ли вкуснее игрокв для бота
		CPlayer *pFrom = GS()->GetPlayer(m_BotTargetID, true, true);
		if(
			// если таргет бота равен самому боту
			m_BotTargetID == GetPlayer()->GetCID() || 
			
			// если игрока на котором таргет
			!pFrom || 

			// - проверяем нет ли стены между Таргетом
			GS()->Collision()->FastIntersectLine(pFrom->GetCharacter()->m_Core.m_Pos, m_Core.m_Pos, 0, 0) ||	
			
			( // - проверяем если у игрока Tenacity выше Таргета
				!GS()->Collision()->FastIntersectLine(pFinderHard->GetCharacter()->m_Core.m_Pos, m_Core.m_Pos, 0, 0) && 
				pFinderHard->GetAttributeCount(Stats::StTenacity, true) > pFrom->GetAttributeCount(Stats::StTenacity, true)
			))
		{
			m_BotTargetID = PlayerID;
		}
		listDmg++;
	}
}

bool BotAI::Spawn(class CPlayer *pPlayer, vec2 Pos)
{
	if(!CCharacter::Spawn(pPlayer, Pos))
		return false;
	
	// Запрещаем дамаг и хоок для человека
	if(GetPlayer()->GetSpawnBot() == SPAWNNPC)
	{
		m_NoAllowDamage = true;
		m_Core.m_ProtectHooked = true;
	}

	// Спавн ботов основной
	m_StartHealth = Health();
	m_BotTargetID = GetPlayer()->GetCID();

	// информация о зарождении жирного моба
	int BotID = GetPlayer()->GetBotID();
	int SubBotID = GetPlayer()->GetBotSub();
	if(GetPlayer()->GetSpawnBot() == SPAWNMOBS && ContextBots::MobBot[SubBotID].Boss)
	{
		for(int i = 0; i < 3; i++)
		{
			CreateSnapProj(GetSnapFullID(), 1, PICKUP_HEALTH, true, false);
			CreateSnapProj(GetSnapFullID(), 1, WEAPON_HAMMER, false, true);
		}

		for(int i = 0 ; i < MAX_PLAYERS; i++)
		{
			if(Server()->GetWorldID(i) != ContextBots::MobBot[SubBotID].WorldID)
				continue;

			GS()->Chat(i, "In your area, spawned, {STR}, HP {INT}", ContextBots::MobBot[SubBotID].Name, &m_StartHealth);
		}
	}
	else if(GetPlayer()->GetSpawnBot() == SPAWNQUESTNPC)
	{
		m_Core.m_LostData = true;

		CreateSnapProj(GetSnapFullID(), 3, PICKUP_HEALTH, true, false);
		CreateSnapProj(GetSnapFullID(), 3, PICKUP_ARMOR, true, false);
	}
	return true;
}

void BotAI::Tick()
{
	if(!CheckInvisibleBot() || !IsAlive())
		return;

	// Переброс на игрока если потерян случайно
	if(Server()->Tick() % (Server()->TickSpeed()/3) == 0)
		FindHardHealth(); 

	// Крюк таймер
	if(m_HookTick)
	{
		m_HookTick--;
		m_Input.m_Hook = true;
		if(!m_HookTick) 
			m_Input.m_Hook = false;
	}
	EngineBots();
	CCharacter::Tick();
}

void BotAI::ShowProgress()
{
	for(auto iparse = m_ListDmgPlayers.begin(); iparse != m_ListDmgPlayers.end(); iparse++)
	{
		CPlayer *pPlayer = GS()->GetPlayer(iparse->first, true);
		if(pPlayer)
		{
			dynamic_string Buffer;
			int SubBotID = GetPlayer()->GetBotSub();
			int Dexterity = ContextBots::MobBot[SubBotID].Health * 7;
			Server()->Localization()->Format(Buffer, pPlayer->GetLanguage(), _("Hard Mob: {s:name} Target: {s:tname}\n"),
				"name", ContextBots::MobBot[SubBotID].Name, "tname", Server()->ClientName(m_BotTargetID), NULL);
			pPlayer->AddInBroadcast(Buffer.buffer()), Buffer.clear();

			int Health = GetPlayer()->GetHealth();
			int ActivePlayers = m_ListDmgPlayers.size();
			float gethp = ( Health * 100.0 ) / m_StartHealth;
			const char *Level = GS()->LevelString(100, (int)gethp, 10, ':', ' ');
			Server()->Localization()->Format(Buffer, pPlayer->GetLanguage(), _("HP {s:proc}({i:hp}/{i:shp}) {i:pl} players\n"),
				"proc", Level, "hp", &Health, "shp", &m_StartHealth, "pl", &ActivePlayers, NULL);
			pPlayer->AddInBroadcast(Buffer.buffer()), Buffer.clear();
			delete Level;
		}
	}	
}

bool BotAI::TakeDamage(vec2 Force, vec2 Source, int Dmg, int From, int Weapon)
{
	if(GetPlayer()->GetSpawnBot() != SPAWNMOBS)
		return false;

	CPlayer *pFrom = GS()->m_apPlayers[From];
	if(pFrom && pFrom->IsBot())
		return false;

	int StableDamage = Health(); // FIX DAMAGE COUNT
	bool Damage = CCharacter::TakeDamage(Force, Source, Dmg, From, Weapon);
	StableDamage -= Health();

	if(From != GetPlayer()->GetCID() && pFrom)
		m_ListDmgPlayers[From] += StableDamage;

	if (Damage)
		return true;
	
	// проверка при смерте
	if(Health() <= 0)
	{
		const int BotID = GetPlayer()->GetBotID();
		const int SubID = GetPlayer()->GetBotSub();
		if(!ContextBots::MobBot[SubID].Boss && GetPlayer()->GetSpawnBot() == SPAWNMOBS && pFrom)
			GS()->Mmo()->Quest()->MobProgress(pFrom, BotID);
	
		ClearTarget();
		Die(From, Weapon);
	}

	return false;
}

void BotAI::Die(int Killer, int Weapon)
{
	// просто убить бота если он не моб
	if(GetPlayer()->GetSpawnBot() != SPAWNMOBS)
		return;

	// Ищим игроков кто ранил человека и даем бонус
	int BotID = GetPlayer()->GetBotID();
	int SubID = GetPlayer()->GetBotSub();
	for(const auto& ld : m_ListDmgPlayers)
	{
		int PlayerID = ld.first;
		CPlayer *pPlayer = GS()->GetPlayer(PlayerID, true, true);
		if(!pPlayer || PlayerID == GetPlayer()->GetCID()) continue;

		for(int i = 0; i < 6; i++)
		{
			int DropItem = ContextBots::MobBot[SubID].DropItem[i];
			int CountItem = ContextBots::MobBot[SubID].CountItem[i];
			if(DropItem <= 0 || CountItem <= 0) 
				continue;

			int RandomDrop = ContextBots::MobBot[SubID].RandomItem[i];
			if(DropItem == itMoney)
			{
				if(RandomDrop <= 0 || rand()%RandomDrop == 0)
					pPlayer->AddMoney(CountItem);
				continue;
			}
			CreateRandomDrop(PlayerID, RandomDrop, DropItem, CountItem);
		}

		if(ContextBots::MobBot[SubID].Boss && GetPlayer()->GetSpawnBot() == SPAWNMOBS)
			GS()->Mmo()->Quest()->MobProgress(pPlayer, BotID);

		// exp
		int DamageExp = (ContextBots::MobBot[SubID].Level*g_Config.m_SvExperienceMob);
		int PowerRaid = GS()->IncreaseCountRaid(DamageExp);
		pPlayer->AddExp(PowerRaid);
		GS()->VResetVotes(PlayerID, MAINMENU);	

		// дать скилл поинт
		if(random_int()%80 == 0)
		{
			pPlayer->GetItem(itSkillPoint).Add(1);
			GS()->Chat(PlayerID, "Skill points increased. Now ~{INT}SP~", &pPlayer->GetItem(itSkillPoint).Count);
		}
	}

	// склад пополнение
	int StorageID = GS()->Mmo()->Storage()->GetStorageMonsterSub(BotID);
	if(StorageID > 0) GS()->Mmo()->Storage()->AddStorage(StorageID, rand()%5);

	// очищаем лист и убиваем игрока
	m_ListDmgPlayers.clear();
	CCharacter::Die(Killer, Weapon);
}

void BotAI::ClearTarget()
{
	int FromID = GetPlayer()->GetCID();
	if(m_BotTargetID != FromID)
		m_BotTargetID = FromID;
}

void BotAI::ChangeWeapons()
{
	int randtime = 1+random_int()%3;
	if(Server()->Tick() % (Server()->TickSpeed()*randtime) == 0)
	{
		int randomweapon = random_int()%5;	
		m_ActiveWeapon = clamp(randomweapon, 0, 4);
	}
	if(!m_HookTick) 
	{
		m_HookTick = rand()%40;
	}
}

// Интерактивы ботов
void BotAI::EngineBots()
{
	m_Input.m_Fire = 0;
	m_Input.m_Jump = 0;

	// рандом для дружественного моба
	int BotID = GetPlayer()->GetBotID();
	if(GetPlayer()->GetSpawnBot() == SPAWNNPC)
		EngineNPC();
	else if(GetPlayer()->GetSpawnBot() == SPAWNMOBS)
		EngineMobs();
	else if(GetPlayer()->GetSpawnBot() == SPAWNQUESTNPC)
		EngineQuestMob();
		
	// избежать стены
    int tx = m_Pos.x+m_Input.m_Direction*45.0f;
    if (tx < 0)  m_Input.m_Direction = 1;
    else if (tx >= GS()->Collision()->GetWidth()*32.0f) m_Input.m_Direction = -1; 

	if (m_LatestPrevInput.m_Fire && m_Input.m_Fire) m_Input.m_Fire = 0;
	if (m_LatestInput.m_Jump && m_Input.m_Jump) m_Input.m_Jump = 0;
	
	// двойной прыжок
	if (m_Input.m_Jump && (m_Jumped&1) && !(m_Jumped&2) && m_Core.m_Vel.y < GS()->Tuning()->m_Gravity)
		m_Input.m_Jump = 0;

	m_LatestPrevInput = m_LatestInput;
	m_LatestInput = m_Input;
}

// Интерактивы NPC
void BotAI::EngineNPC()
{
	int ClientID = GetPlayer()->GetCID();
	int SubBotID = GetPlayer()->GetBotSub();
	bool StaticBot = ContextBots::NpcBot[SubBotID].Static;

	// ------------------------------------------------------------------------------
	// интерактивы бота без найденого игрока
	// ------------------------------------------------------------------------------
	// эмоции ботов
	int EmoteBot = ContextBots::NpcBot[SubBotID].Emote;
	if(EmoteBot && (Server()->Tick() % (Server()->TickSpeed()+rand()%50 * 8)) == 0)
	{
		SetEmote(EmoteBot, 10);
		if(EmoteBot == EMOTE_HAPPY) EmoteBot = rand()%2 == 0 ? EMOTICON_EYES : EMOTICON_HEARTS;
		else if(EmoteBot == EMOTE_BLINK) EmoteBot = EMOTICON_DOTDOT;
		else if(EmoteBot == EMOTE_ANGRY) EmoteBot = EMOTICON_SUSHI+rand()%4;
		else if(EmoteBot == EMOTE_PAIN) EmoteBot = EMOTICON_DROP;
		else if(EmoteBot == EMOTE_SURPRISE) EmoteBot = EMOTICON_EYES;

		GS()->SendEmoticon(GetPlayer()->GetCID(), EmoteBot);
	}

	// направление глаз
	if(Server()->Tick() % Server()->TickSpeed() == 0)
		m_Input.m_TargetY = rand()%4-rand()%8;
	m_Input.m_TargetX = (m_Input.m_Direction*10+1);

	// ------------------------------------------------------------------------------
	// шалаболить ботам с игроками (потом заменить уебищно)
	// ------------------------------------------------------------------------------	
	TalkingBots(ContextBots::NpcBot[SubBotID].TalkText);

	// ------------------------------------------------------------------------------
	// интерактивы бота с найденым игроком
	// ------------------------------------------------------------------------------	
	CPlayer *pFind = SearchPlayer((StaticBot ? 300 : 100));
	if(!pFind || !pFind->GetCharacter())
	{
		// рандомно ставим направление движения
		if(!StaticBot && Server()->Tick() % (Server()->TickSpeed() * (m_Input.m_Direction == 0 ? 5 : 1)) == 0)
			m_Input.m_Direction = -1+rand()%3;
		return;
	}

	pFind->SetTalking(GetPlayer()->GetCID(), false);

	int FindCID = pFind->GetCID();
	m_Input.m_TargetX = static_cast<int>(pFind->GetCharacter()->m_Core.m_Pos.x - m_Pos.x);
	m_Input.m_TargetY = static_cast<int>(pFind->GetCharacter()->m_Core.m_Pos.y - m_Pos.y);
	m_Input.m_Direction = 0;
}

// Интерактивы квестовых мобов
void BotAI::EngineQuestMob()
{
	// направление глаз
	int SubBotID  = GetPlayer()->GetBotSub();
	if(Server()->Tick() % Server()->TickSpeed() == 0)
		m_Input.m_TargetY = rand()%4-rand()%8;
	m_Input.m_TargetX = (m_Input.m_Direction*10+1);

	// эмоции ботов
	int EmoteBot = ContextBots::QuestBot[SubBotID].Emote;
	if(EmoteBot && (Server()->Tick() % (Server()->TickSpeed()+rand()%50 * 8)) == 0)
	{
		SetEmote(EmoteBot, 10);
		GS()->SendEmoticon(GetPlayer()->GetCID(), GetEmoticon(EmoteBot));
	}

	// ------------------------------------------------------------------------------
	// интерактивы бота с найденым игроком
	// ------------------------------------------------------------------------------	
	for(int i = 0; i < MAX_PLAYERS; i++)
	{
		if(!GS()->m_apPlayers[i] || !GS()->m_apPlayers[i]->GetCharacter() ||
			!GetPlayer()->CheckQuestSnapPlayer(i, false) ||
			distance(GS()->m_apPlayers[i]->GetCharacter()->m_Core.m_Pos, m_Core.m_Pos) > 180)
			continue;

		CPlayer *pFind = GS()->m_apPlayers[i]; 
		m_Input.m_TargetX = round_to_int(pFind->GetCharacter()->m_Core.m_Pos.x - m_Pos.x);
		m_Input.m_TargetY = round_to_int(pFind->GetCharacter()->m_Core.m_Pos.y - m_Pos.y);
		m_Input.m_Direction = 0;

		// ------------------------------------------------------------------------------
		// квесты с нпс
		// ------------------------------------------------------------------------------
		if(distance(m_Core.m_Pos, pFind->GetCharacter()->m_Core.m_Pos) > 120)
		{
			pFind->ClearFormatQuestText();
			CGS::InteractiveSub[i].QBI = ContextBots::QuestBot[-1];
			return;
		} 
		else 
			CGS::InteractiveSub[i].QBI = ContextBots::QuestBot[SubBotID];

		if(Server()->Tick() % Server()->TickSpeed()/20 == 0)
			GS()->Mmo()->Quest()->ShowQuestInformation(pFind, ContextBots::QuestBot[SubBotID], false);
	}
}

// Интерактивы мобов враждебных
void BotAI::EngineMobs()
{
	// ------------------------------------------------------------------------------
	// интерактивы бота без найденого игрока
	// ------------------------------------------------------------------------------
	if(Server()->Tick() % Server()->TickSpeed() == 0)
		m_Input.m_TargetY = rand()%2-rand()%4 + m_Core.m_Vel.y;
	m_Input.m_TargetX = (m_Input.m_Direction*10+1) + m_Core.m_Vel.x;

	// эмоции ботов
	int SubBotID = GetPlayer()->GetBotSub();
	int EmoteBot = ContextBots::MobBot[SubBotID].Emote;
	if(EmoteBot && (Server()->Tick() % (Server()->TickSpeed()+rand()%50 * 8)) == 0)
	{
		SetEmote(EmoteBot, 10);
		GS()->SendEmoticon(GetPlayer()->GetCID(), GetEmoticon(EmoteBot));
	}

	// Зелье мобам восстановление Health без агра
	if(m_BotTargetID == GetPlayer()->GetCID() && Health() < m_StartHealth && !GetPlayer()->CheckEffect("RegenHealth")) 
	{
		SetEmote(EMOTE_HAPPY, 3);
		GetPlayer()->GiveEffect("RegenHealth", 5);
	}

	// крюк
	if(!m_HookTick && rand()%120 == 0)
	{
		vec2 InputHook = vec2(0.0f, 0.0f);
		vec2 HookDir(0.0f,0.0f);
		for(int i = 0 ; i < 32; i++)
		{
			float a = 2*i*pi / 32;
			vec2 dir = direction(a);
			vec2 Pos = m_Core.m_Pos+dir*GS()->Tuning()->m_HookLength;
			if((GS()->Collision()->FastIntersectLine(m_Core.m_Pos,Pos,&Pos,0)))
			{
				vec2 HookVel = dir*GS()->Tuning()->m_HookDragAccel;
				if(HookVel.y > 0) HookVel.y *= 0.3f;
				if((HookVel.x < 0 && m_Input.m_Direction < 0) || (HookVel.x > 0 && m_Input.m_Direction > 0))
					HookVel.x *= 0.95f;
				else HookVel.x *= 0.75f;

				HookVel += vec2(0,1)*GS()->Tuning()->m_Gravity;
				HookDir = Pos - m_Core.m_Pos;
			}
		}
		if(length(HookDir) > 32.f && length(HookDir) < 400.0f)
		{
			m_Input.m_TargetX = HookDir.x;
			m_Input.m_TargetY = HookDir.y;
			m_Input.m_Hook = true;
			m_HookTick = 20+rand()%80;
		}
	}

	CPlayer *pPlayer = SearchTenacityPlayer(1000.0f);
	if(!pPlayer || !pPlayer->GetCharacter())
	{
        if (Server()->Tick()-m_BotTick > Server()->TickSpeed()*rand()%150+150)
        {
			vec2 DirPlayer = normalize(vec2(m_Input.m_TargetX, m_Input.m_TargetY)*10 - m_Pos);
            int Action = rand()%3;
            if (Action == 0)
			{
				m_Input.m_Jump = rand()%2;
                m_Input.m_Direction = -1;
			}
            else if (Action == 1)
			{
				m_Input.m_Jump = rand()%2;
                m_Input.m_Direction = 1;
			}
			else if (Action == 2)
                m_Input.m_Direction = 0;

            m_BotTick = Server()->Tick();
        }
		return;
	}

	// ------------------------------------------------------------------------------
	// интерактивы бота с найденым игроком
	// ------------------------------------------------------------------------------
	int Dist = distance(m_Pos, pPlayer->GetCharacter()->m_Core.m_Pos);
	if(m_Pos.y > pPlayer->GetCharacter()->m_Core.m_Pos.y+rand()%400)
		m_Input.m_Jump = 1;

	// интерактив с игроком
	if (Dist < 600.0f)
	{
		bool StaticBot = (ContextBots::MobBot[SubBotID].Spread >= 1);
		if(rand() % 7 == 1)
		{
			if(rand()%30 == 0) m_HookTick = rand()%80;
			m_Input.m_TargetX = static_cast<int>(pPlayer->GetCharacter()->m_Core.m_Pos.x - m_Pos.x);
			m_Input.m_TargetY = static_cast<int>(pPlayer->GetCharacter()->m_Core.m_Pos.y - m_Pos.y);
			if(Dist > (StaticBot ? 250 : 70))
			{
				vec2 DirPlayer = normalize(pPlayer->GetCharacter()->m_Core.m_Pos - m_Pos);
				if (DirPlayer.x < 0) m_Input.m_Direction = -1;
				else m_Input.m_Direction = 1;
			}
			else m_LatestInput.m_Fire = m_Input.m_Fire = 1;
		
			if(!m_HookTick && rand()%60 == 0)
			{
				m_HookTick = 50+rand()%150;
			}	
		}

		// сменять оружие если статик
		if(StaticBot)
		{
			if(ContextBots::MobBot[SubBotID].Boss)
				ShowProgress();
		
			ChangeWeapons();
		}
	}
}

// Поиск игрока среди людей
CPlayer *BotAI::SearchPlayer(int Distance)
{
	for(int i = 0 ; i < MAX_PLAYERS; i ++)
	{
		if(!GS()->m_apPlayers[i] 
			|| !GS()->m_apPlayers[i]->GetCharacter() 
			|| distance(m_Core.m_Pos, GS()->m_apPlayers[i]->GetCharacter()->m_Core.m_Pos) > Distance
			|| GS()->Collision()->FastIntersectLine(GS()->m_apPlayers[i]->GetCharacter()->m_Core.m_Pos, m_Pos, 0, 0))
			continue;
		return GS()->m_apPlayers[i];
	}
	return NULL;
}

// Поиск игрока среди людей который имеет ярость выше всех
CPlayer *BotAI::SearchTenacityPlayer(float Distance)
{
	// ночью боты враждебнее ищем сразу игрока для агра если его нету или есть бот злой
	if(m_BotTargetID == GetPlayer()->GetCID() && 
		(ContextBots::MobBot[GetPlayer()->GetBotSub()].Emote == EMOTE_ANGRY || Server()->GetEnumTypeDay() == DayType::NIGHTTYPE))
	{
		// ищем игрока
		CPlayer *pPlayer = SearchPlayer(800.0f);
		if(pPlayer && pPlayer->GetCharacter()) 
			m_BotTargetID = pPlayer->GetCID();

		return pPlayer;
	}

	// не враждебные мобы
	CPlayer *pPlayer = GS()->GetPlayer(m_BotTargetID, true, true);
	if (m_BotTargetID == GetPlayer()->GetCID() || !pPlayer ||
		GS()->Collision()->FastIntersectLine(pPlayer->GetCharacter()->m_Core.m_Pos, m_Pos, 0, 0))
		return NULL; 

	if(Server()->Tick() % Server()->TickSpeed() == 0)
		SetEmote(EMOTE_ANGRY, 1);

	return pPlayer;
}

// Разговор ботов
void BotAI::TalkingBots(const char *Text)
{
	for(int tID = 0; tID < MAX_PLAYERS; tID++)
	{
		if(!GS()->m_apPlayers[tID])
			continue;

		if(distance(m_Core.m_Pos, GS()->m_apPlayers[tID]->m_ViewPos) < 80)
		{
			if((GS()->Collision()->FastIntersectLine(m_Core.m_Pos, GS()->m_apPlayers[tID]->m_ViewPos, 0, 0)))
				continue;

			if(GS()->CheckClient(tID))
			{

			}
			else if(!m_MessagePlayers[tID])
			{
				m_MessagePlayers[tID] = true;

				GS()->ChatFollow(tID, "{STR}.", Text);
				GS()->CreateSound(m_Pos, 19, CmaskOne(tID));				
			}
		}
		
		if(m_MessagePlayers[tID] && distance(m_Core.m_Pos, GS()->m_apPlayers[tID]->m_ViewPos) >= 80)
		{
			m_MessagePlayers[tID] = false;
		}
	}
}

int BotAI::GetEmoticon(int EmoteEyes) const
{
	if(EmoteEyes == EMOTE_HAPPY) 
		return EMOTICON_EYES;
	else if(EmoteEyes == EMOTE_BLINK) 
		return EMOTICON_DOTDOT;
	else if(EmoteEyes == EMOTE_ANGRY) 
		return EMOTICON_SUSHI+rand()%4;
	else if(EmoteEyes == EMOTE_PAIN) 
		return EMOTICON_DROP;
	else if(EmoteEyes == EMOTE_SURPRISE) 
		return EMOTICON_WTF;
	return -1;
}