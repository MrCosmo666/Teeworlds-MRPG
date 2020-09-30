/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#include <teeother/system/string.h>

#include <game/server/gamecontext.h>
#include "CraftJob.h"

using namespace sqlstr;
std::map < int , CraftJob::CraftStruct > CraftJob::ms_aCraft;

void CraftJob::OnInit()
{
	std::shared_ptr<ResultSet> RES(SJK.SD("*", "tw_craft_list"));
	while(RES->next())
	{
		const int ID = RES->getInt("ID");
		ms_aCraft[ID].m_ReceivedItemID = RES->getInt("GetItem");
		ms_aCraft[ID].m_ReceivedItemCount = RES->getInt("GetItemCount");
		ms_aCraft[ID].m_Price = RES->getInt("Price");
		ms_aCraft[ID].m_WorldID = RES->getInt("WorldID");

		char aBuf[32];
		for(int i = 0; i < 3; i ++)
		{
			str_format(aBuf, sizeof(aBuf), "ItemNeed%d", i);
			ms_aCraft[ID].m_aItemNeedID[i] = RES->getInt(aBuf);
		}
		str_copy(aBuf, RES->getString("ItemNeedCount").c_str(), sizeof(aBuf));
		if (!sscanf(aBuf, "%d %d %d", &ms_aCraft[ID].m_aItemNeedCount[0], &ms_aCraft[ID].m_aItemNeedCount[1], &ms_aCraft[ID].m_aItemNeedCount[2]))
			dbg_msg("Error", "Error on scanf in Crafting");

	}
	Job()->ShowLoadingProgress("Crafts", ms_aCraft.size());	
}

bool CraftJob::OnHandleTile(CCharacter* pChr, int IndexCollision)
{
	CPlayer* pPlayer = pChr->GetPlayer();
	const int ClientID = pPlayer->GetCID();

	if (pChr->GetHelper()->TileEnter(IndexCollision, TILE_CRAFT_ZONE))
	{
		GS()->Chat(ClientID, "You can see menu in the votes!");
		pChr->m_Core.m_ProtectHooked = pChr->m_NoAllowDamage = true;
		GS()->ResetVotes(ClientID, pPlayer->m_OpenVoteMenu);
		return true;
	}
	else if (pChr->GetHelper()->TileExit(IndexCollision, TILE_CRAFT_ZONE))
	{
		GS()->Chat(ClientID, "You left the active zone, menu is restored!");
		pChr->m_Core.m_ProtectHooked = pChr->m_NoAllowDamage = false;
		GS()->ResetVotes(ClientID, pPlayer->m_OpenVoteMenu);
		return true;
	}
	return false;
}

int CraftJob::GetFinalPrice(CPlayer* pPlayer, int CraftID) const
{
	if(!pPlayer)
		return ms_aCraft[CraftID].m_Price;

	const int ClientID = pPlayer->GetCID();
	int Discount = (int)kurosio::translate_to_procent_rest(ms_aCraft[CraftID].m_Price, Job()->Skills()->GetSkillLevel(ClientID, SkillCraftDiscount));
	if(pPlayer->GetItem(itTicketDiscountCraft).IsEquipped())
		Discount += (int)kurosio::translate_to_procent_rest(ms_aCraft[CraftID].m_Price, 20);

	return max(ms_aCraft[CraftID].m_Price - Discount, 0);
}

void CraftJob::ShowCraftList(CPlayer* pPlayer, const char* TypeName, int SelectType)
{
	bool IsNotEmpty = false;
	const int ClientID = pPlayer->GetCID();
	pPlayer->m_Colored = GRAY_COLOR;

	for(const auto& cr : ms_aCraft)
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
		int HideID = NUM_TAB_MENU + InventoryJob::ms_aItemsInfo.size() + cr.first;
		if (InfoGetItem.IsEnchantable())
		{
			GS()->AVHI(ClientID, InfoGetItem.GetIcon(), HideID, LIGHT_GRAY_COLOR, "{STR}{STR} - {INT} gold",
				(pPlayer->GetItem(cr.second.m_ReceivedItemID).m_Count ? "✔ " : "\0"), InfoGetItem.GetName(pPlayer), &Price);

			char aAttributes[128];
			InfoGetItem.FormatAttributes(aAttributes, sizeof(aAttributes), 0);
			GS()->AVM(ClientID, "null", NOPE, HideID, "{STR}", aAttributes);
		}
		else
		{
			GS()->AVHI(ClientID, InfoGetItem.GetIcon(), HideID, LIGHT_GRAY_COLOR, "{STR}x{INT} ({INT}) :: {INT} gold",
				InfoGetItem.GetName(pPlayer), &cr.second.m_ReceivedItemCount, &pPlayer->GetItem(cr.second.m_ReceivedItemID).m_Count, &Price);
		}
		GS()->AVM(ClientID, "null", NOPE, HideID, "{STR}", InfoGetItem.GetDesc(pPlayer));

		for(int i = 0; i < 3; i++)
		{
			const int SearchItemID = cr.second.m_aItemNeedID[i];
			const int SearchCount = cr.second.m_aItemNeedCount[i];
			if(SearchItemID <= 0 || SearchCount <= 0) 
				continue;
			
			InventoryItem &PlSearchItem = pPlayer->GetItem(SearchItemID);
			GS()->AVMI(ClientID, PlSearchItem.Info().GetIcon(), "null", NOPE, HideID, "{STR} {INT}({INT})", PlSearchItem.Info().GetName(pPlayer), &SearchCount, &PlSearchItem.m_Count);
		}
		GS()->AVM(ClientID, "CRAFT", cr.first, HideID, "Craft {STR}", InfoGetItem.GetName(pPlayer));
	}

	if(IsNotEmpty)
		GS()->AV(ClientID, "null", "");
}

void CraftJob::CraftItem(CPlayer *pPlayer, int CraftID)
{
	const int ClientID = pPlayer->GetCID();
	InventoryItem& PlayerCraftItem = pPlayer->GetItem(ms_aCraft[CraftID].m_ReceivedItemID);
	if (PlayerCraftItem.Info().IsEnchantable() && PlayerCraftItem.m_Count > 0)
	{
		GS()->Chat(ClientID, "Enchant item maximal count x1 in a backpack!");
		return;
	}

	// first podding set what is available and required for removal
	dynamic_string Buffer;
	for(int i = 0; i < 3; i++) 
	{
		int SearchItemID = ms_aCraft[CraftID].m_aItemNeedID[i]; 
		int SearchCount = ms_aCraft[CraftID].m_aItemNeedCount[i];
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
	if(pPlayer->CheckFailMoney(Price))
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
		int SearchItemID = ms_aCraft[CraftID].m_aItemNeedID[i]; 
		int SearchCount = ms_aCraft[CraftID].m_aItemNeedCount[i];
		if(SearchItemID <= 0 || SearchCount <= 0) 
			continue;

		pPlayer->GetItem(SearchItemID).Remove(SearchCount);
	}

	const int CraftGetCount = ms_aCraft[CraftID].m_ReceivedItemCount;
	PlayerCraftItem.Add(CraftGetCount);
	if(PlayerCraftItem.Info().IsEnchantable())
	{
		GS()->Chat(-1, "{STR} crafted [{STR}x{INT}].", GS()->Server()->ClientName(ClientID), PlayerCraftItem.Info().GetName(), &CraftGetCount);
		return;
	}

	GS()->CreatePlayerSound(ClientID, SOUND_ITEM_SELL_BUY);
	GS()->Chat(ClientID, "You crafted [{STR}x{INT}].", PlayerCraftItem.Info().GetName(pPlayer), &CraftGetCount);
	GS()->ResetVotes(ClientID, pPlayer->m_OpenVoteMenu);
}

bool CraftJob::OnVotingMenu(CPlayer* pPlayer, const char* CMD, const int VoteID, const int VoteID2, int Get, const char* GetText)
{
	if(PPSTR(CMD, "CRAFT") == 0)
	{
		CraftItem(pPlayer, VoteID);
		return true;
	}

	return false;
}

bool CraftJob::OnHandleMenulist(CPlayer* pPlayer, int Menulist, bool ReplaceMenu)
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
			GS()->AV(ClientID, "null", "");
			GS()->ShowItemValueInformation(pPlayer);
			GS()->AV(ClientID, "null", "");

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
