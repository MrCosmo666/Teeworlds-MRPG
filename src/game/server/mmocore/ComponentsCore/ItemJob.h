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
		char Name[32];
		char Desc[64];
		char Icon[16];
		int Type;
		int Function;
		bool Notify;
		int Dysenthis;
		int MinimalPrice;
		short Attribute[STATS_MAX_FOR_ITEM];
		int AttributeCount[STATS_MAX_FOR_ITEM];
		int MaximalEnchant;
		int EnchantPrice;
		int ProjID;
		
		int GetStatsBonus(int AttributeID) const
		{
			for(int i = 0; i < STATS_MAX_FOR_ITEM; i++)
			{
				if(Attribute[i] == AttributeID)
					return AttributeCount[i];
			}
			return -1;
		}
		const char* GetName(CPlayer* pPlayer = NULL) const;
		const char* GetDesc(CPlayer* pPlayer = NULL) const;
		const char* GetIcon() const { return Icon; };
		bool IsEnchantable() const;
	};

	int SecureCheck(CPlayer *pPlayer, int ItemID, int Count, int Settings, int Enchant);
	int DeSecureCheck(CPlayer *pPlayer, int ItemID, int Count, int Settings);

public:
	typedef ClassItemInformation ItemInformation;
	static std::map < int , ItemInformation > ItemsInfo;

	// TODO: Change it bad
	class ClassItems
	{
		CPlayer* m_pPlayer;
		int itemid_;

	public:
		ClassItems() : Count(0), Settings(0), Enchant(0), Durability(0) {};
		ClassItems(CPlayer* pPlayer, int ItemID) : m_pPlayer(pPlayer), itemid_(ItemID), Count(0), Settings(0), Enchant(0), Durability(0) {};

		int Count;
		int Settings;
		int Enchant;
		int Durability;

		bool SetDurability(int arg_durability);
		bool SetEnchant(int arg_enchantlevel);
		bool SetSettings(int arg_settings);
		bool Remove(int arg_removecount, int arg_settings = 0);
		bool Add(int arg_count, int arg_settings = 0, int arg_enchant = 0, bool arg_message = true);

		bool Equip();
		bool Save();
	
		int GetID() const { return itemid_; }
		int EnchantPrice() const { return Info().EnchantPrice * (Enchant + 1); }
		bool IsEquipped() const { return Count > 0 && Settings > 0 && (Info().Type == ItemType::TYPE_SETTINGS || Info().Type == ItemType::TYPE_MODULE || Info().Type == ItemType::TYPE_EQUIP); }
		ItemInformation& Info() const { return ItemsInfo[itemid_]; };
	};

	typedef ClassItems InventoryItem;
	static std::map < int, std::map < int, InventoryItem > > Items;

	virtual void OnInit();
	virtual void OnInitAccount(CPlayer *pPlayer);
	virtual void OnResetClient(int ClientID);
	virtual bool OnVotingMenu(CPlayer* pPlayer, const char* CMD, const int VoteID, const int VoteID2, int Get, const char* GetText);
	virtual bool OnHandleMenulist(CPlayer* pPlayer, int Menulist, bool ReplaceMenu);

	void FormatAttributes(InventoryItem& pItem, int size, char* pformat);
	void FormatAttributes(ItemInformation& pInfoItem, int Enchant, int size, char* pformat);

	// Основное
	void ListInventory(CPlayer *pPlayer, int TypeList, bool SortedFunction = false);
	void GiveItem(short *SecureCode, CPlayer *pPlayer, int ItemID, int Count, int Settings, int Enchant);
	void RemoveItem(short *SecureCode, CPlayer *pPlayer, int ItemID, int Count, int Settings);
	void ItemSelected(CPlayer *pPlayer, const InventoryItem& pPlayerItem, bool Dress = false);
	int ActionItemCountAllowed(CPlayer* pPlayer, int ItemID);

	void UseItem(int ClientID, int ItemID, int Count);
	void RepairDurabilityFull(CPlayer *pPlayer);
	int GetCountItemsType(CPlayer* pPlayer, int Type) const;

	// TODO: FIX IT (lock .. unlock)
	void AddItemSleep(int AccountID, int ItemID, int GiveCount, int Milliseconds);
};

#endif