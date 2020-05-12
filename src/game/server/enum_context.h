/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#ifndef GAME_ITEMS_H
#define GAME_ITEMS_H

#include <game/server/enum_global.h>
#include <generated/protocol.h>

#include <list>
#include <map>

enum QuestInteractive
{
	QUEST_INT_RANDOM_ACCEPT_ITEM = 1,
	QUEST_INT_DROP_AND_TAKE_IT = 2,

};

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
	StHammerPower			= 16,
	StGunPower				= 17,
	StShotgunPower			= 18,
	StGrenadePower			= 19,
	StRiflePower			= 20,
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
	LastDialog,
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

enum ToplistTypes
{
	GUILDS_LEVELING,
	GUILDS_WEALTHY,
	PLAYERS_LEVELING,
};


enum ItemType
{
	TYPE_USED = 1,
	TYPE_CRAFT,
	TYPE_MODULE,
	TYPE_OTHER,
	TYPE_SETTINGS,
	TYPE_EQUIP,
	TYPE_DECORATION,
	TYPE_POTION,
};

enum QuestState
{
	QUEST_NO_ACCEPT = 0,
	QUEST_ACCEPT,
	QUEST_FINISHED,
};

enum FunctionsNPC
{
	FUNCTION_NPC_NURSE,
	FUNCTION_NPC_GIVE_QUEST,
};

enum MenuList
{
	MAIN_MENU = 1,
	MENU_INVENTORY,
	MENU_INBOX,
	MENU_UPGRADE,
	MENU_SETTINGS,
	MENU_GUIDEDROP,
	MENU_EQUIPMENT,
	MENU_GUILD,
	MENU_GUILD_RANK,
	MENU_GUILD_INVITES,
	MENU_GUILD_HISTORY,
	MENU_HOUSE,
	MENU_HOUSE_DECORATION,
	MENU_GUILD_HOUSE_DECORATION,
	MENU_HOUSE_PLANTS,
	MENU_JOURNAL_MAIN,
	MENU_JOURNAL_FINISHED,
	MENU_TOP_LIST,
	MENU_DUNGEONS,
	MENU_AUCTION_CREATE_SLOT,
};

enum TabHideList
{
	TAB_STAT = 1,
	TAB_PERSONAL,
	TAB_INFORMATION,
	TAB_HOUSE_COMMAND,
	TAB_EQUIP_SELECT,
	TAB_UPGR_DPS,
	TAB_UPGR_TANK,
	TAB_UPGR_HEALER,
	TAB_UPGR_WEAPON,
	TAB_INVENTORY_SELECT,
	TAB_UPGR_JOB,
	TAB_GUILD_STAT,
	TAB_STORAGE,
	TAB_HOUSE_STAT,
	TAB_AETHER,
	TAB_SETTINGS,
	TAB_SETTINGS_MODULES,
	NUM_TAB_MENU_INTERACTIVES,

	// start info
	TAB_INFO_INVENTORY,
	TAB_INFO_HOUSE,
	TAB_INFO_STAT,
	TAB_INFO_CRAFT,
	TAB_INFO_TOP,
	TAB_INFO_DUNGEON,
	TAB_INFO_UPGR,
	TAB_INFO_DECORATION,
	TAB_INFO_HOUSE_PLANT,
	TAB_INFO_GUILD_HOUSE,
	TAB_INFO_LOOT,
	TAB_INFO_SKILL,
	TAB_INFO_AUCTION,
	TAB_INFO_AUCTION_BIND,
	TAB_EQUIP_INFO,
	NUM_TAB_MENU,
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
	MAX_INBOX_LIST = 30,						// максимальное кол-во писем что выводится


	/*
		Список вкладок что можно скрыть и раскрыть
		Передается в качестве AVH как ID, а последющие в качестве HideID
	*/


	/*
		Список вкладок в меню крафта: Некая сортировка по типу крафта
		Передается исключительно в ShowMenuCraft( int )
	*/
	STATS_MAX_FOR_ITEM = 2,

	/*
		Парсинг голосований
		Передается как действие в парсер
	*/
	PARSEINVITEMEMBER = 1,

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
	itGold = 1,						// Деньги обычная валюта
	itHammer = 2,						// Снаряжение Молоток | Молоток
	itMaterial = 7,						// Материал для зачирования вещей
	itTicketGuild = 8,					// Билет для создания гильдии
	itSkillPoint = 9,					// Скилл поинт
	itDecoArmor = 10,					// Декорация Щит
	itEliteDecoHealth = 11,				// Декорация элитная сердце
	itEliteDecoNinja = 12,				// Декорация элитная ниндзя
	itDecoHealth = 13,					// Декорация Сердце
	itPotionManaRegen = 14,				// Зелье регенерации маны
	itPotionHealthRegen = 15,			// Зелье регенерации здоровья
	itCapsuleSurvivalExperience = 16,	// Дает 10-50 опыта
	itLittleBagGold = 17,				// Дает 10-50 золота

	/*
		Все листы сортировок что существуют на сервере
	*/
	SORTINVENTORY = 1,
	SORTEQUIP,
	NUMTABSORT,

	NEWBIE_ZERO_WORLD = 1,

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
	BROADCAST_GAME_PRIORITY,
	BROADCAST_GAME_WARNING,
	BROADCAST_MAIN_INFORMATION,
};

enum SpawnTypes
{
	SPAWN_HUMAN = 0,
	SPAWN_BOT = 1,
	SPAWN_HUMAN_SAFE = 2,
	SPAWN_NUM
};

enum BotsTypes
{
	TYPE_BOT_MOB = 1,
	TYPE_BOT_QUEST = 2,
	TYPE_BOT_NPC = 3,
};

enum
{
	SNAPPLAYER = 1,
	SNAPBOTS = 2,
};

enum SaveType
{
	SAVE_ACCOUNT,			// Save Login Password Data
	SAVE_STATS,				// Save Stats Level Exp and other this type
	SAVE_UPGRADES,			// Save Upgrades Damage and other this type
	SAVE_PLANT_DATA,		// Save Plant Account
	SAVE_MINER_DATA,		// Save Mining Account
	SAVE_GUILD_DATA,		// Save Member Data
	SAVE_POSITION,			// Save Position Player
};

#endif
