/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#include <engine/shared/config.h>
#include <teeother/system/string.h>

#include <game/server/gamecontext.h>
#include "CraftJob.h"

using namespace sqlstr;
std::map < int , CraftJob::CraftStruct > CraftJob::Craft;

// Инициализация класса
void CraftJob::OnInit()
{
	boost::scoped_ptr<ResultSet> RES(SJK.SD("*", "tw_craft_list"));
	while(RES->next())
	{
		const int ID = RES->getInt("ID");
		Craft[ID].GetItemID = RES->getInt("GetItem");
		Craft[ID].GetItemCount = RES->getInt("GetItemCount");

		char aBuf[32];
		for(int i = 0; i < 3; i ++)
		{
			str_format(aBuf, sizeof(aBuf), "ItemNeed%d", i);
			Craft[ID].ItemNeedID[i] = RES->getInt(aBuf);
		}
		str_copy(aBuf, RES->getString("ItemNeedCount").c_str(), sizeof(aBuf));
		if (!sscanf(aBuf, "%d %d %d", &Craft[ID].ItemNeedCount[0], &Craft[ID].ItemNeedCount[1], &Craft[ID].ItemNeedCount[2]))
			dbg_msg("Error", "Error on scanf in Crafting");

		// устанавливаем обычные переменные
		Craft[ID].Price = RES->getInt("Price");
		Craft[ID].WorldID = RES->getInt("WorldID");
	}
	Job()->ShowLoadingProgress("Crafts", Craft.size());	
}

bool CraftJob::OnHandleTile(CCharacter* pChr, int IndexCollision)
{
	CPlayer* pPlayer = pChr->GetPlayer();
	const int ClientID = pPlayer->GetCID();

	if (pChr->GetHelper()->TileEnter(IndexCollision, TILE_CRAFT_ZONE))
	{
		GS()->Chat(ClientID, "List of your craft's, you can see on vote!");
		pChr->m_Core.m_ProtectHooked = pChr->m_NoAllowDamage = true;
		GS()->ResetVotes(ClientID, pPlayer->m_OpenVoteMenu);
		return true;
	}
	else if (pChr->GetHelper()->TileExit(IndexCollision, TILE_CRAFT_ZONE))
	{
		GS()->Chat(ClientID, "You have left the active zone, menu is restored!");
		pChr->m_Core.m_ProtectHooked = pChr->m_NoAllowDamage = false;
		GS()->ResetVotes(ClientID, pPlayer->m_OpenVoteMenu);
		return true;
	}
	return false;
}

bool CraftJob::ItEmptyType(int SelectType) const
{
	for (const auto& foundtype : Craft)
	{
		if (foundtype.second.WorldID != GS()->GetWorldID() || GS()->GetItemInfo(foundtype.second.GetItemID).Type != SelectType)
			continue;
		return false;
	}
	return true;
}

// показать лист крафтов
void CraftJob::ShowCraftList(CPlayer* pPlayer, const char* TypeName, int SelectType)
{
	// скипаем пустой список
	if (ItEmptyType(SelectType))
		return;

	// добавляем голосования
	int ClientID = pPlayer->GetCID();
	pPlayer->m_Colored = GRAY_COLOR;
	GS()->AVL(ClientID, "null", "{STR}", TypeName);

	for(const auto& cr : Craft)
    {
		if(cr.second.WorldID != GS()->GetWorldID())
			continue;

		int HideID = NUM_TAB_MENU + ItemJob::ItemsInfo.size() + cr.first;
		ItemJob::ItemInformation &InfoGetItem = GS()->GetItemInfo(cr.second.GetItemID);
		if (InfoGetItem.Type != SelectType)
			continue;

		int Discount = (int)kurosio::translate_to_procent_rest(cr.second.Price, Job()->Skills()->GetSkillLevel(ClientID, SkillCraftDiscount));
		if(pPlayer->GetItem(itTicketDiscountCraft).IsEquipped())
			Discount += (int)kurosio::translate_to_procent_rest(cr.second.Price, 20);
		
		const int LastPrice = max(cr.second.Price - Discount, 0);
		if (InfoGetItem.IsEnchantable())
		{
			GS()->AVHI(ClientID, InfoGetItem.GetIcon(), HideID, LIGHT_GRAY_COLOR, "{STR}{STR} - {INT} gold",
				(pPlayer->GetItem(cr.second.GetItemID).Count ? "✔ " : "\0"), InfoGetItem.GetName(pPlayer), &LastPrice);

			char aAttributes[128];
			Job()->Item()->FormatAttributes(InfoGetItem, 0, sizeof(aAttributes), aAttributes);
			GS()->AVM(ClientID, "null", NOPE, HideID, "{STR}", aAttributes);
		}
		else
		{
			GS()->AVHI(ClientID, InfoGetItem.GetIcon(), HideID, LIGHT_GRAY_COLOR, "{STR}x{INT} ({INT}) :: {INT} gold",
				InfoGetItem.GetName(pPlayer), &cr.second.GetItemCount, &pPlayer->GetItem(cr.second.GetItemID).Count, &LastPrice);
		}
		GS()->AVM(ClientID, "null", NOPE, HideID, "{STR}", InfoGetItem.GetDesc(pPlayer));

		for(int i = 0; i < 3; i++)
		{
			int SearchItemID = cr.second.ItemNeedID[i];
			int SearchCount = cr.second.ItemNeedCount[i];
			if(SearchItemID <= 0 || SearchCount <= 0) 
				continue;
			
			ItemJob::InventoryItem &PlSearchItem = pPlayer->GetItem(SearchItemID);
			GS()->AVMI(ClientID, PlSearchItem.Info().GetIcon(), "null", NOPE, HideID, "{STR} {INT}({INT})", PlSearchItem.Info().GetName(pPlayer), &SearchCount, &PlSearchItem.Count);
		}
		GS()->AVM(ClientID, "CRAFT", cr.first, HideID, "Craft {STR}", InfoGetItem.GetName(pPlayer));
	}
	GS()->AV(ClientID, "null", "");
}

void CraftJob::CraftItem(CPlayer *pPlayer, int CraftID)
{
	const int ClientID = pPlayer->GetCID();
	ItemJob::InventoryItem& PlayerCraftItem = pPlayer->GetItem(Craft[CraftID].GetItemID);
	if (PlayerCraftItem.Info().IsEnchantable() && PlayerCraftItem.Count > 0)
	{
		GS()->Chat(ClientID, "Enchant item maximal count x1 in a backpack!");
		return;
	}

	// первая подбивка устанавливаем что доступно и требуется для снятия
	dynamic_string Buffer;
	for(int i = 0; i < 3; i++) 
	{
		int SearchItemID = Craft[CraftID].ItemNeedID[i]; 
		int SearchCount = Craft[CraftID].ItemNeedCount[i];
		if(SearchItemID <= 0 || SearchCount <= 0 || pPlayer->GetItem(SearchItemID).Count >= SearchCount) 
			continue;

		char aBuf[48];
		int ItemLeft = SearchCount - pPlayer->GetItem(SearchItemID).Count;
		str_format(aBuf, sizeof(aBuf), "%sx%d ", GS()->GetItemInfo(SearchItemID).GetName(pPlayer), ItemLeft);
		Buffer.append_at(Buffer.length(), aBuf);
	}
	if(Buffer.length() > 0) 
	{
		GS()->Chat(ClientID, "Item left: {STR}", Buffer.buffer());
		Buffer.clear();
		return;
	}
	Buffer.clear();

	dbg_msg("test", "here craft left");

	// дальше уже организуем крафт
	int Discount = (int)kurosio::translate_to_procent_rest(Craft[CraftID].Price, Job()->Skills()->GetSkillLevel(ClientID, SkillCraftDiscount));
	bool TickedDiscountCraft = pPlayer->GetItem(itTicketDiscountCraft).IsEquipped();
	if(TickedDiscountCraft)
		Discount += (int)kurosio::translate_to_procent_rest(Craft[CraftID].Price, 20);

	const int LastPrice = max(Craft[CraftID].Price - Discount, 0);
	if(pPlayer->CheckFailMoney(LastPrice))
		return;

	if(TickedDiscountCraft)
	{
		pPlayer->GetItem(itTicketDiscountCraft).Remove(1);
		GS()->Chat(ClientID, "You used item {STR} and get discount 25%.", GS()->GetItemInfo(itTicketDiscountCraft).Name);
	}

	for(int i = 0; i < 3; i++) 
	{
		int SearchItemID = Craft[CraftID].ItemNeedID[i]; 
		int SearchCount = Craft[CraftID].ItemNeedCount[i];
		if(SearchItemID <= 0 || SearchCount <= 0) 
			continue;

		pPlayer->GetItem(SearchItemID).Remove(SearchCount);
	}

	const int CraftGetCount = Craft[CraftID].GetItemCount;
	PlayerCraftItem.Add(CraftGetCount);
	if(PlayerCraftItem.Info().IsEnchantable())
	{
		GS()->Chat(-1, "{STR} crafted [{STR}x{INT}].", GS()->Server()->ClientName(ClientID), PlayerCraftItem.Info().GetName(), &CraftGetCount);
		return;
	}
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
