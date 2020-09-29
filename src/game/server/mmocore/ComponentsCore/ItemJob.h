/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#ifndef GAME_SERVER_SQLITEM_H
#define GAME_SERVER_SQLITEM_H

#include "../MmoComponent.h"

// define for quick fine-tuning of enchantment
#define PERCENT_OF_ENCHANT 8
#define PERCENT_MAXIMUM_ENCHANT 50

class ItemJob : public MmoComponent
{
	class ClassItemInformation
	{
	public:
		char m_aName[32];
		char m_aDesc[64];
		char m_aIcon[16];
		int m_Type;
		int m_Function;
		int m_Dysenthis;
		int m_MinimalPrice;
		short m_aAttribute[STATS_MAX_FOR_ITEM];
		int m_aAttributeCount[STATS_MAX_FOR_ITEM];
		int m_ProjID;
		
		int GetInfoEnchantStats(int AttributeID) const
		{
			for(int i = 0; i < STATS_MAX_FOR_ITEM; i++)
			{
				if(m_aAttribute[i] > 0 && m_aAttribute[i] == AttributeID)
					return m_aAttributeCount[i];
			}
			return 0;
		}

		int GetInfoEnchantStats(int AttributeID, int Enchant) const
		{
			const int StatSize = GetInfoEnchantStats(AttributeID);
			if(StatSize <= 0)
				return 0;

			const int PercentEnchant = (int)kurosio::translate_to_procent_rest(StatSize, PERCENT_OF_ENCHANT);
			int EnchantStat = StatSize + PercentEnchant * (1 + Enchant);
			
			// the case when with percent will back 0
			if(PercentEnchant <= 0)
				EnchantStat += Enchant;

			return EnchantStat;
		}
		bool IsEnchantable() const;

		const char* GetName(CPlayer* pPlayer = NULL) const;
		const char* GetDesc(CPlayer* pPlayer = NULL) const;
		const char* GetIcon() const { return m_aIcon; };
	};

	int SecureCheck(CPlayer *pPlayer, int ItemID, int Count, int Settings, int Enchant);
	int DeSecureCheck(CPlayer *pPlayer, int ItemID, int Count, int Settings);

public:
	typedef ClassItemInformation ItemInformation;
	static std::map < int , ItemInformation > ms_aItemsInfo;

	// TODO: Change it bad
	class ClassItems
	{
		CPlayer* m_pPlayer;
		int m_ItemID;

	public:
		int m_Count;
		int m_Settings;
		int m_Enchant;
		int m_Durability;

		ClassItems() : m_Count(0), m_Settings(0), m_Enchant(0), m_Durability(0) {};
		ClassItems(CPlayer* pPlayer, int ItemID) : m_pPlayer(pPlayer), m_ItemID(ItemID), m_Count(0), m_Settings(0), m_Enchant(0), m_Durability(0) {};

		void SetPlayer(CPlayer* pPlayer) { m_pPlayer = pPlayer; }
		bool SetDurability(int Durability);
		bool SetEnchant(int Enchant);
		bool SetSettings(int Settings);
		bool Remove(int Count, int Settings = 0);
		bool Add(int Count, int Settings = 0, int Enchant = 0, bool Message = true);
		bool Equip();
		bool Save();

		bool IsEnchantMaxLevel() const 
		{
			for(int i = 0; i < STATS_MAX_FOR_ITEM; i++)
			{
				const int EnchantMax = Info().m_aAttributeCount[i] + (int)kurosio::translate_to_procent_rest(Info().m_aAttributeCount[i], PERCENT_MAXIMUM_ENCHANT);
				if(GetEnchantStats(Info().m_aAttribute[i]) > EnchantMax)
					return true;
			}
			return false;
		}
		bool IsEquipped() const { return m_Count > 0 && m_Settings > 0 && (Info().m_Type == ItemType::TYPE_POTION || Info().m_Type == ItemType::TYPE_SETTINGS || Info().m_Type == ItemType::TYPE_MODULE || Info().m_Type == ItemType::TYPE_EQUIP); }

		int GetID() const { return m_ItemID; }
		int GetEnchantStats(int AttributeID) const { return Info().GetInfoEnchantStats(AttributeID, m_Enchant); }
		int GetEnchantPrice() const;
		
		ItemInformation& Info() const { return ms_aItemsInfo[m_ItemID]; };
	};

	typedef ClassItems InventoryItem;
	static std::map < int, std::map < int, InventoryItem > > ms_aItems;

	virtual void OnInit();
	virtual void OnInitAccount(CPlayer *pPlayer);
	virtual void OnResetClient(int ClientID);
	virtual bool OnVotingMenu(CPlayer* pPlayer, const char* CMD, const int VoteID, const int VoteID2, int Get, const char* GetText);
	virtual bool OnHandleMenulist(CPlayer* pPlayer, int Menulist, bool ReplaceMenu);

	void FormatAttributes(InventoryItem& pItem, int size, char* pformat);
	void FormatAttributes(ItemInformation& pInfoItem, int Enchant, int size, char* pformat);

	// primary
	void ListInventory(CPlayer *pPlayer, int TypeList, bool SortedFunction = false);
	int GiveItem(CPlayer *pPlayer, int ItemID, int Count, int Settings, int Enchant);
	int RemoveItem(CPlayer *pPlayer, int ItemID, int Count, int Settings);
	void ItemSelected(CPlayer *pPlayer, const InventoryItem& pPlayerItem, bool Dress = false);
	int ActionItemCountAllowed(CPlayer* pPlayer, int ItemID);

	void UseItem(int ClientID, int ItemID, int Count);
	void RepairDurabilityFull(CPlayer *pPlayer);
	int GetCountItemsType(CPlayer* pPlayer, int Type) const;

	// TODO: FIX IT (lock .. unlock)
	void AddItemSleep(int AccountID, int ItemID, int GiveCount, int Milliseconds);
};

#endif