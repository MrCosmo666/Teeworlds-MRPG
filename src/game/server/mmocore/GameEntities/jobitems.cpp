/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#include <game/server/gamecontext.h>

#include "jobitems.h"
// 1 - miner / 2 - plant
CJobItems::CJobItems(CGameWorld *pGameWorld, int ItemID, int Level, vec2 Pos, int Type, int Health, int HouseID)
: CEntity(pGameWorld, CGameWorld::ENTTYPE_JOBITEMS, Pos, PickupPhysSize)
{
	m_ItemID = ItemID;
	m_Level = Level;
	m_Type = Type;
	m_Health = Health;
	m_HouseID = HouseID;
	SpawnPositions();

	Reset();
	GameWorld()->InsertEntity(this);
}

void CJobItems::SpawnPositions()
{
	vec2 SwapPos = vec2(m_Pos.x, m_Pos.y-20);
	if (GS()->Collision()->GetCollisionAt(SwapPos.x, SwapPos.y) > 0) 
		m_Pos = SwapPos;
	SwapPos = vec2(m_Pos.x, m_Pos.y+16);
	if (GS()->Collision()->GetCollisionAt(SwapPos.x, SwapPos.y) > 0) 
		m_Pos = SwapPos;
	SwapPos = vec2(m_Pos.x-18, m_Pos.y);
	if (GS()->Collision()->GetCollisionAt(SwapPos.x, SwapPos.y) > 0) 
		m_Pos = SwapPos;
	SwapPos = vec2(m_Pos.x+18, m_Pos.y);
	if (GS()->Collision()->GetCollisionAt(SwapPos.x, SwapPos.y) > 0) 
		m_Pos = SwapPos;
}

void CJobItems::SetSpawn(int Sec)
{
	m_SpawnTick = Server()->Tick() + (Server()->TickSpeed()*Sec);
	m_Progress = m_Health;
}

void CJobItems::Work(int ClientID)
{
	if(ClientID >= MAX_PLAYERS || ClientID < 0 || m_Progress >= m_Health || !GS()->m_apPlayers[ClientID])
		return;

	// not allowed un owner house job 
	if(m_HouseID > 0 && GS()->Mmo()->House()->GetOwnerHouse(m_HouseID) <= 0)
	{
		GS()->SBL(ClientID, BroadcastPriority::BROADCAST_GAME_WARNING, 100, "It is forbidden to pick plants without the owner!");
		return;
	}

	// - - - - - - - - MINING - - - - - - - - 
	CPlayer *pPlayer = GS()->m_apPlayers[ClientID];
	InventoryItem &pPlayerWorkedItem = pPlayer->GetItem(m_ItemID);
	if(m_Type == 1)
	{
		int EquipItem = pPlayer->GetEquippedItem(EQUIP_MINER);
		if (EquipItem <= 0)
		{
			GS()->SBL(ClientID, BroadcastPriority::BROADCAST_GAME_WARNING, 100, "Need equip Pickaxe!");
			return;
		}
		if (pPlayer->Acc().m_aMiner[PlLevel] < m_Level)
		{
			GS()->SBL(ClientID, BroadcastPriority::BROADCAST_GAME_WARNING, 100, "Your level low. {STR} {INT} Level", pPlayerWorkedItem.Info().GetName(pPlayer), &m_Level);
			return;
		}

		InventoryItem& pPlayerEquippedItem = pPlayer->GetItem(EquipItem);
		int Durability = pPlayerEquippedItem.m_Durability;
		if (Durability <= 0)
		{
			GS()->SBL(ClientID, BroadcastPriority::BROADCAST_GAME_WARNING, 100, "Need repair pickaxe!");
			return;
		}

		if (rand() % 10 == 0)
			pPlayerEquippedItem.SetDurability(Durability - 1);

		m_Progress += 3+pPlayer->GetItemsAttributeCount(Stats::StEfficiency);
		GS()->CreateSound(m_Pos, 20, CmaskOne(ClientID));

		GS()->SBL(ClientID, BroadcastPriority::BROADCAST_GAME_INFORMATION, 100, "{STR} [{INT}/{INT}P] : {STR} ({INT}/100%)", 
			pPlayerWorkedItem.Info().GetName(pPlayer), (m_Progress > m_Health ? &m_Health : &m_Progress), &m_Health, 
			pPlayerEquippedItem.Info().GetName(pPlayer), &Durability);

		if(m_Progress >= m_Health)
		{
			GS()->Mmo()->MinerAcc()->Work(pPlayer, m_Level);
			SetSpawn(20);

			const int Count = pPlayer->Acc().m_aMiner[MnrCount];
			pPlayerWorkedItem.Add(Count);
		}
		return;
	}

	// - - - - - - - - PLANTS - - - - - - - - 
	if (pPlayer->Acc().m_aPlant[PlLevel] < m_Level)
	{
		GS()->SBL(ClientID, BroadcastPriority::BROADCAST_GAME_WARNING, 100, "Your level low. {STR} {INT} Level", pPlayerWorkedItem.Info().GetName(pPlayer), &m_Level);
		return;
	}

	m_Progress += 10;
	GS()->CreateSound(m_Pos, 20, CmaskOne(ClientID));

	GS()->SBL(ClientID, BroadcastPriority::BROADCAST_GAME_INFORMATION, 100, "{STR} [{INT}/{INT}P]",
		pPlayerWorkedItem.Info().GetName(pPlayer), (m_Progress > m_Health ? &m_Health : &m_Progress), &m_Health);

	if(m_Progress >= m_Health)
	{
		GS()->Mmo()->PlantsAcc()->Work(pPlayer, m_Level);
		SetSpawn(20);

		int Count = pPlayer->Acc().m_aPlant[PlCounts];
		pPlayerWorkedItem.Add(Count);
	}
}

int CJobItems::SwitchToObject(bool MmoItem)
{
	switch(m_Type)
	{
		case 0/*plants*/: return (MmoItem ? (int)MMO_PICKUP_PLANT : (int)PICKUP_HEALTH);
		case 1/*miner*/: return (MmoItem ? (int)MMO_PICKUP_ORE : (int)PICKUP_ARMOR);
	}
	return -1;
}

void CJobItems::Reset()
{
	m_SpawnTick = -1;
	m_Progress = 0;
}

void CJobItems::Tick()
{
	if(m_SpawnTick > 0)
	{
		if(Server()->Tick() > m_SpawnTick)
			Reset();
		else return;
	}
}

void CJobItems::TickPaused()
{
	if(m_SpawnTick != -1)
		++m_SpawnTick;
}

void CJobItems::Snap(int SnappingClient)
{
	if(m_SpawnTick != -1 || NetworkClipped(SnappingClient))
		return;

	if(SwitchToObject(true) > -1 && GS()->IsMmoClient(SnappingClient))
	{
		CNetObj_MmoPickup *pObj = static_cast<CNetObj_MmoPickup*>(Server()->SnapNewItem(NETOBJTYPE_MMOPICKUP, GetID(), sizeof(CNetObj_MmoPickup)));
		if(!pObj)
			return;

		pObj->m_X = (int)m_Pos.x;
		pObj->m_Y = (int)m_Pos.y;
		pObj->m_Type = SwitchToObject(true);
		return;
	}

	CNetObj_Pickup *pP = static_cast<CNetObj_Pickup *>(Server()->SnapNewItem(NETOBJTYPE_PICKUP, GetID(), sizeof(CNetObj_Pickup)));
	if(!pP)
		return;

	pP->m_X = (int)m_Pos.x;
	pP->m_Y = (int)m_Pos.y;
	pP->m_Type = SwitchToObject(false);
}
