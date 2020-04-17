/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#ifndef GAME_SERVER_SQLITEM_H
#define GAME_SERVER_SQLITEM_H

#include "../component.h"

class ItemSql : public CMmoComponent
{
	// Приватные методы
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

	/*
		После познания в итогом счете нужно будет сделать мамку что будет хранить Глобальное для работы данного класса
		Но прежде всего этот класс будет наследывать функции мамки к примеру мамка хранит (pPlayer) дабы для каждого предмета не хранить отдельно
		Так-же можно внедрить список предметов внутрь что тоже хорошо обращение будет через Глобальный список предметов
		Что то рода 
		{
			ItemPlayerList *PlItem = ListItems()->Get(int index);
			PlItem->Give(5), PlItem->Remove(3), PlItem->Info()->GetName();
		}
	*/
	class ClassItems
	{
		CPlayer *pPlayer;
		int itemid_;
		
	public:
		ClassItems() : Count(NULL), Settings(NULL), Enchant(NULL), Durability(100), pPlayer(NULL), itemid_(NULL) {};

		void SetBasic(CPlayer *Player, int itemid) { pPlayer = Player, itemid_ = itemid; }
		int Count;
		int Settings;
		int Enchant;
		int Durability;
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

		// приравнивание элементу
		ClassItems& operator = (const ClassItems& right) 
		{
			itemid_ = right.itemid_;
			Count = right.Count;
			Settings = right.Settings;
			Enchant = right.Enchant;

			if(pPlayer != NULL)
				Save();

			return *this;
		};

		// копирование элемента
		ClassItems Copy(ClassItems right)
		{
			itemid_ = right.itemid_;
			Count = right.Count;
			Enchant = right.Enchant;
			return *this;
		};
	};	
	typedef ClassItems ItemPlayer;
	static std::map < int , std::map < int , ItemPlayer > > Items;

	virtual void OnInitGlobal();
	virtual void OnInitAccount(CPlayer *pPlayer);

	// Основное
	void ListInventory(CPlayer *pPlayer, int TypeList, bool SortedFunction = false);

	void GiveItem(short *SecureCode, CPlayer *pPlayer, int ItemID, int Count, int Settings, int Enchant);
	void RemoveItem(short *SecureCode, CPlayer *pPlayer, int ItemID, int Count, int Settings);
	void ItemSelected(CPlayer *pPlayer, const ItemPlayer &PlItem, bool Dress = false);
	int ActionItemCountAllowed(CPlayer* pPlayer, int ItemID);

	void RepairDurability(CPlayer *pPlayer);
	bool SetDurabilityItem(CPlayer *pPlayer, int ItemID, int Durability);
	bool SetEnchantItem(CPlayer *pPlayer, int ItemID, int Enchant);
	bool SetSettingsItem(CPlayer *pPlayer, int ItemID, int Settings);
	virtual bool OnParseVotingMenu(CPlayer *pPlayer, const char *CMD, const int VoteID, const int VoteID2, int Get, const char *GetText);	

	void UsedItems(int ClientID, int ItemID, int Count);

};

#endif