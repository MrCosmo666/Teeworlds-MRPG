/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#ifndef GAME_VERSION_H
#define GAME_VERSION_H
	#include <generated/nethash.cpp>

	#define SETTINGS_FILENAME "settings_mmo"
	#define GAME_VERSION "0.7.5"
	#define GAME_NETVERSION_HASH_FORCED "802f1be60a05665f"
	#define GAME_NETVERSION "0.7 " GAME_NETVERSION_HASH_FORCED
	#define CLIENT_VERSION 0x0705
	// версия клиента релиза
	#define GAME_RELEASE_VERSION "1.1.5"

	// весрсия под сервер
	#define CLIENT_VERSION_MOLD 1012
	#define CLIENT_VERSION_MMO 1021

#endif
