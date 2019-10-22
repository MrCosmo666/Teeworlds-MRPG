/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#ifndef GAME_CLIENT_COMPONENTS_ENTITIESSKINS_H
#define GAME_CLIENT_COMPONENTS_ENTITIESSKINS_H
#include <base/vmath.h>
#include <base/tl/sorted_array.h>
#include <game/client/component.h>

class CEntitiesSkins : public CComponent
{
public:
	// do this better and nicer
	struct CEntitiesSkin
	{
		IGraphics::CTextureHandle m_Texture;
		char m_aName[64];

		bool operator<(const CEntitiesSkin& Other) { return str_comp(m_aName, Other.m_aName) < 0; }
	};

	void OnInit();
	void IntitilizeSelectSkin();

	vec3 GetColorV3(int v);
	vec4 GetColorV4(int v);
	int Num();
	const CEntitiesSkin* Get(int Index);
	int Find(const char* pName);

	bool IsLoading() const { return m_IsLoading; }

private:
	bool m_IsLoading;
	sorted_array<CEntitiesSkin> m_aSkins;

	static int SkinScan(const char* pName, int IsDir, int DirType, void* pUser);
};
#endif