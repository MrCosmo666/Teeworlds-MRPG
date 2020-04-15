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
	StSeasonLevel,
	StSpreadShotgun,
	StSpreadGrenade,
	StSpreadRifle,
    StStrength,
	StDexterity,
 	StCriticalHit,
	StDirectCriticalHit,
    StHardness,
	StTenacity,
	StLucky,
	StPiety,
	StVampirism,
	StAmmoRegen,
	StAmmo,
	StEfficiency,
	StExtraction,
	StStrengthQ,
	StDexterityQ,
	StCriticalHitQ,
	StDirectCriticalHitQ,
	StHardnessQ,
	StTenacityQ,
	StLuckyQ,
	StPietyQ,
	StVampirismQ,
	StAmmoRegenQ,
	StAmmoQ,
	StAutoFire
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
};

enum AtributType
{
	AtTank,
	AtHealer,
	AtDps,
	AtWeapon
};

// типы TOP LIST
enum ToplistTypes
{
	GUILDS_LEVELING,
	GUILDS_WEALTHY,
	PLAYERS_LEVELING,
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
	MEMBERRANK,								// гильдия ранги меню
	MEMBERINVITES,							// гильдия приглашения меню
	MEMBERHISTORY,							// гильдия история действий
	CRAFTING,								// крафт меню
	HOUSEMENU,								// дома меню
	HOUSEDECORATION,						// дома декорации меню
	HOUSEPLANTS,							// дома огород меню
	ADVENTUREJOURNAL,						// журнал квестов меню
	FINISHQUESTMENU,						// журнал завершенные квесты меню
	TOPLISTMENU,							// топ лист меню
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

	/*
		Типы домов
	*/
	FUNCTIONHOUSE = 0,						// дом
	FUNCTIONMOTEL,							// отель

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
	NOFUNCTIONITEM = 0,
	ONEUSEDS = NUM_EQUIPS,
	USEDS,
	ITSETTINGS,
	ITPLANTS,
	ITMINER,

	/*
		Типы предметов
		Это влияет на взаимодействие в будующем
	*/
	ITEMUSED = 1,
	ITEMCRAFT,
	ITEMQUEST,
	ITEMUPGRADE,
	ITEMOTHER,
	ITEMSETTINGS,
	ITEMEQUIP,
	ITEMDECORATION,
	ITEMPOTION,

	/*
		Лист придметов суда добавляется лишь то что
		в будующем будет взаимодействованно с кодом
	*/
	NOPE = -1, 
	itMoney						= 1,				// Деньги обычная валюта
	itHammer					= 2,				// Снаряжение Молоток | Молоток
	itMaterial					= 7,				// Материал для зачирования вещей
	itGoods						= 8,				// Товары для загрузки разгрузки
	itTicketGuild				= 9,				// Билет для создания гильдии
	itSpearBronekhods			= 24,				// Снаряжение Молоток | Копье
	itDecoHealth				= 25,				// Декорация Сердце
	itDecoArmor					= 26,				// Декорация Щит
	itEliteDecoHealth			= 27,				// Декорация элитная сердце
	itEliteDecoNinja			= 28,				// Декорация элитная ниндзя
	itPotionHealthRegen			= 38,				// Зелье регенерации здоровья
	itCapsuleSurvivalExperience = 39,				// Дает 1.000 опыта
	itLittleBagGold				= 40,				// Дает 10-100 золота
	itTitleNewHero				= 46,				// Титул новый герой
	itClubSeasonTicket			= 49,				// Клуб сезоный билет
	itSeasonToken				= 50,				// Клуб сезоный жетон
	itPotionQuenchingHunger		= 60,				// Зелье регенерации голода
	itSkillPoint				= 77,				// Скилл поинт
	itAutoHammer				= 78,				// Автоматический хамер

	/*
		Все листы сортировок что существуют на сервере
	*/
	SORTINVENTORY = 1,
	SORTCRAFT,
	SORTEQUIP,
	NUMTABSORT,

	/*
		Статы прогресса квеста
		Имеет некий прогресс
	*/
	QUESTNOACCEPT = 1,
	QUESTACCEPT,
	QUESTFINISHED,

	/*
		Ранги организаций и доступ
		Типы доступа к гильдии
	*/
	MACCESSNO = 0,
	MACCESSINVITEKICK,
	MACCESSUPGHOUSE,
	MACCESSFULL,
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
	SAVESPAACCOUNT,			// Save Spa Account Upgrades and all spa data
	SAVEPLANTACCOUNT,		// Save Plant Account
	SAVEMINERACCOUNT,
	SAVEMEMBERDATA,			// Save Member Data
	SAVEPOSITION,			// Save Position Player
};

#endif
