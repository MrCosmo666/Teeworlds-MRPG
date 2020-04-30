/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#include <game/server/gamecontext.h>
#include <engine/shared/config.h>

#include "healer-health.h"
#include "hearth.h"

CHealthHealer::CHealthHealer(CGameWorld *pGameWorld, CPlayer *pPlayer, int SkillLevel, int ManaUseCost, vec2 Pos)
: CEntity(pGameWorld, CGameWorld::ENTYPE_SKILLTURRETHEART, Pos)
{
	// переданные аргументы
	m_Pos = Pos;
	m_pPlayer = pPlayer;
	m_SkillLevel = SkillLevel;
	m_ManaUseCost = ManaUseCost;

	// обычные настройки без передачи аргументов
	m_LifeSpan = 16*Server()->TickSpeed();
	m_ReloadTick = 2*Server()->TickSpeed();

	// создаем обьект
	GameWorld()->InsertEntity(this);	
	for(int i=0; i<NUM_IDS; i++)
	{
		m_IDs[i] = Server()->SnapNewID();
	}
}

CHealthHealer::~CHealthHealer()
{
	// освобождаем все при уничтожении
	for(int i=0; i<NUM_IDS; i++)
	{
		Server()->SnapFreeID(m_IDs[i]);
	}
}

void CHealthHealer::Reset()
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

void CHealthHealer::Tick()
{
	// проверяем есть ли игрок или нет для использования его функций
	if(!m_pPlayer || !m_pPlayer->GetCharacter() || !m_LifeSpan)
	{
		Reset();
		return;
	}
	
	// все тики что нужны тут
	m_LifeSpan--;

	// перезарядка
	if(m_ReloadTick)
	{
		m_ReloadTick--;
		return;
	}
	
	// находим игроков и кидаем ему сердце
	bool ShowHealthRestore = false;
	for(CCharacter *p = (CCharacter*) GameWorld()->FindFirst(CGameWorld::ENTTYPE_CHARACTER); p; p = (CCharacter *)p->TypeNext())
	{
		if(!p || p->GetPlayer()->IsBot() || distance(p->m_Core.m_Pos, m_Pos) > 520.0f) continue;
		
		// показать восстановление
		int Health = 5 * m_SkillLevel;
		if(!ShowHealthRestore)
		{
			std::string Text = std::to_string(Health) + "HP";
			GS()->CreateText(NULL, false, m_Pos, vec2(0,0), 40, Text.c_str(), GS()->GetWorldID());
			ShowHealthRestore = true;
		}

		// уровень и здоровье для пополнение
		new CHearth(&GS()->m_World, m_Pos, p->GetPlayer(), Health, p->m_Core.m_Vel);
	}

	// устанавливаем перезарядку
	m_ReloadTick = 2*Server()->TickSpeed();

	// если на дальнейшее использование нет маны
	if(m_pPlayer->GetCharacter()->CheckFailMana(m_ManaUseCost))
	{
		Reset();
		return;
	}
}

void CHealthHealer::Snap(int SnappingClient)
{
	if (NetworkClipped(SnappingClient))
		return;

	float AngleStep = 2.0f * pi / CHealthHealer::NUM_IDS;
	float Radius = clamp(50.0f-(int)m_ReloadTick, -50.0f, 50.0f);
	for(int i=0; i<CHealthHealer::NUM_IDS; i++)
	{
		vec2 VertexPos = m_Pos + vec2(Radius * cos(AngleStep*i), Radius * sin(AngleStep*i));
		CNetObj_Pickup *pObj = static_cast<CNetObj_Pickup *>(Server()->SnapNewItem(NETOBJTYPE_PICKUP, m_IDs[i], sizeof(CNetObj_Pickup)));
		if(!pObj)
			return;

		pObj->m_X = (int)VertexPos.x;
		pObj->m_Y = (int)VertexPos.y;
		pObj->m_Type = 1;
	}
	
	CNetObj_Pickup *pObj = static_cast<CNetObj_Pickup *>(Server()->SnapNewItem(NETOBJTYPE_PICKUP, GetID(), sizeof(CNetObj_Pickup)));
	if(!pObj)
		return;

	pObj->m_X = (int)m_Pos.x;
	pObj->m_Y = (int)m_Pos.y;
	pObj->m_Type = 0;
}
