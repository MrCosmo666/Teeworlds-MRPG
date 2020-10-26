/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#ifndef GAME_ENUM_CONTEXT_H
#define GAME_ENUM_CONTEXT_H

#include <game/server/enum_global.h>
#include <generated/protocol.h>

#include <list>
#include <map>

// weapons and player stats
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
	StLuckyDropItem			= 21,
};
 
// player ticks
enum TickState
{
	Die = 1,
	Respawn,
	LastKill,
	LastEmote,
	LastChangeInfo,
	LastChat,
	LastVoteTry,
	LastDialog,
	LastRandomBox,
	NUM_TICK,
};

enum Skill
{
	SkillHeartTurret = 1,	// health recovery turret
	SkillSleepyGravity = 2, // mobbing
	SkillCraftDiscount = 3, // discount on crafting
	SkillMasterWeapon = 4, // automatic gunfire
	SkillBlessingGodWar = 5, // refill ammunition
	SkillNoctisTeleport = 6, // ?knockout? teleport
};

enum AtributType
{
	AtTank,
	AtHealer,
	AtDps,
	AtWeapon,
	AtHardtype,
	AtJob,
	AtOther,
};

enum ToplistTypes
{
	GUILDS_LEVELING,
	GUILDS_WEALTHY,
	PLAYERS_LEVELING,
	PLAYERS_WEALTHY,
};


enum ItemType
{
	TYPE_INVISIBLE = -1,
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
	MENU_JOURNAL_QUEST_INFORMATION,
	MENU_TOP_LIST,
	MENU_DUNGEONS,
	MENU_AUCTION_CREATE_SLOT,
	MENU_SELECT_LANGUAGE,
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
	TAB_LANGUAGES,
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
	TAB_INFO_LANGUAGES,
	TAB_INFO_AUCTION,
	TAB_INFO_AUCTION_BIND,
	TAB_INFO_EQUIP,
	NUM_TAB_MENU,
};

// primary
enum
{
	/*
		Basic kernel server settings
		This is where the most basic server settings are stored
	*/
	MIN_SKINCHANGE_CLIENTVERSION = 0x0703,	// minimum client version for skin change
	MIN_RACE_CLIENTVERSION = 0x0704,		// minimum client version for race type
	MAX_INBOX_LIST = 30,					// maximum number of emails what is displayed
	STATS_MAX_FOR_ITEM = 2,					// maximum number of stats per item

	/*
		All functions of items
		This is sorted in the tabs
	*/
	FUNCTION_ONE_USED = NUM_EQUIPS,
	FUNCTION_USED,
	FUNCTION_SETTINGS,
	FUNCTION_PLANTS,
	FUNCTION_MINER,


	NOPE = -1,
	itGold = 1,							// Money ordinary currency
	itHammer = 2,						// Equipment Hammers
	itMaterial = 7,						// Scraping material
	itTicketGuild = 8,					// Ticket for the creation of the guild
	itSkillPoint = 9,					// Skillpoint
	itDecoArmor = 10,					// Shield Decoration
	itEliteDecoHealth = 11,				// Elite Heart Decoration
	itEliteDecoNinja = 12,				// Elite Ninja Decoration
	itDecoHealth = 13,					// Decoration Heart
	itPotionManaRegen = 14,				// Mana regeneration potion
	itPotionHealthRegen = 15,			// Health regeneration potion
	itCapsuleSurvivalExperience = 16,	// Gives 10-50 experience
	itLittleBagGold = 17,				// Gives 10-50 gold
	itPotionResurrection = 25,			// Resurrection potion
	itExplosiveGun = 33,				// Explosion for gun
	itExplosiveShotgun = 34,			// Explosion for shotgun
	itTicketResetClassStats = 38,		// Ticket to reset the statistics of class upgrades
	itModePVP = 39,						// PVP mode setting
	itTicketResetWeaponStats = 40,		// Ticket to reset the statistics cartridge upgrade
	itTicketDiscountCraft = 43,			// Discount ticket for crafting

	// all sorting sheets that exist on the server
	SORT_INVENTORY = 0,
	SORT_EQUIPING,
	NUM_SORT_TAB,

	// type of decorations
	DECORATIONS_HOUSE = 0,
	DECORATIONS_GUILD_HOUSE,

	// maximum bot slots
	MAX_EQUIPPED_SLOTS_BOTS = EQUIP_WINGS + 1,

	// bot dialogues
	IS_TALKING_EMPTY = 999,
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
	TYPE_BOT_FAKE = 4,
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
	SAVE_GUILD_DATA,		// Save Guild Data
	SAVE_POSITION,			// Save Position Player
	SAVE_LANGUAGE,			// Save Language Client
};

#endif
