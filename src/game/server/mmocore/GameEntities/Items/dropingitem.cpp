/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#include <engine/shared/config.h>

#include <base/math.h>
#include <base/vmath.h>
#include <generated/protocol.h>
#include <game/server/gamecontext.h>

#include "dropingitem.h"

CDropingItem::CDropingItem(CGameWorld *pGameWorld, vec2 Pos, vec2 Vel, float AngleForce, ItemJob::ItemPlayer DropItem, int ForID)
: CEntity(pGameWorld, CGameWorld::ENTTYPE_DROPITEM, Pos)
{
	m_Pos = Pos;
	m_Vel = Vel;
	m_Angle = 0.0f;
	m_AngleForce = AngleForce;

	m_ForID = ForID;
	m_DropItem = DropItem;
	m_DropItem.Settings = 0;
	m_Flashing = false;
	m_StartTick = Server()->Tick();
	m_LifeSpan = Server()->TickSpeed() * 15;

	GameWorld()->InsertEntity(this);
	for(int i=0; i<NUM_IDS; i++)
	{
		m_IDs[i] = Server()->SnapNewID();
	}
}

CDropingItem::~CDropingItem()
{
	for(int i=0; i<NUM_IDS; i++)
	{
		Server()->SnapFreeID(m_IDs[i]);
	}
}

bool CDropingItem::TakeItem(int ClientID)
{
	CPlayer *pPlayer = GS()->m_apPlayers[ClientID];
	if(!pPlayer || !pPlayer->GetCharacter() || (m_ForID >= 0 && m_ForID != ClientID))
		return false;

	// размен зачарованных предметов
	ItemJob::ItemPlayer &PlDropItem = pPlayer->GetItem(m_DropItem.GetID());
	if(PlDropItem.Count > 0 && PlDropItem.Info().StatCount > 0)
	{
		tl_swap(PlDropItem, m_DropItem);
		GS()->Chat(ClientID, "You now own [{STR}+{INT}]", PlDropItem.Info().GetName(pPlayer), &PlDropItem.Enchant);
		GS()->VResetVotes(ClientID, MenuList::MENU_INVENTORY);
		return true;
	}
	
	// выдача просто предмета
	GS()->VResetVotes(ClientID, MenuList::MENU_INVENTORY);
	PlDropItem.Add(m_DropItem.Count, 0, m_DropItem.Enchant);
	GS()->SBL(ClientID, BroadcastPriority::BROADCAST_GAME_WARNING, 10, "\0");
	GS()->m_World.DestroyEntity(this);
	return true;
}

void CDropingItem::Tick()
{
	m_LifeSpan--;
	if(m_LifeSpan < 150)
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
		if(m_LifeSpan < 0)
		{
			GS()->CreatePlayerSpawn(m_Pos);
			GS()->m_World.DestroyEntity(this);
			return;
		}
	}

	m_Vel.y += 0.5f;
	bool Grounded = (bool)GS()->Collision()->CheckPoint(m_Pos.x - 12, m_Pos.y + 12 + 5) || GS()->Collision()->CheckPoint(m_Pos.x + 12, m_Pos.y + 12 + 5);
	if (Grounded)
	{
		m_AngleForce += (m_Vel.x - 0.74f * 6.0f - m_AngleForce) / 2.0f;
		m_Vel.x *= 0.8f;
	}
	else
	{
		m_Angle += clamp(m_AngleForce * 0.04f, -0.6f, 0.6f);
		m_Vel.x *= 0.99f;
	}
	GS()->Collision()->MoveBox(&m_Pos, &m_Vel, vec2(24.0f, 24.0f), 0.4f);

	// проверить точно ли на земле уже после MoveBox
	bool GroundedDouble = (bool)GS()->Collision()->CheckPoint(m_Pos.x - 12, m_Pos.y + 12 + 5) || GS()->Collision()->CheckPoint(m_Pos.x + 12, m_Pos.y + 12 + 5);
	if (GroundedDouble && m_Angle != 0.0f)
		m_Angle = 0.0f;

	// Проверяем есть ли игрок которому предназначен предмет нету то делаем публичным
	{
		CPlayer* pForPlayer = GS()->GetPlayer(m_ForID, true, true);
		if (m_ForID != -1 && !pForPlayer)
			m_ForID = -1;
	}

	CCharacter *pChar = (CCharacter*)GameWorld()->ClosestEntity(m_Pos, 64, CGameWorld::ENTTYPE_CHARACTER, 0);
	if(!pChar || !pChar->GetPlayer() || pChar->GetPlayer()->IsBot())
		return;

	// если не зачарованный предмет
	const ItemJob::ItemPlayer PlDropItem = pChar->GetPlayer()->GetItem(m_DropItem.GetID());
	if(PlDropItem.Info().StatCount <= 0)
	{
		GS()->SBL(pChar->GetPlayer()->GetCID(), BroadcastPriority::BROADCAST_GAME_INFORMATION, 100, "{STR}x{INT} : {STR}",
			m_DropItem.Info().GetName(pChar->GetPlayer()), &m_DropItem.Count, (m_ForID != -1 ? Server()->ClientName(m_ForID) : "Nope"));
		return;
	}

	if (PlDropItem.Count > 0)
	{
		GS()->SBL(pChar->GetPlayer()->GetCID(), BroadcastPriority::BROADCAST_GAME_INFORMATION, 100, "{STR}(+{INT}) -> (+{INT}) : {STR}", 
			m_DropItem.Info().GetName(pChar->GetPlayer()),
			&PlDropItem.Enchant, &m_DropItem.Enchant, 
			(m_ForID != -1 ? Server()->ClientName(m_ForID) : "Nope"));

		return;
	}

	GS()->SBL(pChar->GetPlayer()->GetCID(), BroadcastPriority::BROADCAST_GAME_INFORMATION, 100, "{STR}(+{INT}) : {STR}",
		PlDropItem.Info().GetName(pChar->GetPlayer()), &m_DropItem.Enchant, (m_ForID != -1 ? Server()->ClientName(m_ForID) : "Nope"));
}

void CDropingItem::TickPaused()
{
	m_StartTick++;
}

void CDropingItem::Snap(int SnappingClient)
{
	if(m_Flashing || NetworkClipped(SnappingClient))
		return;

	if(GS()->CheckClient(SnappingClient))
	{
		CNetObj_MmoPickup *pObj = static_cast<CNetObj_MmoPickup*>(Server()->SnapNewItem(NETOBJTYPE_MMOPICKUP, GetID(), sizeof(CNetObj_MmoPickup)));
		if(!pObj)
			return;

		pObj->m_X = (int)m_Pos.x;
		pObj->m_Y = (int)m_Pos.y;
		pObj->m_Type = MMO_PICKUP_BOX;
		pObj->m_Angle = (int)(m_Angle * 256.0f);
		return;
	}

	float AngleStart = (2.0f * pi * Server()->Tick()/static_cast<float>(Server()->TickSpeed()))/10.0f;
	float AngleStep = 2.0f * pi / CDropingItem::BODY;
	float Radius = 30.0f;
	for(int i=0; i<CDropingItem::BODY; i++)
	{
		vec2 PosStart = m_Pos + vec2(Radius * cos(AngleStart + AngleStep*i), Radius * sin(AngleStart + AngleStep*i));
		CNetObj_Projectile *pObj = static_cast<CNetObj_Projectile *>(Server()->SnapNewItem(NETOBJTYPE_PROJECTILE, m_IDs[i], sizeof(CNetObj_Projectile)));
		if(!pObj)
			return;

		pObj->m_X = (int)PosStart.x;
		pObj->m_Y = (int)PosStart.y;
		pObj->m_VelX = 0;
		pObj->m_VelY = 0;
		pObj->m_StartTick = Server()->Tick()-1;
		pObj->m_Type = WEAPON_LASER;
	}

	CNetObj_Pickup *pP = static_cast<CNetObj_Pickup *>(Server()->SnapNewItem(NETOBJTYPE_PICKUP, m_IDs[BODY], sizeof(CNetObj_Pickup)));
	if(!pP)
		return;

	pP->m_X = (int)m_Pos.x;
	pP->m_Y = (int)m_Pos.y;
	pP->m_Type = 0;
} 