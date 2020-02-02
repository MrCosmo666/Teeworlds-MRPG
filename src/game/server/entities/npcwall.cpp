/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#include <generated/server_data.h>
#include <game/server/gamecontext.h>

#include "projectile.h"
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
	for(CCharacter *pChar = (CCharacter*) GameWorld()->FindFirst(CGameWorld::ENTTYPE_CHARACTER); pChar; pChar = (CCharacter *)pChar->TypeNext())
	{
		vec2 IntersectPos = closest_point_on_line(m_Pos, m_To, pChar->m_Core.m_Pos);
		float Distance = distance(IntersectPos, pChar->m_Core.m_Pos);
		if(Distance < 250 && pChar->GetPlayer()->IsBot())
		{
			vec2 IntersectPos = closest_point_on_line(m_Pos, m_To, pChar->m_Core.m_Pos);
			float Distance = distance(IntersectPos, pChar->m_Core.m_Pos);
			if(Distance < 128.0f && length(pChar->m_Core.m_Vel) > 20.0f)
				pChar->m_Core.m_Vel = vec2(0.0f, 0.0f);

			if(Distance < 32.0f)
			{
				float a = (32.0f*1.45f - Distance);
				float Velocity = 0.5f;

				vec2 Dir = normalize(pChar->m_Core.m_Pos - IntersectPos);
				pChar->m_Core.m_Vel += Dir*a*(Velocity*0.75f);
				pChar->m_Core.m_Vel *= 0.85f;
				pChar->m_Core.m_Pos = pChar->m_OldPos + Dir;
			}
			m_Active = true;
		}
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
