#include <engine/shared/config.h>
#include <generated/protocol.h>
#include <game/server/gamecontext.h>
#include "event_health.h"

#include <game/server/mmocore/GameEntities/Skills/healthturret/hearth.h>


// Декорации
CNurseHealthNPC::CNurseHealthNPC(CGameWorld* pGameWorld, int ClientID, vec2 Pos) 
	: CEntity(pGameWorld, CGameWorld::ENTTYPE_EVENTS, Pos)
{
	m_OwnerID = ClientID;
	GameWorld()->InsertEntity(this);
}

void CNurseHealthNPC::Tick()
{
	if(!GS()->m_apPlayers[m_OwnerID] || !GS()->m_apPlayers[m_OwnerID]->GetCharacter())
	{
		GS()->m_World.DestroyEntity(this);
		return;
	}

	if(!GS()->CheckPlayersDistance(m_Pos, 800.0f))
		return;

	CCharacter* pOwnerChar = GS()->m_apPlayers[m_OwnerID]->GetCharacter();
	vec2 Direction = normalize(vec2(pOwnerChar->m_LatestInput.m_TargetX, pOwnerChar->m_LatestInput.m_TargetY));
	m_Pos = pOwnerChar->m_Core.m_Pos + normalize(Direction) * (32.0f + 10.0f);
	if(Server()->Tick() % Server()->TickSpeed() == 0)
	{
		for(CCharacter* p = (CCharacter*)GameWorld()->FindFirst(CGameWorld::ENTTYPE_CHARACTER); p; p = (CCharacter*)p->TypeNext())
		{
			if(!p || p->GetPlayer()->IsBot() || distance(p->m_Core.m_Pos, m_Pos) > 240.0f)
				continue;

			const int Health = clamp(p->GetPlayer()->GetStartHealth() / 20, 1, p->GetPlayer()->GetStartHealth());
			std::string Text = std::to_string(Health) + "HP";
			vec2 DrawPosition = vec2(p->m_Core.m_Pos.x, p->m_Core.m_Pos.y - 90.0f);
			GS()->CreateText(NULL, false, DrawPosition, vec2(0, 0), 40, Text.c_str(), GS()->GetWorldID());

			// уровень и здоровье для пополнение
			new CHearth(&GS()->m_World, m_Pos, p->GetPlayer(), Health, p->m_Core.m_Vel);
		}
	}
}

void CNurseHealthNPC::Snap(int SnappingClient)
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