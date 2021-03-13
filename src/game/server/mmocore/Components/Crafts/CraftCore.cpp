/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#include <base/stdafx.h>

#include "CraftCore.h"

#include <game/server/gamecontext.h>
#include <teeother/system/string.h>

using namespace sqlstr;
void CCraftCore::OnInit()
{
	ResultPtr pRes = SJK.SD("*", "tw_craft_list");
	while(pRes->next())
	{
		const int ID = pRes->getInt("ID");
		CCraftData::ms_aCraft[ID].m_ReceivedItemID = pRes->getInt("GetItem");
		CCraftData::ms_aCraft[ID].m_ReceivedItemCount = pRes->getInt("GetItemCount");
		CCraftData::ms_aCraft[ID].m_Price = pRes->getInt("Price");
		CCraftData::ms_aCraft[ID].m_WorldID = pRes->getInt("WorldID");

		char aBuf[32];
		for(int i = 0; i < 3; i ++)
		{
			str_format(aBuf, sizeof(aBuf), "ItemNeed%d", i);
			CCraftData::ms_aCraft[ID].m_aItemNeedID[i] = pRes->getInt(aBuf);
		}
		str_copy(aBuf, pRes->getString("ItemNeedCount").c_str(), sizeof(aBuf));
		if (!sscanf(aBuf, "%d %d %d", &CCraftData::ms_aCraft[ID].m_aItemNeedCount[0], &CCraftData::ms_aCraft[ID].m_aItemNeedCount[1], &CCraftData::ms_aCraft[ID].m_aItemNeedCount[2]))
			dbg_msg("Error", "Error on scanf in Crafting");
	}

	Job()->ShowLoadingProgress("Crafts", CCraftData::ms_aCraft.size());
}

bool CCraftCore::OnHandleTile(CCharacter* pChr, int IndexCollision)
{
	CPlayer* pPlayer = pChr->GetPlayer();
	const int ClientID = pPlayer->GetCID();

	if (pChr->GetHelper()->TileEnter(IndexCollision, TILE_CRAFT_ZONE))
	{
		GS()->Chat(ClientID, "You can see menu in the votes!");
		pChr->m_Core.m_ProtectHooked = pChr->m_SkipDamage = true;
		GS()->ResetVotes(ClientID, pPlayer->m_OpenVoteMenu);
		return true;
	}
	else if (pChr->GetHelper()->TileExit(IndexCollision, TILE_CRAFT_ZONE))
	{
		GS()->Chat(ClientID, "You left the active zone, menu is restored!");
		pChr->m_Core.m_ProtectHooked = pChr->m_SkipDamage = false;
		GS()->ResetVotes(ClientID, pPlayer->m_OpenVoteMenu);
		return true;
	}
	return false;
}

int CCraftCore::GetFinalPrice(CPlayer* pPlayer, int CraftID) const
{
	if(!pPlayer)
		return CCraftData::ms_aCraft[CraftID].m_Price;

	int Discount = translate_to_percent_rest(CCraftData::ms_aCraft[CraftID].m_Price, pPlayer->GetSkill(SkillCraftDiscount).m_Level);
	if(pPlayer->GetItem(itTicketDiscountCraft).IsEquipped())
		Discount += translate_to_percent_rest(CCraftData::ms_aCraft[CraftID].m_Price, 20);

	return max(CCraftData::ms_aCraft[CraftID].m_Price - Discount, 0);
}

void CCraftCore::ShowCraftList(CPlayer* pPlayer, const char* TypeName, int SelectType)
{
	bool IsNotEmpty = false;
	const int ClientID = pPlayer->GetCID();
	pPlayer->m_VoteColored = GRAY_COLOR;

	for(const auto& cr : CCraftData::ms_aCraft)
	{
		ItemInformation& InfoGetItem = GS()->GetItemInfo(cr.second.m_ReceivedItemID);
		if(InfoGetItem.m_Type != SelectType || cr.second.m_WorldID != GS()->GetWorldID())
			continue;

		if(!IsNotEmpty)
		{
			GS()->AVL(ClientID, "null", "{STR}", TypeName);
			IsNotEmpty = true;
		}

		const int Price = GetFinalPrice(pPlayer, cr.first);
		int HideID = NUM_TAB_MENU + CItemInformation::ms_aItemsInfo.size() + cr.first;
		if (InfoGetItem.IsEnchantable())
		{
			GS()->AVHI(ClientID, InfoGetItem.GetIcon(), HideID, LIGHT_GRAY_COLOR, "{STR}{STR} - {INT} gold",
				(pPlayer->GetItem(cr.second.m_ReceivedItemID).m_Count ? "✔ " : "\0"), InfoGetItem.GetName(pPlayer), Price);

			char aAttributes[128];
			InfoGetItem.FormatAttributes(aAttributes, sizeof(aAttributes), 0);
			GS()->AVM(ClientID, "null", NOPE, HideID, "{STR}", aAttributes);
		}
		else
		{
			GS()->AVHI(ClientID, InfoGetItem.GetIcon(), HideID, LIGHT_GRAY_COLOR, "{STR}x{INT} ({INT}) :: {INT} gold",
				InfoGetItem.GetName(pPlayer), cr.second.m_ReceivedItemCount, pPlayer->GetItem(cr.second.m_ReceivedItemID).m_Count, Price);
		}
		GS()->AVM(ClientID, "null", NOPE, HideID, "{STR}", InfoGetItem.GetDesc(pPlayer));

		for(int i = 0; i < 3; i++)
		{
			const int SearchItemID = cr.second.m_aItemNeedID[i];
			const int SearchCount = cr.second.m_aItemNeedCount[i];
			if(SearchItemID <= 0 || SearchCount <= 0)
				continue;

			InventoryItem &PlSearchItem = pPlayer->GetItem(SearchItemID);
			GS()->AVMI(ClientID, PlSearchItem.Info().GetIcon(), "null", NOPE, HideID, "{STR} {INT}({INT})", PlSearchItem.Info().GetName(pPlayer), SearchCount, PlSearchItem.m_Count);
		}
		GS()->AVM(ClientID, "CRAFT", cr.first, HideID, "Craft {STR}", InfoGetItem.GetName(pPlayer));
	}

	if(IsNotEmpty)
		GS()->AV(ClientID, "null");
}

void CCraftCore::CraftItem(CPlayer *pPlayer, int CraftID)
{
	const int ClientID = pPlayer->GetCID();
	InventoryItem& PlayerCraftItem = pPlayer->GetItem(CCraftData::ms_aCraft[CraftID].m_ReceivedItemID);
	if (PlayerCraftItem.Info().IsEnchantable() && PlayerCraftItem.m_Count > 0)
	{
		GS()->Chat(ClientID, "Enchant item maximal count x1 in a backpack!");
		return;
	}

	// first podding set what is available and required for removal
	dynamic_string Buffer;
	for(int i = 0; i < 3; i++)
	{
		int SearchItemID = CCraftData::ms_aCraft[CraftID].m_aItemNeedID[i];
		int SearchCount = CCraftData::ms_aCraft[CraftID].m_aItemNeedCount[i];
		if(SearchItemID <= 0 || SearchCount <= 0 || pPlayer->GetItem(SearchItemID).m_Count >= SearchCount)
			continue;

		char aBuf[48];
		int ItemLeft = SearchCount - pPlayer->GetItem(SearchItemID).m_Count;
		str_format(aBuf, sizeof(aBuf), "%sx%d ", GS()->GetItemInfo(SearchItemID).GetName(pPlayer), ItemLeft);
		Buffer.append((const char*)aBuf);
	}
	if(Buffer.length() > 0)
	{
		GS()->Chat(ClientID, "Item left: {STR}", Buffer.buffer());
		Buffer.clear();
		return;
	}
	Buffer.clear();

	// we are already organizing the crafting
	const int Price = GetFinalPrice(pPlayer, CraftID);
	if(!pPlayer->SpendCurrency(Price))
		return;

	// delete ticket if equipped
	bool TickedDiscountCraft = pPlayer->GetItem(itTicketDiscountCraft).IsEquipped();
	if(TickedDiscountCraft)
	{
		pPlayer->GetItem(itTicketDiscountCraft).Remove(1);
		GS()->Chat(ClientID, "You used item {STR} and get discount 25%.", GS()->GetItemInfo(itTicketDiscountCraft).m_aName);
	}

	for(int i = 0; i < 3; i++)
	{
		int SearchItemID = CCraftData::ms_aCraft[CraftID].m_aItemNeedID[i];
		int SearchCount = CCraftData::ms_aCraft[CraftID].m_aItemNeedCount[i];
		if(SearchItemID <= 0 || SearchCount <= 0)
			continue;

		pPlayer->GetItem(SearchItemID).Remove(SearchCount);
	}

	const int CraftGetCount = CCraftData::ms_aCraft[CraftID].m_ReceivedItemCount;
	PlayerCraftItem.Add(CraftGetCount);
	if(PlayerCraftItem.Info().IsEnchantable())
	{
		GS()->Chat(-1, "{STR} crafted [{STR}x{INT}].", Server()->ClientName(ClientID), PlayerCraftItem.Info().GetName(), CraftGetCount);
		return;
	}

	GS()->CreatePlayerSound(ClientID, SOUND_ITEM_SELL_BUY);
	GS()->Chat(ClientID, "You crafted [{STR}x{INT}].", PlayerCraftItem.Info().GetName(pPlayer), CraftGetCount);
	GS()->ResetVotes(ClientID, pPlayer->m_OpenVoteMenu);
}

bool CCraftCore::OnHandleVoteCommands(CPlayer* pPlayer, const char* CMD, const int VoteID, const int VoteID2, int Get, const char* GetText)
{
	if(PPSTR(CMD, "CRAFT") == 0)
	{
		CraftItem(pPlayer, VoteID);
		return true;
	}

	return false;
}

bool CCraftCore::OnHandleMenulist(CPlayer* pPlayer, int Menulist, bool ReplaceMenu)
{
	const int ClientID = pPlayer->GetCID();
	if (ReplaceMenu)
	{
		CCharacter* pChr = pPlayer->GetCharacter();
		if (!pChr || !pChr->IsAlive())
			return false;

		if (pChr->GetHelper()->BoolIndex(TILE_CRAFT_ZONE))
		{
			GS()->AVH(ClientID, TAB_INFO_CRAFT, GREEN_COLOR, "Crafting Information");
			GS()->AVM(ClientID, "null", NOPE, TAB_INFO_CRAFT, "If you will not have enough items for crafting");
			GS()->AVM(ClientID, "null", NOPE, TAB_INFO_CRAFT, "You will write those and the amount that is still required");
			GS()->AV(ClientID, "null");
			GS()->ShowVotesItemValueInformation(pPlayer);
			GS()->AV(ClientID, "null");

			ShowCraftList(pPlayer, "Craft | Can be used's", TYPE_USED);
			ShowCraftList(pPlayer, "Craft | Potion's", TYPE_POTION);
			ShowCraftList(pPlayer, "Craft | Equipment's", TYPE_EQUIP);
			ShowCraftList(pPlayer, "Craft | Module's", TYPE_MODULE);
			ShowCraftList(pPlayer, "Craft | Decoration's", TYPE_DECORATION);
			ShowCraftList(pPlayer, "Craft | Other's", TYPE_OTHER);
			return true;
		}
		return false;
	}

	return false;
}
