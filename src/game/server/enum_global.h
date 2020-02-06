/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#ifndef GAME_GLOBAL_ENUM_H
#define GAME_GLOBAL_ENUM_H

#include <base/stringable_enum.h>

// Аккаунт Data Miner
#define MINER(F) \
	F(MnrLevel) \
	F(MnrExp) \
	F(MnrUpgrade) \
	F(MnrCount)
STRINGABLE_ENUM_DECL(MINER)

// Аккаунт Data Craft
#define CRAFT(F) \
	F(CrftLevel) \
	F(CrftExp) \
	F(CrftUpgrade) \
	F(CrftDiscount)
STRINGABLE_ENUM_DECL(CRAFT)

// Аккаунт Data Relax
#define RELAX(F) \
	F(RlxLevel) \
	F(RlxExp) \
	F(RlxExpBonus) \
	F(RlxMoneyBonus) \
	F(RlxSpaBonus) \
	F(RlxUpgrade)
STRINGABLE_ENUM_DECL(RELAX)

// Аккаунт Data Plants
#define PLANT(F) \
	F(PlLevel) \
	F(PlExp) \
	F(PlCounts) \
	F(PlUpgrade)
STRINGABLE_ENUM_DECL(PLANT)

// Улучшения организациям по полям
#define EMEMBERUPGRADE(F) \
    F(AvailableNSTSlots) \
    F(ChairNSTExperience) \
	F(ChairNSTMoney)
STRINGABLE_ENUM_DECL(EMEMBERUPGRADE)
 
enum DayType
{
	NIGHTTYPE = 1,
	DAYTYPE,
	MORNINGTYPE,
	EVENINGTYPE
};

/*
	Основные настройки сервера ядра
	Здесь хранятся самые основные настройки сервера
*/
enum
{
	COUNT_WORLD = 8,						// кол-во миров
	LAST_WORLD = COUNT_WORLD - 1,			// последний мир
	LOCALWORLD = 0,							// локальный мир
	CUTSCENEWELCOMEWORLD = 7,				// катсцена начало игры
};

#endif
