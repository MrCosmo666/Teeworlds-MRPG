/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#include <engine/shared/config.h>

#include <base/math.h>
#include <base/vmath.h>
#include <generated/protocol.h>
#include <game/server/gamecontext.h>

#include "dropingbonuses.h"

CDropingBonuses::CDropingBonuses(CGameWorld *pGameWorld, vec2 Pos, vec2 Dir, int Type, int Count)
: CEntity(pGameWorld, CGameWorld::ENTTYPE_DROPBONUS, Pos), m_Direction(vec2(Dir.x, Dir.y+2/rand()%9))
{
	m_Pos = Pos;
	m_ActualPos = Pos;
	m_ActualDir = Dir;
	m_Direction = Dir;
	m_Count = Count;

	// other var
	m_Type = Type;
	m_FlashTimer = 0;
	m_Flashing = false;
	m_StartTick = Server()->Tick();
	m_LifeSpan = Server()->TickSpeed() * 10;
	
	// create object
	GameWorld()->InsertEntity(this);
}

vec2 CDropingBonuses::GetTimePos(float Time)
{
	float Curvature = 1.25f;
	float Speed = 2750.0f;

	return CalcPos(m_Pos, m_Direction, Curvature, Speed, Time);
}

void CDropingBonuses::Tick()
{
	// check life time
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
	m_LifeSpan--;

	float Pt = (Server()->Tick()-m_StartTick-1)/(float)Server()->TickSpeed();
	float Ct = (Server()->Tick()-m_StartTick)/(float)Server()->TickSpeed();
	vec2 PrevPos = GetTimePos(Pt);
	vec2 CurPos = GetTimePos(Ct);
	
	m_ActualPos = CurPos;
	m_ActualDir = normalize(CurPos - PrevPos);
	
	vec2 LastPos;
	int Collide = GS()->Collision()->IntersectLine(PrevPos, CurPos, NULL, &LastPos);
	if(Collide)
	{			
		m_Pos = LastPos;
		m_ActualPos = m_Pos;
		m_Direction.x *= (100 - 50) / 100.0f;
		m_Direction.y *= (100 - 50) / 65.0f;
		m_StartTick = Server()->Tick();
		m_ActualDir = normalize(m_Direction);
	}

	if(m_LifeSpan < Server()->TickSpeed() * ( 10 - 1 ))
	{
		CCharacter *pChar = (CCharacter*)GameWorld()->ClosestEntity(m_Pos, 16, CGameWorld::ENTTYPE_CHARACTER, 0);
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
}

void CDropingBonuses::TickPaused()
{
	m_StartTick++;
}

void CDropingBonuses::Snap(int SnappingClient)
{
	float Ct = (Server()->Tick()-m_StartTick)/(float)Server()->TickSpeed();
	
	if(m_Flashing || NetworkClipped(SnappingClient, GetTimePos(Ct)))
		return;

	// проверка клиента если чекнут дальше не рисуем
	if(GS()->CheckClient(SnappingClient))
	{
		CNetObj_MmoItems *pObj = static_cast<CNetObj_MmoItems *>(Server()->SnapNewItem(NETOBJTYPE_MMOITEMS, GetID(), sizeof(CNetObj_MmoItems)));
		if(!pObj)
			return;

		pObj->m_X = (int)m_ActualPos.x;
		pObj->m_Y = (int)m_ActualPos.y;
		pObj->m_Type = ITEMS_EXPERIENCE;
		return;
	}

	CNetObj_Pickup *pP = static_cast<CNetObj_Pickup *>(Server()->SnapNewItem(NETOBJTYPE_PICKUP, GetID(), sizeof(CNetObj_Pickup)));
	if(!pP)
		return;

	pP->m_X = (int)m_ActualPos.x+m_ActualDir.x;
	pP->m_Y = (int)m_ActualPos.y+m_ActualDir.y;
	pP->m_Type = m_Type;
}