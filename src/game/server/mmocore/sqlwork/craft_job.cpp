/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#include <engine/shared/config.h>
#include <game/server/gamecontext.h>
#include "craft_job.h"

using namespace sqlstr;
std::map < int , CraftJob::CraftStruct > CraftJob::Craft;

// Инициализация класса
void CraftJob::OnInitGlobal() 
{ 
	// получаем список крафтов
	boost::scoped_ptr<ResultSet> RES(SJK.SD("*", "tw_craftitem", "ORDER BY 'Level' DESC"));
	while(RES->next())
	{
		char aBuf[32];
		int ID = RES->getInt("ID");
		for(int i = 0; i < 3; i ++)
		{
			str_format(aBuf, sizeof(aBuf), "Item_%d", i);
			Craft[ID].Need[i] = RES->getInt(aBuf);
		}
		str_copy(aBuf, RES->getString("CountNeed").c_str(), sizeof(aBuf));
		if (!sscanf(aBuf, "%d %d %d", &Craft[ID].Count[0], &Craft[ID].Count[1], &Craft[ID].Count[2]))
			dbg_msg("Error", "Error on scanf in Crafting");

		// устанавливаем обычные переменные
		Craft[ID].ItemID = RES->getInt("GetItemID");
		Craft[ID].ItemCount = RES->getInt("GetCount");
		Craft[ID].Money = RES->getInt("Price");
		Craft[ID].Level = RES->getInt("Level");
		Craft[ID].Rare = RES->getBoolean("Rare");
		Craft[ID].Tab = RES->getInt("Tab");	
	}
	Job()->ShowLoadingProgress("Crafts", Craft.size());	
	return;	
}


bool CraftJob::OnPlayerHandleTile(CCharacter* pChr, int IndexCollision)
{
	CPlayer* pPlayer = pChr->GetPlayer();
	const int ClientID = pPlayer->GetCID();

	if (pChr->GetHelper()->TileEnter(IndexCollision, TILE_CRAFT_ZONE))
	{
		GS()->ResetVotes(ClientID, MAINMENU);
		GS()->Chat(ClientID, "Information load in Vote!");
		pChr->m_Core.m_ProtectHooked = pChr->m_NoAllowDamage = true;
		return true;
	}
	else if (pChr->GetHelper()->TileExit(IndexCollision, TILE_CRAFT_ZONE))
	{
		GS()->ResetVotes(ClientID, MAINMENU);
		pChr->m_Core.m_ProtectHooked = pChr->m_NoAllowDamage = false;
		return true;
	}

	return false;
}

// показать лист крафтов
void CraftJob::ShowCraftList(CPlayer *pPlayer, int CraftTab)
{
	// добавляем голосования
	int ClientID = pPlayer->GetCID();
	for(const auto& cr : Craft)
    {
		// пропускаем если не равен вкладке
		if(cr.second.Tab != CraftTab) continue;
		
		// бонус дискаунта
		int MoneyDiscount = round_to_int(kurosio::translate_to_procent_rest(cr.second.Money, Job()->Skills()->GetSkillLevel(ClientID, SkillCraftDiscount)));
	
		ItemSql::ItemInformation &InfoSellItem = GS()->GetItemInfo(cr.second.ItemID);

		// выводим основное меню
		int HideID = NUMHIDEMENU + ItemSql::ItemsInfo.size() + cr.first;
		if (InfoSellItem.IsEnchantable())
		{
			GS()->AVHI(ClientID, InfoSellItem.GetIcon(), HideID, LIGHT_RED_COLOR, "{STR}Lvl{INT} : {STR} :: {INT} gold",
				(pPlayer->GetItem(cr.second.ItemID).Count ? "✔ " : "\0"), &cr.second.Level, InfoSellItem.GetName(pPlayer), &MoneyDiscount);
			GS()->AVM(ClientID, "null", NOPE, HideID, "Astro stats +{INT} {STR}", &InfoSellItem.BonusCount, pPlayer->AtributeName(InfoSellItem.BonusID));
		}
		else
		{
			GS()->AVHI(ClientID, InfoSellItem.GetIcon(), HideID, LIGHT_RED_COLOR, "Lvl{INT} : {STR}x{INT} ({INT}) :: {INT} gold",
				&cr.second.Level, InfoSellItem.GetName(pPlayer), &cr.second.ItemCount, &pPlayer->GetItem(cr.second.ItemID).Count, &MoneyDiscount);
		}
		GS()->AVM(ClientID, "null", NOPE, HideID, "{STR}", InfoSellItem.GetDesc(pPlayer));

		// чтобы проще инициализируем под переменной
		dynamic_string Buffer;
		for(int i = 0; i < 3; i++)
		{
			int SearchItemID = cr.second.Need[i], SearchCount = cr.second.Count[i];
			if(SearchItemID <= 0 || SearchCount <= 0) continue;
			
			char aBuf[32];
			ItemSql::ItemPlayer &PlSearchItem = pPlayer->GetItem(SearchItemID);
			str_format(aBuf, sizeof(aBuf), "%sx%d(%d) ", PlSearchItem.Info().GetName(pPlayer), SearchCount, PlSearchItem.Count);
			Buffer.append_at(Buffer.length(), aBuf);
		}
		GS()->AVM(ClientID, "null", NOPE, HideID, "{STR}", Buffer.buffer());
		Buffer.clear();

		// показать все остальное
		GS()->AVM(ClientID, "CRAFT", cr.first, HideID, "Craft {STR}", InfoSellItem.GetName(pPlayer));
	}
}

void CraftJob::StartCraftItem(CPlayer *pPlayer, int CraftID)
{
	if(Craft.find(CraftID) == Craft.end()) 
		return;

	// проверяем имеется ли зачарованный предмет уже
	const int ClientID = pPlayer->GetCID();
	ItemSql::ItemPlayer& PlayerItem = pPlayer->GetItem(Craft[CraftID].ItemID);
	if (PlayerItem.Info().BonusCount > 0 && PlayerItem.Count > 0)
		return GS()->Chat(ClientID, "Enchant item maximal count x1 in a backpack!");

	// проверяем уровень
	if(Craft[CraftID].Level > pPlayer->Acc().Level)
		return GS()->Chat(ClientID, "Your level low for this item craft!");

	// первая подбивка устанавливаем что доступно и требуется для снятия
	dynamic_string Buffer;
	for(int i = 0; i < 3; i++) 
	{
		int SearchItemID = Craft[CraftID].Need[i], SearchCount = Craft[CraftID].Count[i];
		if(SearchItemID <= 0 || SearchCount <= 0 || pPlayer->GetItem(SearchItemID).Count >= SearchCount) continue;

		char aBuf[48];
		ItemSql::ItemPlayer &PlSearchItem = pPlayer->GetItem(SearchItemID);
		int ItemLeft = SearchCount - PlSearchItem.Count;
		str_format(aBuf, sizeof(aBuf), "%sx%d ", PlSearchItem.Info().GetName(pPlayer), ItemLeft);
		Buffer.append_at(Buffer.length(), aBuf);
	}

	// проверяем имеется ли в буффере что то либо то сразу пишем вам нужно ляля 
	if(Buffer.length() > 0) 
	{
		GS()->Chat(ClientID, "Item left: {STR}", Buffer.buffer(), NULL);
		Buffer.clear();
		return;
	}
	Buffer.clear();

	// чекаем деньги
	int MoneyDiscount = round_to_int(kurosio::translate_to_procent_rest(Craft[CraftID].Money, Job()->Skills()->GetSkillLevel(ClientID, SkillCraftDiscount)));
	if(pPlayer->CheckFailMoney(MoneyDiscount)) 
		return;

	// вторая подбивка снимает что стабильно для снятия
	for(int i = 0; i < 3; i++) 
	{
		int SearchItemID = Craft[CraftID].Need[i], SearchCount = Craft[CraftID].Count[i];
		if(SearchItemID <= 0 || SearchCount <= 0) continue;
		pPlayer->GetItem(Craft[CraftID].Need[i]).Remove(Craft[CraftID].Count[i]);
	}

	PlayerItem.Add(Craft[CraftID].ItemCount);
	GS()->ResetVotes(ClientID, pPlayer->m_OpenVoteMenu);

	// пишем в чат и выдаем предмет
	if(Craft[CraftID].Rare)
	{
		GS()->Chat(-1, "{STR} crafted [{STR}x{INT}].", GS()->Server()->ClientName(ClientID), PlayerItem.Info().GetName(), &Craft[CraftID].ItemCount);
		return;
	}
	GS()->Chat(ClientID, "You crafted [{STR}x{INT}].", PlayerItem.Info().GetName(pPlayer), &Craft[CraftID].ItemCount);
}

bool CraftJob::OnParseVotingMenu(CPlayer *pPlayer, const char *CMD, const int VoteID, const int VoteID2, int Get, const char *GetText)
{
	int ClientID = pPlayer->GetCID();
	// крафт действие
	if(PPSTR(CMD, "CRAFT") == 0)
	{
		StartCraftItem(pPlayer, VoteID);
		return true;
	}

	// лист крафта
	if(PPSTR(CMD, "SORTEDCRAFT") == 0)
	{
		pPlayer->m_SortTabs[SORTCRAFT] = VoteID;
		GS()->VResetVotes(ClientID, MAINMENU);
		return true;		
	}
	
	return false;
}

bool CraftJob::OnPlayerHandleMainMenu(CPlayer* pPlayer, int Menulist, bool ReplaceMenu)
{
	const int ClientID = pPlayer->GetCID();
	if (ReplaceMenu)
	{
		CCharacter* pChr = pPlayer->GetCharacter();
		if (!pChr) return false;

		if (Menulist == MAINMENU && pChr->GetHelper()->BoolIndex(TILE_CRAFT_ZONE))
		{
			pPlayer->m_LastVoteMenu = MAINMENU;
			GS()->AVH(ClientID, HCRAFTINFO, GREEN_COLOR, "Crafting Information");
			GS()->AVM(ClientID, "null", NOPE, HCRAFTINFO, "Choose the type of crafts you want to show");
			GS()->AVM(ClientID, "null", NOPE, HCRAFTINFO, "If you will not have enough items for crafting");
			GS()->AVM(ClientID, "null", NOPE, HCRAFTINFO, "You will write those and the amount that is still required");
			GS()->AV(ClientID, "null", "");

			GS()->AVH(ClientID, HCRAFTSELECT, RED_COLOR, "Crafting Select List");
			GS()->AVM(ClientID, "SORTEDCRAFT", CRAFTBASIC, HCRAFTSELECT, "Basic Items");
			GS()->AVM(ClientID, "SORTEDCRAFT", CRAFTARTIFACT, HCRAFTSELECT, "Artifacts");
			GS()->AVM(ClientID, "SORTEDCRAFT", CRAFTWEAPON, HCRAFTSELECT, "Modules & Weapons");
			GS()->AVM(ClientID, "SORTEDCRAFT", CRAFTEAT, HCRAFTSELECT, "Buffs & Eat");
			GS()->AVM(ClientID, "SORTEDCRAFT", CRAFTWORK, HCRAFTSELECT, "Work & Job");
			GS()->AVM(ClientID, "SORTEDCRAFT", CRAFTQUEST, HCRAFTSELECT, "Quests");
			GS()->AV(ClientID, "null", "");
			if (pPlayer->m_SortTabs[SORTCRAFT])
				ShowCraftList(pPlayer, pPlayer->m_SortTabs[SORTCRAFT]);

			return true;
		}
		return false;
	}

	return false;
}
