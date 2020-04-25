/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#include <engine/shared/config.h>

#include <base/math.h>
#include <base/vmath.h>
#include <generated/protocol.h>
#include <game/server/gamecontext.h>

#include "dropingbonuses.h"

CDropingBonuses::CDropingBonuses(CGameWorld *pGameWorld, vec2 Pos, vec2 Vel, int Type, int Count)
: CEntity(pGameWorld, CGameWorld::ENTTYPE_DROPBONUS, Pos)
{
	m_Pos = Pos;
	m_Vel = Vel;
	m_Count = Count;
	m_Type = Type;
	m_FlashTimer = 0;
	m_Flashing = false;
	m_StartTick = Server()->Tick();
	m_LifeSpan = Server()->TickSpeed() * 10;

	GameWorld()->InsertEntity(this);
}

void CDropingBonuses::Tick()
{
	m_LifeSpan--;
	if (m_LifeSpan < 150)
	{
		// effect
		m_FlashTimer--;
		if (m_FlashTimer > 5)
			m_Flashing = true;
		else
		{
			m_Flashing = false;
			if (m_FlashTimer <= 0)
				m_FlashTimer = 10;
		}

		// delete object
		if (m_LifeSpan < 0)
		{
			GS()->CreatePlayerSpawn(m_Pos);
			GS()->m_World.DestroyEntity(this);
			return;
		}
	}
	
	m_Vel.y += 0.5f;

	bool Grounded = false;
	if (GS()->Collision()->CheckPoint(m_Pos.x + 12, m_Pos.y + 12 + 5))
		Grounded = true;
	if (GS()->Collision()->CheckPoint(m_Pos.x - 12, m_Pos.y + 12 + 5))
		Grounded = true;

	if (Grounded)
		m_Vel.x *= 0.8f;
	else
		m_Vel.x *= 0.99f;

	GS()->Collision()->MoveBox(&m_Pos, &m_Vel, vec2(24.0f, 24.0f), 0.4f);


	CCharacter *pChar = (CCharacter*)GameWorld()->ClosestEntity(m_Pos, 16.0f, CGameWorld::ENTTYPE_CHARACTER, 0);
	if(pChar && pChar->GetPlayer() && !pChar->GetPlayer()->IsBot())
	{
		if(m_Type == PICKUP_HEALTH)
		{
			GS()->CreateSound(m_Pos, SOUND_PICKUP_HEALTH);
		}

		if(m_Type == PICKUP_ARMOR)
		{
			pChar->GetPlayer()->AddExp(m_Count);
			GS()->CreateSound(m_Pos, SOUND_PICKUP_ARMOR);
		} 
		GS()->m_World.DestroyEntity(this);
		return;			
	}
}

void CDropingBonuses::TickPaused()
{
	m_StartTick++;
}

void CDropingBonuses::Snap(int SnappingClient)
{
	float Ct = (Server()->Tick()-m_StartTick)/(float)Server()->TickSpeed();
	
	if(m_Flashing || NetworkClipped(SnappingClient))
		return;

	// проверка клиента если чекнут дальше не рисуем
	if(GS()->CheckClient(SnappingClient))
	{
		CNetObj_MmoPickup* pObj = static_cast<CNetObj_MmoPickup*>(Server()->SnapNewItem(NETOBJTYPE_MMOPICKUP, GetID(), sizeof(CNetObj_MmoPickup)));
		if (!pObj)
			return;

		pObj->m_X = (int)m_Pos.x;
		pObj->m_Y = (int)m_Pos.y;
		pObj->m_Type = MMO_PICKUP_EXPERIENCE;
		pObj->m_Angle = 0;
		return;
	}

	CNetObj_Pickup *pP = static_cast<CNetObj_Pickup *>(Server()->SnapNewItem(NETOBJTYPE_PICKUP, GetID(), sizeof(CNetObj_Pickup)));
	if(!pP)
		return;

	pP->m_X = (int)m_Pos.x;
	pP->m_Y = (int)m_Pos.y;
	pP->m_Type = m_Type;
}