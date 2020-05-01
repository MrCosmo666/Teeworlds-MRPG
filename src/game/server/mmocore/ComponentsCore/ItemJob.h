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
		char iItemName[32];
		char iItemDesc[64];
		char iItemIcon[16];
		int Type;
		int Function;
		bool Notify;
		int Dysenthis;
		int MinimalPrice;
		short Stat[STATS_MAX_FOR_ITEM];
		int StatCount[STATS_MAX_FOR_ITEM];
		int MaximalEnchant;
		int iItemEnchantPrice;
		int ItemProjID;

		const char* GetName(CPlayer* pPlayer = NULL) const;
		const char* GetDesc(CPlayer* pPlayer = NULL) const;
		const char* GetIcon() const { return iItemIcon; };
		bool IsEnchantable() const;

		int CheckStatsID(int BonusID)
		{
			for (short i : Stat)
			{
				if (BonusID == i)
					return i;
			}
			return -1;
		}
	};

	int SecureCheck(CPlayer *pPlayer, int ItemID, int Count, int Settings, int Enchant);
	int DeSecureCheck(CPlayer *pPlayer, int ItemID, int Count, int Settings);

public:
	typedef ClassItemInformation ItemInformation;
	static std::map < int , ItemInformation > ItemsInfo;


	class ClassItems
	{
		CPlayer* pPlayer;
		int itemid_;

	public:
		int Count;
		int Settings;
		int Enchant;
		int Durability;

		ClassItems() : pPlayer(NULL), itemid_(0), Count(0), Settings(0), Enchant(0), Durability(100) {};

		void SetBasic(CPlayer* Player, int itemid) { pPlayer = Player, itemid_ = itemid; }

		int GetID() const { return itemid_; }

		bool Remove(int arg_removecount, int arg_settings = 0);
		bool Add(int arg_count, int arg_settings = 0, int arg_enchant = 0, bool arg_message = true);
		int EnchantPrice() const;
		bool SetEnchant(int arg_enchantlevel);
		bool SetSettings(int arg_settings);
		bool EquipItem();
		bool Save();
		bool IsEquipped();

		ItemInformation& Info() const { return ItemsInfo[itemid_]; };

		// копирование элемента
		ClassItems Paste(ClassItems right)
		{
			itemid_ = right.itemid_;
			Count = right.Count;
			Settings = right.Settings;
			Enchant = right.Enchant;

			if (pPlayer != NULL)
				Save();

			return *this;
		};
	};

	typedef ClassItems ItemPlayer;
	static std::map < int, std::map < int, ItemPlayer > > Items;

	virtual void OnInit();
	virtual void OnInitAccount(CPlayer *pPlayer);
	virtual void OnResetClient(int ClientID);
	virtual bool OnVotingMenu(CPlayer* pPlayer, const char* CMD, const int VoteID, const int VoteID2, int Get, const char* GetText);
	virtual bool OnHandleMenulist(CPlayer* pPlayer, int Menulist, bool ReplaceMenu);

	void FormatAttributes(ItemPlayer& pItem, int size, char* pformat);
	void FormatAttributes(ItemInformation& pInfoItem, int Enchant, int size, char* pformat);

	// Основное
	void ListInventory(CPlayer *pPlayer, int TypeList, bool SortedFunction = false);
	void GiveItem(short *SecureCode, CPlayer *pPlayer, int ItemID, int Count, int Settings, int Enchant);
	void RemoveItem(short *SecureCode, CPlayer *pPlayer, int ItemID, int Count, int Settings);
	void ItemSelected(CPlayer *pPlayer, const ItemPlayer &PlItem, bool Dress = false);
	int ActionItemCountAllowed(CPlayer* pPlayer, int ItemID);

	void UseItem(int ClientID, int ItemID, int Count);
	void RepairDurabilityFull(CPlayer *pPlayer);

	bool SetDurability(CPlayer *pPlayer, int ItemID, int Durability);
};

#endif