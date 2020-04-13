/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#include <engine/shared/config.h>

#include <generated/server_data.h>
#include <game/server/gamecontext.h>

#include "character.h"
#include "laser.h"
#include "projectile.h"

#include <game/server/mmocore/questai.h>
#include "games/clicktee.h"
#include "snapfull.h"
#include "jobitems.h"

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
	// another function
	m_pHelper = new CHelperCharacter(this);

	m_Health = 0;
	m_Armor = 0;
	m_TriggeredEvents = 0;
}

CCharacter::~CCharacter()
{
	delete m_pHelper;
	GS()->m_World.m_Core.m_apCharacters[m_pPlayer->GetCID()] = NULL;
}

// Получить оформление для SnapFull
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

	// helper
	m_Mana = 0;
	m_OldPos = Pos;
	m_NoAllowDamage = false;
	m_Event = TILE_CLEAREVENTS;

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

	m_Core.m_WorldID = GS()->CheckPlayerMessageWorldID(m_pPlayer->GetCID());
	if(!m_pPlayer->IsBot())
	{
		m_pPlayer->AddInformationStats();
		m_AmmoRegen = m_pPlayer->GetAttributeCount(Stats::StAmmoRegen, true);

		CreateQuestsSteps();
	}
	GS()->VResetVotes(m_pPlayer->GetCID(), m_pPlayer->m_OpenVoteMenu);
	GS()->m_pController->OnCharacterSpawn(this);
	return true;
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
	int ClientID = m_pPlayer->GetCID();
	int DecoID = CGS::InteractiveSub[ClientID].TempID;
	GS()->ClearInteractiveSub(ClientID);

	if(DecoID > 0 && m_pPlayer->GetItem(DecoID).Count > 0 && GS()->GetItemInfo(DecoID).Type == ITEMDECORATION)
	{
		int HouseID = GS()->Mmo()->House()->PlayerHouseID(m_pPlayer);
		if(GS()->Mmo()->House()->AddDecorationHouse(DecoID, HouseID, m_pHelper->MousePos()))
		{
			GS()->Chat(ClientID, "You added {STR}, to your house!", GS()->GetItemInfo(DecoID).GetName(m_pPlayer));
			GS()->ResetVotes(ClientID, HOUSEDECORATION);
			m_pPlayer->GetItem(DecoID).Remove(1);
			return true;
		}

		GS()->Chat(ClientID, "Distance House and Decoration maximal {INT} block!", &g_Config.m_SvLimitDecoration);
		GS()->Chat(ClientID, "Setting object reset, use repeat!");
		GS()->ResetVotes(ClientID, HOUSEDECORATION);
		return true;
	}
	return false;
}

void CCharacter::FireWeapon()
{
	if(m_ReloadTimer != 0)
		return;

	DoWeaponSwitch();
	vec2 Direction = normalize(vec2(m_LatestInput.m_TargetX, m_LatestInput.m_TargetY));

	bool FullAuto = false;
	if(m_pPlayer->GetAttributeCount(Stats::StAutoFire))
		FullAuto = true;

	// check if we gonna fire
	bool WillFire = false;
	if(CountInput(m_LatestPrevInput.m_Fire, m_LatestInput.m_Fire).m_Presses)
		WillFire = true;

	if(FullAuto && (m_LatestInput.m_Fire&1) && m_aWeapons[m_ActiveWeapon].m_Ammo)
		WillFire = true;

	if(!WillFire)
		return;

	bool IamBot = m_pPlayer->IsBot();
	int SubBotID = GetPlayer()->GetBotSub();
	if(!IamBot)
	{
		// добавить декоратив
		if(DecoInteractive())
			return;

		// если не одето
		if(m_pPlayer->GetItemEquip(m_ActiveWeapon+EQUIP_HAMMER) == -1)
		{
			// hotfix hammer non pick items
			GS()->SBL(m_pPlayer->GetCID(), PRELEGENDARY, 150, _("You need buy this weapon or module and equip!"), NULL);
			if(m_ActiveWeapon == WEAPON_HAMMER)
				GS()->TakeItemCharacter(m_pPlayer->GetCID());
			return;
		}

		// check for ammo
		if(!m_aWeapons[m_ActiveWeapon].m_Ammo)
		{
			// 125ms is a magical limit of how fast a human can click
			m_ReloadTimer = 125 * Server()->TickSpeed() / 1000;
			if(m_LastNoAmmoSound+Server()->TickSpeed() <= Server()->Tick())
			{
				GS()->CreateSound(m_Pos, SOUND_WEAPON_NOAMMO);
				m_LastNoAmmoSound = Server()->Tick();
			}
			return;
		}
	}

	vec2 ProjStartPos = m_Pos+Direction*GetProximityRadius()*0.75f;
	switch(m_ActiveWeapon)
	{
		case WEAPON_HAMMER:
		{
			bool Hits = false;
			m_NumObjectsHit = 0;
			GS()->CreateSound(m_Pos, SOUND_HAMMER_FIRE);
			if(IamBot || InteractiveHammer(Direction, ProjStartPos))
			{
				CCharacter *apEnts[MAX_CLIENTS];
				int Num = GS()->m_World.FindEntities(ProjStartPos, GetProximityRadius()*(IamBot ? 0.5f : 2.5f),
					(CEntity**)apEnts, MAX_CLIENTS, CGameWorld::ENTTYPE_CHARACTER);

				for (int i = 0; i < Num; ++i)
				{
					CCharacter *pTarget = apEnts[i];

					if ((pTarget == this) || pTarget->m_Core.m_LostData || GS()->Collision()->IntersectLine(ProjStartPos, pTarget->m_Pos, NULL, NULL))
						continue;

					// set his velocity to fast upward (for now)
					if(length(pTarget->m_Pos-ProjStartPos) > 0.0f)
						GS()->CreateHammerHit(pTarget->m_Pos-normalize(pTarget->m_Pos-ProjStartPos)*GetProximityRadius()*0.5f);
					else
						GS()->CreateHammerHit(ProjStartPos);

					vec2 Dir;
					if (length(pTarget->m_Pos - m_Pos) > 0.0f)
						Dir = normalize(pTarget->m_Pos - m_Pos);
					else
						Dir = vec2(0.f, -1.f);

					pTarget->TakeDamage(vec2(0.f, -1.f) + normalize(Dir + vec2(0.f, -1.1f)) * 10.0f, Dir*-1, g_pData->m_Weapons.m_Hammer.m_pBase->m_Damage,
						m_pPlayer->GetCID(), m_ActiveWeapon);
					Hits = true;
				}
			}
			if(Hits) m_ReloadTimer = Server()->TickSpeed()/3;
		} break;

		case WEAPON_GUN:
		{
			new CProjectile(GameWorld(), WEAPON_GUN,
				m_pPlayer->GetCID(),
				ProjStartPos,
				Direction,
				(int)(Server()->TickSpeed()*GS()->Tuning()->m_GunLifetime),
				g_pData->m_Weapons.m_Gun.m_pBase->m_Damage, false, 0, -1, WEAPON_GUN);

			GS()->CreateSound(m_Pos, SOUND_GUN_FIRE);
		} break;

		case WEAPON_SHOTGUN:
		{
			int EnchantSpread = IamBot ? 2+ContextBots::MobBot[SubBotID].Spread : m_pPlayer->GetAttributeCount(Stats::StSpreadShotgun);
			int ShotSpread = 1+EnchantSpread;
			if(ShotSpread > 36)
				ShotSpread = 36;

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
					g_pData->m_Weapons.m_Shotgun.m_pBase->m_Damage, false, 0, 15, WEAPON_SHOTGUN);
			}
			Server()->SendMsg(&Msg, MSGFLAG_VITAL, m_pPlayer->GetCID());
			GS()->CreateSound(m_Pos, SOUND_SHOTGUN_FIRE);
		} break;

		case WEAPON_GRENADE:
		{
			int EnchantSpread = IamBot ? ContextBots::MobBot[SubBotID].Spread : m_pPlayer->GetAttributeCount(Stats::StSpreadGrenade);
			int ShotSpread = 1+EnchantSpread;
			if(ShotSpread > 21)
				ShotSpread = 21;
			
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
			int EnchantSpread = IamBot ? ContextBots::MobBot[SubBotID].Spread : m_pPlayer->GetAttributeCount(Stats::StSpreadRifle);
			int ShotSpread = 1+EnchantSpread;
			if(ShotSpread > 36)
				ShotSpread = 36;

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
		int ReloadArt = 0;
		if(m_pPlayer->GetSpawnBot() == SPAWNMOBS)
		{
			int SubBotID = GetPlayer()->GetBotSub();
			ReloadArt = ContextBots::MobBot[SubBotID].Health * 7;
		}
		else if(!m_pPlayer->IsBot())
			ReloadArt = m_pPlayer->GetAttributeCount(Stats::StDexterity);

		m_ReloadTimer = g_pData->m_Weapons.m_aId[m_ActiveWeapon].m_Firedelay * Server()->TickSpeed() / (1000 + ReloadArt);
	}
}

void CCharacter::HandleWeapons()
{
	// check reload timer
	if(m_ReloadTimer)
	{
		m_ReloadTimer--;
		return;
	}

	// fire Weapon, if wanted
	FireWeapon();

	// ammo regen
	int AmmoRegenTime = clamp(5000-m_AmmoRegen, 1000, 10000);
	if(AmmoRegenTime && m_aWeapons[m_ActiveWeapon].m_Ammo >= 0)
	{
		if (m_aWeapons[m_ActiveWeapon].m_AmmoRegenStart < 0)
			m_aWeapons[m_ActiveWeapon].m_AmmoRegenStart = Server()->Tick();

		if ((Server()->Tick() - m_aWeapons[m_ActiveWeapon].m_AmmoRegenStart) >= AmmoRegenTime * Server()->TickSpeed() / 1000)
		{
			int RealAmmo = 10 + m_pPlayer->GetAttributeCount(Stats::StAmmo);
			m_aWeapons[m_ActiveWeapon].m_Ammo = min(m_aWeapons[m_ActiveWeapon].m_Ammo + 1, RealAmmo);
			m_aWeapons[m_ActiveWeapon].m_AmmoRegenStart = -1;
		}
	}
	return;
}

void CCharacter::CreateQuestsSteps()
{
	int ClientID = m_pPlayer->GetCID();
	for(auto quests = QuestBase::Quests[ClientID].begin(); quests != QuestBase::Quests[ClientID].end(); quests++)
	{
		if(quests->second.Type != QUESTACCEPT) 
			continue;

		bool DontStep = false;
		vec2 Pos = GS()->Mmo()->WorldSwap()->PositionQuestBot(ClientID, quests->first);
		for(CQuestAI *pQ = (CQuestAI*) GameWorld()->FindFirst(CGameWorld::ENTTYPE_FINDQUEST); pQ; pQ = (CQuestAI *)pQ->TypeNext())
			if(pQ->GetClientID() == m_pPlayer->GetCID() && pQ->GetQuestID() == quests->first) pQ->Finish();

		if(!DontStep) new CQuestAI(GameWorld(), m_Core.m_Pos, quests->first, ClientID, Pos);
	}
}

void CCharacter::FinishQuestStep(int QuestID)
{
	int ClientID = m_pPlayer->GetCID();
	for(CQuestAI *pQ = (CQuestAI*) GameWorld()->FindFirst(CGameWorld::ENTTYPE_FINDQUEST); pQ; pQ = (CQuestAI *)pQ->TypeNext())
	{
		if(pQ->GetClientID() == m_pPlayer->GetCID() && pQ->GetQuestID() == QuestID)
		{
			pQ->Finish();
			return;
		}
	}
}

bool CCharacter::GiveWeapon(int Weapon, int GiveAmmo)
{
	int RealAmmo = 10 + m_pPlayer->GetAttributeCount(Stats::StAmmo);
	if(m_aWeapons[Weapon].m_Ammo < RealAmmo || !m_aWeapons[Weapon].m_Got)
	{
		m_aWeapons[Weapon].m_Got = true;
		m_aWeapons[Weapon].m_Ammo = min(RealAmmo, GiveAmmo);
		return true;
	}
	return false;
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
	// simulate releasing the fire button
	if((m_Input.m_Fire&1) != 0)
		m_Input.m_Fire++;
	m_Input.m_Fire &= INPUT_STATE_MASK;
	m_Input.m_Jump = 0;
	m_LatestPrevInput = m_LatestInput = m_Input;
}

void CCharacter::Tick()
{
	m_pPlayer->SetStandart(m_Health, m_Mana);
	HandleTunning();

	m_OldPos = m_Pos;
	m_Core.m_Input = m_Input;
	m_Core.Tick(true, &m_pPlayer->m_NextTuningParams);

	// handle death-tiles and leaving gamelayer
	if(GameLayerClipped(m_Pos))
	{
		Die(m_pPlayer->GetCID(), WEAPON_WORLD);
	}

	// handle Weapons
	HandleWeapons();

	if(m_pPlayer->IsBot())
		return;

	HandleAuthedPlayer();

	// another function
	HandleEvents();
	if(IsAlive())
		HandleTilesets();
}

void CCharacter::TickDefered()
{
	if(!CheckInvisibleBot())
		return;

	// если бот то отправляем мало всего по кору
	if(m_pPlayer->IsBot())
	{
		m_Core.Move();
		m_Core.Quantize();
		m_Pos = m_Core.m_Pos;		
		return;
	}
		
	// advance the dummy
	{
		CWorldCore TempWorld;
		m_ReckoningCore.Init(&TempWorld, GS()->Collision());
		m_ReckoningCore.Tick(false);
		m_ReckoningCore.Move();
		m_ReckoningCore.Quantize();
	}

	//lastsentcore
	vec2 StartPos = m_Core.m_Pos;
	vec2 StartVel = m_Core.m_Vel;
	bool StuckBefore = GS()->Collision()->TestBox(m_Core.m_Pos, vec2(28.0f, 28.0f));

	m_Core.Move();
	bool StuckAfterMove = GS()->Collision()->TestBox(m_Core.m_Pos, vec2(28.0f, 28.0f));
	m_Core.Quantize();
	bool StuckAfterQuant = GS()->Collision()->TestBox(m_Core.m_Pos, vec2(28.0f, 28.0f));
	m_Pos = m_Core.m_Pos;

	if(!StuckBefore && (StuckAfterMove || StuckAfterQuant))
	{
		// Hackish solution to get rid of strict-aliasing warning
		union
		{
			float f;
			unsigned u;
		}StartPosX, StartPosY, StartVelX, StartVelY;

		StartPosX.f = StartPos.x;
		StartPosY.f = StartPos.y;
		StartVelX.f = StartVel.x;
		StartVelY.f = StartVel.y;

		char aBuf[256];
		str_format(aBuf, sizeof(aBuf), "STUCK!!! %d %d %d %f %f %f %f %x %x %x %x",
			StuckBefore,
			StuckAfterMove,
			StuckAfterQuant,
			StartPos.x, StartPos.y,
			StartVel.x, StartVel.y,
			StartPosX.u, StartPosY.u,
			StartVelX.u, StartVelY.u);
		GS()->Console()->Print(IConsole::OUTPUT_LEVEL_DEBUG, "game", aBuf);
	}
	
	m_TriggeredEvents |= m_Core.m_TriggeredEvents;

	if(m_pPlayer->GetTeam() == TEAM_SPECTATORS)
	{
		m_Pos.x = m_Input.m_TargetX;
		m_Pos.y = m_Input.m_TargetY;
	}
	else if(m_Core.m_Death)
		Die(m_pPlayer->GetCID(), WEAPON_WORLD);

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

bool CCharacter::CheckInvisibleBot()
{
	if(!m_pPlayer->IsBot()) 
		return true;
	
	for(int i = 0 ; i < MAX_PLAYERS ; i++) 
	{
		if(!GS()->m_apPlayers[i] || distance(m_Pos, GS()->m_apPlayers[i]->m_ViewPos) > 1000.0f) continue;
		return true;
	}
	return false;
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
	if(m_Health >= m_pPlayer->GetStartHealth()) return false;

	int m_OldHealth = m_Health;
	m_Health = clamp(m_Health+Amount, 0, m_pPlayer->GetStartHealth());
	m_pPlayer->AddInformationStats();

	if(IsAlive())
	{
		char aBuf[64];
		str_format(aBuf, sizeof(aBuf), "%d Health", m_Health-m_OldHealth);
		GS()->SendMmoPotion(m_Core.m_Pos, aBuf, true);
	}
	return true;
}

bool CCharacter::IncreaseArmor(int Amount)
{
	if(m_Armor >= 10)
		return false;
	m_Armor = clamp(m_Armor+Amount, 0, 10);
	return true;
}

void CCharacter::Die(int Killer, int Weapon)
{
	// we got to wait 0.5 secs before respawning
	m_Alive = false;
	if(m_pPlayer->GetSpawnBot() == SPAWNMOBS)
	{
		int SubBotID = m_pPlayer->GetBotSub();
		m_pPlayer->m_PlayerTick[TickState::Respawn] = Server()->Tick()+ContextBots::MobBot[SubBotID].RespawnTick*Server()->TickSpeed();
	}
	else m_pPlayer->m_PlayerTick[TickState::Respawn] = Server()->Tick()+Server()->TickSpeed()/2;

	int ModeSpecial = GS()->m_pController->OnCharacterDeath(this, GS()->m_apPlayers[Killer], Weapon);

	char aBuf[256];
	str_format(aBuf, sizeof(aBuf), "kill killer='%d:%s' victim='%d:%s' weapon=%d special=%d",
		Killer, Server()->ClientName(Killer),
		m_pPlayer->GetCID(), Server()->ClientName(m_pPlayer->GetCID()), Weapon, ModeSpecial);
	GS()->Console()->Print(IConsole::OUTPUT_LEVEL_DEBUG, "game", aBuf);

	// a nice sound
	GS()->CreateSound(m_Pos, SOUND_PLAYER_DIE);

	// this is for auto respawn after 3 secs
	m_pPlayer->m_PlayerTick[TickState::Die] = Server()->Tick()/2;
	m_pPlayer->m_Spawned = true;
	GS()->m_World.RemoveEntity(this);
	GS()->m_World.m_Core.m_apCharacters[m_pPlayer->GetCID()] = 0;
	GS()->CreateDeath(m_Pos, m_pPlayer->GetCID());
}

bool CCharacter::TakeDamage(vec2 Force, vec2 Source, int Dmg, int From, int Weapon)
{
	CPlayer *pFrom = GS()->m_apPlayers[From];
	if(From >= 0 && pFrom)
	{
		// Нельзя бить с запретами
		if(From != m_pPlayer->GetCID() && ((pFrom->GetCharacter() && pFrom->GetCharacter()->m_NoAllowDamage) || m_NoAllowDamage))
			return false;
	}
	else if(From < 0 || !pFrom) 
		return false;

	m_Core.m_Vel += Force;
	if(!pFrom->IsBot() && !m_pPlayer->IsBot())
		return false;

	// Если здоровье больше максимального тогда ставим максимальное
	if(m_Health > m_pPlayer->GetStartHealth())
		m_Health = m_pPlayer->GetStartHealth();

	// Уменьшаем дамаг до минимума
	if(From == m_pPlayer->GetCID())
		Dmg = max(1, Dmg/2);

	Dmg = max(1, Dmg);
	int OldHealth = m_Health, OldArmor = m_Armor;
	if(From != m_pPlayer->GetCID() && pFrom->GetCharacter())
	{
		// Обычный урон сила урона
		int EnchantBonus = pFrom->GetAttributeCount(Stats::StStrength, true);
		Dmg += EnchantBonus;

		// Vampirism пополнить здоровье
		int TempInt = pFrom->GetAttributeCount(Stats::StVampirism, true);
		if(rand()%5000 < clamp(TempInt, 100, 3000))
		{
			pFrom->GetCharacter()->IncreaseHealth(max(1, Dmg/2));
			GS()->SendEmoticon(From, 3);
		}
		
		// lucky пропустить урон
		TempInt = pFrom->GetAttributeCount(Stats::StLucky, true);
		if(rand()%5000 < clamp(TempInt, 100, 3000))
		{
			Dmg = 0;
			GS()->SendEmoticon(From, 2);
		}
		
		// Критический урон
		TempInt = pFrom->GetAttributeCount(Stats::StDirectCriticalHit, true);
		if(rand()%5000 < clamp(TempInt, 100, 2200))
		{
			int CritDamage = pFrom->GetAttributeCount(Stats::StCriticalHit, true);
			Dmg = Dmg*2+(CritDamage+rand()%9);
			GS()->SendEmoticon(From, 1);
			pFrom->GetCharacter()->SetEmote(EMOTE_ANGRY, 2);
		}

		// Фикс быстрого уйбиства спредом игроков
		if(pFrom->GetCharacter()->m_ActiveWeapon != WEAPON_HAMMER && 
			distance(m_Core.m_Pos, pFrom->GetCharacter()->m_Core.m_Pos) < ms_PhysSize+90)
			Dmg = max(1, Dmg/3);

		GiveRandomMobEffect(From);
	}

	if(Dmg)
	{
		m_Health -= Dmg;
		m_pPlayer->SetStandart(m_Health, m_Mana);
		m_pPlayer->AddInformationStats();
	}

	// create healthmod indicator
	GS()->CreateDamage(m_Pos, m_pPlayer->GetCID(), Source, OldHealth-m_Health, OldArmor-m_Armor, From == m_pPlayer->GetCID());

	if(From != m_pPlayer->GetCID())
	{
		// Уменьшение прочности
		if(!pFrom->IsBot() && rand() % 10 == 0)
		{
			if(pFrom->DurabilityIsLowMain())
			{
				GS()->Chat(From, "One or more item's low durability.");
				GS()->Chat(From, "Find any store, and select repair items.");
			}
		}

		// Звук удара
		int64 Mask = CmaskOne(From);
		for(int i = 0; i < MAX_PLAYERS; i++)
		{
			if(GS()->m_apPlayers[i] && GS()->m_apPlayers[i]->GetTeam() == TEAM_SPECTATORS)
				Mask |= CmaskOne(i);
		}
		GS()->CreateSound(GS()->m_apPlayers[From]->m_ViewPos, SOUND_HIT, Mask);
	}

	// автозелье здоровья
	if(m_Health <= m_pPlayer->GetStartHealth()/3)
	{
		if(!m_pPlayer->CheckEffect("RegenHealth") && m_pPlayer->GetItem(itPotionHealthRegen).Count > 0 && m_pPlayer->GetItem(itPotionHealthRegen).Settings)
			GS()->Mmo()->Item()->UsedItems(m_pPlayer->GetCID(), itPotionHealthRegen, 1);
	}

	// Проверка при смерти если игрок погиб
	if(m_Health <= 0)
	{
		m_Health = 0;
		if(!m_pPlayer->IsBot()) Die(From, Weapon);
		if (From != m_pPlayer->GetCID() && pFrom->GetCharacter()) pFrom->GetCharacter()->SetEmote(EMOTE_HAPPY, 1);
		return false;
	}

	if (Dmg > 2) GS()->CreateSound(m_Pos, SOUND_PLAYER_PAIN_LONG);
	else GS()->CreateSound(m_Pos, SOUND_PLAYER_PAIN_SHORT);

	m_EmoteType = EMOTE_PAIN;
	m_EmoteStop = Server()->Tick() + 500 * Server()->TickSpeed() / 1000;
	return true;
}

void CCharacter::Snap(int SnappingClient)
{
	if(NetworkClipped(SnappingClient) || !m_pPlayer->CheckQuestSnapPlayer(SnappingClient, true))
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

	pCharacter->m_AmmoCount = 0;
	pCharacter->m_Health = 0;
	pCharacter->m_Armor = 0;
	pCharacter->m_TriggeredEvents = m_TriggeredEvents;

	pCharacter->m_Weapon = m_ActiveWeapon;
	pCharacter->m_AttackTick = m_AttackTick;

	pCharacter->m_Direction = m_Input.m_Direction;

	if(m_pPlayer->GetCID() == SnappingClient || SnappingClient == -1)
	{
		pCharacter->m_Health = m_Health > 10 ? 10 : m_Health;
		pCharacter->m_Armor = m_Armor > 10 ? 10 : m_Armor;
		if(m_aWeapons[m_ActiveWeapon].m_Ammo > 0)
			pCharacter->m_AmmoCount = m_aWeapons[m_ActiveWeapon].m_Ammo;
	}

	if(pCharacter->m_Emote == EMOTE_NORMAL)
	{
		if(250 - ((Server()->Tick() - m_LastAction)%(250)) < 5)
			pCharacter->m_Emote = EMOTE_BLINK;
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
	int Index = GS()->Collision()->GetParseTilesAt(m_Core.m_Pos.x, m_Core.m_Pos.y);
	if(GS()->Mmo()->OnPlayerHandleTile(this, Index))
		return;

	// check all tilesets
	for(int i = TILE_MEMBERHOUSE; i < MAX_TILES; i++) {
		if(m_pHelper->TileEnter(Index, i))
		{
			switch(i)
			{
				// Обновление меню и отправка информации в голосование
				case TILE_STORAGE:
				case TILE_SELLHOUSE:
				case TILE_MOTEL:
				case TILE_AUCTION:
				case TILE_LEARNSKILL:
				{
					GS()->ResetVotes(m_pPlayer->GetCID(), MAINMENU);
					GS()->Chat(m_pPlayer->GetCID(), "Information load in Vote!");
					m_Core.m_ProtectHooked = m_NoAllowDamage = true;
				} break;

				// Дом и проверка позиций и вывод информации
				case TILE_HOUSE:
				{	
					int HouseID = GS()->Mmo()->House()->GetHouse(m_Core.m_Pos);
					if(HouseID > 0)
					{
						// reset votes
						GS()->ResetVotes(m_pPlayer->GetCID(), MAINMENU);
				
						// send info house in broadcast
						int PriceHouse = GS()->Mmo()->House()->GetHousePrice(HouseID);
						GS()->SBL(m_pPlayer->GetCID(), PRERARE, 200, _("House Price: {i:price}gold \n"
							" Owner: {s:owner}.\nInformation load in vote."), "price", &PriceHouse, 
							"owner", GS()->Mmo()->House()->OwnerName(HouseID), NULL);
					}
					m_Core.m_ProtectHooked = m_NoAllowDamage = true;
				} break;

				// Проверка домов
				case TILE_MEMBERHOUSE:
				{
					int HouseID = GS()->Mmo()->Member()->GetPosHouseID(m_Core.m_Pos);
					if(HouseID > 0) GS()->ResetVotes(m_pPlayer->GetCID(), MAINMENU);

					GS()->Chat(m_pPlayer->GetCID(), "Information load in Vote!");	
					m_Core.m_ProtectHooked = m_NoAllowDamage = true;
				} break;

				// Крафт лист
				case TILE_CRAFT:
				{
					GS()->ResetVotes(m_pPlayer->GetCID(), CRAFTING);
					GS()->Chat(m_pPlayer->GetCID(), "Information load in Vote!");	
					m_Core.m_ProtectHooked = m_NoAllowDamage = true;	
				} break;

				// Список квестов
				case TILE_QUESTS:
				{
					GS()->ResetVotes(m_pPlayer->GetCID(), ADVENTUREJOURNAL);
					GS()->Chat(m_pPlayer->GetCID(), "Information load in Vote!");	
					m_Core.m_ProtectHooked = m_NoAllowDamage = true;
				} break;

				// Спа салон
				case TILE_SPASALON:
				{
					if(m_Core.m_Vel.y > 10.0f)
					{
						GS()->CreateDeath(m_Core.m_Pos, m_pPlayer->GetCID());
						GS()->CreatePlayerSound(m_pPlayer->GetCID(), SOUND_PICKUP_NINJA);		
					}
					GS()->ResetVotes(m_pPlayer->GetCID(), MAINMENU);			
					m_Core.m_ProtectHooked = m_NoAllowDamage = true;				
				} break;

				// Дать защиту урона и крюка
				case TILE_BASICRELAX:
				case TILE_CLANRELAX:

				// Инвенты установить
				case TILE_CLEAREVENTS:
				case TILE_EVENTPARTY:
					SetEvent(i);
				break;
				case TILE_WATER:
					GS()->CreateDeath(m_Pos, m_pPlayer->GetCID());
				break;
				case TILE_CLUB:
				{
					int Level = 1 + m_pPlayer->GetItem(itClubSeasonTicket).Enchant;
					GS()->SBL(m_pPlayer->GetCID(), 100000, 100, _("Welcome to Club\nYou season level {i:level}"), "level", &Level, NULL);
					SetEvent(TILE_EVENTPARTY);
					m_NoAllowDamage = true;
				}
				break;
			}
		}
		else if(m_pHelper->TileExit(Index, i))
		{
			switch(i)
			{
				// Снятие урона защиты и крюка и обновить меню
				case TILE_STORAGE:
				case TILE_CRAFT:
				case TILE_SELLHOUSE:
				case TILE_MOTEL:
				case TILE_QUESTS:
				case TILE_AUCTION:
				case TILE_LEARNSKILL:
				case TILE_HOUSE:
				case TILE_MEMBERHOUSE:
				{
					GS()->ResetVotes(m_pPlayer->GetCID(), MAINMENU);
					m_Core.m_ProtectHooked = m_NoAllowDamage = false;
				} break;

				// Спасалон выход из него
				case TILE_SPASALON:
				{
					if(m_Core.m_Vel.y < 10.0f)
					{
						GS()->CreateDeath(m_Core.m_Pos, m_pPlayer->GetCID());
						GS()->CreatePlayerSound(m_pPlayer->GetCID(), SOUND_PICKUP_NINJA);
					}
					GS()->ResetVotes(m_pPlayer->GetCID(), MAINMENU);
					m_Core.m_ProtectHooked = m_NoAllowDamage = false;
				} break;

				// Снятие урона защиты и крюка
				case TILE_BASICRELAX:
				case TILE_CLANRELAX:
				case TILE_WORLDSWAP:
				{
					m_Core.m_ProtectHooked = m_NoAllowDamage = false;
				} break;

				case TILE_WATER:
					GS()->CreateDeath(m_Pos, m_pPlayer->GetCID());
				break;

				case TILE_CLUB:
					SetEvent(TILE_CLEAREVENTS);
					m_NoAllowDamage = false;				
				break;
			}
		}
	}

	// Релакс обычный
	if(m_pHelper->BoolIndex(TILE_BASICRELAX))
	{
		if(Server()->Tick() % Server()->TickSpeed() == 0)
		{
			GS()->Mmo()->SpaAcc()->Work(m_pPlayer->GetCID());
			SetEmote(EMOTE_HAPPY, 1);
		}
	}

	// Релакс клана
	if(m_pHelper->BoolIndex(TILE_CLANRELAX))
	{
		if(Server()->Tick() % Server()->TickSpeed() == 0) 
		{
			const int HouseID = GS()->Mmo()->Member()->GetPosHouseID(m_Core.m_Pos);
			if(HouseID <= 0) return;
	
			const int MemberID = GS()->Mmo()->Member()->GetHouseMemberID(HouseID);
			const int Exp = GS()->Mmo()->Member()->GetMemberChairBonus(MemberID, EMEMBERUPGRADE::ChairNSTExperience);
			const int Money = GS()->Mmo()->Member()->GetMemberChairBonus(MemberID, EMEMBERUPGRADE::ChairNSTMoney);
			m_pPlayer->AddExp(Exp);
			m_pPlayer->AddMoney(Money);
		}
	}

	// Сезоный клуб
	if(m_pHelper->BoolIndex(TILE_CLUB))
	{
		if(!m_pPlayer->GetItem(itClubSeasonTicket).Settings)
		{
			Die(m_pPlayer->GetCID(), WEAPON_SELF);
			GS()->Chat(m_pPlayer->GetCID(), "You need {STR}!", GS()->GetItemInfo(itClubSeasonTicket).GetName(m_pPlayer));			
			return;
		}

		if(Server()->Tick() % Server()->TickSpeed() == 0)
		{
			int SnapID = m_pPlayer->GetCID()*SNAPPLAYER;
			int SeasonLevel = m_pPlayer->GetItem(itClubSeasonTicket).Enchant+1;
			m_pPlayer->AddExp(SeasonLevel*5);
			m_pPlayer->GetItem(itSeasonToken).Add(SeasonLevel);
		}
	}
}

void CCharacter::HandleEvents() { }

void CCharacter::CreateRandomDrop(int DropCID, int Random, int ItemID, int Count)
{
	if((DropCID >= 0 && DropCID < MAX_PLAYERS && 
		!GS()->m_apPlayers[DropCID]) || !GS()->m_apPlayers[DropCID]->GetCharacter() || !m_Alive)
		return;

	// рандом
	int RandomDrop = (Random == 0 ? 0 : rand()%Random);
	if(RandomDrop == 0) GS()->CreateDropItem(m_Core.m_Pos, DropCID, ItemID, Count);
	return;
}

void CCharacter::GiveRandomMobEffect(int FromID)
{
	CPlayer *pPlayer = GS()->GetPlayer(FromID, false, true);
	if(!pPlayer)
		return;

	switch(pPlayer->GetBotID())
	{
		case 18: // Robber
			m_pPlayer->GiveEffect("Poison", 3, 5);
		break;
	}
}

bool CCharacter::InteractiveHammer(vec2 Direction, vec2 ProjStartPos)
{
	// подбор предмета и если нет хамера
	GS()->TakeItemCharacter(m_pPlayer->GetCID());

	// мини игра клик тее
	ClickTee *pClick = (ClickTee*)GameWorld()->ClosestEntity(m_Pos, 180, CGameWorld::ENTTYPE_CLICKGAME, 0);
	if(pClick && pClick->IsSpawned() && !GS()->Collision()->IntersectLine(ProjStartPos, pClick->GetPos(), NULL, NULL))
	{
		SetEmote(EMOTE_ANGRY, 1);
		pClick->TakeDamage(1, m_pPlayer->GetCID(), m_Core.m_Vel);
		GS()->CreateHammerHit(pClick->GetPos());
		m_ReloadTimer = Server()->TickSpeed()/3;
	}

	// работа
	{
		vec2 PosJob = vec2(0, 0);
		CJobItems *pJobItem = (CJobItems*)GameWorld()->ClosestEntity(m_Pos, 15, CGameWorld::ENTTYPE_JOBITEMS, 0);
		if(pJobItem)
		{
			PosJob = pJobItem->GetPos();
			pJobItem->Work(m_pPlayer->GetCID());
			m_ReloadTimer = Server()->TickSpeed()/3;
			return false;
		}
	}

	// копье охотника
	if(m_pPlayer->GetItemEquip(EQUIP_HAMMER) == itSpearBronekhods) {
		int ShotSpread = 8; 
		for(int i = -ShotSpread; i <= ShotSpread; ++i) {
			float Spreading = ShotSpread*0.06f + 0.06f / i;
			float a = angle(Direction) ;
			float v = 1-(absolute(i)/(float)ShotSpread);
			float Speed = mix((float)GS()->Tuning()->m_ShotgunSpeeddiff, 1.0f, v);
			new CProjectile(GameWorld(), WEAPON_SHOTGUN, m_pPlayer->GetCID(), ProjStartPos, vec2(cosf(a), sinf(a))*Speed,
				(int)(Server()->TickSpeed()*GS()->Tuning()->m_ShotgunLifetime),
				1, false, 0, -1, WEAPON_SHOTGUN);
		}
		m_ReloadTimer = Server()->TickSpeed()/2;
		return false;
	}
	return true;
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
	CTuningParams* pTuningParams = &m_pPlayer->m_NextTuningParams;
	
	// тюнинг воды
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

	// боты тюнинг
	if(m_pPlayer->IsBot())
		return;

	// спа салон
	if(m_pHelper->BoolIndex(TILE_SPASALON))
	{
		if(Server()->Tick() % Server()->TickSpeed() == 0)
		{
			GS()->Mmo()->SpaAcc()->Work(m_pPlayer->GetCID());
			SetEmote(EMOTE_HAPPY, 1);
		}
		pTuningParams->m_Gravity = 0.00f;
		pTuningParams->m_GroundFriction = 0.95f;
		pTuningParams->m_GroundControlSpeed = 250.0f / Server()->TickSpeed();
		pTuningParams->m_GroundControlAccel = 1.5f;
		pTuningParams->m_GroundJumpImpulse = 5.0f;
		pTuningParams->m_AirFriction = 0.95f;
		pTuningParams->m_AirControlSpeed = 250.0f / Server()->TickSpeed();
		pTuningParams->m_AirControlAccel = 1.5f;
		pTuningParams->m_AirJumpImpulse = 5.0f;
		pTuningParams->m_PlayerHooking = false;
	}

	// режим полета
	if(m_pPlayer->m_Flymode && m_pPlayer->GetItemEquip(EQUIP_WINGS) > 0)
	{
		pTuningParams->m_Gravity = 0.00f;
		pTuningParams->m_HookLength = 700.0f;
		pTuningParams->m_AirControlAccel = 1.5f;

		vec2 Direction = vec2(m_Core.m_Input.m_TargetX, m_Core.m_Input.m_TargetY);
		m_Core.m_Vel += Direction*0.001f;
	}

	// тайл инвента
	if(m_Event == TILE_EVENTPARTY)
	{
		SetEmote(EMOTE_HAPPY, 1);
		if(rand()%50 == 0)
		{
			GS()->SendEmoticon(m_pPlayer->GetCID(), 1+rand()%2);
			GS()->CreateDeath(m_Core.m_Pos, m_pPlayer->GetCID());
		}
	}
}

void CCharacter::HandleAuthedPlayer()
{
	if(!IsAlive() || !m_pPlayer->IsAuthed())
		return;

	// катсцена
	if(GS()->GetWorldID() == CUTSCENEWELCOMEWORLD)
	{
		m_Core.m_Pos.y = m_OldPos.y;
		m_Core.m_Pos.x += 5;
	}

	// тик весь в одной секунде
	if(Server()->Tick() % Server()->TickSpeed() == 0)
	{
		// мана прибавка
		if(m_Mana < m_pPlayer->GetStartMana())
		{
			m_Mana += clamp(m_pPlayer->GetStartMana() / 20, 1, m_pPlayer->GetStartMana() / 20);
			m_pPlayer->AddInformationStats();
		}

		// голод эффект печали
		if(m_pPlayer->Acc().Hungry <= 0)
			GS()->SendEmoticon(m_pPlayer->GetCID(), 3);
	}
}

bool CCharacter::CheckFailMana(int Mana)
{
	if(m_Mana < Mana)
	{
		m_pPlayer->AddInBroadcast("No mana for use this or for maintenance.\n");
		return true;
	}
	m_pPlayer->AddInformationStats();
	m_Mana -= Mana;
	return false;	
}

void CCharacter::ChangePosition(vec2 NewPos)
{
	if(m_Alive)
	{
		GS()->SendMmoEffect(m_Core.m_Pos, EFFECT_TELEPORT);
		GS()->SendMmoEffect(NewPos, EFFECT_TELEPORT);
		GS()->CreateDeath(m_Core.m_Pos, m_pPlayer->GetCID());
		GS()->CreateDeath(NewPos, m_pPlayer->GetCID());
		m_Core.m_Pos = NewPos;
	}
}

void CCharacter::CreateSnapProj(int SnapID, int Count, int TypeID, bool Dynamic, bool Projectile)
{
	CSnapFull *pSnapItem = (CSnapFull*)GameWorld()->ClosestEntity(m_Pos, 300, CGameWorld::ENTTYPE_SNAPEFFECT, 0);
	if(pSnapItem && pSnapItem->GetOwner() == m_pPlayer->GetCID())
	{
		pSnapItem->AddItem(Count, TypeID, Projectile, Dynamic, SnapID);
		return;
	}

	if(m_Alive)
		new CSnapFull(&GS()->m_World, m_Core.m_Pos, SnapID, m_pPlayer->GetCID(), Count, TypeID, Dynamic, Projectile);
}

void CCharacter::RemoveSnapProj(int Count, int SnapID, bool Effect)
{
	CSnapFull *pSnapItem = (CSnapFull*)GameWorld()->ClosestEntity(m_Pos, 300, CGameWorld::ENTTYPE_SNAPEFFECT, 0);
	if(pSnapItem && pSnapItem->GetOwner() == m_pPlayer->GetCID())
	{
		pSnapItem->RemoveItem(Count, SnapID, Effect);
		return;
	}	
}