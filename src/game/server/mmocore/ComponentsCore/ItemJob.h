/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#ifndef GAME_SERVER_SQLITEM_H
#define GAME_SERVER_SQLITEM_H

#include "../MmoComponent.h"

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
		int m_MaximalEnchant;
		int m_ProjID;
		
		int GetStatsBonus(int AttributeID) const
		{
			for(int i = 0; i < STATS_MAX_FOR_ITEM; i++)
			{
				if(m_aAttribute[i] == AttributeID)
					return m_aAttributeCount[i];
			}
			return -1;
		}
		const char* GetName(CPlayer* pPlayer = NULL) const;
		const char* GetDesc(CPlayer* pPlayer = NULL) const;
		const char* GetIcon() const { return m_aIcon; };
		bool IsEnchantable() const;
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
		int itemid_;

	public:
		ClassItems() : m_Count(0), m_Settings(0), m_Enchant(0), m_Durability(0) {};
		ClassItems(CPlayer* pPlayer, int ItemID) : m_pPlayer(pPlayer), itemid_(ItemID), m_Count(0), m_Settings(0), m_Enchant(0), m_Durability(0) {};
		void SetPlayer(CPlayer* pPlayer) { m_pPlayer = pPlayer; }

		int m_Count;
		int m_Settings;
		int m_Enchant;
		int m_Durability;

		bool SetDurability(int arg_durability);
		bool SetEnchant(int arg_enchantlevel);
		bool SetSettings(int arg_settings);
		bool Remove(int arg_removecount, int arg_settings = 0);
		bool Add(int arg_count, int arg_settings = 0, int arg_enchant = 0, bool arg_message = true);

		bool Equip();
		bool Save();

		int EnchantPrice() const;
		int GetID() const { return itemid_; }
		bool IsEquipped() const { return m_Count > 0 && m_Settings > 0 && (Info().m_Type == ItemType::TYPE_POTION || Info().m_Type == ItemType::TYPE_SETTINGS || Info().m_Type == ItemType::TYPE_MODULE || Info().m_Type == ItemType::TYPE_EQUIP); }
		ItemInformation& Info() const { return ms_aItemsInfo[itemid_]; };
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