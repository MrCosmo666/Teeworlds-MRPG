/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#ifndef GAME_CLIENT_COMPONENTS_ENSKINS_H
#define GAME_CLIENT_COMPONENTS_ENSKINS_H
#include <base/vmath.h>
#include <base/tl/sorted_array.h>
#include <game/client/component.h>

class CEnSkins : public CComponent
{
public:
	// do this better and nicer
	struct CEnSkin
	{
		IGraphics::CTextureHandle m_Texture;
		char m_aName[64];

		bool operator<(const CEnSkin& Other) { return str_comp(m_aName, Other.m_aName) < 0; }
	};

	void OnInit();
	void IntitilizeSelectSkin();

	vec3 GetColorV3(int v);
	vec4 GetColorV4(int v);
	int Num();
	const CEnSkin* Get(int Index);
	int Find(const char* pName);

private:
	sorted_array<CEnSkin> m_aSkins;

	static int SkinScan(const char* pName, int IsDir, int DirType, void* pUser);
};
#endif