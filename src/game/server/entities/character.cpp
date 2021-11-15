/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#include "character.h"
#include <engine/shared/config.h>
#include <generated/server_data.h>

#include <game/server/gamecontext.h>

#include "laser.h"
#include "projectile.h"

#include <game/server/mmocore/Components/Bots/BotData.h>
#include <game/server/mmocore/Components/Guilds/GuildCore.h>
#include <game/server/mmocore/Components/Houses/HouseCore.h>
#include <game/server/mmocore/Components/Quests/QuestCore.h>
#include <game/server/mmocore/Components/Worlds/WorldSwapCore.h>

#include <game/server/mmocore/GameEntities/jobitems.h>
#include <game/server/mmocore/GameEntities/snapfull.h>

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

MACRO_ALLOC_POOL_ID_IMPL(CCharacter, MAX_CLIENTS * ENGINE_MAX_WORLDS + MAX_CLIENTS)

CCharacter::CCharacter(CGameWorld *pWorld)
: CEntity(pWorld, CGameWorld::ENTTYPE_CHARACTER, vec2(0, 0), ms_PhysSize)
{
	m_pHelper = new TileHandle();
	m_DoorHit = false;
	m_Health = 0;
	m_Armor = 0;
	m_TriggeredEvents = 0;
}

CCharacter::~CCharacter()
{
	delete m_pHelper;
	m_pHelper = nullptr;
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
	m_SkipDamage = false;
	m_Core.m_SkipCollideTees = false;
	m_Event = TILE_CLEAR_EVENTS;
	m_Core.m_WorldID = m_pPlayer->GetPlayerWorldID();
	if(!m_pPlayer->IsBot())
	{
		m_pPlayer->m_MoodState = m_pPlayer->GetMoodState();
		GS()->Mmo()->Quest()->UpdateArrowStep(m_pPlayer);
		GS()->Mmo()->Quest()->AcceptNextStoryQuestStep(m_pPlayer);

		m_AmmoRegen = m_pPlayer->GetAttributeCount(Stats::StAmmoRegen, true);
		GS()->ResetVotes(m_pPlayer->GetCID(), m_pPlayer->m_OpenVoteMenu);
		m_pPlayer->ShowInformationStats();
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

bool CCharacter::IsGrounded() const
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
	if(m_pPlayer->GetTempData().m_TempDecoractionID > 0)
	{
		const int DecoID = m_pPlayer->GetTempData().m_TempDecoractionID;
		const int InteractiveType = m_pPlayer->GetTempData().m_TempDecorationType;
		m_pPlayer->GetTempData().m_TempDecoractionID = -1;
		m_pPlayer->GetTempData().m_TempDecorationType = -1;
		if(m_pPlayer->GetItem(DecoID).m_Value <= 0 || GS()->GetItemInfo(DecoID).m_Type != ItemType::TYPE_DECORATION)
			return false;

		if (InteractiveType == DECORATIONS_HOUSE)
		{
			const int HouseID = GS()->Mmo()->House()->PlayerHouseID(m_pPlayer);
			if (GS()->Mmo()->House()->AddDecorationHouse(DecoID, HouseID, GetMousePos()))
			{
				GS()->Chat(ClientID, "You added {STR}, to your house!", GS()->GetItemInfo(DecoID).GetName());
				m_pPlayer->GetItem(DecoID).Remove(1);
				GS()->ResetVotes(ClientID, MenuList::MENU_HOUSE_DECORATION);
				return true;
			}
		}
		else if (InteractiveType == DECORATIONS_GUILD_HOUSE)
		{
			const int GuildID = m_pPlayer->Acc().m_GuildID;
			if (GS()->Mmo()->Member()->AddDecorationHouse(DecoID, GuildID, GetMousePos()))
			{
				GS()->Chat(ClientID, "You added {STR}, to your guild house!", GS()->GetItemInfo(DecoID).GetName());
				m_pPlayer->GetItem(DecoID).Remove(1);
				GS()->ResetVotes(ClientID, MenuList::MENU_GUILD_HOUSE_DECORATION);
				return true;
			}
		}

		GS()->Chat(ClientID, "Distance House and Decoration maximal {INT} block!", g_Config.m_SvLimitDecoration);
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
	if(m_pPlayer->GetSkill(SkillMasterWeapon).IsLearned())
		FullAuto = true;

	bool WillFire = false;
	if(CountInput(m_LatestPrevInput.m_Fire, m_LatestInput.m_Fire).m_Presses)
		WillFire = true;

	if(FullAuto && (m_LatestInput.m_Fire&1) && m_aWeapons[m_ActiveWeapon].m_Ammo)
		WillFire = true;

	if(!WillFire)
		return;

	const bool IsBot = m_pPlayer->IsBot();
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

	const vec2 Direction = normalize(vec2(m_LatestInput.m_TargetX, m_LatestInput.m_TargetY));
	const vec2 ProjStartPos = m_Pos+Direction*GetProximityRadius()*0.75f;
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
			const int Num = GS()->m_World.FindEntities(ProjStartPos, GetProximityRadius()* Radius, (CEntity**)apEnts, MAX_CLIENTS, CGameWorld::ENTTYPE_CHARACTER);
			for (int i = 0; i < Num; ++i)
			{
				CCharacter* pTarget = apEnts[i];
				if((pTarget == this) || GS()->Collision()->IntersectLineWithInvisible(ProjStartPos, pTarget->m_Pos, nullptr, nullptr))
					continue;

				// talking wth bot
				if (!StartedTalking && StartConversation(pTarget->GetPlayer()))
				{
					m_pPlayer->SetTalking(pTarget->GetPlayer()->GetCID(), true);
					GS()->CreatePlayerSound(m_pPlayer->GetCID(), SOUND_PLAYER_SPAWN);
					GS()->CreateHammerHit(ProjStartPos);
					StartedTalking = true;
					Hits = true;

					const int BotID = pTarget->GetPlayer()->GetBotID();
					GS()->ChatFollow(m_pPlayer->GetCID(), "You start dialogue with {STR}!", DataBotInfo::ms_aDataBot[BotID].m_aNameBot);
					continue;
				}

				if (pTarget->m_Core.m_SkipCollideTees)
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
			const int ShotSpread = min(2 + m_pPlayer->GetAttributeCount(Stats::StSpreadShotgun), 36);
			CMsgPacker Msg(NETMSGTYPE_SV_EXTRAPROJECTILE);
			Msg.AddInt(ShotSpread);
			for (int i = 1; i <= ShotSpread; ++i)
			{
				const float Spreading = ((0.0058945f*(9.0f*ShotSpread)/2)) - (0.0058945f*(9.0f*i));
				const float a = GetAngle(Direction) + Spreading;
				const float Speed = (float)GS()->Tuning()->m_ShotgunSpeeddiff + frandom()*0.2f;
				new CProjectile(GameWorld(), WEAPON_SHOTGUN, m_pPlayer->GetCID(), ProjStartPos,
					vec2(cosf(a), sinf(a))*Speed,
					(int)(Server()->TickSpeed() * GS()->Tuning()->m_ShotgunLifetime),
					g_pData->m_Weapons.m_Shotgun.m_pBase->m_Damage, IsExplosive, 0, 15, WEAPON_SHOTGUN);
			}
			Server()->SendMsg(&Msg, MSGFLAG_VITAL, m_pPlayer->GetCID());
			GS()->CreateSound(m_Pos, SOUND_SHOTGUN_FIRE);
		} break;

		case WEAPON_GRENADE:
		{
			const int ShotSpread = min(1 + m_pPlayer->GetAttributeCount(Stats::StSpreadGrenade), 21);
			CMsgPacker Msg(NETMSGTYPE_SV_EXTRAPROJECTILE);
			Msg.AddInt(ShotSpread);
			for (int i = 1; i < ShotSpread; ++i)
			{
				const float Spreading = ((0.0058945f*(9.0f*ShotSpread)/2)) - (0.0058945f*(9.0f*i));
				const float a = GetAngle(Direction) + Spreading;
				new CProjectile(GameWorld(), WEAPON_GRENADE, m_pPlayer->GetCID(), ProjStartPos,
					vec2(cosf(a), sinf(a)),
					(int)(Server()->TickSpeed()*GS()->Tuning()->m_GrenadeLifetime),
					g_pData->m_Weapons.m_Grenade.m_pBase->m_Damage, true, 0, SOUND_GRENADE_EXPLODE, WEAPON_GRENADE);
			}
			Server()->SendMsg(&Msg, MSGFLAG_VITAL, m_pPlayer->GetCID());
			GS()->CreateSound(m_Pos, SOUND_GRENADE_FIRE);
		} break;

		case WEAPON_LASER:
		{
			const int ShotSpread = min(1 + m_pPlayer->GetAttributeCount(Stats::StSpreadRifle), 36);
			for (int i = 1; i < ShotSpread; ++i)
			{
				const float Spreading = ((0.0058945f*(9.0f*ShotSpread)/2)) - (0.0058945f*(9.0f*i));
				const float a = GetAngle(Direction) + Spreading;
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
		const int ReloadArt = m_pPlayer->GetAttributeCount(Stats::StDexterity);
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

bool CCharacter::GiveWeapon(int Weapon, int GiveAmmo)
{
	const int WeaponID = clamp(Weapon, (int)WEAPON_HAMMER, (int)WEAPON_NINJA);
	const bool IsHammer = (bool)(WeaponID == WEAPON_HAMMER);
	if(m_pPlayer->GetEquippedItemID(WeaponID) <= 0 && !IsHammer)
	{
		if(RemoveWeapon(WeaponID) && WeaponID == m_ActiveWeapon)
			m_ActiveWeapon = m_aWeapons[m_LastWeapon].m_Got ? m_LastWeapon : (int)WEAPON_HAMMER;
		return false;
	}

	const int MaximalAmmo = 10 + m_pPlayer->GetAttributeCount(Stats::StAmmo);
	if(m_aWeapons[WeaponID].m_Ammo >= MaximalAmmo)
		return false;

	const int GotAmmo = (int)(IsHammer ? -1 : (m_aWeapons[WeaponID].m_Got ? min(m_aWeapons[WeaponID].m_Ammo + GiveAmmo, MaximalAmmo) : min(GiveAmmo, MaximalAmmo)));
	m_aWeapons[WeaponID].m_Got = true;
	m_aWeapons[WeaponID].m_Ammo = GotAmmo;
	return true;
}

bool CCharacter::RemoveWeapon(int Weapon)
{
	const bool Succesful = m_aWeapons[Weapon].m_Got;
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

	HandleTuning();
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

	HandleWeapons();

	if (m_pPlayer->IsBot() || IsLockedWorld())
		return;

	HandleAuthedPlayer();
	HandleTilesets();
	m_Core.m_SkipCollideTees = false;
}

void CCharacter::TickDefered()
{
	if(!IsAlive())
		return;

	if(m_DoorHit)
	{
		ResetDoorPos();
		m_DoorHit = false;
	}

	// apply drag velocity when the player is not firing ninja
	// and set it back to 0 for the next tick
	if(m_ActiveWeapon != WEAPON_NINJA)
		m_Core.AddDragVelocity();
	m_Core.ResetDragVelocity();

	CCharacterCore::CParams PlayerTunningParams(&m_pPlayer->m_NextTuningParams);
	if(m_pPlayer->IsBot())
	{
		m_Core.Move(&PlayerTunningParams);
		m_Core.Quantize();
		m_Pos = m_Core.m_Pos;
		return;
	}

	// advance the dummy
	{
		CWorldCore TempWorld;
		m_ReckoningCore.Init(&TempWorld, GS()->Collision());
		m_ReckoningCore.Tick(false, &PlayerTunningParams);
		m_ReckoningCore.Move(&PlayerTunningParams);
		m_ReckoningCore.Quantize();
	}

	m_Core.Move(&PlayerTunningParams);
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
		if(m_ReckoningTick + Server()->TickSpeed() * 3 < Server()->Tick() || mem_comp(&Predicted, &Current, sizeof(CNetObj_Character)) != 0)
		{
			m_ReckoningTick = Server()->Tick();
			m_SendCore = m_Core;
			m_ReckoningCore = m_Core;
		}
	}
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
		GS()->CreateTextEffect(m_Core.m_Pos, aBuf, TEXTEFFECT_FLAG_POTION|TEXTEFFECT_FLAG_ADDING);
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
		GS()->CreateTextEffect(m_Core.m_Pos, aBuf, TEXTEFFECT_FLAG_POTION|TEXTEFFECT_FLAG_ADDING);
	}
	return true;
}

void CCharacter::Die(int Killer, int Weapon)
{
	// change to safe zone
	m_Alive = false;
	const int ClientID = m_pPlayer->GetCID();
	if(Weapon != WEAPON_WORLD && !GS()->IsDungeon())
	{
		m_pPlayer->ClearEffects();
		m_pPlayer->UpdateTempData(0, 0);
		const int SafezoneWorldID = GS()->GetRespawnWorld();
		if(SafezoneWorldID >= 0 && !m_pPlayer->IsBot() && GS()->m_apPlayers[Killer])
		{
			// potion resurrection
			CItemData& pItemPlayer = m_pPlayer->GetItem(itPotionResurrection);
			if(pItemPlayer.IsEquipped())
			{
				pItemPlayer.Use(1);
			}
			else
			{
				GS()->Chat(ClientID, "You are dead, you will be treated in {STR}", Server()->GetWorldName(SafezoneWorldID));
				m_pPlayer->GetTempData().m_TempSafeSpawn = true;
			}
		}
	}

	m_pPlayer->m_aPlayerTick[TickState::Respawn] = Server()->Tick() + Server()->TickSpeed() / 2;
	if(m_pPlayer->GetBotType() == BotsTypes::TYPE_BOT_MOB)
	{
		const int SubBotID = m_pPlayer->GetBotSub();
		m_pPlayer->m_aPlayerTick[TickState::Respawn] = Server()->Tick() + MobBotInfo::ms_aMobBot[SubBotID].m_RespawnTick*Server()->TickSpeed();
	}

	// a nice sound
	GS()->m_pController->OnCharacterDeath(this, GS()->m_apPlayers[Killer], Weapon);
	GS()->CreateSound(m_Pos, SOUND_PLAYER_DIE);
	m_pPlayer->ClearTalking();

	// respawn
	m_pPlayer->m_aPlayerTick[TickState::Die] = Server()->Tick()/2;
	m_pPlayer->m_Spawned = true;
	GS()->m_World.RemoveEntity(this);
	GS()->m_World.m_Core.m_apCharacters[ClientID] = nullptr;
	GS()->CreateDeath(m_Pos, ClientID);
}

bool CCharacter::TakeDamage(vec2 Force, int Dmg, int From, int Weapon)
{
	// force
	m_Core.m_Vel += Force;
	const float NormalizeVel = GS()->IsDungeon() ? 16.0f : 32.0f;
	if(length(m_Core.m_Vel) > NormalizeVel)
		m_Core.m_Vel = normalize(m_Core.m_Vel) * NormalizeVel;

	// check disallow damage
	if(!IsAllowedPVP(From))
		return false;

	Dmg = (From == m_pPlayer->GetCID() ? max(1, Dmg/2) : max(1, Dmg));

	int CritDamage = 0;
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

		// vampirism replenish your health
		int TempInt = pFrom->GetAttributeCount(Stats::StVampirism, true);
		if(min(8.0f + (float)TempInt * 0.0015f, 30.0f) > frandom() * 100.0f)
		{
			pFrom->GetCharacter()->IncreaseHealth(max(1, Dmg/2));
			GS()->SendEmoticon(From, EMOTICON_DROP);
		}

		// miss out on damage
		TempInt = pFrom->GetAttributeCount(Stats::StLucky, true);
		if(min(5.0f + (float)TempInt * 0.0015f, 20.0f) > frandom() * 100.0f)
		{
			GS()->SendEmoticon(From, EMOTICON_HEARTS);
			GS()->CreateTextEffect(m_Core.m_Pos, "MISS", TEXTEFFECT_FLAG_MISS);
			return false;
		}

		// critical damage
		TempInt = pFrom->GetAttributeCount(Stats::StDirectCriticalHit, true);
		if(Dmg && !pFrom->IsBot() && min(8.0f + (float)TempInt * 0.0015f, 30.0f) > frandom() * 100.0f)
		{
			CritDamage = 100 + max(pFrom->GetAttributeCount(Stats::StCriticalHit, true), 1);
			const float CritDamageFormula = (float)Dmg + ((float)CritDamage * ((float)Dmg / 100.0f));
			const float CritRange = (CritDamageFormula + (CritDamageFormula / 2.0f) / 2.0f);
			Dmg = (int)CritDamageFormula + random_int()%(int)CritRange;
			
			pFrom->GetCharacter()->SetEmote(EMOTE_ANGRY, 2);
			GS()->SendEmoticon(From, EMOTICON_EXCLAMATION);
		}

		// fix quick killer spread players
		if(pFrom->GetCharacter()->m_ActiveWeapon != WEAPON_HAMMER &&
			distance(m_Core.m_Pos, pFrom->GetCharacter()->m_Core.m_Pos) < ms_PhysSize+90.0f)
			Dmg = max(1, Dmg/3);

		// give effects from player or bot to who got damage
		pFrom->GetCharacter()->GiveRandomEffects(m_pPlayer->GetCID());
	}

	const int OldHealth = m_Health;
	if(Dmg)
	{
		GS()->m_pController->OnCharacterDamage(pFrom, m_pPlayer, min(Dmg, m_Health));
		m_Health -= Dmg;
		m_pPlayer->ShowInformationStats(BroadcastPriority::GAME_INFORMATION);
	}

	// create healthmod indicator
	GS()->CreateDamage(m_Pos, m_pPlayer->GetCID(), OldHealth-m_Health, (bool)(CritDamage > 0), false);

	if(From != m_pPlayer->GetCID())
		GS()->CreatePlayerSound(From, SOUND_HIT);

	// verify death
	if(m_Health <= 0)
	{
		if(From != m_pPlayer->GetCID() && pFrom->GetCharacter())
			pFrom->GetCharacter()->SetEmote(EMOTE_HAPPY, 1);

		// do not kill the bot it is still running in CCharacterBotAI::TakeDamage
		if(m_pPlayer->IsBot())
			return false;

		m_Health = 0;
		Die(From, Weapon);
		return false;
	}

	// health recovery potion
	if(!m_pPlayer->IsBot() && m_Health <= m_pPlayer->GetStartHealth() / 3)
	{
		CItemData& pItemPlayer = m_pPlayer->GetItem(itPotionHealthRegen);
		if(!m_pPlayer->IsActiveEffect("RegenHealth") && pItemPlayer.IsEquipped())
			pItemPlayer.Use(1);
	}

	const bool IsHardDamage = (CritDamage > 0);
	GS()->CreateSound(m_Pos, IsHardDamage ? (int)SOUND_PLAYER_PAIN_LONG : (int)SOUND_PLAYER_PAIN_SHORT);
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

void CCharacter::HandleTilesets()
{
	if(!m_Alive)
		return;

	// get index tileset char pos component items
	const int Index = GS()->Collision()->GetParseTilesAt(m_Core.m_Pos.x, m_Core.m_Pos.y);
	if(!m_pPlayer->IsBot() && GS()->Mmo()->OnPlayerHandleTile(this, Index))
		return;

	// next for all bots & players
	for (int i = TILE_CLEAR_EVENTS; i <= TILE_EVENT_HEALTH; i++)
	{
		if (m_pHelper->TileEnter(Index, i))
			SetEvent(i);
		else if (m_pHelper->TileExit(Index, i)) {}
	}

	// water effect enter exit
	const bool EnterExitWaterTile = m_pHelper->TileEnter(Index, TILE_WATER) || m_pHelper->TileExit(Index, TILE_WATER);
	if (EnterExitWaterTile)
		GS()->CreateDeath(m_Pos, m_pPlayer->GetCID());
}

void CCharacter::HandleEvents()
{
	if(m_Event == TILE_EVENT_PARTY)
	{
		SetEmote(EMOTE_HAPPY, 1);
		if(random_int() % 50 == 0)
		{
			GS()->SendEmoticon(m_pPlayer->GetCID(), 1 + random_int() % 2);
			GS()->CreateDeath(m_Core.m_Pos, m_pPlayer->GetCID());
		}
	}

	if(m_Event == TILE_EVENT_LIKE)
	{
		SetEmote(EMOTE_HAPPY, 1);
		if(Server()->Tick() % Server()->TickSpeed() == 0)
			GS()->CreateEffect(m_Core.m_Pos, EFFECT_SPASALON);
	}
}

void CCharacter::GiveRandomEffects(int To)
{
	//CPlayer* pPlayerTo = GS()->GetPlayer(To);
	//if(!pPlayerTo && To != m_pPlayer->GetCID())
	//	return;

	// Here effects ( buffs ) from player for TO
}

bool CCharacter::InteractiveHammer(vec2 Direction, vec2 ProjStartPos)
{
	if (m_pPlayer->IsBot())
		return false;

	if (GS()->TakeItemCharacter(m_pPlayer->GetCID()))
		return true;

	CJobItems* pJobItem = (CJobItems*)GameWorld()->ClosestEntity(m_Pos, 15, CGameWorld::ENTTYPE_JOBITEMS, nullptr);
	if(pJobItem)
	{
		pJobItem->Work(m_pPlayer->GetCID());
		m_ReloadTimer = Server()->TickSpeed() / 3;
		return true;
	}
	return false;
}
/*
void CCharacter::InteractiveGun(vec2 Direction, vec2 ProjStartPos)
{
	return;
}

void CCharacter::InteractiveShotgun(vec2 Direction, vec2 ProjStartPos)
{
	return;
}

void CCharacter::InteractiveGrenade(vec2 Direction, vec2 ProjStartPos)
{
	return;
}

void CCharacter::InteractiveRifle(vec2 Direction, vec2 ProjStartPos)
{
	return;
}
*/
void CCharacter::HandleTuning()
{
	CTuningParams* pTuningParams = &m_pPlayer->m_NextTuningParams;

	// skip collision
	if(m_Core.m_SkipCollideTees)
		pTuningParams->m_PlayerCollision = 0;

	// behavior mobs
	const int MobID = m_pPlayer->GetBotSub();
	if(m_pPlayer->GetBotType() == BotsTypes::TYPE_BOT_NPC || m_pPlayer->GetBotType() == BotsTypes::TYPE_BOT_QUEST)
	{
		// walk effect
		pTuningParams->m_GroundControlSpeed = 5.0f;
		pTuningParams->m_GroundControlAccel = 1.0f;
	}
	else if(m_pPlayer->GetBotType() == BotsTypes::TYPE_BOT_MOB)
	{
		// effect slower
		if(str_comp(MobBotInfo::ms_aMobBot[MobID].m_aBehavior, "Slime") == 0)
		{
			pTuningParams->m_Gravity = 0.25f;
			pTuningParams->m_GroundJumpImpulse = 8.0f;

			pTuningParams->m_AirFriction = 0.75f;
			pTuningParams->m_AirControlAccel = 1.0f;
			pTuningParams->m_AirControlSpeed = 3.75f;
			pTuningParams->m_AirJumpImpulse = 8.0f;

			pTuningParams->m_HookFireSpeed = 30.0f;
			pTuningParams->m_HookDragAccel = 1.5f;
			pTuningParams->m_HookDragSpeed = 8.0f;
			pTuningParams->m_PlayerHooking = 0;
		}
	}

	// flight mode
	if(m_pPlayer->m_Flymode && m_pPlayer->GetEquippedItemID(EQUIP_WINGS) > 0)
	{
		pTuningParams->m_Gravity = 0.00f;
		pTuningParams->m_HookLength = 700.0f;
		pTuningParams->m_AirControlAccel = 1.5f;

		const vec2 Direction = vec2(m_Core.m_Input.m_TargetX, m_Core.m_Input.m_TargetY);
		m_Core.m_Vel += Direction * 0.001f;
	}

	// water
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

	// potions and buffs are different
	HandleBuff(pTuningParams);
}

void CCharacter::HandleBuff(CTuningParams* TuningParams)
{
	if(m_pPlayer->IsActiveEffect("Slowdown"))
	{
		TuningParams->m_Gravity = 0.35f;
		TuningParams->m_GroundFriction = 0.30f;
		TuningParams->m_GroundControlSpeed = 60.0f / Server()->TickSpeed();
		TuningParams->m_GroundControlAccel = 0.4f;
		TuningParams->m_GroundJumpImpulse = 3.0f;
		TuningParams->m_AirFriction = 0.4f;
		TuningParams->m_AirControlSpeed = 60.0f / Server()->TickSpeed();
		TuningParams->m_AirControlAccel = 0.4f;
		TuningParams->m_AirJumpImpulse = 3.0f;
		TuningParams->m_HookLength = 0.0f;
	}

	// poisons
	if(Server()->Tick() % Server()->TickSpeed() == 0)
	{
		if(m_pPlayer->IsActiveEffect("Fire"))
		{
			const int ExplodeDamageSize = translate_to_percent_rest(m_pPlayer->GetStartHealth(), 3);
			GS()->CreateExplosion(m_Core.m_Pos, m_pPlayer->GetCID(), WEAPON_GRENADE, 0);
			TakeDamage(vec2(0, 0), ExplodeDamageSize, m_pPlayer->GetCID(), WEAPON_SELF);
		}
		if(m_pPlayer->IsActiveEffect("Poison"))
		{
			const int PoisonSize = translate_to_percent_rest(m_pPlayer->GetStartHealth(), 3);
			TakeDamage(vec2(0, 0), PoisonSize, m_pPlayer->GetCID(), WEAPON_SELF);
		}
		if(m_pPlayer->IsActiveEffect("RegenHealth"))
		{
			const int RestoreHealth = translate_to_percent_rest(m_pPlayer->GetStartHealth(), 3);
			IncreaseHealth(RestoreHealth);
		}
		if(m_pPlayer->IsActiveEffect("RegenMana"))
		{
			const int RestoreMana = translate_to_percent_rest(m_pPlayer->GetStartMana(), 5);
			IncreaseMana(RestoreMana);
		}
	}
}

void CCharacter::UpdateEquipingStats(int ItemID)
{
	if(!m_Alive || !m_pPlayer->IsAuthed())
		return;

	if(m_Health > m_pPlayer->GetStartHealth())
	{
		GS()->Chat(m_pPlayer->GetCID(), "Your health has been lowered.");
		GS()->Chat(m_pPlayer->GetCID(), "You may have removed equipment that gave it away.");
		m_Health = m_pPlayer->GetStartHealth();
	}

	const CItemDataInfo pInformationItem = GS()->GetItemInfo(ItemID);
	if((pInformationItem.m_Function >= EQUIP_HAMMER && pInformationItem.m_Function <= EQUIP_RIFLE))
		m_pPlayer->GetCharacter()->GiveWeapon(pInformationItem.m_Function, 3);

	if(pInformationItem.GetInfoEnchantStats(Stats::StAmmoRegen) > 0)
		m_AmmoRegen = m_pPlayer->GetAttributeCount(Stats::StAmmoRegen, true);
}

void CCharacter::HandleAuthedPlayer()
{
	if(!m_pPlayer->IsAuthed())
		return;

	// recovery mana
	if(m_Mana < m_pPlayer->GetStartMana() && Server()->Tick() % (Server()->TickSpeed() * 3) == 0)
		IncreaseMana(m_pPlayer->GetStartMana() / 20);

	HandleEvents();
}

bool CCharacter::IsAllowedPVP(int FromID) const
{
	CPlayer* pFrom = GS()->GetPlayer(FromID, false, true);
	if(!pFrom || (m_SkipDamage || pFrom->GetCharacter()->m_SkipDamage) || (m_pPlayer->IsBot() && pFrom->IsBot()))
		return false;

	// pvp only for mobs
	if((m_pPlayer->IsBot() && m_pPlayer->GetBotType() != BotsTypes::TYPE_BOT_MOB) || (pFrom->IsBot() && pFrom->GetBotType() != BotsTypes::TYPE_BOT_MOB))
		return false;

	// disable damage on invisible wall
	if(GS()->Collision()->GetParseTilesAt(GetPos().x, GetPos().y) == TILE_INVISIBLE_WALL
		|| GS()->Collision()->GetParseTilesAt(pFrom->GetCharacter()->GetPos().x, pFrom->GetCharacter()->GetPos().y) == TILE_INVISIBLE_WALL)
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
		if(pFrom->Acc().m_GuildID > 0 && pFrom->Acc().m_GuildID == m_pPlayer->Acc().m_GuildID)
			return false;
	}

	// anti pvp strong
	const int FromAttributeLevel = pFrom->GetLevelAllAttributes();
	const int PlayerAttributeLevel = m_pPlayer->GetLevelAllAttributes();
	if(!pFrom->IsBot() && !m_pPlayer->IsBot() && ((FromAttributeLevel - PlayerAttributeLevel > g_Config.m_SvStrongAntiPVP) || (PlayerAttributeLevel - FromAttributeLevel > g_Config.m_SvStrongAntiPVP)))
		return false;

	return true;
}

bool CCharacter::IsLockedWorld()
{
	if(m_Alive && (Server()->Tick() % Server()->TickSpeed() * 3) == 0  && m_pPlayer->IsAuthed())
	{
		const int NecessaryQuest = GS()->Mmo()->WorldSwap()->GetNecessaryQuest();
		if(NecessaryQuest > 0 && !m_pPlayer->GetQuest(NecessaryQuest).IsComplected())
		{
			const int CheckHouseID = GS()->Mmo()->Member()->GetPosHouseID(m_Core.m_Pos);
			if(CheckHouseID <= 0)
			{
				m_pPlayer->GetTempData().m_TempTeleportX = m_pPlayer->GetTempData().m_TempTeleportY = -1;
				GS()->Chat(m_pPlayer->GetCID(), "This chapter is still closed, you magically transported first zone!");
				m_pPlayer->ChangeWorld(MAIN_WORLD_ID);
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
		GS()->Broadcast(m_pPlayer->GetCID(), BroadcastPriority::GAME_WARNING, 100, "No mana for use this or for maintenance.");
		return true;
	}

	m_Mana -= Mana;
	if(m_Mana <= m_pPlayer->GetStartMana() / 5 && !m_pPlayer->IsActiveEffect("RegenMana") && m_pPlayer->GetItem(itPotionManaRegen).IsEquipped())
		m_pPlayer->GetItem(itPotionManaRegen).Use(1);

	m_pPlayer->ShowInformationStats(BroadcastPriority::GAME_INFORMATION);
	return false;
}

void CCharacter::ChangePosition(vec2 NewPos)
{
	if(!m_Alive)
		return;

	GS()->CreateEffect(m_Core.m_Pos, EFFECT_TELEPORT);
	GS()->CreateEffect(NewPos, EFFECT_TELEPORT);
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

	// skip if not NPC, or it is not drawn
	CPlayerBot* pTargetBot = static_cast<CPlayerBot*>(pTarget);
	if (!pTargetBot || pTargetBot->GetBotType() == BotsTypes::TYPE_BOT_MOB || !pTargetBot->IsActiveSnappingBot(m_pPlayer->GetCID()))
		return false;
	return true;
}

// decoration player's
void CCharacter::CreateSnapProj(int SnapID, int Value, int TypeID, bool Dynamic, bool Projectile)
{
	CSnapFull* pSnapItem = (CSnapFull*)GameWorld()->ClosestEntity(m_Pos, 300, CGameWorld::ENTTYPE_SNAPEFFECT, nullptr);
	if(pSnapItem && pSnapItem->GetOwner() == m_pPlayer->GetCID())
	{
		pSnapItem->AddItem(Value, TypeID, Projectile, Dynamic, SnapID);
		return;
	}
	new CSnapFull(&GS()->m_World, m_Core.m_Pos, SnapID, m_pPlayer->GetCID(), Value, TypeID, Dynamic, Projectile);
}

void CCharacter::RemoveSnapProj(int Value, int SnapID, bool Effect)
{
	CSnapFull* pSnapItem = (CSnapFull*)GameWorld()->ClosestEntity(m_Pos, 300, CGameWorld::ENTTYPE_SNAPEFFECT, nullptr);
	if(pSnapItem && pSnapItem->GetOwner() == m_pPlayer->GetCID())
	{
		pSnapItem->RemoveItem(Value, SnapID, Effect);
		return;
	}
}
