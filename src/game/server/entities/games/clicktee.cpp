/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#include <generated/server_data.h>
#include <game/server/gamecontext.h>
#include <game/server/player.h>

#include "clicktee.h"

ClickTee::ClickTee(CGameWorld *pGameWorld, vec2 Pos)
: CEntity(pGameWorld, CGameWorld::ENTTYPE_CLICKGAME, Pos, 14)
{
	m_Pos = Pos;
	m_Spawn = true;
	m_ReloadTick = 30;

	SpawnedClick();
	GameWorld()->InsertEntity(this);	
	for(int i=0; i<NUM_IDS; i++)
		m_IDs[i] = Server()->SnapNewID();
}

ClickTee::~ClickTee()
{
	for(int i=0; i<NUM_IDS; i++)
		Server()->SnapFreeID(m_IDs[i]);
}

void ClickTee::SpawnedClick()
{
	m_Spawn = true;
	m_Health = 100;
}

void ClickTee::Tick()
{
	m_ReloadTick--;
	if(m_ReloadTick <= 0)
		m_ReloadTick = 30;
}

bool ClickTee::TakeDamage(int Damage, int ClientID, vec2 Force)
{
	if(m_Health <= 0)
		return m_Spawn = false;

	m_Health -= Damage;

	GS()->CreateText(this, true, vec2(0, -70), vec2(0, 0), 5, std::to_string(m_Health).c_str(), GS()->Server()->GetWorldID(ClientID));

	GS()->CreateDamage(m_Pos, ClientID, m_Pos, Damage >= 5 ? 5 : Damage, 0, false);
	GS()->CreateSound(m_Pos, SOUND_PICKUP_ARMOR);

	GS()->CreateDropBonuses(m_Pos, 1, rand()%10+1, 3);
	return true;
}

void ClickTee::TickPaused()
{
	m_Spawn = false;
}

void ClickTee::Snap(int SnappingClient)
{
	if(!m_Spawn || NetworkClipped(SnappingClient))
		return;

	float AngleStep = 2.0f * pi / ClickTee::NUM_SIDE;
	float Radius = 50.0f-(int)m_ReloadTick;
	for(int i=0; i<ClickTee::NUM_SIDE; i++)
	{
		vec2 VertexPos = m_Pos + vec2(Radius * cos(AngleStep*i), Radius * sin(AngleStep*i));
		CNetObj_Projectile *pObj = static_cast<CNetObj_Projectile *>(Server()->SnapNewItem(NETOBJTYPE_PROJECTILE, m_IDs[i], sizeof(CNetObj_Projectile)));
		if(!pObj)
			return;
			
		pObj->m_X = (int)VertexPos.x;
		pObj->m_Y = (int)VertexPos.y;
		pObj->m_VelX = 0;
		pObj->m_VelY = 0;
		pObj->m_StartTick = Server()->Tick()-5;
		pObj->m_Type = WEAPON_HAMMER;
	}
}
