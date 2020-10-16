/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#include <game/server/gamecontext.h>
#include "ItemInventory.h"

int randomRangecount(int startrandom, int endrandom, int count)
{
	int result = 0;
	for(int i = 0; i < count; i++)
	{
		int random = startrandom + random_int() % (endrandom - startrandom);
		result += random;
	}
	return result;
}

ItemInformation& CInventoryItem::Info() const
{
	return InventoryJob::ms_aItemsInfo[m_ItemID];
};

void CInventoryItem::SetItemOwner(CPlayer* pPlayer)
{
	m_pPlayer = pPlayer;
	m_pGS = m_pPlayer->GS();
}

bool CInventoryItem::SetEnchant(int Enchant)
{
	if(m_Count < 1 || !m_pPlayer || !m_pPlayer->IsAuthed())
		return false;

	m_Enchant = Enchant;
	return Save();
}

bool CInventoryItem::SetSettings(int Settings)
{
	if(m_Count < 1 || !m_pPlayer || !m_pPlayer->IsAuthed())
		return false;

	m_Settings = Settings;
	return Save();
}

bool CInventoryItem::SetDurability(int Durability)
{
	if(m_Count < 1 || !m_pPlayer || !m_pPlayer->IsAuthed())
		return false;

	m_Durability = Durability;
	return Save();
}

bool CInventoryItem::Add(int Count, int Settings, int Enchant, bool Message)
{
	if(Count < 1 || !m_pPlayer || !m_pPlayer->IsAuthed())
		return false;

	const int ClientID = m_pPlayer->GetCID();
	if(Info().IsEnchantable())
	{
		if(m_Count > 0)
		{
			GS()->Chat(ClientID, "This item cannot have more than 1 item");
			return false;
		}
		Count = 1;
	}

	GS()->Mmo()->Item()->GiveItem(m_pPlayer, m_ItemID, Count, Settings, Enchant);

	// check the empty slot if yes then put the item on
	const bool AutoEquip = (Info().m_Type == ItemType::TYPE_EQUIP && m_pPlayer->GetEquippedItemID(Info().m_Function) <= 0) || Info().m_Type == TYPE_MODULE;
	if(AutoEquip)
	{
		if(!IsEquipped())
			Equip();

		char aAttributes[128];
		Info().FormatAttributes(aAttributes, sizeof(aAttributes), Enchant);
		GS()->Chat(ClientID, "Auto equip {STR} - {STR}", Info().GetName(m_pPlayer), aAttributes);
		GS()->CreatePlayerSound(ClientID, SOUND_ITEM_EQUIP);
	}

	if(!Message || Info().m_Type == ItemType::TYPE_SETTINGS)
		return true;

	if(Info().m_Type == ItemType::TYPE_EQUIP || Info().m_Type == ItemType::TYPE_MODULE)
		GS()->Chat(-1, "{STR} got of the {STR}x{INT}!", GS()->Server()->ClientName(ClientID), Info().GetName(), &Count);
	else if(Info().m_Type != ItemType::TYPE_INVISIBLE)
		GS()->Chat(ClientID, "You got of the {STR}x{INT}!", Info().GetName(m_pPlayer), &Count);

	return true;
}

bool CInventoryItem::Remove(int Count, int Settings)
{
	Count = min(Count, m_Count);
	if(Count <= 0 || !m_pPlayer)
		return false;

	if(IsEquipped())
		Equip();

	const int Code = GS()->Mmo()->Item()->RemoveItem(m_pPlayer, m_ItemID, Count, Settings);
	return (bool)(Code > 0);
}

bool CInventoryItem::Equip()
{
	if(m_Count < 1 || !m_pPlayer || !m_pPlayer->IsAuthed())
		return false;

	m_Settings ^= true;

	if(Info().m_Type == ItemType::TYPE_EQUIP)
	{
		const int EquipID = Info().m_Function;
		int EquipItemID = m_pPlayer->GetEquippedItemID(EquipID, m_ItemID);
		while(EquipItemID >= 1)
		{
			InventoryItem& EquipItem = m_pPlayer->GetItem(EquipItemID);
			EquipItem.SetSettings(0);
			EquipItemID = m_pPlayer->GetEquippedItemID(EquipID, m_ItemID);
		}

		if(Info().m_Function == EQUIP_DISCORD)
			GS()->Mmo()->SaveAccount(m_pPlayer, SaveType::SAVE_STATS);

		GS()->ChangeEquipSkin(m_pPlayer->GetCID(), m_ItemID);
	}

	if(m_pPlayer->GetCharacter())
		m_pPlayer->GetCharacter()->UpdateEquipingStats(m_ItemID);

	m_pPlayer->ShowInformationStats();
	return Save();
}

bool CInventoryItem::Use(int Count)
{
	Count = Info().m_Function == FUNCTION_ONE_USED ? 1 : min(Count, m_Count);
	if(Count <= 0 || !m_pPlayer || !m_pPlayer->IsAuthed())
		return false;

	const int ClientID = m_pPlayer->GetCID();
	if(m_ItemID == itPotionHealthRegen && Remove(Count, 0))
	{
		m_pPlayer->GiveEffect("RegenHealth", 15);
		GS()->ChatFollow(ClientID, "You used {STR}x{INT}", Info().GetName(m_pPlayer), &Count);
	}

	if(m_ItemID == itPotionManaRegen && Remove(Count, 0))
	{
		m_pPlayer->GiveEffect("RegenMana", 15);
		GS()->ChatFollow(ClientID, "You used {STR}x{INT}", Info().GetName(m_pPlayer), &Count);
	}

	if(m_ItemID == itPotionResurrection && Remove(Count, 0))
	{
		m_pPlayer->GetTempData().m_TempSafeSpawn = false;
		m_pPlayer->GetTempData().m_TempHealth = m_pPlayer->GetStartHealth();
		GS()->ChatFollow(ClientID, "You used {STR}x{INT}", Info().GetName(m_pPlayer), &Count);
	}

	if(m_ItemID == itTicketDiscountCraft)
	{
		GS()->Chat(ClientID, "This item can only be used (Auto Use, and then craft).");
	}

	if(m_ItemID == itCapsuleSurvivalExperience && Remove(Count, 0))
	{
		int Getting = randomRangecount(10, 50, Count);
		GS()->Chat(-1, "{STR} used {STR}x{INT} and got {INT} Survival Experience.", GS()->Server()->ClientName(ClientID), Info().GetName(), &Count, &Getting);
		m_pPlayer->AddExp(Getting);
	}

	if(m_ItemID == itLittleBagGold && Remove(Count, 0))
	{
		int Getting = randomRangecount(10, 50, Count);
		GS()->Chat(-1, "{STR} used {STR}x{INT} and got {INT} gold.", GS()->Server()->ClientName(ClientID), Info().GetName(), &Count, &Getting);
		m_pPlayer->AddMoney(Getting);
	}

	if(m_ItemID == itTicketResetClassStats && Remove(Count, 0))
	{
		int BackUpgrades = 0;
		for(const auto& at : CGS::ms_aAttributsInfo)
		{
			if(str_comp_nocase(at.second.m_aFieldName, "unfield") == 0 || at.second.m_UpgradePrice <= 0 || m_pPlayer->Acc().m_aStats[at.first] <= 0)
				continue;

			// skip weapon spreading
			if(at.second.m_Type == AtributType::AtWeapon)
				continue;

			BackUpgrades += (int)(m_pPlayer->Acc().m_aStats[at.first] * at.second.m_UpgradePrice);
			m_pPlayer->Acc().m_aStats[at.first] = 0;
		}

		GS()->Chat(-1, "{STR} used {STR} returned {INT} upgrades.", GS()->Server()->ClientName(ClientID), Info().GetName(), &BackUpgrades);
		m_pPlayer->Acc().m_Upgrade += BackUpgrades;
		GS()->Mmo()->SaveAccount(m_pPlayer, SaveType::SAVE_UPGRADES);
	}

	if(m_ItemID == itTicketResetWeaponStats && Remove(Count, 0))
	{
		int BackUpgrades = 0;
		for(const auto& at : CGS::ms_aAttributsInfo)
		{
			if(str_comp_nocase(at.second.m_aFieldName, "unfield") == 0 || at.second.m_UpgradePrice <= 0 || m_pPlayer->Acc().m_aStats[at.first] <= 0)
				continue;

			// skip all stats allow only weapons
			if(at.second.m_Type != AtributType::AtWeapon)
				continue;

			int BackCount = m_pPlayer->Acc().m_aStats[at.first];
			if(at.first == Stats::StSpreadShotgun)
				BackCount = m_pPlayer->Acc().m_aStats[at.first] - 3;
			else if(at.first == Stats::StSpreadGrenade || at.first == Stats::StSpreadRifle)
				BackCount = m_pPlayer->Acc().m_aStats[at.first] - 1;

			if(BackCount <= 0)
				continue;

			BackUpgrades += (int)(BackCount * at.second.m_UpgradePrice);
			m_pPlayer->Acc().m_aStats[at.first] -= BackCount;
		}

		GS()->Chat(-1, "{STR} used {STR} returned {INT} upgrades.", GS()->Server()->ClientName(ClientID), Info().GetName(), &BackUpgrades);
		m_pPlayer->Acc().m_Upgrade += BackUpgrades;
		GS()->Mmo()->SaveAccount(m_pPlayer, SaveType::SAVE_UPGRADES);
	}

	GS()->UpdateVotes(ClientID, MenuList::MENU_INVENTORY);
	return true;
}

bool CInventoryItem::Drop(int Count)
{
	Count = min(Count, m_Count);
	if(Count <= 0 || !m_pPlayer || !m_pPlayer->IsAuthed() || !m_pPlayer->GetCharacter())
		return false;

	CCharacter* m_pCharacter = m_pPlayer->GetCharacter();
	vec2 Force = vec2(m_pCharacter->m_Core.m_Input.m_TargetX, m_pCharacter->m_Core.m_Input.m_TargetY);
	if(length(Force) > 8.0f)
		Force = normalize(Force) * 8.0f;

	CInventoryItem DropItem = *this;
	DropItem.m_Count = Count;
	if(Remove(Count))
	{
		GS()->CreateDropItem(m_pCharacter->m_Core.m_Pos, -1, DropItem, Force);
		return true;
	}
	return false;
}

bool CInventoryItem::Save()
{
	if(m_pPlayer && m_pPlayer->IsAuthed())
	{
		SJK.UD("tw_accounts_items", "Count = '%d', Settings = '%d', Enchant = '%d', Durability = '%d' WHERE OwnerID = '%d' AND ItemID = '%d'",
			m_Count, m_Settings, m_Enchant, m_Durability, m_pPlayer->Acc().m_AuthID, m_ItemID);
		return true;
	}
	return false;
}

int CInventoryItem::GetEnchantPrice() const
{
	int FinishedPrice = 0;
	for(int i = 0; i < STATS_MAX_FOR_ITEM; i++)
	{
		if(CGS::ms_aAttributsInfo.find(Info().m_aAttribute[i]) == CGS::ms_aAttributsInfo.end())
			continue;

		int UpgradePrice;
		const int Attribute = Info().m_aAttribute[i];
		const int TypeAttribute = CGS::ms_aAttributsInfo[Attribute].m_Type;

		if(TypeAttribute == AtributType::AtHardtype || Attribute == Stats::StStrength)
		{
			UpgradePrice = max(12, CGS::ms_aAttributsInfo[Attribute].m_UpgradePrice) * 14;
		}
		else if(TypeAttribute == AtributType::AtJob || TypeAttribute == AtributType::AtWeapon || Attribute == Stats::StLuckyDropItem)
		{
			UpgradePrice = max(20, CGS::ms_aAttributsInfo[Attribute].m_UpgradePrice) * 14;
		}
		else
		{
			UpgradePrice = max(4, CGS::ms_aAttributsInfo[Attribute].m_UpgradePrice) * 5;
		}

		const int PercentEnchant = max(1, (int)kurosio::translate_to_procent_rest(Info().m_aAttributeCount[i], PERCENT_OF_ENCHANT));
		FinishedPrice += UpgradePrice * (PercentEnchant * (1 + m_Enchant));
	}
	return FinishedPrice;
}
