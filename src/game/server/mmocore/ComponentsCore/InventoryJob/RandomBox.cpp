/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#include <game/server/gamecontext.h>
#include "RandomBox.h"

bool CRandomBox::Start(CPlayer *pPlayer, int Seconds) 
{
	if(!pPlayer || !pPlayer->IsAuthed())
		return false;

	if(pPlayer->m_aPlayerTick[LastRandomBox] > pPlayer->GS()->Server()->Tick())
	{
		pPlayer->GS()->Broadcast(pPlayer->GetCID(), BroadcastPriority::BROADCAST_MAIN_INFORMATION, 100, "Wait until the last random box opens!");
		return false;
	}

	Seconds *= pPlayer->GS()->Server()->TickSpeed();
	pPlayer->m_aPlayerTick[LastRandomBox] = pPlayer->GS()->Server()->Tick() + Seconds;
	std::sort(m_ArrayItems.begin(), m_ArrayItems.end(), [](const StructRandomBoxItem &pLeft, const StructRandomBoxItem &pRight) { return pLeft.m_Chance < pRight.m_Chance; });
	new CRandomBoxRandomizer(&pPlayer->GS()->m_World, pPlayer, pPlayer->Acc().m_AuthID, Seconds, m_ArrayItems);
	return true;
};

CRandomBoxRandomizer::CRandomBoxRandomizer(CGameWorld* pGameWorld, CPlayer* pPlayer, int PlayerAuthID, int LifeTime, std::vector<StructRandomBoxItem> List)
	: CEntity(pGameWorld, CGameWorld::ENTTYPE_RANDOM_BOX, pPlayer->m_ViewPos)
{
	m_LifeTime = LifeTime;
	m_pPlayer = pPlayer;
	m_PlayerAuthID = PlayerAuthID;
	std::copy(List.begin(), List.end(), std::back_inserter(m_List));
	GameWorld()->InsertEntity(this);
}

std::vector<StructRandomBoxItem>::iterator CRandomBoxRandomizer::SelectRandomItem()
{
	const float RandomDrop = frandom() * 100.0f;
	auto pItem = std::find_if(m_List.begin(), m_List.end(), [RandomDrop](const StructRandomBoxItem &pItem) { return RandomDrop < pItem.m_Chance; });
	return pItem != m_List.end() ? pItem : std::prev(m_List.end());
}

void CRandomBoxRandomizer::Tick()
{
	if(!m_LifeTime || m_LifeTime % Server()->TickSpeed() == 0)
	{
		auto pSelectedItem = SelectRandomItem();
		if(m_pPlayer && m_pPlayer->GetCharacter())
		{
			vec2 PlayerPosition = m_pPlayer->GetCharacter()->m_Core.m_Pos;
			GS()->CreateText(nullptr, false, vec2(PlayerPosition.x, PlayerPosition.y - 80), vec2(0, -0.3f), 15, GS()->GetItemInfo(pSelectedItem->m_ItemID).GetName());
		}

		// give usually
		if(!m_LifeTime)
		{
			// a case when a client changes the world or comes out while choosing a random object.
			if(!m_pPlayer)
			{
				GS()->SendInbox(m_PlayerAuthID, "Random Box", "Item was not received by you personally.", pSelectedItem->m_ItemID, pSelectedItem->m_Count);
				GS()->m_World.DestroyEntity(this);
				return;
			}

			m_pPlayer->GetItem(pSelectedItem->m_ItemID).Add(pSelectedItem->m_Count);
			GS()->CreateDeath(m_pPlayer->m_ViewPos, m_pPlayer->GetCID());
			GS()->m_World.DestroyEntity(this);
			return;
		}
	}
	m_LifeTime--;
}

void CRandomBoxRandomizer::Snap(int SnappingClient)
{
	if(NetworkClipped(SnappingClient))
		return;

	// then add some effects?
}