/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#include <game/server/gamecontext.h>
#include <engine/shared/config.h>

#include "noctis_teleport.h"

CNoctisTeleport::CNoctisTeleport(CGameWorld *pGameWorld, CCharacter* pPlayerChar, int SkillBonus, int PowerLevel)
: CEntity(pGameWorld, CGameWorld::ENTYPE_NOCTIS_TELEPORT, pPlayerChar->GetPos(), 64.0f)
{
	// переданные аргументы
	m_pPlayerChar = pPlayerChar;
	m_Direction = vec2(m_pPlayerChar->m_Core.m_Input.m_TargetX, m_pPlayerChar->m_Core.m_Input.m_TargetY);
	m_PowerLevel = PowerLevel;
	m_Radius = min(200 + SkillBonus, 400);
	m_LifeSpan = 3 * Server()->TickSpeed();
	GameWorld()->InsertEntity(this);
}

CNoctisTeleport::~CNoctisTeleport() {}

void CNoctisTeleport::Reset()
{
	if(m_pPlayerChar && m_pPlayerChar->IsAlive())
	{
		GS()->CreateSound(m_Pos, SOUND_GRENADE_EXPLODE);
	}

	// уничтожаем обьект
	GS()->m_World.DestroyEntity(this);
	return;
}

void CNoctisTeleport::Tick()
{
	m_LifeSpan--;
	if(!m_pPlayerChar || !m_pPlayerChar->IsAlive())
	{
		Reset();
		return;
	}

	vec2 To = m_Pos + normalize(m_Direction) * GetProximityRadius();
	vec2 Size = vec2(GetProximityRadius()/2, GetProximityRadius()/2);
	if(!m_LifeSpan || GS()->Collision()->TestBox(To, Size) || GS()->m_World.IntersectClosestDoorEntity(To, GetProximityRadius()))
	{
		m_pPlayerChar->ChangePosition(m_Pos);
		Reset();
		return;
	}
	m_Pos += normalize(m_Direction) * 16.0f;
}

void CNoctisTeleport::Snap(int SnappingClient)
{
	if (NetworkClipped(SnappingClient))
		return;
	
	CNetObj_Pickup *pObj = static_cast<CNetObj_Pickup *>(Server()->SnapNewItem(NETOBJTYPE_PICKUP, GetID(), sizeof(CNetObj_Pickup)));
	if(!pObj)
		return;

	pObj->m_X = (int)m_Pos.x;
	pObj->m_Y = (int)m_Pos.y;
	pObj->m_Type = 0;
}
