/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#ifndef GAME_SERVER_ITEM_INFORMATION_H
#define GAME_SERVER_ITEM_INFORMATION_H

// define for quick fine-tuning of enchantment
#define PERCENT_OF_ENCHANT 8
#define PERCENT_MAXIMUM_ENCHANT 50

class CItemInformation
{
public:
	char m_aName[32];
	char m_aDesc[64];
	char m_aIcon[16];
	int m_Type;
	int m_Function;
	int m_Dysenthis;
	int m_MinimalPrice;
	int m_aAttribute[STATS_MAX_FOR_ITEM];
	int m_aAttributeCount[STATS_MAX_FOR_ITEM];
	int m_ProjID;

	// main functions
	const char* GetName(class CPlayer* pPlayer = nullptr) const;
	const char* GetDesc(class CPlayer* pPlayer = nullptr) const;
	const char* GetIcon() const { return m_aIcon; };

	// equip modules types functions
	int GetInfoEnchantStats(int AttributeID) const;
	int GetInfoEnchantStats(int AttributeID, int Enchant) const;

	bool IsEnchantable() const;
	bool IsEnchantMaxLevel(int Enchant) const;

	void FormatAttributes(char* pBuffer, int Size, int Enchant) const;
	void FormatEnchantLevel(char* pBuffer, int Size, int Enchant) const;
};
typedef CItemInformation ItemInformation;

#endif