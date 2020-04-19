/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#ifndef GAME_SERVER_SQLITEM_H
#define GAME_SERVER_SQLITEM_H

#include "../component.h"

class ItemSql : public CMmoComponent
{
	int SecureCheck(CPlayer *pPlayer, int ItemID, int Count, int Settings, int Enchant);
	int DeSecureCheck(CPlayer *pPlayer, int ItemID, int Count, int Settings);

public:

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
		short BonusID;
		int BonusCount;
		int MaximalEnchant;
		int iItemEnchantPrice;
		bool Dropable;
	
		const char *GetName(CPlayer *pPlayer = NULL) const;
		const char *GetDesc(CPlayer *pPlayer = NULL) const;
		const char *GetIcon() const { return iItemIcon; };
	};
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
		int EnchantMaterCount() const;
		void SetEnchant(int arg_enchantlevel);
		bool SetSettings(int arg_settings);
		bool EquipItem();
		void Save();

		bool IsEquipped();

		// информацияп предмета нахуй всякие ItemInformation где сможет быть игрок
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

	virtual void OnInitGlobal();
	virtual void OnInitAccount(CPlayer *pPlayer);
	virtual bool OnParseVotingMenu(CPlayer* pPlayer, const char* CMD, const int VoteID, const int VoteID2, int Get, const char* GetText);
	virtual bool OnPlayerHandleMainMenu(CPlayer* pPlayer, int Menulist);

	// Основное
	void ListInventory(CPlayer *pPlayer, int TypeList, bool SortedFunction = false);
	void GiveItem(short *SecureCode, CPlayer *pPlayer, int ItemID, int Count, int Settings, int Enchant);
	void RemoveItem(short *SecureCode, CPlayer *pPlayer, int ItemID, int Count, int Settings);
	void ItemSelected(CPlayer *pPlayer, const ItemPlayer &PlItem, bool Dress = false);
	int ActionItemCountAllowed(CPlayer* pPlayer, int ItemID);

	void UseItem(int ClientID, int ItemID, int Count);
	void RepairDurabilityFull(CPlayer *pPlayer);
	bool SetDurability(CPlayer *pPlayer, int ItemID, int Durability);
	bool SetEnchant(CPlayer *pPlayer, int ItemID, int Enchant);
	bool SetSettings(CPlayer *pPlayer, int ItemID, int Settings);


};

#endif