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

bool BotAI::Spawn(class CPlayer *pPlayer, vec2 Pos)
{
	if(!CCharacter::Spawn(pPlayer, Pos))
		return false;
	
	// Запрещаем дамаг и хоок для человека
	if(GetPlayer()->GetSpawnBot() == SpawnBot::SPAWN_NPC)
	{
		m_NoAllowDamage = true;
		m_Core.m_ProtectHooked = true;
	}

	// Спавн ботов основной
	m_StartHealth = Health();
	ClearTarget();

	// информация о зарождении жирного моба
	int SubBotID = GetPlayer()->GetBotSub();
	if(GetPlayer()->GetSpawnBot() == SpawnBot::SPAWN_MOBS && BotJob::MobBot[SubBotID].Boss)
	{
		for(int i = 0; i < 3; i++)
		{
			CreateSnapProj(GetSnapFullID(), 1, PICKUP_HEALTH, true, false);
			CreateSnapProj(GetSnapFullID(), 1, WEAPON_HAMMER, false, true);
		}
		GS()->ChatWorldID(BotJob::MobBot[SubBotID].WorldID, "[Raid]", "In your area, spawned, {STR}, HP {INT}", BotJob::MobBot[SubBotID].Name, &m_StartHealth);
	}
	else if(GetPlayer()->GetSpawnBot() == SpawnBot::SPAWN_QUEST_NPC)
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
			/*
			dynamic_string Buffer;
			int SubBotID = GetPlayer()->GetBotSub();
			Server()->Localization()->Format(Buffer, pPlayer->GetLanguage(), _("Hard Mob: {s:name} Target: {s:tname}\n"),
				"name", ContextBots::MobBot[SubBotID].Name, "tname", Server()->ClientName(m_BotTargetID), NULL);
			pPlayer->AddInBroadcast(Buffer.buffer()), Buffer.clear();

			int Health = GetPlayer()->GetHealth();
			float gethp = ( Health * 100.0 ) / m_StartHealth;
			const char *Level = GS()->LevelString(100, (int)gethp, 10, ':', ' ');
			Server()->Localization()->Format(Buffer, pPlayer->GetLanguage(), _("Health {STR}({INT}/{INT})\n"), Level, &Health, &m_StartHealth, NULL);
			pPlayer->AddInBroadcast(Buffer.buffer()), Buffer.clear();
			delete Level;
			*/
		}
	}	
}

bool BotAI::TakeDamage(vec2 Force, vec2 Source, int Dmg, int From, int Weapon)
{
	if (From < 0 || From > MAX_CLIENTS || !GS()->m_apPlayers[From])
		return false;

	if(GetPlayer()->GetSpawnBot() != SpawnBot::SPAWN_MOBS || GS()->m_apPlayers[From]->IsBot())
		return false;

	// до урона и после урона здоровье
	int StableDamage = Health();
	bool BotDie = CCharacter::TakeDamage(Force, Source, Dmg, From, Weapon);
	StableDamage -= Health();

	// установить агрессию на того от кого пришел урон
	if (From != GetPlayer()->GetCID())
	{
		m_ListDmgPlayers[From] += StableDamage;
		if (m_BotTargetID == GetPlayer()->GetCID())
			SetTarget(From);
	}

	// проверка при смерте
	if(BotDie)
	{
		ClearTarget();

		for (const auto& ld : m_ListDmgPlayers)
		{
			int ParseClientID = ld.first;
			CPlayer* pPlayer = GS()->GetPlayer(ParseClientID, true, true);
			if (!pPlayer || ParseClientID == GetPlayer()->GetCID() || distance(pPlayer->m_ViewPos, m_Core.m_Pos) > 1000.0f)
				continue;

			DieRewardPlayer(pPlayer, Force);
		}
		Die(From, Weapon);
	}

	return false;
}

void BotAI::Die(int Killer, int Weapon)
{
	// просто убить бота если он не моб
	if(GetPlayer()->GetSpawnBot() != SpawnBot::SPAWN_MOBS)
		return;

	// склад пополнение
	int BotID = GetPlayer()->GetBotID();
	int StorageID = GS()->Mmo()->Storage()->GetStorageMonsterSub(BotID);
	if(StorageID > 0) 
		GS()->Mmo()->Storage()->AddStorageGoods(StorageID, random_int()%5);

	// очищаем лист и убиваем игрока
	m_ListDmgPlayers.clear();
	CCharacter::Die(Killer, Weapon);
}

void BotAI::CreateRandomDropItem(int DropCID, int Random, int ItemID, int Count, vec2 Force)
{
	if (DropCID < 0 || DropCID >= MAX_PLAYERS || !GS()->m_apPlayers[DropCID] || !GS()->m_apPlayers[DropCID]->GetCharacter() || !IsAlive())
		return;

	int RandomDrop = (Random == 0 ? 0 : random_int() % Random);
	if (RandomDrop == 0)
		GS()->CreateDropItem(m_Core.m_Pos, DropCID, ItemID, Count, 0, Force);
	return;
}

void BotAI::DieRewardPlayer(CPlayer* pPlayer, vec2 ForceDies)
{
	int ClientID = pPlayer->GetCID();
	int BotID = GetPlayer()->GetBotID();
	int SubID = GetPlayer()->GetBotSub();

	if (GetPlayer()->GetSpawnBot() == SpawnBot::SPAWN_MOBS)
		GS()->Mmo()->Quest()->AddMobProgress(pPlayer, BotID);

	// создаем дроп
	for (int i = 0; i < 6; i++)
	{
		int DropItem = BotJob::MobBot[SubID].DropItem[i];
		int CountItem = BotJob::MobBot[SubID].CountItem[i];
		if (DropItem <= 0 || CountItem <= 0)
			continue;

		int RandomDrop = BotJob::MobBot[SubID].RandomItem[i];
		if (DropItem == itMoney)
		{
			if (RandomDrop <= 0 || random_int() % RandomDrop == 0)
				pPlayer->AddMoney(CountItem);
			continue;
		}
		CreateRandomDropItem(ClientID, RandomDrop, DropItem, CountItem, ForceDies);
	}

	// exp
	int DamageExp = (BotJob::MobBot[SubID].Level * g_Config.m_SvExperienceMob);
	int PowerRaid = GS()->IncreaseCountRaid(DamageExp);
	GS()->CreateDropBonuses(m_Core.m_Pos, 1, DamageExp / 2, (1+random_int() % 2), ForceDies);
	pPlayer->AddExp(PowerRaid);

	// дать скилл поинт
	if (random_int() % 80 == 0)
	{
		pPlayer->GetItem(itSkillPoint).Add(1);
		GS()->Chat(ClientID, "Skill points increased. Now ~{INT}SP~", &pPlayer->GetItem(itSkillPoint).Count);
	}
}

void BotAI::ClearTarget()
{
	int FromID = GetPlayer()->GetCID();
	if (m_BotTargetID != FromID)
	{
		m_EmotionsStyle = 1 + random_int() % 5;
		m_BotTargetID = FromID;
		m_BotTargetLife = 0;
		m_BotTargetCollised = 0;
	}
}

void BotAI::SetTarget(int ClientID)
{
	m_EmotionsStyle = EMOTE_ANGRY;
	m_BotTargetID = ClientID;
}

void BotAI::ChangeWeapons()
{
	int randtime = 1+random_int()%3;
	if(Server()->Tick() % (Server()->TickSpeed()*randtime) == 0)
	{
		int randomweapon = random_int()%5;	
		m_ActiveWeapon = clamp(randomweapon, 0, 4);
	}
}

// Интерактивы ботов
void BotAI::EngineBots()
{
	m_Input.m_Fire = 0;
	m_Input.m_Jump = 0;

	// рандом для дружественного моба
	if(GetPlayer()->GetSpawnBot() == SpawnBot::SPAWN_NPC)
		EngineNPC();
	else if(GetPlayer()->GetSpawnBot() == SpawnBot::SPAWN_MOBS)
		EngineMobs();
	else if(GetPlayer()->GetSpawnBot() == SpawnBot::SPAWN_QUEST_NPC)
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
	int SubBotID = GetPlayer()->GetBotSub();
	bool StaticBot = BotJob::NpcBot[SubBotID].Static;

	// ------------------------------------------------------------------------------
	// интерактивы бота без найденого игрока
	// ------------------------------------------------------------------------------
	// эмоции ботов
	int EmoteBot = BotJob::NpcBot[SubBotID].Emote;
	EmoteActions(EmoteBot);

	// направление глаз
	if(Server()->Tick() % Server()->TickSpeed() == 0)
		m_Input.m_TargetY = random_int()%4- random_int()%8;
	m_Input.m_TargetX = (m_Input.m_Direction*10+1);

	// ------------------------------------------------------------------------------
	// интерактивы бота с найденым игроком
	// ------------------------------------------------------------------------------
	bool PlayerFinding = false;
	for (int i = 0; i < MAX_PLAYERS; i++)
	{
		CPlayer *pFind = GS()->GetPlayer(i, true, true);
		if (pFind && distance(pFind->GetCharacter()->m_Core.m_Pos, m_Core.m_Pos) < 300 &&
			!GS()->Collision()->FastIntersectLine(pFind->GetCharacter()->m_Core.m_Pos, m_Core.m_Pos, 0, 0))
		{
			pFind->SetTalking(GetPlayer()->GetCID(), false);
			m_Input.m_TargetX = static_cast<int>(pFind->GetCharacter()->m_Core.m_Pos.x - m_Pos.x);
			m_Input.m_TargetY = static_cast<int>(pFind->GetCharacter()->m_Core.m_Pos.y - m_Pos.y);
			m_Input.m_Direction = 0;
			PlayerFinding = true;
		}
	}

	if (!PlayerFinding && !StaticBot && Server()->Tick() % (Server()->TickSpeed() * (m_Input.m_Direction == 0 ? 5 : 1)) == 0)
		m_Input.m_Direction = -1 + random_int() % 3;
}

// Интерактивы квестовых мобов
void BotAI::EngineQuestMob()
{
	// направление глаз
	if(Server()->Tick() % Server()->TickSpeed() == 0)
		m_Input.m_TargetY = random_int()%4- random_int()%8;
	m_Input.m_TargetX = (m_Input.m_Direction*10+1);
	EmoteActions(EMOTE_BLINK);

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
		pFind->SetTalking(GetPlayer()->GetCID(), false);
	}
}

// Интерактивы мобов враждебных
void BotAI::EngineMobs()
{
	// ------------------------------------------------------------------------------
	// интерактивы бота без найденого игрока
	// ------------------------------------------------------------------------------
	if(Server()->Tick() % Server()->TickSpeed() == 0)
		m_Input.m_TargetY = random_int()%30- random_int()%60 + m_Core.m_Vel.y;
	m_Input.m_TargetX = m_Input.m_Direction* random_int()%30;

	// крюк
	if (!m_Input.m_Hook)
	{
		if ((m_Core.m_Vel.y < 0 && m_Input.m_TargetY > 0) || (m_Core.m_Vel.y > 0 && m_Input.m_TargetY < 0) ||
			(m_Core.m_Vel.x > 0 && m_Input.m_TargetX > 0 && m_Input.m_Direction == -1) ||
			(m_Core.m_Vel.x < 0 && m_Input.m_TargetX < 0 && m_Input.m_Direction == 1) || random_int() % 4 == 0)
		{
			vec2 HookDir = GetHookPos(vec2(m_Input.m_TargetX, m_Input.m_TargetY));
			if ((int)HookDir.x > 0 && (int)HookDir.y > 0)
			{
				vec2 AimDir = HookDir - m_Core.m_Pos;
				m_Input.m_TargetX = AimDir.x;
				m_Input.m_TargetY = AimDir.y;
				m_LatestInput.m_TargetX = (int)AimDir.x;
				m_LatestInput.m_TargetY = (int)AimDir.y;
				m_Input.m_Hook = true;
				return;
			}
		}
	}
	if (m_Input.m_Hook && random_int() % 18 == 0)
		m_Input.m_Hook = false;

	// эмоции ботов
	int SubBotID = GetPlayer()->GetBotSub();
	EmoteActions(m_EmotionsStyle);

	// Зелье мобам восстановление Health без агра
	if(m_BotTargetID == GetPlayer()->GetCID() && Health() < m_StartHealth && !GetPlayer()->CheckEffect("RegenHealth")) 
	{
		SetEmote(EMOTE_HAPPY, 3);
		GetPlayer()->GiveEffect("RegenHealth", 5);
	}

	CPlayer *pPlayer = SearchTenacityPlayer(1000.0f);
	if(!pPlayer || !pPlayer->GetCharacter())
	{
        if (Server()->Tick() - m_BotTick > Server()->TickSpeed()* random_int()%300)
        {
            int Action = random_int()%3;
            if (Action == 0)
			{
				m_Input.m_Jump = random_int()%2;
                m_Input.m_Direction = -1;
			}
            else if (Action == 1)
			{
				m_Input.m_Jump = random_int()%2;
                m_Input.m_Direction = 1;
			}
			else if (Action == 2)
                m_Input.m_Direction = 0;

            m_BotTick = Server()->Tick();
        }
		return;
	}

	bool NeedJumping = (m_Pos.y > pPlayer->GetCharacter()->m_Core.m_Pos.y + random_int() % 400
		|| (GS()->Collision()->GetCollisionAt(m_Pos.x + 32.0f, m_Pos.y) && m_Core.m_Direction == 1)
		|| (GS()->Collision()->GetCollisionAt(m_Pos.x - 32.0f, m_Pos.y) && m_Core.m_Direction == -1));

	if (NeedJumping)
		m_Input.m_Jump = 1;


	// ------------------------------------------------------------------------------
	// интерактивы бота с найденым игроком
	// ------------------------------------------------------------------------------
	int Dist = distance(m_Pos, pPlayer->GetCharacter()->m_Core.m_Pos);
	if (Dist < 600.0f)
	{
		bool StaticBot = (BotJob::MobBot[SubBotID].Spread >= 1);
		if(random_int() % 7 == 1)
		{
			m_Input.m_TargetX = static_cast<int>(pPlayer->GetCharacter()->m_Core.m_Pos.x - m_Pos.x);
			m_Input.m_TargetY = static_cast<int>(pPlayer->GetCharacter()->m_Core.m_Pos.y - m_Pos.y);
			if(m_BotTargetCollised || Dist > (StaticBot ? 250 : 70))
			{
				vec2 DirPlayer = normalize(pPlayer->GetCharacter()->m_Core.m_Pos - m_Pos);
				if (DirPlayer.x < 0) 
					m_Input.m_Direction = -1;
				else 
					m_Input.m_Direction = 1;
			}
			else 
				m_LatestInput.m_Fire = m_Input.m_Fire = 1;
		}

		// сменять оружие если статик
		if(StaticBot)
		{
			if(BotJob::MobBot[SubBotID].Boss)
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
			|| GS()->Collision()->FastIntersectLine(GS()->m_apPlayers[i]->GetCharacter()->m_Core.m_Pos, m_Pos, 0, 0)
			|| Server()->GetWorldID(i) != GS()->GetWorldID())
			continue;
		return GS()->m_apPlayers[i];
	}
	return NULL;
}

// Поиск игрока среди людей который имеет ярость выше всех
CPlayer *BotAI::SearchTenacityPlayer(float Distance)
{
	bool ActiveTargetID = m_BotTargetID != GetPlayer()->GetCID();

	// ночью боты враждебнее ищем сразу игрока для агра если его нету или есть бот злой
	if(!ActiveTargetID && (GS()->IsDungeon() || Server()->GetEnumTypeDay() == DayType::NIGHTTYPE || random_int() % 300 == 0))
	{
		// ищем игрока
		CPlayer *pPlayer = SearchPlayer(Distance);
		if(pPlayer && pPlayer->GetCharacter()) 
			SetTarget(pPlayer->GetCID());
		return pPlayer;
	}

	// сбрасываем агрессию если игрок далеко
	CPlayer* pPlayer = GS()->GetPlayer(m_BotTargetID, true, true);
	if (ActiveTargetID && (!pPlayer 
		|| (pPlayer && (distance(m_Core.m_Pos, pPlayer->GetCharacter()->m_Core.m_Pos) > 600.0f || Server()->GetWorldID(m_BotTargetID) != GS()->GetWorldID()))))
		ClearTarget();

	// не враждебные мобы
	if (!ActiveTargetID || !pPlayer)
		return NULL; 

	// сбрасываем время жизни таргета
	m_BotTargetCollised = GS()->Collision()->FastIntersectLine(pPlayer->GetCharacter()->m_Core.m_Pos, m_Pos, 0, 0);
	if (m_BotTargetLife && m_BotTargetCollised)
	{
		m_BotTargetLife--;
		if (!m_BotTargetLife)
			ClearTarget();
	}
	else
	{
		m_BotTargetLife = Server()->TickSpeed() * 2;
	}

	// ищем более сильного
	for (int i = 0; i < MAX_PLAYERS; i++)
	{
		// проверяем на дистанцию игрока
		CPlayer* pFinderHard = GS()->GetPlayer(i, true, true);
		if (!pFinderHard || distance(pFinderHard->GetCharacter()->m_Core.m_Pos, m_Core.m_Pos) > 600.0f)
			continue;

		// проверяем есть ли вкуснее игрокв для бота
		bool FinderCollised = (bool)GS()->Collision()->FastIntersectLine(pFinderHard->GetCharacter()->m_Core.m_Pos, m_Core.m_Pos, 0, 0);
		if (!FinderCollised && ((m_BotTargetLife <= 10 && m_BotTargetCollised)
			|| pFinderHard->GetAttributeCount(Stats::StHardness, true) > pPlayer->GetAttributeCount(Stats::StHardness, true)))
			SetTarget(i);
	}

	return pPlayer;
}

void BotAI::EmoteActions(int EmotionStyle)
{
	if (EmotionStyle < EMOTE_PAIN || EmotionStyle > EMOTE_BLINK)
		return;

	if ((Server()->Tick() % (Server()->TickSpeed() * 3 + random_int() % 10)) == 0)
	{
		if (EmotionStyle == EMOTE_BLINK)
		{
			SetEmote(EMOTE_BLINK, 1 + random_int() % 2);
			GS()->SendEmoticon(GetPlayer()->GetCID(), EMOTICON_DOTDOT);

		}
		else if (EmotionStyle == EMOTE_HAPPY)
		{
			SetEmote(EMOTE_HAPPY, 1 + random_int() % 2);
			GS()->SendEmoticon(GetPlayer()->GetCID(), (random_int() % 2 == 0 ? (int)EMOTICON_HEARTS : (int)EMOTICON_EYES));

		}
		else if (EmotionStyle == EMOTE_ANGRY)
		{
			SetEmote(EMOTE_ANGRY, 1 + random_int() % 2);
			GS()->SendEmoticon(GetPlayer()->GetCID(), (EMOTICON_SPLATTEE + random_int() % 3));
		}		
		else if (EmotionStyle == EMOTE_PAIN)
		{
			SetEmote(EMOTE_PAIN, 1 + random_int() % 2);
			GS()->SendEmoticon(GetPlayer()->GetCID(), EMOTICON_DROP);
		}
	}
}

vec2 BotAI::GetHookPos(vec2 Position)
{
	vec2 HookPos = vec2(0, 0);
	int HookLength = GS()->Tuning()->m_HookLength - 50.0f;
	// search for when going up
	// right
	if (Position.x > 0 && Position.y < -0.2)
	{
		// search up right first
		vec2 SearchDir = normalize(vec2(2, -3));

		vec2 TmpDir;
		float Angle;
		for (int i = 0; i < 45; i += 3)
		{
			Angle = (-i * (3.14159265f / 180.0f));
			TmpDir.x = (SearchDir.x * cos(Angle)) - (SearchDir.y * sin(Angle));
			TmpDir.y = (SearchDir.x * sin(Angle)) + (SearchDir.y * cos(Angle));
			SearchDir = TmpDir;
			if (distance(HookPos, GetPos()) > 40 && !GS()->Collision()->CheckPoint(TmpDir.x, TmpDir.y, CCollision::COLFLAG_NOHOOK)
				&& GS()->Collision()->FastIntersectLine(GetPos(), GetPos() + (SearchDir * HookLength), &TmpDir, &HookPos))
				break;
		}

		return HookPos;
	}

	// left
	if (Position.x < 0 && Position.y < -0.2)
	{
		// search up left
		vec2 SearchDir = normalize(vec2(-2, -3));

		vec2 TmpDir;
		float Angle;
		for (int i = 0; i < 45; i += 3)
		{
			Angle = (i * (3.14159265f / 180.0f));
			TmpDir.x = (SearchDir.x * cos(Angle)) - (SearchDir.y * sin(Angle));
			TmpDir.y = (SearchDir.x * sin(Angle)) + (SearchDir.y * cos(Angle));
			SearchDir = TmpDir;
			if (distance(HookPos, GetPos()) > 40 && !GS()->Collision()->CheckPoint(TmpDir.x, TmpDir.y, CCollision::COLFLAG_NOHOOK)
				&& GS()->Collision()->FastIntersectLine(GetPos(), GetPos() + (SearchDir * HookLength), &TmpDir, &HookPos))
				break;
		}

		return HookPos;
	}

	// go down faster \o/
	if (Position.x > 0 && Position.y > 0.2)
	{
		// search down right
		vec2 SearchDir = normalize(vec2(1, 2));

		vec2 TmpDir;
		float Angle;
		for (int i = 0; i < 15; i += 3)
		{
			Angle = (-i * (3.14159265f / 180.0f));
			TmpDir.x = (SearchDir.x * cos(Angle)) - (SearchDir.y * sin(Angle));
			TmpDir.y = (SearchDir.x * sin(Angle)) + (SearchDir.y * cos(Angle));
			SearchDir = TmpDir;
			if (distance(HookPos, GetPos()) > 40 && !GS()->Collision()->CheckPoint(TmpDir.x, TmpDir.y, CCollision::COLFLAG_NOHOOK)
				&& GS()->Collision()->FastIntersectLine(GetPos(), GetPos() + (SearchDir * HookLength), &TmpDir, &HookPos))
				break;
		}

		return HookPos;
	}

	if (Position.x < 0 && Position.y > 0.2)
	{
		// search down left
		vec2 SearchDir = normalize(vec2(-1, 2));

		vec2 TmpDir;
		float Angle;
		for (int i = 0; i < 15; i += 3)
		{
			Angle = (i * (3.14159265f / 180.0f));
			TmpDir.x = (SearchDir.x * cos(Angle)) - (SearchDir.y * sin(Angle));
			TmpDir.y = (SearchDir.x * sin(Angle)) + (SearchDir.y * cos(Angle));
			SearchDir = TmpDir;
			if (distance(HookPos, GetPos()) > 40 && !GS()->Collision()->CheckPoint(TmpDir.x, TmpDir.y, CCollision::COLFLAG_NOHOOK)
				&& GS()->Collision()->FastIntersectLine(GetPos(), GetPos() + (SearchDir * HookLength), &TmpDir, &HookPos))
				break;
		}

		return HookPos;
	}

	return HookPos;
}
