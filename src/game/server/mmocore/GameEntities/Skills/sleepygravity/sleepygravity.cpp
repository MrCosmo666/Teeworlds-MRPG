/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#include <game/server/gamecontext.h>
#include <engine/shared/config.h>

#include "sleepygravity.h"

CSleepyGravity::CSleepyGravity(CGameWorld *pGameWorld, CPlayer* pPlayer, int SkillBonus, int PowerLevel, vec2 Pos)
: CEntity(pGameWorld, CGameWorld::ENTYPE_SLEEPYGRAVITY, Pos)
{
	// переданные аргументы
	m_pPlayer = pPlayer;
	m_PowerLevel = PowerLevel;
	m_Radius = min(200 + PowerLevel * 2, 400);
	m_LifeSpan = (10 + SkillBonus) * Server()->TickSpeed();
	GameWorld()->InsertEntity(this);	
	for(int i=0; i<NUM_IDS; i++)
	{
		m_IDs[i] = Server()->SnapNewID();
	}
}

CSleepyGravity::~CSleepyGravity()
{
	// освобождаем все при уничтожении
	for(int i=0; i<NUM_IDS; i++)
	{
		Server()->SnapFreeID(m_IDs[i]);
	}
}

void CSleepyGravity::Reset()
{
	// если игрок есть создаем эффекты
	if(m_pPlayer && m_pPlayer->GetCharacter())
	{
		GS()->CreateSound(m_Pos, SOUND_GRENADE_EXPLODE);
	}

	// уничтожаем обьект
	GS()->m_World.DestroyEntity(this);
	return;
}

void CSleepyGravity::Tick()
{
	m_LifeSpan--;
	if(!m_pPlayer || !m_pPlayer->GetCharacter() || !m_LifeSpan)
	{
		Reset();
		return;
	}
	
	for(CCharacter *p = (CCharacter*) GameWorld()->FindFirst(CGameWorld::ENTTYPE_CHARACTER); p; p = (CCharacter *)p->TypeNext())
	{
		if(!GS()->Mmo()->Skills()->CheckInteraction(p, m_Pos, m_Radius))
			continue;

		vec2 Dir = normalize(p->m_Core.m_Pos - m_Pos);
		p->m_Core.m_Vel -= Dir * (1.20f);
	}
}

void CSleepyGravity::Snap(int SnappingClient)
{
	if (NetworkClipped(SnappingClient))
		return;
	
	float AngleStep = 2.0f * pi / CSleepyGravity::NUM_IDS;
	for(int i=0; i<CSleepyGravity::NUM_IDS; i++)
	{
		vec2 VertexPos = m_Pos + vec2(m_Radius * cos(AngleStep*i), m_Radius * sin(AngleStep*i));
		CNetObj_Projectile *pObj = static_cast<CNetObj_Projectile *>(Server()->SnapNewItem(NETOBJTYPE_PROJECTILE, m_IDs[i], sizeof(CNetObj_Projectile)));
		if(!pObj)
			return;

		pObj->m_X = (int)VertexPos.x;
		pObj->m_Y = (int)VertexPos.y;
		pObj->m_VelX = 0;
		pObj->m_VelY = 0;
		pObj->m_StartTick = Server()->Tick()-1;
		pObj->m_Type = WEAPON_HAMMER;
	}
	
	CNetObj_Pickup *pObj = static_cast<CNetObj_Pickup *>(Server()->SnapNewItem(NETOBJTYPE_PICKUP, GetID(), sizeof(CNetObj_Pickup)));
	if(!pObj)
		return;

	pObj->m_X = (int)m_Pos.x;
	pObj->m_Y = (int)m_Pos.y;
	pObj->m_Type = 0;
}
