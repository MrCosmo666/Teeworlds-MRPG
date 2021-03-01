/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#ifndef GAME_SERVER_ITEM_INVENTORY_H
#define GAME_SERVER_ITEM_INVENTORY_H
#include <game/server/enum_context.h>

#include "ItemInformation.h"

class CInventoryItem
{
	class CGS* m_pGS;
	class CPlayer* m_pPlayer;
	CGS* GS() const { return m_pGS; }
	bool Save();

public:
	int m_ItemID;
	int m_Count;
	int m_Settings;
	int m_Enchant;
	int m_Durability;

	ItemInformation& Info() const;
	int GetID() const { return m_ItemID; }

	// main functions
	void SetItemOwner(CPlayer* pPlayer);
	bool SetEnchant(int Enchant);
	bool SetSettings(int Settings);
	bool SetDurability(int Durability);

	bool Add(int Count, int Settings = 0, int Enchant = 0, bool Message = true);
	bool Remove(int Count, int Settings = 0);
	bool Equip();
	bool Use(int Count);
	bool Drop(int Count);

	// equip modules types functions
	int GetEnchantStats(int AttributeID) const { return Info().GetInfoEnchantStats(AttributeID, m_Enchant); }
	int GetEnchantPrice() const { return Info().GetEnchantPrice(m_Enchant); }

	bool IsEquipped() const { return m_Count > 0 && m_Settings > 0 && (Info().m_Type == ItemType::TYPE_POTION || Info().m_Type == ItemType::TYPE_SETTINGS || Info().m_Type == ItemType::TYPE_MODULE || Info().m_Type == ItemType::TYPE_EQUIP); }
	bool IsEnchantMaxLevel() const { return Info().IsEnchantMaxLevel(m_Enchant); }

	void FormatEnchantLevel(char* pBuffer, int Size) const { Info().FormatEnchantLevel(pBuffer, Size, m_Enchant); }
	void FormatAttributes(char* pBuffer, int Size) const { Info().FormatAttributes(pBuffer, Size, m_Enchant); }
};
typedef CInventoryItem InventoryItem;

#endif