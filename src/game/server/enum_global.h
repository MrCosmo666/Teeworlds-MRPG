/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#ifndef GAME_GLOBAL_ENUM_H
#define GAME_GLOBAL_ENUM_H

#include <base/stringable_enum.h>

#define GRAY_COLOR vec3(40, 42, 45)
#define LIGHT_GRAY_COLOR vec3(15, 15, 16)
#define SMALL_LIGHT_GRAY_COLOR vec3(10, 11, 11)
#define GOLDEN_COLOR vec3(35, 20, 2)
#define LIGHT_GOLDEN_COLOR vec3(16, 7, 0)
#define RED_COLOR vec3(40, 15, 15)
#define LIGHT_RED_COLOR vec3(16, 7, 5)
#define SMALL_LIGHT_RED_COLOR vec3(10, 5, 3)
#define BLUE_COLOR vec3(10, 22, 40)
#define LIGHT_BLUE_COLOR vec3(2, 7, 16)
#define PURPLE_COLOR vec3(32, 10, 40)
#define LIGHT_PURPLE_COLOR vec3(16, 5, 20)
#define GREEN_COLOR vec3(15, 40, 15)
#define LIGHT_GREEN_COLOR vec3(0, 16, 0)

// Miner data
#define MINER(F) \
	F(MnrLevel) \
	F(MnrExp) \
	F(MnrUpgrade) \
	F(MnrCount)
STRINGABLE_ENUM_DECL(MINER)

// Plant data
#define PLANT(F) \
	F(PlLevel) \
	F(PlExp) \
	F(PlCounts) \
	F(PlUpgrade)
STRINGABLE_ENUM_DECL(PLANT)

// Improvements to organizations by field
#define EMEMBERUPGRADE(F) \
    F(AvailableNSTSlots) \
    F(ChairNSTExperience)
STRINGABLE_ENUM_DECL(EMEMBERUPGRADE)

/*
	Basic kernel server settings
	This is where the most basic server settings are stored
*/
enum DayType
{
	NIGHT_TYPE = 1,
	DAY_TYPE,
	MORNING_TYPE,
	EVENING_TYPE
};

enum
{
	ENGINE_MAX_WORLDS = 64,		// TODO: change to dynamic allocator, we allocate memory blocks for the server in advance for 64 worlds
	MAIN_WORLD_ID = 0,
	FAKE_DISCORD_WORLD_ID = 7,

	MAX_DROPPED_FROM_MOBS = 5,  // maximum number of items that can be dropped by mobs
};

#endif
