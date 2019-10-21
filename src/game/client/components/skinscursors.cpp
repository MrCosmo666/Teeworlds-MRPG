/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#include <math.h>

#include <base/system.h>
#include <base/math.h>
#include <base/color.h>  

#include <engine/graphics.h>
#include <engine/storage.h>
#include <engine/shared/config.h>

#include "skinscursors.h"

int CcSkins::SkinScan(const char* pName, int IsDir, int DirType, void* pUser)
{
	CcSkins* pSelf = (CcSkins*)pUser;
	const char *pSuffix = str_endswith(pName, ".png");
	if (IsDir || !pSuffix) return 0;

	// имя скина и проверяем если скин является загружаемым вначале
	char aSkinName[128];
	str_truncate(aSkinName, sizeof(aSkinName), pName, pSuffix - pName);
	if (str_comp(aSkinName, g_Config.m_GameCursor) == 0) return 0;

	// расположение скина
	char aBuf[512];
	str_format(aBuf, sizeof(aBuf), "cursor/%s", pName);

	// загружаем файл
	CImageInfo Info;
	if (!pSelf->Graphics()->LoadPNG(&Info, aBuf, DirType))
	{
		str_format(aBuf, sizeof(aBuf), "failed to load cursor %s", pName);
		pSelf->Console()->Print(IConsole::OUTPUT_LEVEL_ADDINFO, "game", aBuf);
		return 0;
	}

	// загружаем скин и добавляем в массив
	CcSkin Skin;
	Skin.m_Texture = pSelf->Graphics()->LoadTextureRaw(Info.m_Width, Info.m_Height, Info.m_Format, Info.m_pData, Info.m_Format, 0);
	str_copy(Skin.m_aName, aSkinName, sizeof(Skin.m_aName));
	pSelf->m_aSkins.add(Skin);
	return 0;
}

// инициализация всех скинов при выборе
void CcSkins::IntitilizeSelectSkin()
{
	// сканируем скины
	Storage()->ListDirectory(IStorage::TYPE_ALL, "cursor", SkinScan, this);
}

void CcSkins::OnInit()
{
	// очищаем весь лист скинов
	m_aSkins.clear();

	// если не смогли загрузить стандартный скин
	char aBuf[128];
	str_format(aBuf, sizeof(aBuf), "cursor/%s.png", g_Config.m_GameCursor);

	// загружаем установленный
	CImageInfo Info;
	if (!Graphics()->LoadPNG(&Info, aBuf, IStorage::TYPE_ALL))
	{
		// загружаем станадртный если ошибка с поставленым
		if (!Graphics()->LoadPNG(&Info, "cursor/!gui_cursor.png", IStorage::TYPE_ALL))
			Console()->Print(IConsole::OUTPUT_LEVEL_ADDINFO, "game", "failed to load default data/cursor/!gui_cursor.png");
		else 
			str_copy(g_Config.m_GameCursor, "!gui_cursor", sizeof(g_Config.m_GameCursor));
	}

	// устанавливаем текстуру
	CcSkin StartSkin;
	StartSkin.m_Texture = Graphics()->LoadTextureRaw(Info.m_Width, Info.m_Height, Info.m_Format, Info.m_pData, Info.m_Format, 0);
	str_copy(StartSkin.m_aName, g_Config.m_GameCursor, sizeof(StartSkin.m_aName));
	m_aSkins.add(StartSkin);
}

int CcSkins::Num()
{
	return m_aSkins.size();
}

const CcSkins::CcSkin* CcSkins::Get(int Index)
{
	return &m_aSkins[max(0, Index % m_aSkins.size())];
}

int CcSkins::Find(const char* pName)
{
	for (int i = 0; i < m_aSkins.size(); i++)
	{
		if (str_comp(m_aSkins[i].m_aName, pName) == 0)
			return i;
	}
	return -1;
}

vec3 CcSkins::GetColorV3(int v)
{
	return HslToRgb(vec3(((v >> 16) & 0xff) / 255.0f, ((v >> 8) & 0xff) / 255.0f, 0.5f + (v & 0xff) / 255.0f * 0.5f));
}

vec4 CcSkins::GetColorV4(int v)
{
	vec3 r = GetColorV3(v);
	return vec4(r.r, r.g, r.b, 1.0f);
}