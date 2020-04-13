/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#include <engine/shared/config.h>
#include <game/server/gamecontext.h>
#include "craft.h"

using namespace sqlstr;
std::map < int , CraftSql::CraftStruct > CraftSql::Craft;

// Инициализация класса
void CraftSql::OnInitGlobal() 
{ 
	// получаем список крафтов
	boost::scoped_ptr<ResultSet> RES(SJK.SD("*", "tw_craftitem", "ORDER BY 'CraftLevel' DESC"));
	while(RES->next())
	{
		// получаем текстовую и форматируем ее
		int ID = RES->getInt("ID");

		// получаем предметы
		char aBuf[32];
		for(int i = 0; i < 3; i ++)
		{
			str_format(aBuf, sizeof(aBuf), "Craft_Item_%d", i);
			Craft[ID].Need[i] = RES->getInt(aBuf);
		}

		// получим количество
		char StringNeedCount[32];
		str_copy(StringNeedCount, RES->getString("Craft_Count_Need").c_str(), sizeof(StringNeedCount));
		sscanf(StringNeedCount, "%d %d %d", &Craft[ID].Count[0], &Craft[ID].Count[1], &Craft[ID].Count[2]);

		// устанавливаем обычные переменные
		Craft[ID].ItemID = RES->getInt("CraftIID");
		Craft[ID].ItemCount = RES->getInt("CraftICount");
		Craft[ID].Money = RES->getInt("CraftMoney");
		Craft[ID].Level = RES->getInt("CraftLevel");
		Craft[ID].Rare = RES->getBoolean("Rare");
		Craft[ID].Tab = RES->getInt("CraftTab");	
	}
	Job()->ShowLoadingProgress("Crafts", Craft.size());	
	return;	
}


// показать лист крафтов
void CraftSql::ShowCraftList(CPlayer *pPlayer, int CraftTab)
{
	// добавляем голосования
	int ClientID = pPlayer->GetCID();
	for(const auto& cr : Craft)
    {
		// пропускаем если не равен вкладке
		if(cr.second.Tab != CraftTab) continue;
		
		// бонус дискаунта
		int MoneyDiscount = round_to_int(kurosio::translate_to_procent_rest(cr.second.Money, Job()->Skills()->GetSkillLevel(ClientID, SkillCraftDiscount)));
		int Price = clamp(cr.second.Money - MoneyDiscount, 0, cr.second.Money - MoneyDiscount);
		
		ItemSql::ItemInformation &InfoSellItem = GS()->GetItemInfo(cr.second.ItemID);

		// выводим основное меню
		int HideID = NUMHIDEMENU + ItemSql::ItemsInfo.size() + cr.first;
		GS()->AVHI(ClientID, InfoSellItem.GetIcon(), HideID, vec3(15,25,55), "LV:{INT} | {STR}x{INT} : {INT} Gold",
			&cr.second.Level, InfoSellItem.GetName(pPlayer), &cr.second.ItemCount, &MoneyDiscount);
		GS()->AVM(ClientID, "null", NOPE, HideID, "{STR}", InfoSellItem.GetDesc(pPlayer));

		// бонус предметов
		if(CGS::AttributInfo.find(InfoSellItem.BonusID) != CGS::AttributInfo.end() && InfoSellItem.BonusCount)
		{
			GS()->AVM(ClientID, "null", NOPE, HideID, "Astro stats +{INT} {STR}", &InfoSellItem.BonusCount, pPlayer->AtributeName(InfoSellItem.BonusID));
		}

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

void CraftSql::StartCraftItem(CPlayer *pPlayer, int CraftID)
{
	if(Craft.find(CraftID) == Craft.end()) return;

	// проверяем уровень
	const int ClientID = pPlayer->GetCID();
	if(Craft[CraftID].Level > pPlayer->Acc().Level)
		return GS()->Chat(ClientID, "Your 'Craft level' low for this item craft!");

	// проверяем имеется ли зачарованный предмет уже
	ItemSql::ItemPlayer &PlayerItem = pPlayer->GetItem(Craft[CraftID].ItemID);
	if(PlayerItem.Info().BonusCount > 0 && PlayerItem.Count > 0)	
		return GS()->Chat(ClientID, "Thing that enchanting can only be 1!");

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
	int Price = clamp(Craft[CraftID].Money - MoneyDiscount, 0, Craft[CraftID].Money - MoneyDiscount);
	if(pPlayer->CheckFailMoney(MoneyDiscount)) return;

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

bool CraftSql::OnParseVotingMenu(CPlayer *pPlayer, const char *CMD, const int VoteID, const int VoteID2, int Get, const char *GetText)
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
		GS()->VResetVotes(ClientID, CRAFTING);
		return true;		
	}
	
	return false;
}