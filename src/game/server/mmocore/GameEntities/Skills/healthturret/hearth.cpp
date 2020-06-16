/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#include <game/server/gamecontext.h>
#include "hearth.h"

CHearth::CHearth(CGameWorld *pGameWorld, vec2 Pos, CPlayer *pPlayer, int Health, vec2 InitialVel)
: CEntity(pGameWorld, CGameWorld::ENTYPE_HEARTLIFE, Pos)
{
	// устанавливаем значения по аргументам
	m_Pos = Pos;
	m_InitialVel = InitialVel/2;
	m_pPlayer = pPlayer;
	m_InitialAmount = 1.0f;
	m_Health = Health;

	// создаем обьект
	GameWorld()->InsertEntity(this);
}

void CHearth::Tick()
{
	// проверяем есть ли игрок или нет для использования его функций
	if(!m_pPlayer || !m_pPlayer->GetCharacter())
	{
		GS()->m_World.DestroyEntity(this);
		return;
	}

	// если дистанция больше 24.0
	CCharacter *pChar = m_pPlayer->GetCharacter();
	float Dist = distance(m_Pos, pChar->m_Core.m_Pos);
	if(Dist > 24.0f)
	{
		vec2 Dir = normalize(pChar->m_Core.m_Pos - m_Pos);
		m_Pos += Dir*clamp(Dist, 0.0f, 16.0f) * (1.0f - m_InitialAmount) + m_InitialVel * m_InitialAmount;
		
		m_InitialAmount *= 0.98f;
		return;
	}

	// выдаем все или создаем эффект что полное здоровье
	GS()->CreateSound(m_Pos, 15);
	pChar->IncreaseHealth(m_Health);

	
	// уничтожаем обьект
	GS()->m_World.DestroyEntity(this);
	return;
}

void CHearth::Snap(int SnappingClient)
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
