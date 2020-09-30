/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#include <teeother/components/localization.h>

#include <game/server/gamecontext.h>
#include "ItemInformation.h"

using namespace sqlstr;

const char *CItemInformation::GetName(CPlayer *pPlayer) const
{
	if(!pPlayer) return m_aName;
	return pPlayer->GS()->Server()->Localization()->Localize(pPlayer->GetLanguage(), m_aName);
}

const char *CItemInformation::GetDesc(CPlayer *pPlayer) const
{
	if(!pPlayer) return m_aDesc;
	return pPlayer->GS()->Server()->Localization()->Localize(pPlayer->GetLanguage(), m_aDesc);	
}

bool CItemInformation::IsEnchantable() const
{
	for (int i = 0; i < STATS_MAX_FOR_ITEM; i++)
	{
		if (CGS::AttributInfo.find(m_aAttribute[i]) != CGS::AttributInfo.end() && m_aAttributeCount[i] > 0)
			return true;
	}
	return false;
}

bool CItemInformation::IsEnchantMaxLevel(int Enchant) const
{
	for(int i = 0; i < STATS_MAX_FOR_ITEM; i++)
	{
		if(CGS::AttributInfo.find(m_aAttribute[i]) == CGS::AttributInfo.end())
			continue;

		const int EnchantMax = m_aAttributeCount[i] + (int)kurosio::translate_to_procent_rest(m_aAttributeCount[i], PERCENT_MAXIMUM_ENCHANT);
		if(GetInfoEnchantStats(m_aAttribute[i], Enchant) > EnchantMax)
			return true;
	}
	return false;
}

void CItemInformation::FormatEnchantLevel(char* pBuffer, int Size, int Enchant) const
{
	if(Enchant > 0)
	{
		char aBuf[128];
		str_format(aBuf, sizeof(aBuf), "[%s]", IsEnchantMaxLevel(Enchant) ? "Max" : std::string("+" + std::to_string(Enchant)).c_str());
		str_copy(pBuffer, aBuf, Size);
		return;
	}
	str_copy(pBuffer, "\0", Size);
}