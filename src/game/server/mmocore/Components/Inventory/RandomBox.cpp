/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#include "RandomBox.h"

#include <game/server/gamecontext.h>

bool CRandomBox::Start(CPlayer *pPlayer, int Seconds, CItemData* pPlayerUsesItem)
{
	if(!pPlayer || !pPlayer->IsAuthed())
		return false;

	if(pPlayer->m_aPlayerTick[LastRandomBox] > pPlayer->GS()->Server()->Tick())
	{
		pPlayer->GS()->Broadcast(pPlayer->GetCID(), BroadcastPriority::MAIN_INFORMATION, 100, "Wait until the last random box opens!");
		return false;
	}

	// the item always uses 1 (USED_ONE)
	if(!pPlayerUsesItem || pPlayerUsesItem->Remove(1))
	{
		Seconds *= pPlayer->GS()->Server()->TickSpeed();
		pPlayer->m_aPlayerTick[LastRandomBox] = pPlayer->GS()->Server()->Tick() + Seconds;
		std::sort(m_ArrayItems.begin(), m_ArrayItems.end(), [](const StructRandomBoxItem& pLeft, const StructRandomBoxItem& pRight) { return pLeft.m_Chance < pRight.m_Chance; });
		new CRandomBoxRandomizer(&pPlayer->GS()->m_World, pPlayer, pPlayer->Acc().m_UserID, Seconds, m_ArrayItems, pPlayerUsesItem);
	}
	return true;
};

CRandomBoxRandomizer::CRandomBoxRandomizer(CGameWorld* pGameWorld, CPlayer* pPlayer, int PlayerAccountID, int LifeTime, std::vector<StructRandomBoxItem> List, CItemData* pPlayerUsesItem)
	: CEntity(pGameWorld, CGameWorld::ENTTYPE_RANDOM_BOX, pPlayer->m_ViewPos)
{
	m_LifeTime = LifeTime;
	m_pPlayer = pPlayer;
	m_PlayerAccountID = PlayerAccountID;
	m_pPlayerUsesItem = pPlayerUsesItem;
	std::copy(List.begin(), List.end(), std::back_inserter(m_List));
	GameWorld()->InsertEntity(this);
}

std::vector<StructRandomBoxItem>::iterator CRandomBoxRandomizer::SelectRandomItem()
{
	const float RandomDrop = frandom() * 100.0f;
	const auto pItem = std::find_if(m_List.begin(), m_List.end(), [RandomDrop](const StructRandomBoxItem &pItem) { return RandomDrop < pItem.m_Chance; });
	return pItem != m_List.end() ? pItem : std::prev(m_List.end());
}

void CRandomBoxRandomizer::Tick()
{
	if(!m_LifeTime || m_LifeTime % Server()->TickSpeed() == 0)
	{
		const auto pRandomItem = SelectRandomItem();
		if(m_pPlayer && m_pPlayer->GetCharacter())
		{
			const vec2 PlayerPosition = m_pPlayer->GetCharacter()->m_Core.m_Pos;
			GS()->CreateText(nullptr, false, vec2(PlayerPosition.x, PlayerPosition.y - 80), vec2(0, -0.3f), 15, GS()->GetItemInfo(pRandomItem->m_ItemID).GetName());
		}

		if(!m_LifeTime)
		{
			// a case when a client changes the world or comes out while choosing a random object.
			CItemData* pPlayerRandomItem = m_pPlayer ? &m_pPlayer->GetItem(pRandomItem->m_ItemID) : nullptr;
			if(!m_pPlayer || (pPlayerRandomItem->Info().IsEnchantable() && pPlayerRandomItem->m_Value > 0))
				GS()->SendInbox("System", m_PlayerAccountID, "Random Box", "Item was not received by you personally.", pRandomItem->m_ItemID, pRandomItem->m_Value);
			else
			{
				m_pPlayer->GetItem(pRandomItem->m_ItemID).Add(pRandomItem->m_Value);
				GS()->CreateDeath(m_pPlayer->m_ViewPos, m_pPlayer->GetCID());
			}

			// infromation
			if(m_pPlayer && m_pPlayerUsesItem)
			{
				const char* pClientName = GS()->Server()->ClientName(m_pPlayer->GetCID());
				GS()->Chat(-1, "{STR} uses {STR} and got {STR}x{INT}!", pClientName, m_pPlayerUsesItem->Info().GetName(), pPlayerRandomItem->Info().GetName(), pRandomItem->m_Value);
			}
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