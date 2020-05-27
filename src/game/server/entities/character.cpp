/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#include <engine/shared/config.h>

#include <generated/server_data.h>
#include <game/server/gamecontext.h>

#include "character.h"
#include "laser.h"
#include "projectile.h"

#include <game/server/mmocore/GameEntities/quest_path_finder.h>
#include <game/server/mmocore/GameEntities/snapfull.h>
#include <game/server/mmocore/GameEntities/jobitems.h>

//input count
struct CInputCount
{
	int m_Presses;
	int m_Releases;
};

CInputCount CountInput(int Prev, int Cur)
{
	CInputCount c = {0, 0};
	Prev &= INPUT_STATE_MASK;
	Cur &= INPUT_STATE_MASK;
	int i = Prev;

	while(i != Cur)
	{
		i = (i+1)&INPUT_STATE_MASK;
		if(i&1) c.m_Presses++;
		else c.m_Releases++;
	}
	return c;
}

MACRO_ALLOC_POOL_ID_IMPL(CCharacter, MAX_CLIENTS*COUNT_WORLD+MAX_CLIENTS)

CCharacter::CCharacter(CGameWorld *pWorld)
: CEntity(pWorld, CGameWorld::ENTTYPE_CHARACTER, vec2(0, 0), ms_PhysSize)
{
	m_pHelper = new TileHandle(this);
	m_DoorHit = false;
	m_Health = 0;
	m_Armor = 0;
	m_TriggeredEvents = 0;
}

CCharacter::~CCharacter()
{
	delete m_pHelper;
	GS()->m_World.m_Core.m_apCharacters[m_pPlayer->GetCID()] = nullptr;
}

int CCharacter::GetSnapFullID() const
{
	return m_pPlayer->GetCID() * SNAPPLAYER;
}

bool CCharacter::Spawn(CPlayer *pPlayer, vec2 Pos)
{
	m_pPlayer = pPlayer;

	m_EmoteStop = -1;
	m_LastAction = -1;
	m_LastNoAmmoSound = -1;
	m_ActiveWeapon = WEAPON_HAMMER;
	m_LastWeapon = WEAPON_HAMMER;
	m_QueuedWeapon = -1;

	m_Pos = Pos;
	m_Core.Reset();
	m_Core.Init(&GS()->m_World.m_Core, GS()->Collision());
	m_Core.m_Pos = m_Pos;
	GS()->m_World.m_Core.m_apCharacters[m_pPlayer->GetCID()] = &m_Core;

	m_ReckoningTick = 0;
	mem_zero(&m_SendCore, sizeof(m_SendCore));
	mem_zero(&m_ReckoningCore, sizeof(m_ReckoningCore));
	GS()->m_World.InsertEntity(this);
	m_Alive = true;

	m_Mana = 0;
	m_OldPos = Pos;
	m_NoAllowDamage = false;
	m_Core.m_LostData = false;
	m_Event = TILE_CLEAR_EVENTS;
	m_Core.m_WorldID = GS()->GetClientWorldID(m_pPlayer->GetCID());
	if(!m_pPlayer->IsBot())
	{
		m_pPlayer->m_MoodState = m_pPlayer->GetMoodState();
		GS()->Mmo()->Quest()->UpdateArrowStep(m_pPlayer->GetCID());
		if(GS()->Mmo()->Quest()->CheckNewStories(m_pPlayer))
			GS()->Chat(m_pPlayer->GetCID(), "There are new stories of familiar NPCs");

		m_AmmoRegen = m_pPlayer->GetAttributeCount(Stats::StAmmoRegen, true);
		m_pPlayer->ShowInformationStats();
		GS()->ResetVotes(m_pPlayer->GetCID(), m_pPlayer->m_OpenVoteMenu);
	}

	const bool Spawned = GS()->m_pController->OnCharacterSpawn(this);
	return Spawned;
}

void CCharacter::SetWeapon(int W)
{
	if(W == m_ActiveWeapon)
		return;

	m_LastWeapon = m_ActiveWeapon;
	m_QueuedWeapon = -1;
	m_ActiveWeapon = W;
	GS()->CreateSound(m_Pos, SOUND_WEAPON_SWITCH);

	if(m_ActiveWeapon < 0 || m_ActiveWeapon >= NUM_WEAPONS)
		m_ActiveWeapon = 0;
	m_aWeapons[m_ActiveWeapon].m_AmmoRegenStart = -1;
}

bool CCharacter::IsGrounded()
{
	if(GS()->Collision()->CheckPoint(m_Pos.x+GetProximityRadius()/2, m_Pos.y+GetProximityRadius()/2+5))
		return true;
	if(GS()->Collision()->CheckPoint(m_Pos.x-GetProximityRadius()/2, m_Pos.y+GetProximityRadius()/2+5))
		return true;
	return false;
}

void CCharacter::DoWeaponSwitch()
{
	// make sure we can switch
	if(m_ReloadTimer != 0 || m_QueuedWeapon == -1 || m_aWeapons[WEAPON_NINJA].m_Got)
		return;

	// switch Weapon
	SetWeapon(m_QueuedWeapon);
}

void CCharacter::HandleWeaponSwitch()
{
	int WantedWeapon = m_ActiveWeapon;
	if(m_QueuedWeapon != -1)
		WantedWeapon = m_QueuedWeapon;

	// select Weapon
	int Next = CountInput(m_LatestPrevInput.m_NextWeapon, m_LatestInput.m_NextWeapon).m_Presses;
	int Prev = CountInput(m_LatestPrevInput.m_PrevWeapon, m_LatestInput.m_PrevWeapon).m_Presses;

	if(Next < 128) // make sure we only try sane stuff
	{
		while(Next) // Next Weapon selection
		{
			WantedWeapon = (WantedWeapon+1)%NUM_WEAPONS;
			if(m_aWeapons[WantedWeapon].m_Got)
				Next--;
		}
	}

	if(Prev < 128) // make sure we only try sane stuff
	{
		while(Prev) // Prev Weapon selection
		{
			WantedWeapon = (WantedWeapon-1)<0?NUM_WEAPONS-1:WantedWeapon-1;
			if(m_aWeapons[WantedWeapon].m_Got)
				Prev--;
		}
	}

	// Direct Weapon selection
	if(m_LatestInput.m_WantedWeapon)
		WantedWeapon = m_Input.m_WantedWeapon-1;

	// check for insane values
	if(WantedWeapon >= 0 && WantedWeapon < NUM_WEAPONS && WantedWeapon != m_ActiveWeapon && m_aWeapons[WantedWeapon].m_Got)
		m_QueuedWeapon = WantedWeapon;

	DoWeaponSwitch();
}

bool CCharacter::DecoInteractive()
{
	const int ClientID = m_pPlayer->GetCID();
	if(m_pPlayer->GetTempData().TempDecoractionID > 0)
	{
		const int DecoID = m_pPlayer->GetTempData().TempDecoractionID;
		const int InteractiveType = m_pPlayer->GetTempData().TempDecorationType;
		m_pPlayer->GetTempData().TempDecoractionID = -1;
		m_pPlayer->GetTempData().TempDecorationType = -1;
		if(m_pPlayer->GetItem(DecoID).Count <= 0 || GS()->GetItemInfo(DecoID).Type != ItemType::TYPE_DECORATION)
			return false;

		if (InteractiveType == DECOTYPE_HOUSE)
		{
			const int HouseID = GS()->Mmo()->House()->PlayerHouseID(m_pPlayer);
			if (GS()->Mmo()->House()->AddDecorationHouse(DecoID, HouseID, m_pHelper->MousePos()))
			{
				GS()->Chat(ClientID, "You added {STR}, to your house!", GS()->GetItemInfo(DecoID).GetName(m_pPlayer));
				m_pPlayer->GetItem(DecoID).Remove(1);
				GS()->ResetVotes(ClientID, MenuList::MENU_HOUSE_DECORATION);
				return true;
			}
		}
		else if (InteractiveType == DECOTYPE_GUILD_HOUSE)
		{
			const int GuildID = m_pPlayer->Acc().GuildID;
			if (GS()->Mmo()->Member()->AddDecorationHouse(DecoID, GuildID, m_pHelper->MousePos()))
			{
				GS()->Chat(ClientID, "You added {STR}, to your guild house!", GS()->GetItemInfo(DecoID).GetName(m_pPlayer));
				m_pPlayer->GetItem(DecoID).Remove(1);
				GS()->ResetVotes(ClientID, MenuList::MENU_GUILD_HOUSE_DECORATION);
				return true;
			}
		}

		GS()->Chat(ClientID, "Distance House and Decoration maximal {INT} block!", &g_Config.m_SvLimitDecoration);
		GS()->Chat(ClientID, "Setting object reset, use repeat!");
		GS()->ResetVotes(ClientID, MenuList::MENU_HOUSE_DECORATION);
		return true;
	}
	return false;
}

void CCharacter::FireWeapon()
{
	if(m_ReloadTimer != 0)
		return;

	DoWeaponSwitch();
	bool FullAuto = false;
	if(GS()->Mmo()->Skills()->GetSkillLevel(m_pPlayer->GetCID(), SkillMasterWeapon))
		FullAuto = true;

	bool WillFire = false;
	if(CountInput(m_LatestPrevInput.m_Fire, m_LatestInput.m_Fire).m_Presses)
		WillFire = true;

	if(FullAuto && (m_LatestInput.m_Fire&1) && m_aWeapons[m_ActiveWeapon].m_Ammo)
		WillFire = true;

	if(!WillFire)
		return;

	const bool IsBot = m_pPlayer->IsBot();
	const int SubBotID = GetPlayer()->GetBotSub();
	if(!IsBot)
	{
		if(DecoInteractive())
			return;

		if(!m_aWeapons[m_ActiveWeapon].m_Ammo)
		{
			m_ReloadTimer = 125 * Server()->TickSpeed() / 1000;
			if(m_LastNoAmmoSound+Server()->TickSpeed() <= Server()->Tick())
			{
				GS()->CreateSound(m_Pos, SOUND_WEAPON_NOAMMO);
				m_LastNoAmmoSound = Server()->Tick();
			}
			return;
		}
	}

	vec2 Direction = normalize(vec2(m_LatestInput.m_TargetX, m_LatestInput.m_TargetY));
	vec2 ProjStartPos = m_Pos+Direction*GetProximityRadius()*0.75f;
	switch(m_ActiveWeapon)
	{
		case WEAPON_HAMMER:
		{
			if (InteractiveHammer(Direction, ProjStartPos))
			{
				m_ReloadTimer = Server()->TickSpeed() / 3;
				return;
			}

			bool Hits = false;
			bool StartedTalking = false;
			const float PlayerRadius = (float)m_pPlayer->GetAttributeCount(Stats::StHammerPower, true);
			const float Radius = clamp(PlayerRadius / 5.0f, 1.7f, 8.0f);
			GS()->CreateSound(m_Pos, SOUND_HAMMER_FIRE);
			
			CCharacter *apEnts[MAX_CLIENTS];
			int Num = GS()->m_World.FindEntities(ProjStartPos, GetProximityRadius()* Radius, (CEntity**)apEnts, MAX_CLIENTS, CGameWorld::ENTTYPE_CHARACTER);
			for (int i = 0; i < Num; ++i)
			{
				CCharacter* pTarget = apEnts[i];
				if((pTarget == this) || GS()->Collision()->IntersectLineWithInvisible(ProjStartPos, pTarget->m_Pos, 0, 0))
					continue;

				// talking wth bot
				if (!StartedTalking && StartConversation(pTarget->GetPlayer()))
				{
					m_pPlayer->ClearTalking();
					m_pPlayer->SetTalking(pTarget->GetPlayer()->GetCID(), false);
					GS()->CreateHammerHit(ProjStartPos);
					StartedTalking = true;
					continue;
				}

				if (pTarget->m_Core.m_LostData)
					continue;

				if(length(pTarget->m_Pos-ProjStartPos) > 0.0f)
					GS()->CreateHammerHit(pTarget->m_Pos-normalize(pTarget->m_Pos-ProjStartPos) * GetProximityRadius() * Radius);
				else
					GS()->CreateHammerHit(ProjStartPos);

				vec2 Dir = vec2(0.f, -1.f);
				if (length(pTarget->m_Pos - m_Pos) > 0.0f)
					Dir = normalize(pTarget->m_Pos - m_Pos);

				pTarget->TakeDamage(vec2(0.f, -1.f) + normalize(Dir + vec2(0.f, -1.1f)) * 10.0f, g_pData->m_Weapons.m_Hammer.m_pBase->m_Damage, m_pPlayer->GetCID(), m_ActiveWeapon);
				Hits = true;
			}
			if(Hits) 
				m_ReloadTimer = Server()->TickSpeed()/3;
		} break;

		case WEAPON_GUN:
		{
			const bool IsExplosive = m_pPlayer->GetItem(itExplosiveGun).IsEquipped();
			new CProjectile(GameWorld(), WEAPON_GUN, m_pPlayer->GetCID(), ProjStartPos, Direction, (int)(Server()->TickSpeed()*GS()->Tuning()->m_GunLifetime),
				g_pData->m_Weapons.m_Gun.m_pBase->m_Damage, IsExplosive, 0, -1, WEAPON_GUN);

			GS()->CreateSound(m_Pos, SOUND_GUN_FIRE);
		} break;

		case WEAPON_SHOTGUN:
		{
			const bool IsExplosive = m_pPlayer->GetItem(itExplosiveShotgun).IsEquipped();
			const int EnchantSpread = IsBot ? 2+BotJob::MobBot[SubBotID].Spread : m_pPlayer->GetAttributeCount(Stats::StSpreadShotgun);
			const int ShotSpread = clamp(1+EnchantSpread, 1, 36);
			CMsgPacker Msg(NETMSGTYPE_SV_EXTRAPROJECTILE);
			Msg.AddInt(ShotSpread);
			for (int i = 1; i <= ShotSpread; ++i)
			{
				float Spreading = ((0.0058945f*(9.0f*ShotSpread)/2)) - (0.0058945f*(9.0f*i));
				float a = GetAngle(Direction) + Spreading;
				new CProjectile(GameWorld(), WEAPON_SHOTGUN,
					m_pPlayer->GetCID(),
					ProjStartPos,
					vec2(cosf(a), sinf(a))*1.2f,
					(int)(Server()->TickSpeed()*GS()->Tuning()->m_ShotgunLifetime),
					g_pData->m_Weapons.m_Shotgun.m_pBase->m_Damage, IsExplosive, 0, 15, WEAPON_SHOTGUN);
			}
			Server()->SendMsg(&Msg, MSGFLAG_VITAL, m_pPlayer->GetCID());
			GS()->CreateSound(m_Pos, SOUND_SHOTGUN_FIRE);
		} break;

		case WEAPON_GRENADE:
		{
			const int EnchantSpread = IsBot ? BotJob::MobBot[SubBotID].Spread : m_pPlayer->GetAttributeCount(Stats::StSpreadGrenade);
			const int ShotSpread = clamp(1 + EnchantSpread, 1, 21);
			CMsgPacker Msg(NETMSGTYPE_SV_EXTRAPROJECTILE);
			Msg.AddInt(ShotSpread);
			for (int i = 1; i < ShotSpread; ++i)
			{
				float Spreading = ((0.0058945f*(9.0f*ShotSpread)/2)) - (0.0058945f*(9.0f*i));
				float a = GetAngle(Direction) + Spreading;
				new CProjectile(GameWorld(), WEAPON_GRENADE, m_pPlayer->GetCID(), ProjStartPos, vec2(cosf(a), sinf(a))*1.0f,
					(int)(Server()->TickSpeed()*GS()->Tuning()->m_GrenadeLifetime),
					g_pData->m_Weapons.m_Grenade.m_pBase->m_Damage, true, 0, SOUND_GRENADE_EXPLODE, WEAPON_GRENADE);
			}
			Server()->SendMsg(&Msg, MSGFLAG_VITAL, m_pPlayer->GetCID());
			GS()->CreateSound(m_Pos, SOUND_GRENADE_FIRE);
		} break;

		case WEAPON_LASER:
		{
			const int EnchantSpread = IsBot ? 2 + BotJob::MobBot[SubBotID].Spread : m_pPlayer->GetAttributeCount(Stats::StSpreadRifle);
			const int ShotSpread = clamp(1 + EnchantSpread, 1, 36);
			for (int i = 1; i < ShotSpread; ++i)
			{
				float Spreading = ((0.0058945f*(9.0f*ShotSpread)/2)) - (0.0058945f*(9.0f*i));
				float a = GetAngle(Direction) + Spreading;
				new CLaser(GameWorld(), m_Pos, vec2(cosf(a), sinf(a)), GS()->Tuning()->m_LaserReach, m_pPlayer->GetCID());
			}
			GS()->CreateSound(m_Pos, SOUND_LASER_FIRE);
		} break;
		case WEAPON_NINJA:
			break;
	}
	m_AttackTick = Server()->Tick();

	if(m_aWeapons[m_ActiveWeapon].m_Ammo > 0)
		m_aWeapons[m_ActiveWeapon].m_Ammo--;

	if(!m_ReloadTimer)
	{
		int ReloadArt = m_pPlayer->GetAttributeCount(Stats::StDexterity);
		m_ReloadTimer = g_pData->m_Weapons.m_aId[m_ActiveWeapon].m_Firedelay * Server()->TickSpeed() / (1000 + ReloadArt);
	}
}

void CCharacter::HandleWeapons()
{
	if(m_ReloadTimer)
	{
		m_ReloadTimer--;
		return;
	}

	FireWeapon();

	if(m_aWeapons[m_ActiveWeapon].m_Ammo >= 0)
	{
		const int AmmoRegenTime = (m_ActiveWeapon == (int)WEAPON_GUN ? (Server()->TickSpeed() / 2) : (max(5000 - m_AmmoRegen, 1000)) / 10);
		if (m_aWeapons[m_ActiveWeapon].m_AmmoRegenStart < 0)
			m_aWeapons[m_ActiveWeapon].m_AmmoRegenStart = Server()->Tick() + AmmoRegenTime;

		if (m_aWeapons[m_ActiveWeapon].m_AmmoRegenStart <= Server()->Tick())
		{
			const int RealAmmo = 10 + m_pPlayer->GetAttributeCount(Stats::StAmmo);
			m_aWeapons[m_ActiveWeapon].m_Ammo = min(m_aWeapons[m_ActiveWeapon].m_Ammo + 1, RealAmmo);
			m_aWeapons[m_ActiveWeapon].m_AmmoRegenStart = -1;
		}
	}
}

void CCharacter::CreateQuestsStep(int QuestID)
{
	const int ClientID = m_pPlayer->GetCID();
	vec2 Pos = GS()->Mmo()->WorldSwap()->GetPositionQuestBot(ClientID, QuestID);
	if (QuestJob::Quests[ClientID].find(QuestID) == QuestJob::Quests[ClientID].end() || (Pos.x == 0.0f && Pos.y == 0.0f))
		return;

	const int Progress = QuestJob::Quests[ClientID][QuestID].Progress;
	new CQuestPathFinder(GameWorld(), m_Core.m_Pos, ClientID, QuestID, Progress, Pos);
}

bool CCharacter::GiveWeapon(int Weapon, int GiveAmmo)
{
	const int WeaponID = clamp(Weapon, (int)WEAPON_HAMMER, (int)WEAPON_NINJA);
	const bool IsHammer = (bool)(WeaponID == WEAPON_HAMMER);
	if(m_pPlayer->GetEquippedItem(WeaponID) <= 0 && !IsHammer)
	{
		if(RemoveWeapon(WeaponID))
			m_ActiveWeapon = m_LastWeapon;
		return false;
	}

	const int MaximalAmmo = 10 + m_pPlayer->GetAttributeCount(Stats::StAmmo);
	if(m_aWeapons[WeaponID].m_Ammo >= MaximalAmmo)
		return false;

	if(!IsHammer)
	{
		const int GivesAmmo = m_aWeapons[WeaponID].m_Got ? min(m_aWeapons[WeaponID].m_Ammo + GiveAmmo, MaximalAmmo) : min(GiveAmmo, MaximalAmmo);
		m_aWeapons[WeaponID].m_Got = true;
		m_aWeapons[WeaponID].m_Ammo = (Weapon == (int)WEAPON_HAMMER ? -1 : GivesAmmo);
	}
	else
	{
		m_aWeapons[WeaponID].m_Got = true;
		m_aWeapons[WeaponID].m_Ammo = -1;
	}
	return true;
}

bool CCharacter::RemoveWeapon(int Weapon)
{
	bool Succesful = m_aWeapons[Weapon].m_Got;
	m_aWeapons[Weapon].m_Got = false;
	m_aWeapons[Weapon].m_Ammo = -1;
	return Succesful;
}

void CCharacter::SetEmote(int Emote, int Sec)
{
	if (m_Alive && m_EmoteStop < Server()->Tick())
	{
		m_EmoteType = Emote;
		m_EmoteStop = Server()->Tick() + Sec*Server()->TickSpeed();
	}
}

void CCharacter::OnPredictedInput(CNetObj_PlayerInput *pNewInput)
{
	// check for changes
	if(mem_comp(&m_Input, pNewInput, sizeof(CNetObj_PlayerInput)) != 0)
		m_LastAction = Server()->Tick();

	// copy new input
	mem_copy(&m_Input, pNewInput, sizeof(m_Input));
	m_NumInputs++;

	// it is not allowed to aim in the center
	if(m_Input.m_TargetX == 0 && m_Input.m_TargetY == 0)
		m_Input.m_TargetY = -1;
}

void CCharacter::OnDirectInput(CNetObj_PlayerInput *pNewInput)
{
	mem_copy(&m_LatestPrevInput, &m_LatestInput, sizeof(m_LatestInput));
	mem_copy(&m_LatestInput, pNewInput, sizeof(m_LatestInput));

	// it is not allowed to aim in the center
	if(m_LatestInput.m_TargetX == 0 && m_LatestInput.m_TargetY == 0)
		m_LatestInput.m_TargetY = -1;

	if(m_NumInputs > 2 && m_pPlayer->GetTeam() != TEAM_SPECTATORS)
	{
		HandleWeaponSwitch();
		FireWeapon();
	}
	mem_copy(&m_LatestPrevInput, &m_LatestInput, sizeof(m_LatestInput));
}

void CCharacter::ResetInput()
{
	m_Input.m_Direction = 0;
	m_Input.m_Hook = 0;
	if((m_Input.m_Fire&1) != 0)
		m_Input.m_Fire++;
	m_Input.m_Fire &= INPUT_STATE_MASK;
	m_Input.m_Jump = 0;
	m_LatestPrevInput = m_LatestInput = m_Input;
}

void CCharacter::Tick()
{
	if(!IsAlive())
		return;
		
	HandleTunning();
	m_Core.m_Input = m_Input;
	m_Core.Tick(true, &m_pPlayer->m_NextTuningParams);
	m_pPlayer->UpdateTempData(m_Health, m_Mana);

	if(GameLayerClipped(m_Pos))
		Die(m_pPlayer->GetCID(), WEAPON_SELF);

	if (!m_DoorHit)
	{
		m_OlderPos = m_OldPos;
		m_OldPos = m_Core.m_Pos;
	}

	// запретить дальше НПС и Квестовым
	const bool IsBot = m_pPlayer->IsBot();
	if (IsBot && m_pPlayer->GetBotType() != BotsTypes::TYPE_BOT_MOB)
		return;

	HandleWeapons();

	// запретить дальше Мобам
	if (IsBot || IsLockedWorld())
		return;

	// another function
	if(IsAlive())
	{
		HandleAuthedPlayer();
		HandleEvents();
		HandleTilesets();

		m_Core.m_LostData = false;
		m_NoAllowDamage = false;
	}
}

void CCharacter::TickDefered()
{
	if(!IsAlive())
		return;
	
	if (m_DoorHit)
	{
		ResetDoorPos();
		m_DoorHit = false;
	}

	if(m_pPlayer->IsBot())
	{	
		CCharacterCore::CParams CoreTickParams(&m_pPlayer->m_NextTuningParams);
		m_Core.Move(&CoreTickParams);
		m_Core.Quantize();
		m_Pos = m_Core.m_Pos;		
		return;
	}

	// advance the dummy
	{
		CCharacterCore::CParams CoreTickParams(&GameWorld()->m_Core.m_Tuning);
		CWorldCore TempWorld;
		m_ReckoningCore.Init(&TempWorld, GS()->Collision());
		m_ReckoningCore.Tick(false, &CoreTickParams);
		m_ReckoningCore.Move(&CoreTickParams);
		m_ReckoningCore.Quantize();
	}

	CCharacterCore::CParams CoreTickParams(&m_pPlayer->m_NextTuningParams);
	m_Core.Move(&CoreTickParams);
	m_Core.Quantize();
	m_Pos = m_Core.m_Pos;
	m_TriggeredEvents |= m_Core.m_TriggeredEvents;

	if(m_pPlayer->GetTeam() == TEAM_SPECTATORS)
	{
		m_Pos.x = m_Input.m_TargetX;
		m_Pos.y = m_Input.m_TargetY;
	}
	else if(m_Core.m_Death)
		Die(m_pPlayer->GetCID(), WEAPON_SELF);

	// update the m_SendCore if needed
	{
		CNetObj_Character Predicted;
		CNetObj_Character Current;
		mem_zero(&Predicted, sizeof(Predicted));
		mem_zero(&Current, sizeof(Current));
		m_ReckoningCore.Write(&Predicted);
		m_Core.Write(&Current);

		// only allow dead reackoning for a top of 3 seconds
		if(m_ReckoningTick+Server()->TickSpeed()*3 < Server()->Tick() || mem_comp(&Predicted, &Current, sizeof(CNetObj_Character)) != 0)
		{
			m_ReckoningTick = Server()->Tick();
			m_SendCore = m_Core;
			m_ReckoningCore = m_Core;
		}
	}
}

void CCharacter::TickPaused()
{
	++m_AttackTick;
	++m_ReckoningTick;
	if(m_LastAction != -1)
		++m_LastAction;
	if(m_aWeapons[m_ActiveWeapon].m_AmmoRegenStart > -1)
		++m_aWeapons[m_ActiveWeapon].m_AmmoRegenStart;
	if(m_EmoteStop > -1)
		++m_EmoteStop;
}

bool CCharacter::IncreaseHealth(int Amount)
{
	if(m_Health >= m_pPlayer->GetStartHealth()) 
		return false;

	const int OldHealth = m_Health;
	Amount = clamp(Amount, 1, Amount);
	m_Health = clamp(m_Health+Amount, 0, m_pPlayer->GetStartHealth());
	m_pPlayer->ShowInformationStats();

	if(IsAlive())
	{
		char aBuf[32];
		str_format(aBuf, sizeof(aBuf), "%d Health", m_Health - OldHealth);
		GS()->SendMmoPotion(m_Core.m_Pos, aBuf, true);
	}
	return true;
}

bool CCharacter::IncreaseMana(int Amount)
{
	if(m_Mana >= m_pPlayer->GetStartMana())
		return false;

	const int OldMana = m_Mana;
	Amount = clamp(Amount, 1, Amount);
	m_Mana = clamp(m_Mana + Amount, 0, m_pPlayer->GetStartMana());
	m_pPlayer->ShowInformationStats();

	if(IsAlive())
	{
		char aBuf[32];
		str_format(aBuf, sizeof(aBuf), "%d Mana", m_Mana - OldMana);
		GS()->SendMmoPotion(m_Core.m_Pos, aBuf, true);
	}
	return true;
}

void CCharacter::Die(int Killer, int Weapon)
{
	// change to safe zone
	const int ClientID = m_pPlayer->GetCID();
	if(Weapon != WEAPON_WORLD && !GS()->IsDungeon())
	{
		m_pPlayer->UpdateTempData(0, 0);
		const int SafezoneWorldID = GS()->GetRespawnWorld();
		if(SafezoneWorldID >= 0 && !m_pPlayer->IsBot() && GS()->m_apPlayers[Killer])
		{
			// potion resurrection
			if(m_pPlayer->GetItem(itPotionResurrection).IsEquipped())
				GS()->Mmo()->Item()->UseItem(ClientID, itPotionResurrection, 1);
			else
			{
				GS()->Chat(ClientID, "You are dead, you will be treated in {STR}", Server()->GetWorldName(SafezoneWorldID));
				m_pPlayer->GetTempData().TempActiveSafeSpawn = true;
			}
		}
	}

	m_Alive = false;
	m_pPlayer->m_PlayerTick[TickState::Respawn] = Server()->Tick() + Server()->TickSpeed() / 2;
	if(m_pPlayer->GetBotType() == BotsTypes::TYPE_BOT_MOB)
	{
		int SubBotID = m_pPlayer->GetBotSub();
		m_pPlayer->m_PlayerTick[TickState::Respawn] = Server()->Tick()+BotJob::MobBot[SubBotID].RespawnTick*Server()->TickSpeed();
	}

	// a nice sound
	GS()->m_pController->OnCharacterDeath(this, GS()->m_apPlayers[Killer], Weapon);
	GS()->CreateSound(m_Pos, SOUND_PLAYER_DIE);
	m_pPlayer->ClearTalking();

	// respawn
	m_pPlayer->m_PlayerTick[TickState::Die] = Server()->Tick()/2;
	m_pPlayer->m_Spawned = true;
	GS()->m_World.RemoveEntity(this);
	GS()->m_World.m_Core.m_apCharacters[ClientID] = 0;
	GS()->CreateDeath(m_Pos, ClientID);
}

bool CCharacter::TakeDamage(vec2 Force, int Dmg, int From, int Weapon)
{
	m_Core.m_Vel += Force;
	if(length(m_Core.m_Vel) > 32.0f)
		m_Core.m_Vel = normalize(m_Core.m_Vel) * 32.0f;

	if(!IsAllowedPVP(From))
		return false;

	Dmg = (From == m_pPlayer->GetCID() ? max(1, Dmg/2) : max(1, Dmg));

	CPlayer* pFrom = GS()->GetPlayer(From);
	if(From != m_pPlayer->GetCID() && pFrom->GetCharacter())
	{
		if(pFrom->GetCharacter()->m_ActiveWeapon == WEAPON_GUN)
			Dmg += pFrom->GetAttributeCount(Stats::StGunPower, true);
		else if(pFrom->GetCharacter()->m_ActiveWeapon == WEAPON_SHOTGUN)
			Dmg += pFrom->GetAttributeCount(Stats::StShotgunPower, true);
		else if(pFrom->GetCharacter()->m_ActiveWeapon == WEAPON_GRENADE)
			Dmg += pFrom->GetAttributeCount(Stats::StGrenadePower, true);
		else if(pFrom->GetCharacter()->m_ActiveWeapon == WEAPON_LASER)
			Dmg += pFrom->GetAttributeCount(Stats::StRiflePower, true);
		else
			Dmg += pFrom->GetAttributeCount(Stats::StHammerPower, true);

		const int EnchantBonus = pFrom->GetAttributeCount(Stats::StStrength, true);
		Dmg += EnchantBonus;
	
		// Vampirism пополнить здоровье
		int TempInt = pFrom->GetAttributeCount(Stats::StVampirism, true);
		if(rand()%5000 < clamp(TempInt, 100, 3000))
		{
			pFrom->GetCharacter()->IncreaseHealth(max(1, Dmg/2));
			GS()->SendEmoticon(From, EMOTICON_DROP);
		}
		
		// lucky пропустить урон
		TempInt = pFrom->GetAttributeCount(Stats::StLucky, true);
		if(!pFrom->IsBot() && rand()%5000 < clamp(TempInt, 100, 3000))
		{
			Dmg = 0;
			GS()->SendEmoticon(From, EMOTICON_HEARTS);
		}
		
		// Критический урон
		TempInt = pFrom->GetAttributeCount(Stats::StDirectCriticalHit, true);
		if(!pFrom->IsBot() && rand()%5000 < clamp(TempInt, 100, 2200))
		{
			const int CritDamage = pFrom->GetAttributeCount(Stats::StCriticalHit, true);
			Dmg = Dmg*2+(CritDamage+rand()%9);
			pFrom->GetCharacter()->SetEmote(EMOTE_ANGRY, 2);
			GS()->SendEmoticon(From, EMOTICON_EXCLAMATION);
		}

		// Фикс быстрого уйбиства спредом игроков
		if(pFrom->GetCharacter()->m_ActiveWeapon != WEAPON_HAMMER && 
			distance(m_Core.m_Pos, pFrom->GetCharacter()->m_Core.m_Pos) < ms_PhysSize+90.0f)
			Dmg = max(1, Dmg/3);

		// TODO: Impl it
		GiveRandomMobEffect(From);
	}

	int OldHealth = m_Health;
	if(Dmg)
	{
		m_Health -= Dmg;
		m_pPlayer->ShowInformationStats();
	}
	
	// create healthmod indicator
	GS()->CreateDamage(m_Pos, m_pPlayer->GetCID(), OldHealth-m_Health, false);

	if(From != m_pPlayer->GetCID())
		GS()->CreatePlayerSound(From, SOUND_HIT);

	// перекинуть на BotAI
	if(m_pPlayer->IsBot())
	{
		bool IsDie = (bool)(m_Health <= 0);
		return IsDie;
	}

	// автозелье здоровья
	if(m_Health <= m_pPlayer->GetStartHealth()/3)
	{
		if(!m_pPlayer->CheckEffect("RegenHealth") && m_pPlayer->GetItem(itPotionHealthRegen).IsEquipped())
			GS()->Mmo()->Item()->UseItem(m_pPlayer->GetCID(), itPotionHealthRegen, 1);
	}

	// Проверка при смерти если игрок погиб
	if(m_Health <= 0)
	{
		m_Health = 0;
		m_pPlayer->ShowInformationStats();
		Die(From, Weapon);
		if (From != m_pPlayer->GetCID() && pFrom->GetCharacter()) 
			pFrom->GetCharacter()->SetEmote(EMOTE_HAPPY, 1);
		return false;
	}

	if (Dmg > 2) 
		GS()->CreateSound(m_Pos, SOUND_PLAYER_PAIN_LONG);
	else 
		GS()->CreateSound(m_Pos, SOUND_PLAYER_PAIN_SHORT);

	m_EmoteType = EMOTE_PAIN;
	m_EmoteStop = Server()->Tick() + 500 * Server()->TickSpeed() / 1000;
	return true;
}

void CCharacter::Snap(int SnappingClient)
{
	if(NetworkClipped(SnappingClient) || !m_pPlayer->IsActiveSnappingBot(SnappingClient))
		return;
	
	CNetObj_Character *pCharacter = static_cast<CNetObj_Character *>(Server()->SnapNewItem(NETOBJTYPE_CHARACTER, m_pPlayer->GetCID(), sizeof(CNetObj_Character)));
	if(!pCharacter)
		return;

	// write down the m_Core
	if(!m_ReckoningTick || GS()->m_World.m_Paused)
	{
		// no dead reckoning when paused because the client doesn't know
		// how far to perform the reckoning
		pCharacter->m_Tick = 0;
		m_Core.Write(pCharacter);
	}
	else
	{
		pCharacter->m_Tick = m_ReckoningTick;
		m_SendCore.Write(pCharacter);
	}

	// set emote
	if (m_EmoteStop < Server()->Tick())
	{
		m_EmoteType = EMOTE_NORMAL;
		m_EmoteStop = -1;
	}

	pCharacter->m_Emote = m_EmoteType;	
	if(250 - ((Server()->Tick() - m_LastAction) % (250)) < 5)
		pCharacter->m_Emote = EMOTE_BLINK;

	pCharacter->m_AmmoCount = 0;
	pCharacter->m_Health = 0;
	pCharacter->m_Armor = 0;
	pCharacter->m_TriggeredEvents = m_TriggeredEvents;
	pCharacter->m_Weapon = m_ActiveWeapon;
	pCharacter->m_AttackTick = m_AttackTick;
	pCharacter->m_Direction = m_Input.m_Direction;

	if(m_pPlayer->GetCID() == SnappingClient || SnappingClient == -1)
	{
		const float HealthTranslate = (float)m_Health / (float)m_pPlayer->GetStartHealth() * 10.0f;
		const float ManaTranslate = (float)m_Mana / (float)m_pPlayer->GetStartMana() * 10.0f;
		pCharacter->m_Health = m_Health <= 0 ? 0 : clamp((int)HealthTranslate, 1, 10);
		pCharacter->m_Armor = m_Mana <= 0 ? 0 : clamp((int)ManaTranslate, 1, 10);
		if(m_aWeapons[m_ActiveWeapon].m_Ammo > 0)
		{
			const int StartAmmo = 10 + m_pPlayer->GetAttributeCount(Stats::StAmmo);
			const float AmmoTranslate = (float)m_aWeapons[m_ActiveWeapon].m_Ammo / (float)StartAmmo * 10.0f;
			pCharacter->m_AmmoCount = m_aWeapons[m_ActiveWeapon].m_Ammo <= 0 ? 0 : clamp((int)AmmoTranslate, 1, 10);
		}
	}
}

void CCharacter::PostSnap()
{
	m_TriggeredEvents = 0;
}

// another function
void CCharacter::HandleTilesets()
{
	// get index tileset char pos
	const int Index = GS()->Collision()->GetParseTilesAt(m_Core.m_Pos.x, m_Core.m_Pos.y);
	if(!m_pPlayer->IsBot() && GS()->Mmo()->OnPlayerHandleTile(this, Index))
		return;

	// инвенты
	for (int i = TILE_CLEAR_EVENTS; i <= TILE_EVENT_HEALTH; i++)
	{
		if (m_pHelper->TileEnter(Index, i))
			SetEvent(i);
		else if (m_pHelper->TileExit(Index, i)) {}
	}

	// вода
	if (m_pHelper->TileEnter(Index, TILE_WATER))
		GS()->CreateDeath(m_Pos, m_pPlayer->GetCID());
	else if (m_pHelper->TileExit(Index, TILE_WATER))
		GS()->CreateDeath(m_Pos, m_pPlayer->GetCID());
}

void CCharacter::HandleEvents() 
{
	if(m_pPlayer->IsBot())
		return;

	// тайл инвента
	int ClientID = m_pPlayer->GetCID();
	if(m_Event == TILE_EVENT_PARTY)
	{
		SetEmote(EMOTE_HAPPY, 1);
		if(rand() % 50 == 0)
		{
			GS()->SendEmoticon(ClientID, 1 + rand() % 2);
			GS()->CreateDeath(m_Core.m_Pos, ClientID);
		}
	}

	// тайл инвента
	if(m_Event == TILE_EVENT_LIKE)
	{
		SetEmote(EMOTE_HAPPY, 1);
		if(Server()->Tick() % Server()->TickSpeed() == 0)
			GS()->SendMmoEffect(m_Core.m_Pos, EFFECT_SPASALON);
	}
}

void CCharacter::GiveRandomMobEffect(int FromID)
{
	CPlayer* pFrom = GS()->GetPlayer(FromID);
	if(!pFrom || !pFrom->IsBot() || pFrom->GetBotType() != BotsTypes::TYPE_BOT_MOB || BotJob::MobBot[pFrom->GetBotSub()].Effect[0] == '\0')
		return;
	m_pPlayer->GiveEffect(BotJob::MobBot[pFrom->GetBotSub()].Effect, 3+rand()%3, 40);
}

bool CCharacter::InteractiveHammer(vec2 Direction, vec2 ProjStartPos)
{
	if (m_pPlayer->IsBot())
		return false;

	// подбор предмета
	if (GS()->TakeItemCharacter(m_pPlayer->GetCID()))
		return true;

	// работа
	vec2 PosJob = vec2(0, 0);
	CJobItems* pJobItem = (CJobItems*)GameWorld()->ClosestEntity(m_Pos, 15, CGameWorld::ENTTYPE_JOBITEMS, 0);
	if(pJobItem)
	{
		PosJob = pJobItem->GetPos();
		pJobItem->Work(m_pPlayer->GetCID());
		m_ReloadTimer = Server()->TickSpeed() / 3;
		return true;
	}
	return false;
}

void CCharacter::InteractiveGun(vec2 Direction, vec2 ProjStartPos)
{

}

void CCharacter::InteractiveShotgun(vec2 Direction, vec2 ProjStartPos)
{

}

void CCharacter::InteractiveGrenade(vec2 Direction, vec2 ProjStartPos)
{

}

void CCharacter::InteractiveRifle(vec2 Direction, vec2 ProjStartPos)
{

}

void CCharacter::HandleTunning()
{
	// тюнинг воды
	CTuningParams* pTuningParams = &m_pPlayer->m_NextTuningParams;
	if(m_pHelper->BoolIndex(TILE_WATER))
	{
		pTuningParams->m_Gravity = -0.05f;
		pTuningParams->m_GroundFriction = 0.95f;
		pTuningParams->m_GroundControlSpeed = 250.0f / Server()->TickSpeed();
		pTuningParams->m_GroundControlAccel = 1.5f;
		pTuningParams->m_AirFriction = 0.95f;
		pTuningParams->m_AirControlSpeed = 250.0f / Server()->TickSpeed();
		pTuningParams->m_AirControlAccel = 1.5f;
		SetEmote(EMOTE_BLINK, 1);	
	}

	// smooth skip collision
	if(m_Core.m_LostData)
		pTuningParams->m_PlayerCollision = 0;
		
	// боты тюнинг
	if(m_pPlayer->IsBot())
		return;

	// режим полета
	if(m_pPlayer->m_Flymode && m_pPlayer->GetEquippedItem(EQUIP_WINGS) > 0)
	{
		pTuningParams->m_Gravity = 0.00f;
		pTuningParams->m_HookLength = 700.0f;
		pTuningParams->m_AirControlAccel = 1.5f;

		vec2 Direction = vec2(m_Core.m_Input.m_TargetX, m_Core.m_Input.m_TargetY);
		m_Core.m_Vel += Direction * 0.001f;
	}

	// effects
	if(m_pPlayer->CheckEffect("Slowdown"))
	{
		pTuningParams->m_Gravity = 0.35f;
		pTuningParams->m_GroundFriction = 0.30f;
		pTuningParams->m_GroundControlSpeed = 60.0f / Server()->TickSpeed();
		pTuningParams->m_GroundControlAccel = 0.4f;
		pTuningParams->m_GroundJumpImpulse = 3.0f;
		pTuningParams->m_AirFriction = 0.4f;
		pTuningParams->m_AirControlSpeed = 60.0f / Server()->TickSpeed();
		pTuningParams->m_AirControlAccel = 0.4f;
		pTuningParams->m_AirJumpImpulse = 3.0f;
		pTuningParams->m_HookLength = 0.0f;
	}
	if(Server()->Tick() % Server()->TickSpeed() == 0)
	{
		if(m_pPlayer->CheckEffect("Fire"))
		{
			const int ExplodeDamageSize = kurosio::translate_to_procent_rest(m_pPlayer->GetStartHealth(), 3);
			GS()->CreateExplosion(m_Core.m_Pos, m_pPlayer->GetCID(), WEAPON_GRENADE, 0);
			TakeDamage(vec2(0, 0), ExplodeDamageSize, m_pPlayer->GetCID(), WEAPON_SELF);
		}
		if(m_pPlayer->CheckEffect("Poison"))
		{
			const int PoisonSize = kurosio::translate_to_procent_rest(m_pPlayer->GetStartHealth(), 3);
			TakeDamage(vec2(0, 0), PoisonSize, m_pPlayer->GetCID(), WEAPON_SELF);
		}
		if(m_pPlayer->CheckEffect("RegenHealth"))
		{
			const int RestoreHealth = kurosio::translate_to_procent_rest(m_pPlayer->GetStartHealth(), 3);
			IncreaseHealth(RestoreHealth);
		}
		if(m_pPlayer->CheckEffect("RegenMana"))
		{
			const int RestoreMana = kurosio::translate_to_procent_rest(m_pPlayer->GetStartMana(), 5);
			IncreaseMana(RestoreMana);
		}
	}

}

void CCharacter::UpdateEquipingStats(int ItemID)
{
	if(!IsAlive() || !m_pPlayer->IsAuthed())
		return;

	if(m_Health > m_pPlayer->GetStartHealth())
	{
		GS()->Chat(m_pPlayer->GetCID(), "Your health has been lowered.");
		GS()->Chat(m_pPlayer->GetCID(), "You may have removed equipment that gave it away.");
		m_Health = m_pPlayer->GetStartHealth();
	}

	const ItemJob::ItemInformation pInformationItem = GS()->GetItemInfo(ItemID);
	if((pInformationItem.Function >= EQUIP_HAMMER && pInformationItem.Function <= EQUIP_RIFLE))
		m_pPlayer->GetCharacter()->GiveWeapon(pInformationItem.Function, 3);

	if(pInformationItem.GetStatsBonus(Stats::StAmmoRegen) > 0)
		m_AmmoRegen = m_pPlayer->GetAttributeCount(Stats::StAmmoRegen, true);
}

void CCharacter::HandleAuthedPlayer()
{
	if(!IsAlive() || !m_pPlayer->IsAuthed())
		return;

	// мана прибавка
	if(m_Mana < m_pPlayer->GetStartMana() && Server()->Tick() % (Server()->TickSpeed() * 3) == 0)
	{
		IncreaseMana(m_pPlayer->GetStartMana() / 20);
		m_pPlayer->ShowInformationStats();
	}
}

bool CCharacter::IsAllowedPVP(int FromID)
{
	if(FromID < 0 || FromID >= MAX_CLIENTS)
		return false;

	CPlayer* pFrom = GS()->m_apPlayers[FromID];
	if(!pFrom || !pFrom->GetCharacter())
		return false;
	// anti pvp no allowed damage
	if(m_NoAllowDamage || pFrom->GetCharacter()->m_NoAllowDamage)
		return false;
	// anti pvp for bots
	if(m_pPlayer->IsBot() && pFrom->IsBot())
		return false;
	// pvp only for mobs
	if((m_pPlayer->IsBot() && m_pPlayer->GetBotType() != BotsTypes::TYPE_BOT_MOB) || (pFrom->IsBot() && pFrom->GetBotType() != BotsTypes::TYPE_BOT_MOB))
		return false;

	// players anti pvp
	if(!m_pPlayer->IsBot() && !pFrom->IsBot())
	{
		// anti settings pvp
		if(!pFrom->GetItem(itModePVP).IsEquipped() || !m_pPlayer->GetItem(itModePVP).IsEquipped())
			return false;

		// anti pvp on safe world or dungeon
		if(!GS()->IsAllowedPVP() || GS()->IsDungeon())
			return false;

		// anti pvp for guild players
		if(pFrom->Acc().GuildID > 0 && pFrom->Acc().GuildID == m_pPlayer->Acc().GuildID)
			return false;

		// anti pvp disallow
		if(g_Config.m_SvStrongAntiPVP <= 0)
			return false;
	}

	// anti pvp strong
	const int FromAttributeLevel = pFrom->GetLevelDisciple(AtributType::AtDps) + pFrom->GetLevelDisciple(AtributType::AtTank) + pFrom->GetAttributeCount(AtributType::AtHealer);
	const int PlayerAttributeLevel = m_pPlayer->GetLevelDisciple(AtributType::AtDps) + m_pPlayer->GetLevelDisciple(AtributType::AtTank) + m_pPlayer->GetAttributeCount(AtributType::AtHealer);
	if(!pFrom->IsBot() && !m_pPlayer->IsBot() && ((FromAttributeLevel - PlayerAttributeLevel > g_Config.m_SvStrongAntiPVP) || (PlayerAttributeLevel - FromAttributeLevel > g_Config.m_SvStrongAntiPVP)))
		return false;

	return true;
}

bool CCharacter::IsLockedWorld()
{
	if(m_Alive && (Server()->Tick() % Server()->TickSpeed() * 3) == 0  && m_pPlayer->IsAuthed())
	{
		const int NecessaryQuest = GS()->Mmo()->WorldSwap()->GetNecessaryQuest();
		if(NecessaryQuest > 0 && !GS()->Mmo()->Quest()->IsCompletedQuest(m_pPlayer->GetCID(), NecessaryQuest))
		{
			const int CheckHouseID = GS()->Mmo()->Member()->GetPosHouseID(m_Core.m_Pos);
			if(CheckHouseID <= 0)
			{
				m_pPlayer->GetTempData().TempTeleportX = m_pPlayer->GetTempData().TempTeleportY = -1;
				GS()->Chat(m_pPlayer->GetCID(), "This chapter is still closed, you magically transported first zone!");
				m_pPlayer->ChangeWorld(NEWBIE_ZERO_WORLD);
				return true;
			}
		}
	}
	return false;
}

bool CCharacter::CheckFailMana(int Mana)
{
	if(m_Mana < Mana)
	{
		GS()->SBL(m_pPlayer->GetCID(), BroadcastPriority::BROADCAST_GAME_WARNING, 100, "No mana for use this or for maintenance.");
		return true;
	}

	m_Mana -= Mana;
	if(m_Mana <= m_pPlayer->GetStartMana() / 5  && !m_pPlayer->CheckEffect("RegenMana") && m_pPlayer->GetItem(itPotionManaRegen).IsEquipped())
		GS()->Mmo()->Item()->UseItem(m_pPlayer->GetCID(), itPotionManaRegen, 1);

	m_pPlayer->ShowInformationStats();
	return false;	
}

void CCharacter::ChangePosition(vec2 NewPos)
{
	if(!m_Alive)
		return;

	GS()->SendMmoEffect(m_Core.m_Pos, EFFECT_TELEPORT);
	GS()->SendMmoEffect(NewPos, EFFECT_TELEPORT);
	GS()->CreateDeath(m_Core.m_Pos, m_pPlayer->GetCID());
	GS()->CreateDeath(NewPos, m_pPlayer->GetCID());
	m_Core.m_Pos = NewPos;
}

void CCharacter::ResetDoorPos()
{
	m_Core.m_Pos = m_OlderPos;
	m_Core.m_Vel = vec2(0, 0);
	if (m_Core.m_Jumped >= 2)
		m_Core.m_Jumped = 1;
}

// talking system
bool CCharacter::StartConversation(CPlayer *pTarget)
{
	if (!m_pPlayer || m_pPlayer->IsBot() || !pTarget->IsBot())
		return false;

	CPlayerBot* pTargetBot = static_cast<CPlayerBot*>(pTarget);
	if (!pTargetBot || pTargetBot->GetBotType() == BotsTypes::TYPE_BOT_MOB || !pTargetBot->IsActiveSnappingBot(m_pPlayer->GetCID()))
		return false;

	return true;
}

// decoration player's
void CCharacter::CreateSnapProj(int SnapID, int Count, int TypeID, bool Dynamic, bool Projectile)
{
	CSnapFull* pSnapItem = (CSnapFull*)GameWorld()->ClosestEntity(m_Pos, 300, CGameWorld::ENTTYPE_SNAPEFFECT, 0);
	if(pSnapItem && pSnapItem->GetOwner() == m_pPlayer->GetCID())
	{
		pSnapItem->AddItem(Count, TypeID, Projectile, Dynamic, SnapID);
		return;
	}
	new CSnapFull(&GS()->m_World, m_Core.m_Pos, SnapID, m_pPlayer->GetCID(), Count, TypeID, Dynamic, Projectile);
}

void CCharacter::RemoveSnapProj(int Count, int SnapID, bool Effect)
{
	CSnapFull* pSnapItem = (CSnapFull*)GameWorld()->ClosestEntity(m_Pos, 300, CGameWorld::ENTTYPE_SNAPEFFECT, 0);
	if(pSnapItem && pSnapItem->GetOwner() == m_pPlayer->GetCID())
	{
		pSnapItem->RemoveItem(Count, SnapID, Effect);
		return;
	}
}
