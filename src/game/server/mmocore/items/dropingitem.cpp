/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#include <engine/shared/config.h>

#include <base/math.h>
#include <base/vmath.h>
#include <generated/protocol.h>
#include <game/server/gamecontext.h>

#include "dropingitem.h"

CDropingItem::CDropingItem(CGameWorld *pGameWorld, vec2 Pos, vec2 Dir, ItemSql::ItemPlayer DropItem, int ForID)
: CEntity(pGameWorld, CGameWorld::ENTTYPE_DROPITEM, Pos), m_Direction(vec2(Dir.x, Dir.y+2/rand()%9))
{
	m_Pos = Pos;
	m_ActualPos = Pos;
	m_ActualDir = Dir;
	m_Direction = Dir;

	// other var
	m_ForID = ForID;
	m_DropItem = DropItem;
	m_DropItem.Settings = 0;
	m_Flashing = false;
	m_StartTick = Server()->Tick();
	m_LifeSpan = Server()->TickSpeed() * 15;

	// create object
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
	ItemSql::ItemPlayer &PlDropItem = pPlayer->GetItem(m_DropItem.GetID());
	if(PlDropItem.Count > 0 && PlDropItem.Info().BonusCount > 0)
	{
		tl_swap(PlDropItem, m_DropItem);
		GS()->Chat(ClientID, "You equip item [{STR} +{INT}]", PlDropItem.Info().GetName(pPlayer), &PlDropItem.Enchant);
		GS()->VResetVotes(ClientID, INVENTORY);
		return true;
	}
	
	// выдача просто предмета
	GS()->VResetVotes(ClientID, INVENTORY);
	PlDropItem.Add(m_DropItem.Count, 0, m_DropItem.Enchant);
	GS()->SBL(ClientID, 10000, 10, "\0");
	GS()->m_World.DestroyEntity(this);
	return true;
}

vec2 CDropingItem::GetTimePos(float Time)
{
	float Curvature = 1.25f;
	float Speed = 2750.0f;

	return CalcPos(m_Pos, m_Direction, Curvature, Speed, Time);
}

void CDropingItem::Tick()
{
	// check life time
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
	m_LifeSpan--;

	float Pt = (Server()->Tick()-m_StartTick-1)/(float)Server()->TickSpeed();
	float Ct = (Server()->Tick()-m_StartTick)/(float)Server()->TickSpeed();
	vec2 PrevPos = GetTimePos(Pt), CurPos = GetTimePos(Ct);
	m_ActualPos = CurPos;
	m_ActualDir = normalize(CurPos - PrevPos);

	vec2 LastPos;
	int Collide = GS()->Collision()->IntersectLine(PrevPos, CurPos, NULL, &LastPos);
	if(Collide)
	{			
		m_Pos = LastPos;
		m_ActualPos = m_Pos;
		m_Direction.x *= (100 - 50) / 100.0f;
		m_Direction.y *= (100 - 50) / 65.0f;
		m_StartTick = Server()->Tick();
		m_ActualDir = normalize(m_Direction);
	}

	// Проверяем есть ли игрок которому предназначен предмет нету то делаем публичным
	if(m_ForID != -1 && !GS()->m_apPlayers[m_ForID])
		m_ForID = -1;	

	if(m_LifeSpan > Server()->TickSpeed() * ( 5 - 1 ))
		return;
	
	CCharacter *pChar = (CCharacter*)GameWorld()->ClosestEntity(m_Pos, 64, CGameWorld::ENTTYPE_CHARACTER, 0);
	if(!pChar || !pChar->GetPlayer() || pChar->GetPlayer()->IsBot())
		return;

	char ItemBuf[128];
	const ItemSql::ItemPlayer PlDropItem = pChar->GetPlayer()->GetItem(m_DropItem.GetID());

	// если не зачарованный предмет
	if(PlDropItem.Info().BonusCount <= 0)
	{
		str_format(ItemBuf, sizeof(ItemBuf), "^351\nHammer: %sx%d : %s\n", 
			PlDropItem.Info().GetName(pChar->GetPlayer()), m_DropItem.Count, (m_ForID != -1 ? Server()->ClientName(m_ForID) : "Nope"));
		pChar->GetPlayer()->AddInBroadcast(ItemBuf);
		return;
	}

	// зачарованный предмет
	str_format(ItemBuf, sizeof(ItemBuf), "^351\nHammer: %sx%d(+%d) : %s\n", 
		PlDropItem.Info().GetName(pChar->GetPlayer()), m_DropItem.Count, m_DropItem.Enchant, (m_ForID != -1 ? Server()->ClientName(m_ForID) : "Nope"));
	pChar->GetPlayer()->AddInBroadcast(ItemBuf);

	if(PlDropItem.Count > 0)
	{
		str_format(ItemBuf, sizeof(ItemBuf), "Swap [Dropped Level +%d] -> [Equiped Level +%d]\n", m_DropItem.Enchant, PlDropItem.Enchant);
		pChar->GetPlayer()->AddInBroadcast(ItemBuf);				
	}
}

void CDropingItem::TickPaused()
{
	m_StartTick++;
}

void CDropingItem::Snap(int SnappingClient)
{
	float Ct = (Server()->Tick()-m_StartTick)/(float)Server()->TickSpeed();
	
	if(m_Flashing || NetworkClipped(SnappingClient, GetTimePos(Ct)))
		return;

	if(GS()->CheckClient(SnappingClient))
	{
		CNetObj_MmoPickup *pObj = static_cast<CNetObj_MmoPickup*>(Server()->SnapNewItem(NETOBJTYPE_MMOPICKUP, GetID(), sizeof(CNetObj_MmoPickup)));
		if(!pObj)
			return;

		pObj->m_X = (int)m_ActualPos.x;
		pObj->m_Y = (int)m_ActualPos.y;
		pObj->m_Type = MMO_PICKUP_BOX;
		pObj->m_Angle = 0;
		return;
	}

	float AngleStart = (2.0f * pi * Server()->Tick()/static_cast<float>(Server()->TickSpeed()))/10.0f;
	float AngleStep = 2.0f * pi / CDropingItem::BODY;
	float Radius = 30.0f;
	for(int i=0; i<CDropingItem::BODY; i++)
	{
		vec2 PosStart = m_ActualPos + vec2(Radius * cos(AngleStart + AngleStep*i), Radius * sin(AngleStart + AngleStep*i));
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

	pP->m_X = (int)m_ActualPos.x;
	pP->m_Y = (int)m_ActualPos.y;
	pP->m_Type = 0;
} 