/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#include <generated/server_data.h>
#include <game/server/gamecontext.h>

#include "character.h"
#include "pickup.h"

CPickup::CPickup(CGameWorld *pGameWorld, int Type, vec2 Pos)
: CEntity(pGameWorld, CGameWorld::ENTTYPE_PICKUP, Pos, PickupPhysSize)
{
	m_Type = Type;

	Reset();
	GameWorld()->InsertEntity(this);
}

void CPickup::Reset()
{
	if (g_pData->m_aPickups[m_Type].m_Spawndelay > 0)
		m_SpawnTick = Server()->Tick() + Server()->TickSpeed() * g_pData->m_aPickups[m_Type].m_Spawndelay;
	else
		m_SpawnTick = -1;
}

void CPickup::Tick()
{
	// wait for respawn
	if(m_SpawnTick > 0)
	{
		if(Server()->Tick() > m_SpawnTick)
		{
			// respawn
			m_SpawnTick = -1;

			if(m_Type == PICKUP_GRENADE || m_Type == PICKUP_SHOTGUN || m_Type == PICKUP_LASER)
				GS()->CreateSound(m_Pos, SOUND_WEAPON_SPAWN);
		}
		else
			return;
	}

	CCharacter *pChr = (CCharacter *)GS()->m_World.ClosestEntity(m_Pos, 20.0f, CGameWorld::ENTTYPE_CHARACTER, 0);
	if(pChr && pChr->IsAlive())
	{
		bool Picked = false;
		switch (m_Type)
		{
			case PICKUP_HEALTH:
				if(pChr->IncreaseHealth(1))
				{
					Picked = true;
					GS()->CreateSound(m_Pos, SOUND_PICKUP_HEALTH);
				}
				break;

			case PICKUP_ARMOR:
				if(pChr->IncreaseMana(1))
				{
					Picked = true;
					GS()->CreateSound(m_Pos, SOUND_PICKUP_ARMOR);
				}
				break;

			case PICKUP_GRENADE:
				if(pChr->GiveWeapon(WEAPON_GRENADE, g_pData->m_Weapons.m_aId[WEAPON_GRENADE].m_Maxammo))
				{
					Picked = true;
					GS()->CreateSound(m_Pos, SOUND_PICKUP_GRENADE);
					if(pChr->GetPlayer())
						GS()->SendWeaponPickup(pChr->GetPlayer()->GetCID(), WEAPON_GRENADE);
				}
				break;
			case PICKUP_SHOTGUN:
				if(pChr->GiveWeapon(WEAPON_SHOTGUN, g_pData->m_Weapons.m_aId[WEAPON_SHOTGUN].m_Maxammo))
				{
					Picked = true;
					GS()->CreateSound(m_Pos, SOUND_PICKUP_SHOTGUN);
					if(pChr->GetPlayer())
						GS()->SendWeaponPickup(pChr->GetPlayer()->GetCID(), WEAPON_SHOTGUN);
				}
				break;
			case PICKUP_LASER:
				if(pChr->GiveWeapon(WEAPON_LASER, g_pData->m_Weapons.m_aId[WEAPON_LASER].m_Maxammo))
				{
					Picked = true;
					GS()->CreateSound(m_Pos, SOUND_PICKUP_SHOTGUN);
					if(pChr->GetPlayer())
						GS()->SendWeaponPickup(pChr->GetPlayer()->GetCID(), WEAPON_LASER);
				}
				break;

			default:
				break;
		};

		if(Picked)
		{
			char aBuf[256];
			str_format(aBuf, sizeof(aBuf), "pickup player='%d:%s' item=%d",
				pChr->GetPlayer()->GetCID(), Server()->ClientName(pChr->GetPlayer()->GetCID()), m_Type);
			GS()->Console()->Print(IConsole::OUTPUT_LEVEL_DEBUG, "game", aBuf);
			int RespawnTime = g_pData->m_aPickups[m_Type].m_Respawntime;
			if(RespawnTime >= 0)
				m_SpawnTick = Server()->Tick() + Server()->TickSpeed() * RespawnTime;
		}
	}
}

void CPickup::TickPaused()
{
	if(m_SpawnTick != -1)
		++m_SpawnTick;
}

void CPickup::Snap(int SnappingClient)
{
	if(m_SpawnTick != -1 || NetworkClipped(SnappingClient))
		return;

	CNetObj_Pickup *pP = static_cast<CNetObj_Pickup *>(Server()->SnapNewItem(NETOBJTYPE_PICKUP, GetID(), sizeof(CNetObj_Pickup)));
	if(!pP)
		return;

	pP->m_X = (int)m_Pos.x;
	pP->m_Y = (int)m_Pos.y;
	pP->m_Type = m_Type;
}
