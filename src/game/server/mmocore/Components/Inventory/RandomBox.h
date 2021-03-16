/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#ifndef GAME_SERVER_INVENTORY_RANDOM_BOX_H
#define GAME_SERVER_INVENTORY_RANDOM_BOX_H
#include <game/server/entity.h>

#include "ItemData.h"

class CPlayer;

struct StructRandomBoxItem
{
	int m_ItemID;
	int m_Count;
	float m_Chance;
};

class CRandomBox
{
	std::vector <StructRandomBoxItem> m_ArrayItems;

public:
	void Add(int ItemID, int Count, float Chance)
	{
		StructRandomBoxItem Item;
		Item.m_Chance = Chance;
		Item.m_Count = Count;
		Item.m_ItemID = ItemID;
		m_ArrayItems.push_back(Item);
	}

	bool Start(CPlayer* pPlayer, int LifeTime, CItemData* pPlayerUsesItem = nullptr);
};

class CRandomBoxRandomizer : public CEntity
{
	int m_LifeTime;
	int m_PlayerAccountID;
	CPlayer* m_pPlayer;
	CItemData* m_pPlayerUsesItem;
	std::vector<StructRandomBoxItem> m_List;

public:
	CRandomBoxRandomizer(CGameWorld* pGameWorld, CPlayer* pPlayer, int PlayerAccountID, int LifeTime, std::vector<StructRandomBoxItem> List, CItemData* pPlayerUsesItem);

	std::vector<StructRandomBoxItem>::iterator SelectRandomItem();
	void Tick() override;
	void Snap(int SnappingClient) override;
};

#endif