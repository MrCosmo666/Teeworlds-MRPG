#include <engine/shared/config.h>
#include <generated/protocol.h>
#include <game/server/gamecontext.h>
#include "event_health.h"

// Декорации
CEventHealth::CEventHealth(CGameWorld* pGameWorld, vec2 Pos) 
	: CEntity(pGameWorld, CGameWorld::ENTTYPE_EVENTS, Pos)
{
	m_LifeTime = Server()->TickSpeed() * 2;
	GameWorld()->InsertEntity(this);
}

void CEventHealth::Tick()
{
	m_LifeTime--;
	if(!m_LifeTime)
	{
		GS()->m_World.DestroyEntity(this);
		return;
	}

	m_Pos.y -= 1.5f;

}

void CEventHealth::Snap(int SnappingClient)
{
	if (NetworkClipped(SnappingClient))
		return;

	CNetObj_Pickup* pP = static_cast<CNetObj_Pickup*>(Server()->SnapNewItem(NETOBJTYPE_PICKUP, GetID(), sizeof(CNetObj_Pickup)));
	if(!pP)
		return;

	pP->m_X = (int)m_Pos.x;
	pP->m_Y = (int)m_Pos.y;
	pP->m_Type = PICKUP_HEALTH;
}