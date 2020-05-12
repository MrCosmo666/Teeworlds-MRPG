/* (c) Alexandre Díaz. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#include <new>
#include <engine/shared/config.h>
#include <game/server/gamecontext.h>
#include <game/mapitems.h>
#include "character_bot_ai.h"

#include <game/server/mmocore/GameEntities/Skills/healthturret/hearth.h>

MACRO_ALLOC_POOL_ID_IMPL(CCharacterBotAI, MAX_CLIENTS*COUNT_WORLD+MAX_CLIENTS)

CCharacterBotAI::CCharacterBotAI(CGameWorld *pWorld) : CCharacter(pWorld) {}

CCharacterBotAI::~CCharacterBotAI() 
{
	RemoveSnapProj(100, GetSnapFullID());
	RemoveSnapProj(100, GetSnapFullID(), true);
}

// Получить оформление для SnapFull
int CCharacterBotAI::GetSnapFullID() const 
{
	return GetPlayer()->GetCID() * SNAPBOTS;
}

bool CCharacterBotAI::Spawn(class CPlayer *pPlayer, vec2 Pos)
{
	if(!CCharacter::Spawn(pPlayer, Pos))
		return false;
	
	m_StartHealth = Health();
	ClearTarget();

	// информация о зарождении жирного моба
	int SubBotID = GetPlayer()->GetBotSub();
	if(GetPlayer()->GetBotType() == BotsTypes::TYPE_BOT_MOB && BotJob::MobBot[SubBotID].Boss)
	{
		for(int i = 0; i < 3; i++)
		{
			CreateSnapProj(GetSnapFullID(), 1, PICKUP_HEALTH, true, false);
			CreateSnapProj(GetSnapFullID(), 1, WEAPON_HAMMER, false, true);
		}
		if (!GS()->IsDungeon())
		{
			GS()->ChatWorldID(BotJob::MobBot[SubBotID].WorldID, "", "In your zone emerging {STR}!", BotJob::MobBot[SubBotID].GetName());
		}
	}
	else if(GetPlayer()->GetBotType() == BotsTypes::TYPE_BOT_QUEST)
	{
		m_Core.m_LostData = true;
		CreateSnapProj(GetSnapFullID(), 2, PICKUP_HEALTH, true, false);
		CreateSnapProj(GetSnapFullID(), 2, PICKUP_ARMOR, true, false);
	}
	else if(GetPlayer()->GetBotType() == BotsTypes::TYPE_BOT_NPC)
	{
		m_NoAllowDamage = true;
		m_Core.m_ProtectHooked = true;

		const int Function = BotJob::NpcBot[SubBotID].Function;
		if(Function == FunctionsNPC::FUNCTION_NPC_NURSE)
			new CEntityFunctionNurse(&GS()->m_World, GetPlayer()->GetCID(), m_Core.m_Pos);

		else if(Function == FunctionsNPC::FUNCTION_NPC_GIVE_QUEST)
		{
			CreateSnapProj(GetSnapFullID(), 3, PICKUP_ARMOR, false, false);
		}
	}
	return true;
}

void CCharacterBotAI::Tick()
{
	if(!GS()->CheckPlayersDistance(m_Core.m_Pos, 1000.0f) || !IsAlive())
		return;

	EngineBots();
	CCharacter::Tick();
}

void CCharacterBotAI::ShowProgress()
{
	for(const auto & ListDmgPlayer : m_ListDmgPlayers)
	{
		CPlayer *pPlayer = GS()->GetPlayer(ListDmgPlayer.first, true);
		if(pPlayer)
		{
			int Health = GetPlayer()->GetHealth();
			float gethp = ( Health * 100.0 ) / m_StartHealth;
			char *Progress = GS()->LevelString(100, (int)gethp, 10, ':', ' ');
			GS()->SBL(ListDmgPlayer.first, BroadcastPriority::BROADCAST_GAME_PRIORITY, 10, "Health {STR}({INT}/{INT})", Progress, &Health, &m_StartHealth);
			delete Progress;
		}
	}	
}

bool CCharacterBotAI::TakeDamage(vec2 Force, vec2 Source, int Dmg, int From, int Weapon)
{
	if (From < 0 || From > MAX_CLIENTS || !GS()->m_apPlayers[From])
		return false;

	if(GetPlayer()->GetBotType() != BotsTypes::TYPE_BOT_MOB || GS()->m_apPlayers[From]->IsBot())
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

void CCharacterBotAI::Die(int Killer, int Weapon)
{
	if(GetPlayer()->GetBotType() != BotsTypes::TYPE_BOT_MOB)
		return;

	m_ListDmgPlayers.clear();
	CCharacter::Die(Killer, Weapon);
}

void CCharacterBotAI::CreateRandomDropItem(int DropCID, int Random, int ItemID, int Count, vec2 Force)
{
	if (DropCID < 0 || DropCID >= MAX_PLAYERS || !GS()->m_apPlayers[DropCID] || !GS()->m_apPlayers[DropCID]->GetCharacter() || !IsAlive())
		return;

	int RandomDrop = (Random == 0 ? 0 : random_int() % Random);
	if (RandomDrop == 0)
		GS()->CreateDropItem(m_Core.m_Pos, DropCID, ItemID, Count, 0, Force);
	return;
}

void CCharacterBotAI::DieRewardPlayer(CPlayer* pPlayer, vec2 ForceDies)
{
	int ClientID = pPlayer->GetCID();
	int BotID = GetPlayer()->GetBotID();
	int SubID = GetPlayer()->GetBotSub();

	if (GetPlayer()->GetBotType() == BotsTypes::TYPE_BOT_MOB)
		GS()->Mmo()->Quest()->AddMobProgress(pPlayer, BotID);

	for (int i = 0; i < 6; i++)
	{
		int DropItem = BotJob::MobBot[SubID].DropItem[i];
		int CountItem = BotJob::MobBot[SubID].CountItem[i];
		if (DropItem <= 0 || CountItem <= 0)
			continue;

		int RandomDrop = BotJob::MobBot[SubID].RandomItem[i];
		CreateRandomDropItem(ClientID, RandomDrop, DropItem, CountItem, ForceDies);
	}

	int MultiplierExperience = kurosio::computeExperience(BotJob::MobBot[SubID].Level) / g_Config.m_SvKillmobsIncreaseLevel;
	int MultiplierRaid = clamp(GS()->IncreaseCountRaid(MultiplierExperience), 1, GS()->IncreaseCountRaid(MultiplierExperience));
	pPlayer->AddExp(MultiplierRaid);

	int MultiplierDrops = clamp(MultiplierRaid / 2, 1, MultiplierRaid);
	GS()->CreateDropBonuses(m_Core.m_Pos, 1, MultiplierDrops, (1+random_int() % 2), ForceDies);

	int MultiplierGolds = BotJob::MobBot[SubID].Power / g_Config.m_SvStrongGold;
	pPlayer->AddMoney(MultiplierGolds);

	if (random_int() % 80 == 0)
	{
		pPlayer->GetItem(itSkillPoint).Add(1);
		GS()->Chat(ClientID, "Skill points increased. Now ~{INT}SP~", &pPlayer->GetItem(itSkillPoint).Count);
	}
}

void CCharacterBotAI::ClearTarget()
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

void CCharacterBotAI::SetTarget(int ClientID)
{
	m_EmotionsStyle = EMOTE_ANGRY;
	m_BotTargetID = ClientID;
}

void CCharacterBotAI::ChangeWeapons()
{
	int randtime = 1+random_int()%3;
	if(Server()->Tick() % (Server()->TickSpeed()*randtime) == 0)
	{
		int randomweapon = random_int()%5;	
		m_ActiveWeapon = clamp(randomweapon, 0, 4);
	}
}

// Интерактивы ботов
void CCharacterBotAI::EngineBots()
{
	m_Input.m_Fire = 0;
	m_Input.m_Jump = 0;

	// рандом для дружественного моба
	if(GetPlayer()->GetBotType() == BotsTypes::TYPE_BOT_NPC)
		EngineNPC();
	else if(GetPlayer()->GetBotType() == BotsTypes::TYPE_BOT_MOB)
		EngineMobs();
	else if(GetPlayer()->GetBotType() == BotsTypes::TYPE_BOT_QUEST)
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
void CCharacterBotAI::EngineNPC()
{
	const int SubBotID = GetPlayer()->GetBotSub();
	const bool StaticBot = BotJob::NpcBot[SubBotID].Static;

	// ------------------------------------------------------------------------------
	// интерактивы бота без найденого игрока
	// ------------------------------------------------------------------------------
	// эмоции ботов
	const int EmoteBot = BotJob::NpcBot[SubBotID].Emote;
	EmoteActions(EmoteBot);

	// направление глаз
	if(Server()->Tick() % Server()->TickSpeed() == 0)
		m_Input.m_TargetY = random_int()%4- random_int()%8;
	m_Input.m_TargetX = (m_Input.m_Direction*10+1);

	// ------------------------------------------------------------------------------
	// интерактивы бота с найденым игроком
	// ------------------------------------------------------------------------------
	bool PlayerFinding = false;
	if(BotJob::NpcBot[SubBotID].Function == FunctionsNPC::FUNCTION_NPC_NURSE)
	{
		PlayerFinding = FunctionNurseNPC();
	}
	else
	{
		PlayerFinding = BaseFunctionNPC();
	}

	if (!PlayerFinding && !StaticBot && Server()->Tick() % (Server()->TickSpeed() * (m_Input.m_Direction == 0 ? 5 : 1)) == 0)
		m_Input.m_Direction = -1 + random_int() % 3;
}

// Интерактивы квестовых мобов
void CCharacterBotAI::EngineQuestMob()
{
	// направление глаз
	int SubBotID = GetPlayer()->GetBotSub();
	if(Server()->Tick() % Server()->TickSpeed() == 0)
		m_Input.m_TargetY = random_int()%4- random_int()%8;
	m_Input.m_TargetX = (m_Input.m_Direction*10+1);
	EmoteActions(EMOTE_BLINK);

	// ------------------------------------------------------------------------------
	// интерактивы бота с найденым игроком
	// ------------------------------------------------------------------------------	
	for (int i = 0; i < MAX_PLAYERS; i++)
	{
		CPlayer* pFind = GS()->GetPlayer(i, true, true);
		if (pFind && distance(pFind->GetCharacter()->m_Core.m_Pos, m_Core.m_Pos) < 128.0f
			&& !GS()->Collision()->IntersectLine(pFind->GetCharacter()->m_Core.m_Pos, m_Core.m_Pos, 0, 0)
			&& GetPlayer()->IsActiveSnappingBot(i))
		{
			if (!(BotJob::QuestBot[SubBotID].m_Talk).empty())
				GS()->SBL(i, BroadcastPriority::BROADCAST_GAME_INFORMATION, 10, "Start dialog with NPC [attack hammer]");

			m_Input.m_TargetX = round_to_int(pFind->GetCharacter()->m_Core.m_Pos.x - m_Pos.x);
			m_Input.m_TargetY = round_to_int(pFind->GetCharacter()->m_Core.m_Pos.y - m_Pos.y);
			m_Input.m_Direction = 0;
		}
	}
}

// Интерактивы мобов враждебных
void CCharacterBotAI::EngineMobs()
{
	// ------------------------------------------------------------------------------
	// интерактивы бота без найденого игрока
	// ------------------------------------------------------------------------------
	if(Server()->Tick() % Server()->TickSpeed() == 0)
		m_Input.m_TargetY = random_int()%30- random_int()%60 + m_Core.m_Vel.y;
	m_Input.m_TargetX = m_Input.m_Direction* random_int()%30;

	// крюк
	if(!m_Input.m_Hook)
	{
		if((m_Core.m_Vel.y < 0 && m_Input.m_TargetY > 0) || (m_Core.m_Vel.y > 0 && m_Input.m_TargetY < 0) ||
			(m_Core.m_Vel.x > 0 && m_Input.m_TargetX > 0 && m_Input.m_Direction == -1) ||
			(m_Core.m_Vel.x < 0 && m_Input.m_TargetX < 0 && m_Input.m_Direction == 1) || random_int() % 4 == 0)
		{
			vec2 HookDir = GetHookPos(vec2(m_Input.m_TargetX, m_Input.m_TargetY));
			if((int)HookDir.x > 0 && (int)HookDir.y > 0)
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
	if(m_Input.m_Hook && random_int() % 18 == 0)
		m_Input.m_Hook = false;

	// эмоции ботов
	int SubBotID = GetPlayer()->GetBotSub();
	EmoteActions(m_EmotionsStyle);

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
		m_Input.m_TargetX = static_cast<int>(pPlayer->GetCharacter()->m_Core.m_Pos.x - m_Pos.x);
		m_Input.m_TargetY = static_cast<int>(pPlayer->GetCharacter()->m_Core.m_Pos.y - m_Pos.y);

		bool StaticBot = (BotJob::MobBot[SubBotID].Spread >= 1);
		if(random_int() % 7 == 1)
		{
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
CPlayer *CCharacterBotAI::SearchPlayer(int Distance)
{
	for(int i = 0 ; i < MAX_PLAYERS; i ++)
	{
		if(!GS()->m_apPlayers[i] 
			|| !GS()->m_apPlayers[i]->GetCharacter() 
			|| distance(m_Core.m_Pos, GS()->m_apPlayers[i]->GetCharacter()->m_Core.m_Pos) > Distance
			|| GS()->Collision()->IntersectLine(GS()->m_apPlayers[i]->GetCharacter()->m_Core.m_Pos, m_Pos, 0, 0)
			|| Server()->GetWorldID(i) != GS()->GetWorldID())
			continue;
		return GS()->m_apPlayers[i];
	}
	return nullptr;
}

// Поиск игрока среди людей который имеет ярость выше всех
CPlayer *CCharacterBotAI::SearchTenacityPlayer(float Distance)
{
	bool ActiveTargetID = m_BotTargetID != GetPlayer()->GetCID();

	// ночью боты враждебнее ищем сразу игрока для агра если его нету или есть бот злой
	if(!ActiveTargetID && (GS()->IsDungeon() || random_int() % 50 == 0))
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
		return nullptr; 

	// сбрасываем время жизни таргета
	m_BotTargetCollised = GS()->Collision()->IntersectLine(pPlayer->GetCharacter()->m_Core.m_Pos, m_Pos, 0, 0);
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
		bool FinderCollised = (bool)GS()->Collision()->IntersectLine(pFinderHard->GetCharacter()->m_Core.m_Pos, m_Core.m_Pos, 0, 0);
		if (!FinderCollised && ((m_BotTargetLife <= 10 && m_BotTargetCollised)
			|| pFinderHard->GetAttributeCount(Stats::StHardness, true) > pPlayer->GetAttributeCount(Stats::StHardness, true)))
			SetTarget(i);
	}

	return pPlayer;
}

void CCharacterBotAI::EmoteActions(int EmotionStyle)
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

vec2 CCharacterBotAI::GetHookPos(vec2 Position)
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
				&& GS()->Collision()->IntersectLine(GetPos(), GetPos() + (SearchDir * HookLength), &TmpDir, &HookPos))
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
				&& GS()->Collision()->IntersectLine(GetPos(), GetPos() + (SearchDir * HookLength), &TmpDir, &HookPos))
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
				&& GS()->Collision()->IntersectLine(GetPos(), GetPos() + (SearchDir * HookLength), &TmpDir, &HookPos))
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
				&& GS()->Collision()->IntersectLine(GetPos(), GetPos() + (SearchDir * HookLength), &TmpDir, &HookPos))
				break;
		}
		return HookPos;
	}
	return HookPos;
}

// - - - - - - - - - - - - - - - - - - - BASE FUNCTION
bool CCharacterBotAI::BaseFunctionNPC()
{
	bool PlayerFinding = false;
	const int SubBotID = GetPlayer()->GetBotSub();
	for(int i = 0; i < MAX_PLAYERS; i++)
	{
		CPlayer* pFind = GS()->GetPlayer(i, true, true);
		if(pFind && distance(pFind->GetCharacter()->m_Core.m_Pos, m_Core.m_Pos) < 128.0f &&
			!GS()->Collision()->IntersectLine(pFind->GetCharacter()->m_Core.m_Pos, m_Core.m_Pos, 0, 0))
		{
			if(!(BotJob::NpcBot[SubBotID].m_Talk).empty())
				GS()->SBL(i, BroadcastPriority::BROADCAST_GAME_INFORMATION, 10, "Start dialog with NPC [attack hammer]");

			m_Input.m_TargetX = static_cast<int>(pFind->GetCharacter()->m_Core.m_Pos.x - m_Pos.x);
			m_Input.m_TargetY = static_cast<int>(pFind->GetCharacter()->m_Core.m_Pos.y - m_Pos.y);
			m_Input.m_Direction = 0;
			PlayerFinding = true;
		}
	}
	return PlayerFinding;
}

// - - - - - - - - - - - - - - - - - - - FUNCTION NURSE
bool CCharacterBotAI::FunctionNurseNPC()
{
	bool PlayerFinding = false;
	if(Server()->Tick() % Server()->TickSpeed() != 0)
	{
		CPlayer* pFind = SearchPlayer(256.0f);
		if(pFind && pFind->GetCharacter() && pFind->GetHealth() < pFind->GetStartHealth())
		{
			m_Input.m_TargetX = static_cast<int>(pFind->GetCharacter()->m_Core.m_Pos.x - m_Pos.x);
			m_Input.m_TargetY = static_cast<int>(pFind->GetCharacter()->m_Core.m_Pos.y - m_Pos.y);
		}
		return true;
	}

	char aBuf[16];
	for(int i = 0; i < MAX_PLAYERS; i++)
	{
		CPlayer* pFind = GS()->GetPlayer(i, true, true);
		if(!pFind || pFind->GetHealth() >= pFind->GetStartHealth() ||
			distance(pFind->GetCharacter()->m_Core.m_Pos, m_Core.m_Pos) >= 256.0f ||
			GS()->Collision()->IntersectLine(pFind->GetCharacter()->m_Core.m_Pos, m_Core.m_Pos, 0, 0))
			continue;

		m_Input.m_TargetX = static_cast<int>(pFind->GetCharacter()->m_Core.m_Pos.x - m_Pos.x);
		m_Input.m_TargetY = static_cast<int>(pFind->GetCharacter()->m_Core.m_Pos.y - m_Pos.y);
		m_LatestInput.m_TargetX = m_Input.m_TargetX;
		m_LatestInput.m_TargetY = m_Input.m_TargetX;

		const int Health = pFind->GetStartHealth() / 20;
		vec2 DrawPosition = vec2(pFind->GetCharacter()->m_Core.m_Pos.x, pFind->GetCharacter()->m_Core.m_Pos.y - 90.0f);
		str_format(aBuf, sizeof(aBuf), "%dHP", Health);
		GS()->CreateText(NULL, false, DrawPosition, vec2(0, 0), 40, aBuf, GS()->GetWorldID());
		new CHearth(&GS()->m_World, m_Pos, pFind, Health, pFind->GetCharacter()->m_Core.m_Vel);

		m_Input.m_Direction = 0;
		PlayerFinding = true;
	}
	return PlayerFinding;
}

CEntityFunctionNurse::CEntityFunctionNurse(CGameWorld* pGameWorld, int ClientID, vec2 Pos)
	: CEntity(pGameWorld, CGameWorld::ENTTYPE_EVENTS, Pos)
{
	m_OwnerID = ClientID;
	GameWorld()->InsertEntity(this);
}
void CEntityFunctionNurse::Tick()
{
	if(!GS()->m_apPlayers[m_OwnerID] || !GS()->m_apPlayers[m_OwnerID]->GetCharacter())
	{
		GS()->m_World.DestroyEntity(this);
		return;
	}

	CCharacter* pOwnerChar = GS()->m_apPlayers[m_OwnerID]->GetCharacter();
	vec2 Direction = normalize(vec2(pOwnerChar->m_LatestInput.m_TargetX, pOwnerChar->m_LatestInput.m_TargetY));
	m_Pos = pOwnerChar->m_Core.m_Pos + normalize(Direction) * (28.0f);
}
void CEntityFunctionNurse::Snap(int SnappingClient)
{
	if(NetworkClipped(SnappingClient))
		return;

	CNetObj_Pickup* pP = static_cast<CNetObj_Pickup*>(Server()->SnapNewItem(NETOBJTYPE_PICKUP, GetID(), sizeof(CNetObj_Pickup)));
	if(!pP)
		return;

	pP->m_X = (int)m_Pos.x;
	pP->m_Y = (int)m_Pos.y;
	pP->m_Type = PICKUP_HEALTH;
}
