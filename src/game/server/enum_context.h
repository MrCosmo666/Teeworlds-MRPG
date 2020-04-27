/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#ifndef GAME_ITEMS_H
#define GAME_ITEMS_H

#include <game/server/enum_global.h>
#include <generated/protocol.h>

#include <list>
#include <map>

// Статы оружия и хар-ки игрока
enum Stats
{
	StSpreadShotgun			= 1,
	StSpreadGrenade			= 2,
	StSpreadRifle			= 3,
    StStrength				= 4,
	StDexterity				= 5,
 	StCriticalHit			= 6,
	StDirectCriticalHit		= 7,
    StHardness				= 8,
	StLucky					= 9,
	StPiety					= 10,
	StVampirism				= 11,
	StAmmoRegen				= 12,
	StAmmo					= 13,
	StEfficiency			= 14,
	StExtraction			= 15,
	StStrengthQ				= 16,
	StDexterityQ			= 17,
	StCriticalHitQ			= 18,
	StDirectCriticalHitQ	= 19,
	StHardnessQ				= 20,
	StLuckyQ				= 21,
	StPietyQ				= 22,
	StVampirismQ			= 23,
	StAmmoRegenQ			= 24,
	StAmmoQ					= 25,
	StHammerPower			= 26,
	StGunPower				= 27,
	StShotgunPower			= 28,
	StGrenadePower			= 29,
	StRiflePower			= 30,
};
 
// Таймеры Игрока
enum TickState
{
	Die = 1,
	Respawn,
	LastKill,
	LastEmote,
	LastChangeInfo,
	LastChat,
	LastVoteTry,
	CheckClient,
	NUM_TICK,
};

enum Skill
{
	SkillHeartTurret = 1,	// турель восстановления здоровья
	SkillSleepyGravity = 2, // притягивает мобов вокруг себя
	SkillCraftDiscount = 3, // скидка на крафт
	SkillMasterWeapon = 4, // автоматический огонь с оружий
};

enum AtributType
{
	AtTank,
	AtHealer,
	AtDps,
	AtWeapon,
	AtHardtype
};

// типы TOP LIST
enum ToplistTypes
{
	GUILDS_LEVELING,
	GUILDS_WEALTHY,
	PLAYERS_LEVELING,
};


// типы предметов
enum ItemType
{
	TYPE_USED = 1,
	TYPE_CRAFT,
	TYPE_QUEST,
	TYPE_MODULE,
	TYPE_OTHER,
	TYPE_SETTINGS,
	TYPE_EQUIP,
	TYPE_DECORATION,
	TYPE_POTION,
};

// положение квеста
enum QuestState
{
	QUEST_NO_ACCEPT = 0,
	QUEST_ACCEPT,
	QUEST_FINISHED,
};


// Основное
enum
{
	/*
		Основные настройки сервера ядра
		Здесь хранятся самые основные настройки сервера
	*/
	MIN_SKINCHANGE_CLIENTVERSION = 0x0703,	// минимальная версия клиента для смены скина
	MIN_RACE_CLIENTVERSION = 0x0704,		// минимальная версия клиента для типа гонки
	MAX_MAILLIST = 30,						// максимальное кол-во писем что выводится

	/*
		Список листов меню используется:
		в голосования по типу "MENU", передается в качестве ID
	*/
	MAINMENU = 1,							// основное меню
	INVENTORY,								// инвентарь меню
	INBOXLIST,								// письма меню
	UPGRADES,								// апгрейды меню
	SETTINGS,								// настройки меню
	GUIDEDROP,								// гайды по дропам меню
	EQUIPMENU,								// снаряжение меню
	MEMBERMENU,								// гильдия меню
	GUILDRANK,								// гильдия ранги меню
	MEMBERINVITES,							// гильдия приглашения меню
	MEMBERHISTORY,							// гильдия история действий
	HOUSEMENU,								// дома меню
	HOUSEDECORATION,						// дома декорации меню
	HOUSEGUILDDECORATION,					// дома декорации гильдии меню
	HOUSEPLANTS,							// дома огород меню
	ADVENTUREJOURNAL,						// журнал квестов меню
	FINISHQUESTMENU,						// журнал завершенные квесты меню
	TOPLISTMENU,							// топ лист меню
	DUNGEONSMENU,							// лист данжей
	AUCTIONSETSLOT,							// создание слота аукцион меню

	/*
		Список вкладок что можно скрыть и раскрыть
		Передается в качестве AVH как ID, а последющие в качестве HideID
	*/
	HSTAT = 1,
	HPERSONAL,
	HINFORMATION,
	HHOMECOMMAND,
	HCRAFTSELECT,
	HEQUIPSELECT,
	HUPGDPS,
	HUPGTANK,
	HUPGHEALER,
	HUPGWEAPON,
	HINVSELECT,
	HDISCORDTHEME,
	HJOBUPGRADE,
	HMEMBERSTATS,
	HSTORAGEUSE,
	HCAFELIST,
	HHOUSESTATS,
	HHOUSEMOTEL,
	HHOUSEDECO,
	HHOUSEAVAILABLE,
	HBUSINESSAVAILABLE,
	HTELEPORTLIST,
	HSETTINGSS,
	HSETTINGSU,
	HQUESTITEM, // start info
	HINVINFO,
	HHOMEINFO,
	HUPGRADESTATS,
	HCRAFTINFO,
	HTOPMENUINFO,
	HDUNGEONSINFO,
	HUPGRINFO,
	HQUESTSINFO,
	HDECORATION,
	HPLANTS,
	HMEMHOMEINFO,
	HCHANCELOOTINFO,
	HSKILLLEARN,
	HSPAINFO,
	HSTORAGEINFO,
	HDISCORDINFO,
	HQUESTINFO,
	HAUCTIONINFO,
	HAUCTIONSLOTINFO,
	HEQUIPINFO,
	NUMHIDEMENU,

	/*
		Список вкладок в меню крафта: Некая сортировка по типу крафта
		Передается исключительно в ShowMenuCraft( int )
	*/
	CRAFTBASIC = 1,
	CRAFTARTIFACT,
	CRAFTWEAPON,
	CRAFTEAT,
	CRAFTWORK,
	CRAFTQUEST,

	/*
		Парсинг голосований
		Передается как действие в парсер
	*/
	PARSESELLHOUSE = 1,
	PARSEINVITEMEMBER,

	// Редкость броадкастов
	PRELOW = 1,
	PRENORMAL,
	PREHIGHT,
	PRERARE,
	PRERAREUP,
	PRELEGENDARY,

	/*
		Все функции предметов
		Это сортируется в вкладках
	*/
	FUNCTION_ONE_USED = NUM_EQUIPS,
	FUNCTION_USED,
	FUNCTION_SETTINGS,
	FUNCTION_PLANTS,
	FUNCTION_MINER,


	NOPE = -1,
	itMoney = 1,				// Деньги обычная валюта
	itHammer = 2,				// Снаряжение Молоток | Молоток
	itMaterial = 7,				// Материал для зачирования вещей
	itGoods = 8,				// Товары для загрузки разгрузки
	itTicketGuild = 9,				// Билет для создания гильдии
	itSkillPoint = 10,				// Скилл поинт
	itDecoArmor = 11,				// Декорация Щит
	itEliteDecoHealth = 12,				// Декорация элитная сердце
	itEliteDecoNinja = 13,				// Декорация элитная ниндзя
	itDecoHealth = 14,				// Декорация Сердце
	itPotionHealthRegen = 18,				// Зелье регенерации здоровья
	itCapsuleSurvivalExperience = 19,				// Дает 1.000 опыта
	itLittleBagGold = 20,				// Дает 10-100 золота
	itTitleNewHero = 23,				// Титул новый герой

	/*
		Все листы сортировок что существуют на сервере
	*/
	SORTINVENTORY = 1,
	SORTCRAFT,
	SORTEQUIP,
	NUMTABSORT,

	/* Вид декораций */
	DECOTYPE_HOUSE = 0,
	DECOTYPE_GUILD_HOUSE,

	// максимальные слоты снаражения ботов
	EQUIP_MAX_BOTS = EQUIP_RIFLE + 1,
};

enum GuildAccess
{
	ACCESS_LEADER = -1,
	ACCESS_NO,
	ACCESS_INVITE_KICK,
	ACCESS_UPGRADE_HOUSE,
	ACCESS_FULL,
};

enum BroadcastPriority
{
	BROADCAST_BASIC_STATS,
	BROADCAST_GAME_INFORMATION,
	BROADCAST_GAME_WARNING,
	BROADCAST_MAIN_INFORMATION,
};

enum
{
	// Спавн точки и тип
	SPAWNWORLD = 0,
	SPAWNMOBS = 1,
	SPAWNQUESTNPC = 2,
	SPAWNNPC = 3,

	// Классы
	CLASSTANK,
	CLASSHEALER,
	CLASSDPS,

	SNAPPLAYER = 1,
	SNAPBOTS = 2,
};

// Сохранения лист
enum
{
	SAVEACCOUNT,			// Save Login Password Data
	SAVESTATS,				// Save Stats Level Exp and other this type
	SAVEUPGRADES,			// Save Upgrades Damage and other this type
	SAVEPLANTACCOUNT,		// Save Plant Account
	SAVEMINERACCOUNT,
	SAVEMEMBERDATA,			// Save Member Data
	SAVEPOSITION,			// Save Position Player
};

#endif
