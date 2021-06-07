/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#ifndef GAME_CLIENT_RENDER_H
#define GAME_CLIENT_RENDER_H

#include <engine/graphics.h>
#include <base/vmath.h>
#include <generated/protocol.h>
#include <game/mapitems.h>
#include "ui.h"


// sprite renderings
enum
{
	SPRITE_FLAG_FLIP_Y = 1,
	SPRITE_FLAG_FLIP_X = 2,

	LAYERRENDERFLAG_OPAQUE = 1,
	LAYERRENDERFLAG_TRANSPARENT = 2,

	TILERENDERFLAG_EXTEND = 4,
};

enum LineDirectionFlag
{
	LINE_LEFT = 1 << 0,
	LINE_DOWN = 1 << 1,
};

class CTeeRenderInfo
{
public:
	CTeeRenderInfo()
	{
		for(int i = 0; i < NUM_SKINPARTS; i++)
			m_aColors[i] = vec4(1,1,1,1);
		m_Size = 1.0f;
		m_GotAirJump = 1;
	};

	IGraphics::CTextureHandle m_aTextures[NUM_SKINPARTS];
	IGraphics::CTextureHandle m_HatTexture;
	IGraphics::CTextureHandle m_BotTexture;
	int m_HatSpriteIndex;
	vec4 m_BotColor;
	vec4 m_aColors[NUM_SKINPARTS];
	float m_Size;
	int m_GotAirJump;
};

typedef void (*ENVELOPE_EVAL)(float TimeOffset, int Env, float *pChannels, void *pUser);
class CTextCursor;

class CRenderTools
{
	void DrawRoundRectExt(float x, float y, float w, float h, float r, int Corners);
	void DrawRoundRectExt4(float x, float y, float w, float h, vec4 ColorTopLeft, vec4 ColorTopRight, vec4 ColorBottomLeft, vec4 ColorBottomRight, float r, int Corners);

public:
	class IGraphics *m_pGraphics;
	class CUI *m_pUI;

	class IGraphics *Graphics() const { return m_pGraphics; }
	class CUI *UI() const { return m_pUI; }

	void SelectSprite(struct CDataSprite *pSprite, int Flags=0, int sx=0, int sy=0);
	void SelectSprite(int id, int Flags=0, int sx=0, int sy=0);

	void DrawSprite(float x, float y, float size);
	void DrawSprite(float x, float y, float Width, float Height);

	// rects
	void DrawRoundRect(const CUIRect *r, vec4 Color, float Rounding);

	void DrawUIRect(const CUIRect *pRect, vec4 Color, int Corners, float Rounding);
	void DrawUIRect4(const CUIRect *pRect, vec4 ColorTopLeft, vec4 ColorTopRight, vec4 ColorBottomLeft, vec4 ColorBottomRight, int Corners, float Rounding);
	void DrawUIRectMonochromeGradient(const CUIRect* pRect, vec4 Color, int Corners, float Rounding);

	void DrawLine(float x, float y, float tox, float toy, vec4 Color);
	void DrawUIRectLine(const CUIRect* pRect, vec4 Color, int DirectionType = LineDirectionFlag::LINE_LEFT);

	// object render methods (gc_render_obj.cpp)
	void RenderTee(class CAnimState* pAnim, const CTeeRenderInfo* pInfo, int Emote, vec2 Dir, vec2 Pos);
	void RenderTeeHand(const CTeeRenderInfo* pInfo, vec2 CenterPos, vec2 Dir, float AngleOffset,
		vec2 PostRotOffset);

	// map render methods (gc_render_map.cpp)
	static void RenderEvalEnvelope(CEnvPoint *pPoints, int NumPoints, int Channels, float Time, float *pResult);
	void RenderQuads(CQuad *pQuads, int NumQuads, int Flags, ENVELOPE_EVAL pfnEval, void *pUser);
	void RenderTilemap(CTile *pTiles, int w, int h, float Scale, vec4 Color, int RenderFlags, ENVELOPE_EVAL pfnEval, void *pUser, int ColorEnv, int ColorEnvOffset);

	// helpers
	void RenderCursor(int ImageID, float CenterX, float CenterY, float Size);
	void MapScreenToWorld(float CenterX, float CenterY, float ParallaxX, float ParallaxY,
		float OffsetX, float OffsetY, float Aspect, float Zoom, float aPoints[4]);
	void MapScreenToGroup(float CenterX, float CenterY, CMapItemGroup *pGroup, float Zoom);

	float DrawClientID(ITextRender* pTextRender, float FontSize, vec2 Position, int ID,
					  const vec4& BgColor = vec4(1, 1, 1, 0.5f), const vec4& TextColor = vec4(0.1f, 0.1f, 0.1f, 1.0f));
	float GetClientIdRectSize(float FontSize);

	// mrpg client
	void RenderWings(CAnimState* pAnim, int SpriteID, vec2 Dir, vec2 Pos, vec2 PosWings, vec2 Size);
	void DrawUIBar(ITextRender* pTextRender, CUIRect Rect, vec4 Color, int Num, int Max, const char* pText, const int Shares, const float Rounding = 2.0f, const float MarginSize = 0.0f, float FontOffset = 0.0f);
	float DrawUIText(ITextRender* pTextRender, vec2 CursorPosition, const char* pText,
		const vec4& BgColor, const vec4& TextColor, float FontSize);
};

#endif
