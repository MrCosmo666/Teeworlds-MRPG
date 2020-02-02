/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#include <generated/server_data.h>
#include <game/server/gamecontext.h>
#include <game/server/player.h>

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
	if (GS()->Collision()->GetCollisionAt(SwapPos.x, SwapPos.y) > 0) m_Pos = SwapPos;
	SwapPos = vec2(m_Pos.x, m_Pos.y+16);
	if (GS()->Collision()->GetCollisionAt(SwapPos.x, SwapPos.y) > 0) m_Pos = SwapPos;
	SwapPos = vec2(m_Pos.x-18, m_Pos.y);
	if (GS()->Collision()->GetCollisionAt(SwapPos.x, SwapPos.y) > 0) m_Pos = SwapPos;
	SwapPos = vec2(m_Pos.x+18, m_Pos.y);
	if (GS()->Collision()->GetCollisionAt(SwapPos.x, SwapPos.y) > 0) m_Pos = SwapPos;
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

	char aBuf[128];
	CPlayer *pPlayer = GS()->m_apPlayers[ClientID];
	ItemSql::ItemPlayer &PlDropItem = pPlayer->GetItem(m_ItemID);
	if(m_Type == 1)
	{
		int EquipItem = pPlayer->GetItemEquip(EQUIP_MINER);
		if(EquipItem <= 0) return pPlayer->AddInBroadcast("Need equip Pickaxe\n");

		// проверка уровня на доступность
		ItemSql::ItemPlayer &PlEquipItem = pPlayer->GetItem(EquipItem);
		if(pPlayer->Acc().Miner[PlLevel] < m_Level)
		{
			str_format(aBuf, sizeof(aBuf), "^611Your level low. %s %d Level\n", PlDropItem.Info().GetName(pPlayer), m_Level);
			pPlayer->AddInBroadcast(aBuf);
			return;
		}

		int Durability = PlEquipItem.Durability;
		{ // кирка поломка и информация о ремонте
			if(rand()%10 == 0) GS()->Mmo()->Item()->SetDurabilityItem(pPlayer, EquipItem, Durability-1);
			if(Durability <= 0) return pPlayer->AddInBroadcast("Need repair Pickaxe\n"); 
		}

		m_Progress += 3+pPlayer->EnchantAttributes(Stats::StEfficiency);
		GS()->CreateSound(m_Pos, 20, CmaskOne(ClientID));
		str_format(aBuf, sizeof(aBuf), "Level %d Mining %s work [HP %d/%d]\n", 
			m_Level, PlDropItem.Info().GetName(pPlayer), (m_Progress > m_Health ? m_Health : m_Progress), m_Health);
		pPlayer->AddInBroadcast(aBuf);
		str_format(aBuf, sizeof(aBuf), "Durability %s (%d/100%%)\n", PlEquipItem.Info().GetName(pPlayer), Durability);
		pPlayer->AddInBroadcast(aBuf);

		if(m_Progress >= m_Health)
		{
			SetSpawn(20);
			GS()->Mmo()->MinerAcc()->Work(pPlayer, m_Level*5);

			const int Count = pPlayer->Acc().Miner[MnrCount];
			PlDropItem.Add(Count);
		}
		return;
	}

	// проверка уровня на доступность
	if(pPlayer->Acc().Plant[PlLevel] < m_Level)
	{
		str_format(aBuf, sizeof(aBuf), "^611Your level low. %s %d Level\n", PlDropItem.Info().GetName(pPlayer), m_Level);
		pPlayer->AddInBroadcast(aBuf);
		return;
	}

	m_Progress += 10;
	GS()->CreateSound(m_Pos, 20, CmaskOne(ClientID));
	str_format(aBuf, sizeof(aBuf), "Level %d Harvest %s work [HP %d/%d]\n", 
		m_Level, PlDropItem.Info().GetName(pPlayer), m_Progress, m_Health);
	pPlayer->AddInBroadcast(aBuf);

	if(m_Progress >= m_Health)
	{
		SetSpawn(20);
		GS()->Mmo()->PlantsAcc()->Work(ClientID, m_Level*5);

		int Count = pPlayer->Acc().Plant[PlCounts];
		PlDropItem.Add(Count);
	}
}

int CJobItems::SwitchToObject(bool MmoItem)
{
	switch(m_Type)
	{
		case 0/*plants*/: return (MmoItem ? (int)ITEMS_PLANT : (int)PICKUP_HEALTH);
		case 1/*miner*/: return (MmoItem ? (int)ITEMS_ORE : (int)PICKUP_ARMOR);
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

	if(SwitchToObject(true) > -1 && GS()->CheckClient(SnappingClient))
	{
		CNetObj_MmoItems *pObj = static_cast<CNetObj_MmoItems *>(Server()->SnapNewItem(NETOBJTYPE_MMOITEMS, GetID(), sizeof(CNetObj_MmoItems)));
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
