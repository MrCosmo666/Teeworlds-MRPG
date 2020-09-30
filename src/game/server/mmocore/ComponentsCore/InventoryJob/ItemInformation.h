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
	bool IsEnchantMaxLevel(int Enchant) const;
	void FormatEnchantLevel(char* pBuffer, int Size, int Enchant) const;

	const char* GetName(class CPlayer* pPlayer = nullptr) const;
	const char* GetDesc(class CPlayer* pPlayer = nullptr) const;
	const char* GetIcon() const { return m_aIcon; };
};
typedef CItemInformation ItemInformation;

#endif