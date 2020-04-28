/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#include <engine/shared/config.h>
#include <generated/server_data.h>
#include <game/server/gamecontext.h>

#include "npcwall.h"

CNPCWall::CNPCWall(CGameWorld *pGameWorld, vec2 Pos, bool Left)
: CEntity(pGameWorld, CGameWorld::ENTTYPE_NPCWALL, Pos)
{
	m_To = Pos;
	// прячем точку стены
	if(!Left) 
		m_Pos.y += 30;
	else 
		m_Pos.x -= 30;

	// эффект поиска крыши
	m_To = GS()->Collision()->FindDirCollision(100, m_To, (Left ? 'x' : 'y'), (Left ? '+' : '-'));
	m_Active = false; 

	GameWorld()->InsertEntity(this);
}

void CNPCWall::Tick()
{
	// ищем ботов игроков
	m_Active = false;	

	for (CCharacter* pChar = (CCharacter*)GameWorld()->FindFirst(CGameWorld::ENTTYPE_CHARACTER); pChar; pChar = (CCharacter*)pChar->TypeNext())
	{
		if (!pChar->GetPlayer()->IsBot())
			continue;

		vec2 IntersectPos = closest_point_on_line(m_Pos, m_To, pChar->m_Core.m_Pos);
		float Distance = distance(IntersectPos, pChar->m_Core.m_Pos);
		if (Distance <= g_Config.m_SvDoorRadiusHit)
			pChar->m_DoorHit = true;
	}
}

void CNPCWall::Snap(int SnappingClient)
{
	if(!m_Active || NetworkClipped(SnappingClient))
		return;

	CNetObj_Laser *pObj = static_cast<CNetObj_Laser *>(Server()->SnapNewItem(NETOBJTYPE_LASER, GetID(), sizeof(CNetObj_Laser)));
	if(!pObj)
		return;

	pObj->m_X = (int)m_Pos.x;
	pObj->m_Y = (int)m_Pos.y;
	pObj->m_FromX = (int)m_To.x;
	pObj->m_FromY = (int)m_To.y;
	pObj->m_StartTick = Server()->Tick()-6; 
}
