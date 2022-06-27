/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#include <engine/graphics.h>
#include <engine/storage.h>
#include <engine/shared/config.h>
#include <generated/client_data.h>
#include <game/client/component.h>
#include <engine/editor.h>

#include "skinchanger.h"

CCSkinChanger::CCSkinChanger()
{
	m_GameSkins.m_IsLoaded = false;
	m_Particles.m_IsLoaded = false;
	m_Cursors.m_IsLoaded = false;
	m_Emoticons.m_IsLoaded = false;
}

void CCSkinChanger::DelayedInit()
{
	m_GameSkins.DelayedInit();
	m_Particles.DelayedInit();
	m_Cursors.DelayedInit();
	m_Emoticons.DelayedInit();
	m_Entities.DelayedInit();
}

// texture-type abstract entity
IGraphics::CTextureHandle CCSkinChanger::CTextureEntity::Get(int Index) const
{
	return m_Info[clamp(Index, 0, m_Count)].m_aTextures;
}

IGraphics::CTextureHandle CCSkinChanger::CTextureEntity::GetDefault() const
{
	return m_DefaultTexture;
}

const char* CCSkinChanger::CTextureEntity::GetName(int Index) const
{
	return m_Info[clamp(Index, 0, m_Count)].m_aName;
}

int CCSkinChanger::CTextureEntity::Num() const
{
	return m_Count;
}

int CCSkinChanger::CTextureEntity::EntityScan(const char *pName, int IsDir, int DirType, void *pUser)
{
	CCSkinChanger::CTextureEntity *pSelf = (CCSkinChanger::CTextureEntity *)pUser;
	if (IsDir || !str_endswith(pName, ".png"))
		return 0;
	if (pSelf->m_Count >= MAX_TEXTURES)
		return 1;

	if (!pSelf->LoadEntity(pName, DirType, &pSelf->m_Info[pSelf->m_Count].m_aTextures))
		return 0;
	// write name
	str_copy(pSelf->m_Info[pSelf->m_Count].m_aName, pName, sizeof(pSelf->m_Info[pSelf->m_Count].m_aName));

	pSelf->m_Count++;
	return 0;
}

// game skins
CCSkinChanger::CGameSkins::CGameSkins()
{
	m_Count = 0;
}

void CCSkinChanger::CGameSkins::DelayedInit()
{
	m_DefaultTexture = g_pData->m_aImages[IMAGE_GAME].m_Id; // save default texture
	if (g_Config.m_GameTexture[0] != '\0')
	{
		if (LoadEntity(g_Config.m_GameTexture, IStorageEngine::TYPE_ALL, &m_InitialTexture))
		{
			g_pData->m_aImages[IMAGE_GAME].m_Id = m_InitialTexture;
			char aBuf[256];
			str_format(aBuf, sizeof(aBuf), "gameskins/%s", g_Config.m_GameTexture);
			return;
		}
		g_Config.m_GameTexture[0] = '\0';
	}
}

void CCSkinChanger::CGameSkins::LoadEntities()
{
	// unload all textures (don't think that's necessary)
	for (int i = 0; i < m_Count; i++)
		Graphics()->UnloadTexture(&(m_Info[i].m_aTextures));

	Storage()->ListDirectory(IStorageEngine::TYPE_ALL, "gameskins", EntityScan, this);
	m_IsLoaded = true;
}

bool CCSkinChanger::CGameSkins::LoadEntity(const char *pName, int DirType, IGraphics::CTextureHandle *pTexture)
{
	char aBuf[512];
	str_format(aBuf, sizeof(aBuf), "gameskins/%s", pName);
	CImageInfo Info;

	if (!Graphics()->LoadPNG(&Info, aBuf, DirType))
	{
		str_format(aBuf, sizeof(aBuf), "failed to load gameskin '%s'", pName);
		Console()->Print(IConsole::OUTPUT_LEVEL_ADDINFO, "entities", aBuf);
		return false;
	}

	// load entities
	Graphics()->UnloadTexture(pTexture);
	*pTexture = Graphics()->LoadTextureRaw(Info.m_Width, Info.m_Height, Info.m_Format, Info.m_pData, Info.m_Format, 0);
	mem_free(Info.m_pData);
	return true;
}

void CCSkinChanger::CGameSkins::Reload(int id)
{
	if (id < 0)
		g_pData->m_aImages[IMAGE_GAME].m_Id = GetDefault();
	else
		g_pData->m_aImages[IMAGE_GAME].m_Id = Get(id);
}

// particles
CCSkinChanger::CParticles::CParticles()
{
	m_Count = 0;
}
void CCSkinChanger::CParticles::DelayedInit()
{
	m_DefaultTexture = g_pData->m_aImages[IMAGE_PARTICLES].m_Id; // save default texture
	if (g_Config.m_GameParticles[0] != '\0')
	{
		if (LoadEntity(g_Config.m_GameParticles, IStorageEngine::TYPE_ALL, &m_InitialTexture))
		{
			g_pData->m_aImages[IMAGE_PARTICLES].m_Id = m_InitialTexture;
			char aBuf[256];
			str_format(aBuf, sizeof(aBuf), "particles/%s", g_Config.m_GameParticles);
			return;
		}
		g_Config.m_GameParticles[0] = '\0';
	}
}
void CCSkinChanger::CParticles::LoadEntities()
{
	// unload all textures (don't think that's necessary)
	for (int i = 0; i < m_Count; i++)
		Graphics()->UnloadTexture(&(m_Info[i].m_aTextures));

	Storage()->ListDirectory(IStorageEngine::TYPE_ALL, "particles", EntityScan, this);
	m_IsLoaded = true;
}
bool CCSkinChanger::CParticles::LoadEntity(const char *pName, int DirType, IGraphics::CTextureHandle *pTexture)
{
	char aBuf[512];
	str_format(aBuf, sizeof(aBuf), "particles/%s", pName);
	CImageInfo Info;
	if (!Graphics()->LoadPNG(&Info, aBuf, DirType))
	{
		str_format(aBuf, sizeof(aBuf), "failed to load particles '%s'", pName);
		Console()->Print(IConsole::OUTPUT_LEVEL_ADDINFO, "entities", aBuf);
		return false;
	}

	// load entities
	Graphics()->UnloadTexture(pTexture);
	*pTexture = Graphics()->LoadTextureRaw(Info.m_Width, Info.m_Height, Info.m_Format, Info.m_pData, Info.m_Format, 0);
	mem_free(Info.m_pData);
	return true;
}
void CCSkinChanger::CParticles::Reload(int id)
{
	if (id < 0)
		g_pData->m_aImages[IMAGE_PARTICLES].m_Id = GetDefault();
	else
		g_pData->m_aImages[IMAGE_PARTICLES].m_Id = Get(id);
}

// cursors
CCSkinChanger::CCursors::CCursors()
{
	m_Count = 0;
}
void CCSkinChanger::CCursors::DelayedInit()
{
	m_DefaultTexture = g_pData->m_aImages[IMAGE_CURSOR].m_Id; // save default texture
	if (g_Config.m_GameCursor[0] != '\0')
	{
		if (LoadEntity(g_Config.m_GameCursor, IStorageEngine::TYPE_ALL, &m_InitialTexture))
		{
			g_pData->m_aImages[IMAGE_CURSOR].m_Id = m_InitialTexture;
			char aBuf[256];
			str_format(aBuf, sizeof(aBuf), "cursor/%s", g_Config.m_GameCursor);
			return;
		}
		g_Config.m_GameCursor[0] = '\0';
	}
}
void CCSkinChanger::CCursors::LoadEntities()
{
	// unload all textures (don't think that's necessary)
	for (int i = 0; i < m_Count; i++)
		Graphics()->UnloadTexture(&(m_Info[i].m_aTextures));

	Storage()->ListDirectory(IStorageEngine::TYPE_ALL, "cursor", EntityScan, this);
	m_IsLoaded = true;
}
bool CCSkinChanger::CCursors::LoadEntity(const char *pName, int DirType, IGraphics::CTextureHandle *pTexture)
{
	char aBuf[512];
	str_format(aBuf, sizeof(aBuf), "cursor/%s", pName);
	CImageInfo Info;
	if (!Graphics()->LoadPNG(&Info, aBuf, DirType))
	{
		str_format(aBuf, sizeof(aBuf), "failed to load cursor '%s'", pName);
		Console()->Print(IConsole::OUTPUT_LEVEL_ADDINFO, "entities", aBuf);
		return false;
	}

	// load entities
	Graphics()->UnloadTexture(pTexture);
	*pTexture = Graphics()->LoadTextureRaw(Info.m_Width, Info.m_Height, Info.m_Format, Info.m_pData, Info.m_Format, 0);
	mem_free(Info.m_pData);
	return true;
}
void CCSkinChanger::CCursors::Reload(int id)
{
	if (id < 0)
		g_pData->m_aImages[IMAGE_CURSOR].m_Id = GetDefault();
	else
		g_pData->m_aImages[IMAGE_CURSOR].m_Id = Get(id);
}

// emoticons
CCSkinChanger::CEmoticons::CEmoticons()
{
	m_Count = 0;
}
void CCSkinChanger::CEmoticons::DelayedInit()
{
	m_DefaultTexture = g_pData->m_aImages[IMAGE_EMOTICONS].m_Id; // save default texture
	if (g_Config.m_GameEmoticons[0] != '\0')
	{
		if (LoadEntity(g_Config.m_GameEmoticons, IStorageEngine::TYPE_ALL, &m_InitialTexture))
		{
			g_pData->m_aImages[IMAGE_EMOTICONS].m_Id = m_InitialTexture;
			char aBuf[256];
			str_format(aBuf, sizeof(aBuf), "emoticons/%s", g_Config.m_GameEmoticons);
			return;
		}
		g_Config.m_GameEmoticons[0] = '\0';
	}
}
void CCSkinChanger::CEmoticons::LoadEntities()
{
	// unload all textures (don't think that's necessary)
	for (int i = 0; i < m_Count; i++)
		Graphics()->UnloadTexture(&(m_Info[i].m_aTextures));

	Storage()->ListDirectory(IStorageEngine::TYPE_ALL, "emoticons", EntityScan, this);
	m_IsLoaded = true;
}
bool CCSkinChanger::CEmoticons::LoadEntity(const char *pName, int DirType, IGraphics::CTextureHandle *pTexture)
{
	char aBuf[512];
	str_format(aBuf, sizeof(aBuf), "emoticons/%s", pName);
	CImageInfo Info;
	if (!Graphics()->LoadPNG(&Info, aBuf, DirType))
	{
		str_format(aBuf, sizeof(aBuf), "failed to load emoticons '%s'", pName);
		Console()->Print(IConsole::OUTPUT_LEVEL_ADDINFO, "entities", aBuf);
		return false;
	}

	// load entities
	Graphics()->UnloadTexture(pTexture);
	*pTexture = Graphics()->LoadTextureRaw(Info.m_Width, Info.m_Height, Info.m_Format, Info.m_pData, Info.m_Format, 0);
	mem_free(Info.m_pData);
	return true;
}
void CCSkinChanger::CEmoticons::Reload(int id)
{
	if (id < 0) g_pData->m_aImages[IMAGE_EMOTICONS].m_Id = GetDefault();
	else g_pData->m_aImages[IMAGE_EMOTICONS].m_Id = Get(id);
}

// entities
CCSkinChanger::CEntities::CEntities()
{
	m_Count = 0;
}
void CCSkinChanger::CEntities::DelayedInit() { }

void CCSkinChanger::CEntities::LoadEntities()
{
	m_DefaultTexture = Graphics()->LoadTexture("editor/entities.png", IStorageEngine::TYPE_ALL, CImageInfo::FORMAT_AUTO, IGraphics::TEXLOAD_MULTI_DIMENSION);

	// unload all textures (don't think that's necessary)
	for (int i = 0; i < m_Count; i++)
		Graphics()->UnloadTexture(&(m_Info[i].m_aTextures));

	Storage()->ListDirectory(IStorageEngine::TYPE_ALL, "entities", EntityScan, this);
	m_IsLoaded = true;
}
bool CCSkinChanger::CEntities::LoadEntity(const char *pName, int DirType, IGraphics::CTextureHandle *pTexture)
{
	char aBuf[512];
	str_format(aBuf, sizeof(aBuf), "entities/%s", pName);
	CImageInfo Info;
	if (!Graphics()->LoadPNG(&Info, aBuf, DirType))
	{
		str_format(aBuf, sizeof(aBuf), "failed to load entities '%s'", pName);
		Console()->Print(IConsole::OUTPUT_LEVEL_ADDINFO, "entities", aBuf);
		return false;
	}

	// load entities
	Graphics()->UnloadTexture(pTexture);
	*pTexture = Graphics()->LoadTextureRaw(Info.m_Width, Info.m_Height, Info.m_Format, Info.m_pData, Info.m_Format, 0);
	mem_free(Info.m_pData);
	return true;
}
void CCSkinChanger::CEntities::Reload(int id)
{
	if (id < 0) g_Config.m_GameEntities[0] = '\0';
	else str_copy(g_Config.m_GameEntities, GetName(id), sizeof(g_Config.m_GameEntities));
}