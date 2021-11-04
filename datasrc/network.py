from datatypes import *

Dialogs = Enum("DIALOG_STYLE", ["MSGBOX", "INPUT", "LIST", "PASSWORD"])
Pickups = Enum("PICKUP", ["HEALTH", "ARMOR", "GRENADE", "SHOTGUN", "LASER", "NINJA", "GUN", "HAMMER"])
Emotes = Enum("EMOTE", ["NORMAL", "PAIN", "HAPPY", "SURPRISE", "ANGRY", "BLINK"])
Emoticons = Enum("EMOTICON", ["OOP", "EXCLAMATION", "HEARTS", "DROP", "DOTDOT", "MUSIC", "SORRY", "GHOST", "SUSHI", "SPLATTEE", "DEVILTEE", "ZOMG", "ZZZ", "WTF", "EYES", "QUESTION"])
Votes = Enum("VOTE", ["UNKNOWN", "START_OP", "START_KICK", "START_SPEC", "END_ABORT", "END_PASS", "END_FAIL"]) # todo 0.8: add RUN_OP, RUN_KICK, RUN_SPEC; rem UNKNOWN
ChatModes = Enum("CHAT", ["NONE", "ALL", "TEAM", "WHISPER"])

PlayerFlags = Flags("PLAYERFLAG", ["ADMIN", "CHATTING", "SCOREBOARD", "READY", "DEAD", "WATCHING", "BOT"])
GameFlags = Flags("GAMEFLAG", ["TEAMS", "FLAGS", "SURVIVAL", "RACE"])
GameStateFlags = Flags("GAMESTATEFLAG", ["WARMUP", "SUDDENDEATH", "ROUNDOVER", "GAMEOVER", "PAUSED", "STARTCOUNTDOWN"])
CoreEventFlags = Flags("COREEVENTFLAG", ["GROUND_JUMP", "AIR_JUMP", "HOOK_ATTACH_PLAYER", "HOOK_ATTACH_GROUND", "HOOK_HIT_NOHOOK"])
RaceFlags = Flags("RACEFLAG", ["HIDE_KILLMSG", "FINISHMSG_AS_CHAT", "KEEP_WANTED_WEAPON"])

GameMsgIDs = Enum("GAMEMSG", ["TEAM_SWAP", "SPEC_INVALIDID", "TEAM_SHUFFLE", "TEAM_BALANCE", "CTF_DROP", "CTF_RETURN",
							
							"TEAM_ALL", "TEAM_BALANCE_VICTIM", "CTF_GRAB",
							
							"CTF_CAPTURE",
							
							"GAME_PAUSED"]) # todo 0.8: sort (1 para)

# mmotee
WorldType = Enum("WORLD", ["STANDARD", "DUNGEON"])
MoodType = Enum("MOOD", ["ANGRY", "AGRESSED_TANK", "AGRESSED_OTHER", "NORMAL", "FRIENDLY", "QUESTING", "PLAYER_TANK"])
MmoPickups = Enum("MMO_PICKUP", ["BOX", "EXPERIENCE", "PLANT", "ORE", "SIDE_ARROW", "MAIN_ARROW", "DROP"])
Equip = Enum("EQUIP", ["HAMMER", "GUN", "SHOTGUN", "GRENADE", "RIFLE", "MINER", "WINGS", "DISCORD"])
Effects = Enum("EFFECT", ["SPASALON", "TELEPORT"])
AuthCodes = Enum("AUTH", ["ALL_UNKNOWN", "ALL_MUSTCHAR", "REGISTER_GOOD", "LOGIN_GOOD", "LOGIN_ALREADY", 
							"LOGIN_WRONG", "LOGIN_NICKNAME", "REGISTER_ERROR_NICK"])




						
RawHeader = '''

#include <engine/message.h>

enum
{
	INPUT_STATE_MASK=0x3f
};

enum
{
	TEAM_SPECTATORS=-1,
	TEAM_RED,
	TEAM_BLUE,
	NUM_TEAMS,

	FLAG_MISSING=-3,
	FLAG_ATSTAND,
	FLAG_TAKEN,

	SPEC_FREEVIEW=0,
	SPEC_PLAYER,
	SPEC_FLAGRED,
	SPEC_FLAGBLUE,
	NUM_SPECMODES,

	SKINPART_BODY = 0,
	SKINPART_MARKING,
	SKINPART_DECORATION,
	SKINPART_HANDS,
	SKINPART_FEET,
	SKINPART_EYES,
	NUM_SKINPARTS,

	EFFECTENCHANT = 10,
};

enum
{
    MAILLETTERFLAG_REFRESH = 1 << 0,
    MAILLETTERFLAG_READ = 1 << 1,
    MAILLETTERFLAG_ACCEPT = 1 << 2,
    MAILLETTERFLAG_DELETE = 1 << 3,
    MAILLETTER_MAX_CAPACITY = 15,
};

enum
{
    TALKED_FLAG_EMPTY_FULL = 1 << 0,
    TALKED_FLAG_BOT = 1 << 1,
    TALKED_FLAG_PLAYER = 1 << 2,
    TALKED_FLAG_SAYS_PLAYER = 1 << 3,
    TALKED_FLAG_SAYS_BOT = 1 << 4,
    TALKED_FLAG_FULL = TALKED_FLAG_PLAYER|TALKED_FLAG_BOT,
};

'''

RawSource = '''
#include <engine/message.h>
#include "protocol.h"
'''

Enums = [
	Pickups,
	Emotes,
	Emoticons,
	Votes,
	ChatModes,
	GameMsgIDs,

    # mmotee
    Effects,
	Equip,
	MmoPickups,
    AuthCodes,
	MoodType,
	WorldType,
    Dialogs,
]

Flags = [
	PlayerFlags,
	GameFlags,
	GameStateFlags,
	CoreEventFlags,
    RaceFlags,
]

Objects = [

	NetObject("PlayerInput", [
		NetIntRange("m_Direction", -1, 1),
		NetIntAny("m_TargetX"),
		NetIntAny("m_TargetY"),

		NetBool("m_Jump"),
		NetIntAny("m_Fire"),
		NetBool("m_Hook"),

		NetFlag("m_PlayerFlags", PlayerFlags),

		NetIntRange("m_WantedWeapon", 0, 'NUM_WEAPONS-1'),
		NetIntAny("m_NextWeapon"),
		NetIntAny("m_PrevWeapon"),
	]),

	NetObject("Projectile", [
		NetIntAny("m_X"),
		NetIntAny("m_Y"),
		NetIntAny("m_VelX"),
		NetIntAny("m_VelY"),

		NetIntRange("m_Type", 0, 'NUM_WEAPONS-1'),
		NetTick("m_StartTick"),
	]),

	NetObject("Laser", [
		NetIntAny("m_X"),
		NetIntAny("m_Y"),
		NetIntAny("m_FromX"),
		NetIntAny("m_FromY"),

		NetTick("m_StartTick"),
	]),

	NetObject("Pickup", [
		NetIntAny("m_X"),
		NetIntAny("m_Y"),

		NetEnum("m_Type", Pickups),
	]),

	NetObject("Flag", [
		NetIntAny("m_X"),
		NetIntAny("m_Y"),

		NetIntRange("m_Team", 'TEAM_RED', 'TEAM_BLUE')
	]),

	NetObject("GameData", [
		NetTick("m_GameStartTick"),
		NetFlag("m_GameStateFlags", GameStateFlags),
		NetTick("m_GameStateEndTick"),
	]),

	NetObject("GameDataTeam", [
		NetIntAny("m_TeamscoreRed"),
		NetIntAny("m_TeamscoreBlue"),
	]),

	NetObject("GameDataFlag", [
		NetIntRange("m_FlagCarrierRed", 'FLAG_MISSING', 'MAX_CLIENTS-1'),
		NetIntRange("m_FlagCarrierBlue", 'FLAG_MISSING', 'MAX_CLIENTS-1'),
		NetTick("m_FlagDropTickRed"),
		NetTick("m_FlagDropTickBlue"),
	]),

	NetObject("CharacterCore", [
		NetTick("m_Tick"),
		NetIntAny("m_X"),
		NetIntAny("m_Y"),
		NetIntAny("m_VelX"),
		NetIntAny("m_VelY"),

		NetIntAny("m_Angle"),
		NetIntRange("m_Direction", -1, 1),

		NetIntRange("m_Jumped", 0, 3),
		NetIntRange("m_HookedPlayer", -1, 'MAX_CLIENTS-1'),
		NetIntRange("m_HookState", -1, 5),
		NetTick("m_HookTick"),

		NetIntAny("m_HookX"),
		NetIntAny("m_HookY"),
		NetIntAny("m_HookDx"),
		NetIntAny("m_HookDy"),
	]),

	NetObject("Character:CharacterCore", [
		NetIntRange("m_Health", 0, 10),
		NetIntRange("m_Armor", 0, 10),
		NetIntAny("m_AmmoCount"),
		NetIntRange("m_Weapon", 0, 'NUM_WEAPONS-1'),
		NetEnum("m_Emote", Emotes),
		NetTick("m_AttackTick"),
		NetFlag("m_TriggeredEvents", CoreEventFlags),
	]),

	NetObject("PlayerInfo", [
		NetFlag("m_PlayerFlags", PlayerFlags),
		NetIntAny("m_Score"),
		NetIntAny("m_Latency"),
	]),

	NetObject("SpectatorInfo", [
		NetIntRange("m_SpecMode", 0, 'NUM_SPECMODES-1'),
		NetIntRange("m_SpectatorID", -1, 'MAX_CLIENTS-1'),
		NetIntAny("m_X"),
		NetIntAny("m_Y"),
	]),

	## Demo

	NetObject("De_ClientInfo", [
		NetBool("m_Local"),
		NetIntRange("m_Team", 'TEAM_SPECTATORS', 'TEAM_BLUE'),

		NetArray(NetIntAny("m_aName"), 4),
		NetArray(NetIntAny("m_aClan"), 3),

		NetIntAny("m_Country"),

		NetArray(NetArray(NetIntAny("m_aaSkinPartNames"), 6), 6),
		NetArray(NetBool("m_aUseCustomColors"), 6),
		NetArray(NetIntAny("m_aSkinPartColors"), 6),
	]),

	NetObject("De_GameInfo", [
		NetFlag("m_GameFlags", GameFlags),
		
		NetIntRange("m_ScoreLimit", 0, 'max_int'),
		NetIntRange("m_TimeLimit", 0, 'max_int'),

		NetIntRange("m_MatchNum", 0, 'max_int'),
		NetIntRange("m_MatchCurrent", 0, 'max_int'),
	]),

	NetObject("De_TuneParams", [
		# todo: should be done differently
		NetArray(NetIntAny("m_aTuneParams"), 32),
	]),

	## Events

	NetEvent("Common", [
		NetIntAny("m_X"),
		NetIntAny("m_Y"),
	]),


	NetEvent("Explosion:Common", []),
	NetEvent("Spawn:Common", []),
	NetEvent("HammerHit:Common", []),

	NetEvent("Death:Common", [
		NetIntRange("m_ClientID", 0, 'MAX_CLIENTS-1'),
	]),

	NetEvent("SoundWorld:Common", [
		NetIntAny("m_SoundID"),
	]),

	NetEvent("Damage:Common", [ # Unused yet
		NetIntRange("m_ClientID", 0, 'MAX_CLIENTS-1'),
		NetIntAny("m_Angle"),
		NetIntRange("m_HealthAmount", 0, 9),
		NetIntRange("m_ArmorAmount", 0, 9),
		NetBool("m_Self"),
	]),

	## Race
	# todo 0.8: move up
	NetObject("PlayerInfoRace", [
		NetTick("m_RaceStartTick"),
	]),

	NetObject("GameDataRace", [
		NetIntRange("m_BestTime", -1, 'max_int'),
		NetIntRange("m_Precision", 0, 3),
		NetFlag("m_RaceFlags", RaceFlags),
	]),
    
    ## mmotee events
	NetEvent("EffectMmo:Common", [
		NetEnum("m_EffectID", Effects),
	]),

	NetEvent("TextEffect:Common", [
		NetArray(NetIntAny("m_aText"), 4),
		NetIntAny("m_Flag"),
	]),

    ## mmotee general object
	NetObject("Mmo_ClientInfo", [
		NetBool("m_Local"),
		NetIntAny("m_Level"),
		NetIntAny("m_Exp"),
		
		NetIntAny("m_Health"),
		NetIntAny("m_HealthStart"),
		NetIntAny("m_Armor"), 

		NetArray(NetIntAny("m_Potions"), 12),
		NetArray(NetIntAny("m_Gold"), 6),
		NetArray(NetIntAny("m_StateName"), 6),

		NetEnum("m_MoodType", MoodType),
		NetEnum("m_WorldType", WorldType),
		NetBool("m_ActiveQuest"),
	]),

	# mmotee send pickup item
	NetObject("MmoPickup", [
		NetIntAny("m_X"),
		NetIntAny("m_Y"),
		NetIntAny("m_Angle"),

		NetEnum("m_Type", MmoPickups),
	]),
    
	# mmotee send projectile item
	NetObject("MmoProj", [
		NetIntAny("m_X"),
		NetIntAny("m_Y"),
		NetIntAny("m_VelX"),
		NetIntAny("m_VelY"),

		NetIntAny("m_Type"),
		NetIntRange("m_Weapon", 0, 'NUM_WEAPONS-1'),
		NetTick("m_StartTick"),
	]),
]

Messages = [

	### Server messages
	NetMessage("Sv_Motd", [
		NetString("m_pMessage"),
	]),

	NetMessage("Sv_Broadcast", [
		NetString("m_pMessage"),
	]),

	NetMessage("Sv_Chat", [
		NetIntRange("m_Mode", 0, 'NUM_CHATS-1'),
		NetIntRange("m_ClientID", -1, 'MAX_CLIENTS-1'),
		NetIntRange("m_TargetID", -1, 'MAX_CLIENTS-1'),
		NetStringStrict("m_pMessage"),
	]),

	NetMessage("Sv_Team", [
		NetIntRange("m_ClientID", -1, 'MAX_CLIENTS-1'),
		NetIntRange("m_Team", 'TEAM_SPECTATORS', 'TEAM_BLUE'),
		NetBool("m_Silent"),
		NetTick("m_CooldownTick"),
	]),

	NetMessage("Sv_KillMsg", [
		NetIntRange("m_Killer", -2, 'MAX_CLIENTS-1'),
		NetIntRange("m_Victim", 0, 'MAX_CLIENTS-1'),
		NetIntRange("m_Weapon", -3, 'NUM_WEAPONS-1'),
		NetIntAny("m_ModeSpecial"),
	]),

	NetMessage("Sv_TuneParams", []),
	NetMessage("Sv_ExtraProjectile", []),
	NetMessage("Sv_ReadyToEnter", []),

	NetMessage("Sv_WeaponPickup", [
		NetIntRange("m_Weapon", 0, 'NUM_WEAPONS-1'),
	]),

	NetMessage("Sv_Emoticon", [
		NetIntRange("m_ClientID", 0, 'MAX_CLIENTS-1'),
		NetEnum("m_Emoticon", Emoticons),
	]),

	NetMessage("Sv_VoteClearOptions", []),

	NetMessage("Sv_VoteOptionListAdd", []),

	NetMessage("Sv_VoteOptionAdd", [
		NetStringStrict("m_pDescription"),
	]),

	NetMessage("Sv_VoteOptionRemove", [
		NetStringStrict("m_pDescription"),
	]),

	NetMessage("Sv_VoteSet", [
		NetIntRange("m_ClientID", -1, 'MAX_CLIENTS-1'),
		NetEnum("m_Type", Votes),
		NetIntRange("m_Timeout", 0, 60),
		NetStringStrict("m_pDescription"),
		NetStringStrict("m_pReason"),
	]),

	NetMessage("Sv_VoteStatus", [
		NetIntRange("m_Yes", 0, 'MAX_CLIENTS'),
		NetIntRange("m_No", 0, 'MAX_CLIENTS'),
		NetIntRange("m_Pass", 0, 'MAX_CLIENTS'),
		NetIntRange("m_Total", 0, 'MAX_CLIENTS'),
	]),

	NetMessage("Sv_ServerSettings", [
		NetBool("m_KickVote"),
		NetIntRange("m_KickMin", 0, 'MAX_CLIENTS'),
		NetBool("m_SpecVote"),
		NetBool("m_TeamLock"),
		NetBool("m_TeamBalance"),
		NetIntRange("m_PlayerSlots", 0, 'MAX_CLIENTS'),
	]),

	NetMessage("Sv_ClientInfo", [
		NetIntRange("m_ClientID", 0, 'MAX_CLIENTS-1'),
		NetBool("m_Local"),
		NetIntRange("m_Team", 'TEAM_SPECTATORS', 'TEAM_BLUE'),
		NetStringStrict("m_pName"),
		NetStringStrict("m_pClan"),
		NetIntAny("m_Country"),
		NetArray(NetStringStrict("m_apSkinPartNames"), 6),
		NetArray(NetBool("m_aUseCustomColors"), 6),
		NetArray(NetIntAny("m_aSkinPartColors"), 6),
		NetBool("m_Silent"),
	]),

	NetMessage("Sv_GameInfo", [
		NetFlag("m_GameFlags", GameFlags),
		
		NetIntRange("m_ScoreLimit", 0, 'max_int'),
		NetIntRange("m_TimeLimit", 0, 'max_int'),

		NetIntRange("m_MatchNum", 0, 'max_int'),
		NetIntRange("m_MatchCurrent", 0, 'max_int'),
	]),

	NetMessage("Sv_ClientDrop", [
		NetIntRange("m_ClientID", 0, 'MAX_CLIENTS-1'),
		NetStringStrict("m_pReason"),
		NetBool("m_Silent"),
	]),

	NetMessage("Sv_GameMsg", []),

	## Demo messages
	NetMessage("De_ClientEnter", [
		NetStringStrict("m_pName"),
		NetIntRange("m_ClientID", -1, 'MAX_CLIENTS-1'),
		NetIntRange("m_Team", 'TEAM_SPECTATORS', 'TEAM_BLUE'),
	]),

	NetMessage("De_ClientLeave", [
		NetStringStrict("m_pName"),
		NetIntRange("m_ClientID", -1, 'MAX_CLIENTS-1'),
		NetStringStrict("m_pReason"),
	]),

	### Client messages
	NetMessage("Cl_Say", [
		NetIntRange("m_Mode", 0, 'NUM_CHATS-1'),
		NetIntRange("m_Target", -1, 'MAX_CLIENTS-1'),
		NetStringStrict("m_pMessage"),
	]),

	NetMessage("Cl_SetTeam", [
		NetIntRange("m_Team", 'TEAM_SPECTATORS', 'TEAM_BLUE'),
	]),

	NetMessage("Cl_SetSpectatorMode", [
		NetIntRange("m_SpecMode", 0, 'NUM_SPECMODES-1'),
		NetIntRange("m_SpectatorID", -1, 'MAX_CLIENTS-1'),
	]),

	NetMessage("Cl_StartInfo", [
		NetStringStrict("m_pName"),
		NetStringStrict("m_pClan"),
		NetIntAny("m_Country"),
		NetArray(NetStringStrict("m_apSkinPartNames"), 6),
		NetArray(NetBool("m_aUseCustomColors"), 6),
		NetArray(NetIntAny("m_aSkinPartColors"), 6),
	]),

	NetMessage("Cl_Kill", []),

	NetMessage("Cl_ReadyChange", []),

	NetMessage("Cl_Emoticon", [
		NetEnum("m_Emoticon", Emoticons),
	]),

	NetMessage("Cl_Vote", [
		NetIntRange("m_Vote", -1, 1),
	]),

	NetMessage("Cl_CallVote", [
		NetStringStrict("m_Type"),
		NetStringStrict("m_Value"),
		NetStringStrict("m_Reason"),
		NetBool("m_Force"),
	]),

	# todo 0.8: move up
	NetMessage("Sv_SkinChange", [
		NetIntRange("m_ClientID", 0, 'MAX_CLIENTS-1'),
		NetArray(NetStringStrict("m_apSkinPartNames"), 6),
		NetArray(NetBool("m_aUseCustomColors"), 6),
		NetArray(NetIntAny("m_aSkinPartColors"), 6),
	]),

	NetMessage("Cl_SkinChange", [
		NetArray(NetStringStrict("m_apSkinPartNames"), 6),
		NetArray(NetBool("m_aUseCustomColors"), 6),
		NetArray(NetIntAny("m_aSkinPartColors"), 6),
	]),
    
	## Race
	NetMessage("Sv_RaceFinish", [
		NetIntRange("m_ClientID", 0, 'MAX_CLIENTS-1'),
		NetIntRange("m_Time", -1, 'max_int'),
		NetIntAny("m_Diff"),
		NetBool("m_RecordPersonal"),
		NetBool("m_RecordServer", default=False),
	]),

	NetMessage("Sv_Checkpoint", [
		NetIntAny("m_Diff"),
	]),
    
	NetMessage("Sv_CommandInfo", [
			NetStringStrict("m_Name"),
			NetStringStrict("m_ArgsFormat"),
			NetStringStrict("m_HelpText")
	]),

	NetMessage("Sv_CommandInfoRemove", [
			NetStringStrict("m_Name")
	]),

	NetMessage("Cl_Command", [
			NetStringStrict("m_Name"),
			NetStringStrict("m_Arguments")
	]),
    
	# mmotee client
	# -------------
	NetMessage("Cl_IsMmoServer", 
	[
		NetIntAny("m_Version"),
	]),

	# authirized client
	NetMessage("Cl_ClientAuth", 
	[
        NetStringStrict("m_Login"),
        NetStringStrict("m_Password"),
		NetBool("m_SelectRegister"),
	]),

	# interactive mmo
	NetMessage("Cl_TalkInteractive", []),
    
	# mmotee server
	# -------------
	NetMessage("Sv_AfterIsMmoServer", []),

    # mmotee send equip items
    NetMessage("Sv_EquipItems", 
	[
		NetIntRange("m_ClientID", 0, 'MAX_CLIENTS-1'),
		NetArray(NetIntAny("m_EquipID"),  9),
		NetArray(NetBool("m_EnchantItem"),  9),
	]),
    
    # mmotee send add vote client
	NetMessage("Sv_VoteMmoOptionAdd", 
	[
		NetStringStrict("m_pDescription"),
        NetIntAny("m_pHexColor"),
		NetArray(NetIntAny("m_pIcon"), 4),
	]),

	# authirized client
	NetMessage("Sv_ClientProgressAuth", 
	[
		NetIntAny("m_Code"),
	]),

	# talk type how broadcast
	NetMessage("Sv_TalkText",
	[
		NetStringStrict("m_pText"),
		NetIntAny("m_ConversationWithClientID"),
		NetEnum("m_Emote", Emotes),
        NetIntAny("m_Flag"),
	]),

	# clear talk text
	NetMessage("Sv_ClearTalkText", []),

	# added item to Questing Process
	NetMessage("Sv_AddQuestingProcessing",
	[
		NetStringStrict("m_pText"),
		NetArray(NetIntAny("m_pIcon"), 4),
		NetIntAny("m_pRequiresNum"),
		NetIntAny("m_pHaveNum"),
        NetBool("m_pGivingTable"),
	]),

	# clear all items on Questing Process
	NetMessage("Sv_ClearQuestingProcessing", []),

	# progress bar
	NetMessage("Sv_ProgressBar",
	[
		NetStringStrict("m_pText"),
		NetIntAny("m_pCount"),
		NetIntAny("m_pRequires"),
	]),

    # music on map
	NetMessage("Sv_WorldMusic",
	[
		NetIntAny("m_pSoundID"),
		NetIntRange("m_pVolume", 1, 10),
	]),

    # -------------
    # mrpg gui boxes
	NetMessage("Sv_SendGuiInformationBox", 
    [
		NetStringStrict("m_pMsg"),
    ]),
    
    # -------------
    # mrpg inbox / TODO: optimize
	NetMessage("Cl_MailLetterActions", 
	[
		NetIntAny("m_MailLetterID"),
		NetIntAny("m_MailLetterFlags"),
	]),
    
    NetMessage("Sv_SendMailLetterInfo",
	[
		NetIntAny("m_MailLetterID"), # unique value by which to receive the letter
		NetStringStrict("m_pTitle"),
        NetStringStrict("m_pFrom"),
		NetStringStrict("m_pMsg"),
        NetBool("m_IsRead"),
        
		NetStringStrict("m_pJsonAttachementItem"),
	]),
    
	NetMessage("Cl_SendMailLetterTo", 
	[
		NetStringStrict("m_pTitle"),
		NetStringStrict("m_pMsg"),
		NetStringStrict("m_pPlayer"),
		NetIntAny("m_FromClientID"),
        
        # todo: add support sending items / after implementations of items / and from
	]),
    
    # - - - -
    # dialogs
	NetMessage("DialogCommon", 
    [
        NetStringStrict("m_pTitle"),
		NetStringStrict("m_pButton1"),
		NetStringStrict("m_pButton2"),
	]),

    # dialog msgbox
	NetMessage("Sv_Dialog_Msgbox:DialogCommon", 
    [
		NetStringStrict("m_pMsg"),
	]),

    # dialog input
	NetMessage("Sv_Dialog_Input:DialogCommon", 
    [
		NetStringStrict("m_pMsg"),
	]),
    
    # dialog list
    NetMessage("Sv_Dialog_List:DialogCommon", []),
    NetMessage("Sv_Dialog_List_AddItem", 
    [
        NetStringStrict("m_pTitle"),
    ]),
]
