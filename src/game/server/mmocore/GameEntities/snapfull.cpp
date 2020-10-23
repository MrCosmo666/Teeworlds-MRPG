/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#include <game/server/gamecontext.h>
#include <engine/shared/config.h>
#include "snapfull.h"

CSnapFull::CSnapFull(CGameWorld *pGameWorld, vec2 Pos, int SnapID, int ClientID, int Num, int Type, bool Changing, bool Projectile)
: CEntity(pGameWorld, CGameWorld::ENTTYPE_SNAPEFFECT, Pos)
{
	m_Pos = Pos;
	m_ClientID = ClientID;
	m_LoadingTick = Server()->TickSpeed();
	GameWorld()->InsertEntity(this);

	m_SnapItem.clear();
	AddItem(Num, Type, Projectile, Changing, SnapID);
}

CSnapFull::~CSnapFull()
{
	for(auto items : m_SnapItem)
		Server()->SnapFreeID(items.m_ID);

	m_SnapItem.clear();
}

void CSnapFull::AddItem(int Count, int Type, bool Projectile, bool Dynamic, int SnapID)
{
	for(int i = 0; i < Count; i++)
	{
		SnapItem Itemrz;
		Itemrz.m_ID = Server()->SnapNewID();
		Itemrz.m_Type = Type; 
		Itemrz.m_Changing = Dynamic;
		Itemrz.m_SnapID = SnapID;
		Itemrz.m_Projectile = Projectile;
		m_SnapItem.push_back(Itemrz);
	}	
}

void CSnapFull::RemoveItem(int Count, int SnapID, bool Effect)
{
	for (auto items = m_SnapItem.begin(); items != m_SnapItem.end(); ) 
	{ 
		if(Count <= 0)
			break;
		
		if(items->m_SnapID == SnapID)
		{
			if(Effect)
			{
				CPlayer *pOwner = GS()->m_apPlayers[m_ClientID];
				if(pOwner)
					GS()->CreateDeath(m_Pos, m_ClientID);
			}
			Server()->SnapFreeID(items->m_ID);
			items = m_SnapItem.erase(items);
			Count--;
		}
		else ++items;
	}
}

void CSnapFull::Tick()
{
	CPlayer *pOwner = GS()->m_apPlayers[m_ClientID];
	if(!pOwner || !pOwner->GetCharacter())
	{
		GS()->m_World.DestroyEntity(this);
		return;
	}
		
	if(!m_boolreback) 
	{
		m_LoadingTick--;
		if(m_LoadingTick <= 1)
			m_boolreback = true;
	}
	else 
	{
		m_LoadingTick++;
		if(m_LoadingTick >= 30)
			m_boolreback = false;
	}
	m_Pos = pOwner->GetCharacter()->m_Core.m_Pos;
}

void CSnapFull::Snap(int SnappingClient)
{
	if(NetworkClipped(SnappingClient))
		return;

	// skip non interactive bot
	CPlayer* pOwner = GS()->m_apPlayers[m_ClientID];
	if(pOwner->IsActiveSnappingBot(SnappingClient) != 2)
		return;

	float AngleStart = (2.0f * pi * Server()->Tick()/static_cast<float>(Server()->TickSpeed()))/3.0f;
	float AngleStep = 2.0f * pi / m_SnapItem.size();

	int idsize = 0;
	for(auto items : m_SnapItem)
	{
		float Radius = 48.0f + (items.m_Changing ? m_LoadingTick : 0.0f);
		vec2 PosStart = m_Pos + vec2(Radius * cos(AngleStart + AngleStep*idsize), Radius * sin(AngleStart + AngleStep*idsize));
		idsize++;

		if(items.m_Projectile)
		{
			CNetObj_Projectile *pObj = static_cast<CNetObj_Projectile *>(Server()->SnapNewItem(NETOBJTYPE_PROJECTILE, items.m_ID, sizeof(CNetObj_Projectile)));
			if(!pObj)
				continue;

			pObj->m_X = (int)PosStart.x;
			pObj->m_Y = (int)PosStart.y;
			pObj->m_VelX = 0;
			pObj->m_VelY = 0;
			pObj->m_StartTick = Server()->Tick()-1;
			pObj->m_Type = items.m_Type;
			continue;
		}

		CNetObj_Pickup *pObj = static_cast<CNetObj_Pickup *>(Server()->SnapNewItem(NETOBJTYPE_PICKUP, items.m_ID, sizeof(CNetObj_Pickup)));
		if(!pObj)
			continue;

		pObj->m_X = (int)PosStart.x;
		pObj->m_Y = (int)PosStart.y;
		pObj->m_Type = items.m_Type;		
	}
}
