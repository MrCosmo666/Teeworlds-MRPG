/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#ifndef GAME_SERVER_COMPONENT_GUILD_DATA_H
#define GAME_SERVER_COMPONENT_GUILD_DATA_H

struct CGuildData
{
	enum
	{
		AVAILABLE_SLOTS = 0,
		CHAIR_EXPERIENCE = 1,
		NUM_GUILD_UPGRADES,
	};
	CGuildData()
	{
		m_Upgrades[AVAILABLE_SLOTS] = { "Available slots", "AvailableSlots", 0 };
		m_Upgrades[CHAIR_EXPERIENCE] = { "Chair experience", "ChairExperience", 0 };
	}
	struct CFieldStruct
	{
		CFieldStruct() = default;
		CFieldStruct(const CFieldStruct& pField) = default;

		char m_aName[64];
		char m_aFieldName[64];
		int m_Value;
	};
	CFieldStruct m_Upgrades[NUM_GUILD_UPGRADES];

	char m_aName[32];
	int m_Level;
	int m_Exp;
	int m_OwnerID;
	int m_Bank;
	int m_Score;

	static std::map < int, CGuildData > ms_aGuild;
};

struct CGuildHouseData
{
	int m_PosX;
	int m_PosY;
	int m_DoorX;
	int m_DoorY;
	int m_TextX;
	int m_TextY;
	int m_WorldID;
	int m_Price;
	int m_Payment;
	int m_GuildID;
	class GuildDoor* m_pDoor;

	static std::map < int, CGuildHouseData > ms_aHouseGuild;
};

struct CGuildRankData
{
	char m_aRank[32];
	int m_GuildID;
	int m_Access;

	static std::map < int, CGuildRankData > ms_aRankGuild;
};

#endif
