/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#include <base/math.h>

#include <engine/shared/config.h>
#include <engine/graphics.h>
#include <engine/map.h>
#include <engine/textrender.h>
#include <generated/client_data.h>
#include "animstate.h"
#include "render.h"

static float gs_SpriteWScale;
static float gs_SpriteHScale;

void CRenderTools::SelectSprite(CDataSprite *pSpr, int Flags, int sx, int sy)
{
	int x = pSpr->m_X+sx;
	int y = pSpr->m_Y+sy;
	int w = pSpr->m_W;
	int h = pSpr->m_H;
	int cx = pSpr->m_pSet->m_Gridx;
	int cy = pSpr->m_pSet->m_Gridy;

	float f = sqrtf(h*h + w*w);
	gs_SpriteWScale = w/f;
	gs_SpriteHScale = h/f;

	float x1 = x / (float)cx + 0.5f / (float)(cx * 32);
	float x2 = (x + w) / (float)cx - 0.5f / (float)(cx * 32);
	float y1 = y / (float)cy + 0.5f / (float)(cy * 32);
	float y2 = (y + h) / (float)cy - 0.5f / (float)(cy * 32);

	float Temp = 0;

	if(Flags&SPRITE_FLAG_FLIP_Y)
	{
		Temp = y1;
		y1 = y2;
		y2 = Temp;
	}

	if(Flags&SPRITE_FLAG_FLIP_X)
	{
		Temp = x1;
		x1 = x2;
		x2 = Temp;
	}

	Graphics()->QuadsSetSubset(x1, y1, x2, y2);
}

void CRenderTools::SelectSprite(int Id, int Flags, int sx, int sy)
{
	if(Id < 0 || Id >= g_pData->m_NumSprites)
		return;
	SelectSprite(&g_pData->m_aSprites[Id], Flags, sx, sy);
}

void CRenderTools::DrawSprite(float x, float y, float Size)
{
	IGraphics::CQuadItem QuadItem(x, y, Size*gs_SpriteWScale, Size*gs_SpriteHScale);
	Graphics()->QuadsDraw(&QuadItem, 1);
}

void CRenderTools::DrawSprite(float x, float y, float Width, float Height)
{
	IGraphics::CQuadItem QuadItem(x, y, Width, Height);
	Graphics()->QuadsDraw(&QuadItem, 1);
}

void CRenderTools::DrawRoundRectExt(float x, float y, float w, float h, float r, int Corners)
{
	IGraphics::CFreeformItem ArrayF[32];
	int NumItems = 0;
	int Num = 8;
	for(int i = 0; i < Num; i+=2)
	{
		float a1 = i/(float)Num * pi/2;
		float a2 = (i+1)/(float)Num * pi/2;
		float a3 = (i+2)/(float)Num * pi/2;
		float Ca1 = cosf(a1);
		float Ca2 = cosf(a2);
		float Ca3 = cosf(a3);
		float Sa1 = sinf(a1);
		float Sa2 = sinf(a2);
		float Sa3 = sinf(a3);

		if(Corners&1) // TL
			ArrayF[NumItems++] = IGraphics::CFreeformItem(
			x+r, y+r,
			x+(1-Ca1)*r, y+(1-Sa1)*r,
			x+(1-Ca3)*r, y+(1-Sa3)*r,
			x+(1-Ca2)*r, y+(1-Sa2)*r);

		if(Corners&2) // TR
		ArrayF[NumItems++] = IGraphics::CFreeformItem(
			x+w-r, y+r,
			x+w-r+Ca1*r, y+(1-Sa1)*r,
			x+w-r+Ca3*r, y+(1-Sa3)*r,
			x+w-r+Ca2*r, y+(1-Sa2)*r);

		if(Corners&4) // BL
		ArrayF[NumItems++] = IGraphics::CFreeformItem(
			x+r, y+h-r,
			x+(1-Ca1)*r, y+h-r+Sa1*r,
			x+(1-Ca3)*r, y+h-r+Sa3*r,
			x+(1-Ca2)*r, y+h-r+Sa2*r);

		if(Corners&8) // BR
		ArrayF[NumItems++] = IGraphics::CFreeformItem(
			x+w-r, y+h-r,
			x+w-r+Ca1*r, y+h-r+Sa1*r,
			x+w-r+Ca3*r, y+h-r+Sa3*r,
			x+w-r+Ca2*r, y+h-r+Sa2*r);

		if(Corners&16) // ITL
		ArrayF[NumItems++] = IGraphics::CFreeformItem(
			x, y,
			x+(1-Ca1)*r, y-r+Sa1*r,
			x+(1-Ca3)*r, y-r+Sa3*r,
			x+(1-Ca2)*r, y-r+Sa2*r);

		if(Corners&32) // ITR
		ArrayF[NumItems++] = IGraphics::CFreeformItem(
			x+w, y,
			x+w-r+Ca1*r, y-r+Sa1*r,
			x+w-r+Ca3*r, y-r+Sa3*r,
			x+w-r+Ca2*r, y-r+Sa2*r);

		if(Corners&64) // IBL
		ArrayF[NumItems++] = IGraphics::CFreeformItem(
			x, y+h,
			x+(1-Ca1)*r, y+h+(1-Sa1)*r,
			x+(1-Ca3)*r, y+h+(1-Sa3)*r,
			x+(1-Ca2)*r, y+h+(1-Sa2)*r);

		if(Corners&128) // IBR
		ArrayF[NumItems++] = IGraphics::CFreeformItem(
			x+w, y+h,
			x+w-r+Ca1*r, y+h+(1-Sa1)*r,
			x+w-r+Ca3*r, y+h+(1-Sa3)*r,
			x+w-r+Ca2*r, y+h+(1-Sa2)*r);
	}
	Graphics()->QuadsDrawFreeform(ArrayF, NumItems);

	IGraphics::CQuadItem ArrayQ[9];
	NumItems = 0;
	ArrayQ[NumItems++] = IGraphics::CQuadItem(x+r, y+r, w-r*2, h-r*2); // center
	ArrayQ[NumItems++] = IGraphics::CQuadItem(x+r, y, w-r*2, r); // top
	ArrayQ[NumItems++] = IGraphics::CQuadItem(x+r, y+h-r, w-r*2, r); // bottom
	ArrayQ[NumItems++] = IGraphics::CQuadItem(x, y+r, r, h-r*2); // left
	ArrayQ[NumItems++] = IGraphics::CQuadItem(x+w-r, y+r, r, h-r*2); // right

	if(!(Corners&1)) ArrayQ[NumItems++] = IGraphics::CQuadItem(x, y, r, r); // TL
	if(!(Corners&2)) ArrayQ[NumItems++] = IGraphics::CQuadItem(x+w, y, -r, r); // TR
	if(!(Corners&4)) ArrayQ[NumItems++] = IGraphics::CQuadItem(x, y+h, r, -r); // BL
	if(!(Corners&8)) ArrayQ[NumItems++] = IGraphics::CQuadItem(x+w, y+h, -r, -r); // BR

	Graphics()->QuadsDrawTL(ArrayQ, NumItems);
}

void CRenderTools::DrawRoundRectExt4(float x, float y, float w, float h, vec4 ColorTopLeft, vec4 ColorTopRight, vec4 ColorBottomLeft, vec4 ColorBottomRight, float r, int Corners)
{
	int Num = 8;
	for(int i = 0; i < Num; i+=2)
	{
		float a1 = i/(float)Num * pi/2;
		float a2 = (i+1)/(float)Num * pi/2;
		float a3 = (i+2)/(float)Num * pi/2;
		float Ca1 = cosf(a1);
		float Ca2 = cosf(a2);
		float Ca3 = cosf(a3);
		float Sa1 = sinf(a1);
		float Sa2 = sinf(a2);
		float Sa3 = sinf(a3);

		if(Corners&1) // TL
		{
			Graphics()->SetColor(ColorTopLeft.r, ColorTopLeft.g, ColorTopLeft.b, ColorTopLeft.a);
			IGraphics::CFreeformItem ItemF = IGraphics::CFreeformItem(
									x+r, y+r,
									x+(1-Ca1)*r, y+(1-Sa1)*r,
									x+(1-Ca3)*r, y+(1-Sa3)*r,
									x+(1-Ca2)*r, y+(1-Sa2)*r);
			Graphics()->QuadsDrawFreeform(&ItemF, 1);
		}

		if(Corners&2) // TR
		{
			Graphics()->SetColor(ColorTopRight.r, ColorTopRight.g, ColorTopRight.b, ColorTopRight.a);
			IGraphics::CFreeformItem ItemF = IGraphics::CFreeformItem(
									x+w-r, y+r,
									x+w-r+Ca1*r, y+(1-Sa1)*r,
									x+w-r+Ca3*r, y+(1-Sa3)*r,
									x+w-r+Ca2*r, y+(1-Sa2)*r);
			Graphics()->QuadsDrawFreeform(&ItemF, 1);
		}

		if(Corners&4) // BL
		{
			Graphics()->SetColor(ColorBottomLeft.r, ColorBottomLeft.g, ColorBottomLeft.b, ColorBottomLeft.a);
			IGraphics::CFreeformItem ItemF = IGraphics::CFreeformItem(
									x+r, y+h-r,
									x+(1-Ca1)*r, y+h-r+Sa1*r,
									x+(1-Ca3)*r, y+h-r+Sa3*r,
									x+(1-Ca2)*r, y+h-r+Sa2*r);
			Graphics()->QuadsDrawFreeform(&ItemF, 1);
		}

		if(Corners&8) // BR
		{
			Graphics()->SetColor(ColorBottomRight.r, ColorBottomRight.g, ColorBottomRight.b, ColorBottomRight.a);
			IGraphics::CFreeformItem ItemF = IGraphics::CFreeformItem(
									x+w-r, y+h-r,
									x+w-r+Ca1*r, y+h-r+Sa1*r,
									x+w-r+Ca3*r, y+h-r+Sa3*r,
									x+w-r+Ca2*r, y+h-r+Sa2*r);
			Graphics()->QuadsDrawFreeform(&ItemF, 1);
		}

		if(Corners&16) // ITL
		{
			Graphics()->SetColor(ColorTopLeft.r, ColorTopLeft.g, ColorTopLeft.b, ColorTopLeft.a);
			IGraphics::CFreeformItem ItemF = IGraphics::CFreeformItem(
									x, y,
									x+(1-Ca1)*r, y-r+Sa1*r,
									x+(1-Ca3)*r, y-r+Sa3*r,
									x+(1-Ca2)*r, y-r+Sa2*r);
			Graphics()->QuadsDrawFreeform(&ItemF, 1);
		}

		if(Corners&32) // ITR
		{
			Graphics()->SetColor(ColorTopRight.r, ColorTopRight.g, ColorTopRight.b, ColorTopRight.a);
			IGraphics::CFreeformItem ItemF = IGraphics::CFreeformItem(
									x+w, y,
									x+w-r+Ca1*r, y-r+Sa1*r,
									x+w-r+Ca3*r, y-r+Sa3*r,
									x+w-r+Ca2*r, y-r+Sa2*r);
			Graphics()->QuadsDrawFreeform(&ItemF, 1);
		}

		if(Corners&64) // IBL
		{
			Graphics()->SetColor(ColorBottomLeft.r, ColorBottomLeft.g, ColorBottomLeft.b, ColorBottomLeft.a);
			IGraphics::CFreeformItem ItemF = IGraphics::CFreeformItem(
									x, y+h,
									x+(1-Ca1)*r, y+h+(1-Sa1)*r,
									x+(1-Ca3)*r, y+h+(1-Sa3)*r,
									x+(1-Ca2)*r, y+h+(1-Sa2)*r);
			Graphics()->QuadsDrawFreeform(&ItemF, 1);
		}

		if(Corners&128) // IBR
		{
			Graphics()->SetColor(ColorBottomRight.r, ColorBottomRight.g, ColorBottomRight.b, ColorBottomRight.a);
			IGraphics::CFreeformItem ItemF = IGraphics::CFreeformItem(
									x+w, y+h,
									x+w-r+Ca1*r, y+h+(1-Sa1)*r,
									x+w-r+Ca3*r, y+h+(1-Sa3)*r,
									x+w-r+Ca2*r, y+h+(1-Sa2)*r);
			Graphics()->QuadsDrawFreeform(&ItemF, 1);
		}
	}

	Graphics()->SetColor4(ColorTopLeft, ColorTopRight, ColorBottomLeft, ColorBottomRight);
	IGraphics::CQuadItem ItemQ = IGraphics::CQuadItem(x+r, y+r, w-r*2, h-r*2); // center
	Graphics()->QuadsDrawTL(&ItemQ, 1);
	Graphics()->SetColor4(ColorTopLeft, ColorTopRight, ColorTopLeft, ColorTopRight);
	ItemQ = IGraphics::CQuadItem(x+r, y, w-r*2, r); // top
	Graphics()->QuadsDrawTL(&ItemQ, 1);
	Graphics()->SetColor4(ColorBottomLeft, ColorBottomRight, ColorBottomLeft, ColorBottomRight);
	ItemQ = IGraphics::CQuadItem(x+r, y+h-r, w-r*2, r); // bottom
	Graphics()->QuadsDrawTL(&ItemQ, 1);
	Graphics()->SetColor4(ColorTopLeft, ColorTopLeft, ColorBottomLeft, ColorBottomLeft);
	ItemQ = IGraphics::CQuadItem(x, y+r, r, h-r*2); // left
	Graphics()->QuadsDrawTL(&ItemQ, 1);
	Graphics()->SetColor4(ColorTopRight, ColorTopRight, ColorBottomRight, ColorBottomRight);
	ItemQ = IGraphics::CQuadItem(x+w-r, y+r, r, h-r*2); // right
	Graphics()->QuadsDrawTL(&ItemQ, 1);

	if(!(Corners&1))
	{
		Graphics()->SetColor(ColorTopLeft.r, ColorTopLeft.g, ColorTopLeft.b, ColorTopLeft.a);
		IGraphics::CQuadItem ItemQ = IGraphics::CQuadItem(x, y, r, r); // TL
		Graphics()->QuadsDrawTL(&ItemQ, 1);
	}
	if(!(Corners&2))
	{
		Graphics()->SetColor(ColorTopRight.r, ColorTopRight.g, ColorTopRight.b, ColorTopRight.a);
		IGraphics::CQuadItem ItemQ = IGraphics::CQuadItem(x+w, y, -r, r); // TR
		Graphics()->QuadsDrawTL(&ItemQ, 1);
	}
	if(!(Corners&4))
	{
		Graphics()->SetColor(ColorBottomLeft.r, ColorBottomLeft.g, ColorBottomLeft.b, ColorBottomLeft.a);
		IGraphics::CQuadItem ItemQ = IGraphics::CQuadItem(x, y+h, r, -r); // BL
		Graphics()->QuadsDrawTL(&ItemQ, 1);
	}
	if(!(Corners&8))
	{
		Graphics()->SetColor(ColorBottomRight.r, ColorBottomRight.g, ColorBottomRight.b, ColorBottomRight.a);
		IGraphics::CQuadItem ItemQ = IGraphics::CQuadItem(x+w, y+h, -r, -r); // BR
		Graphics()->QuadsDrawTL(&ItemQ, 1);
	}
}

void CRenderTools::DrawRoundRect(const CUIRect *r, vec4 Color, float Rounding)
{
	DrawUIRect(r, Color, CUI::CORNER_ALL, Rounding);
}

void CRenderTools::DrawUIRect(const CUIRect *r, vec4 Color, int Corners, float Rounding)
{
	Graphics()->TextureClear();

	// TODO: FIX US
	Graphics()->QuadsBegin();
	Graphics()->SetColor(Color.r*Color.a, Color.g*Color.a, Color.b*Color.a, Color.a);
	DrawRoundRectExt(r->x,r->y,r->w,r->h,Rounding, Corners);
	Graphics()->QuadsEnd();
}

void CRenderTools::DrawUIRectMonochromeGradient(const CUIRect* pRect, vec4 Color, int Corners, float Rounding)
{
	const vec4 ColorDownGradient = vec4(max(Color.r - 0.05f, 0.0f), max(Color.g - 0.05f, 0.0f), max(Color.b - 0.05f, 0.0f), Color.a);
	DrawUIRect4(pRect, Color, Color, ColorDownGradient, ColorDownGradient, Corners, Rounding);
}

void CRenderTools::DrawLine(float x, float y, float tox, float toy, vec4 Color)
{
	Graphics()->TextureClear();

	Graphics()->LinesBegin();
	Graphics()->SetColor(Color.r, Color.g, Color.b, Color.a);
	IGraphics::CLineItem Line = IGraphics::CLineItem(x, y, tox, toy);
	Graphics()->LinesDraw(&Line, 1);
	Graphics()->SetColor(1.0f, 1.0f, 1.0f, 1.0f);
	Graphics()->LinesEnd();
}

void CRenderTools::DrawUIRectLine(const CUIRect* pRect, vec4 Color, int LineFlags)
{
	float x = pRect->x, y = pRect->y, tox = pRect->x + pRect->w, toy = pRect->y + pRect->h;
	if(LineFlags & LineDirectionFlag::LINE_LEFT)
	{
		tox = pRect->x + pRect->w;
		toy = pRect->y;
	}
	else if(LineFlags & LineDirectionFlag::LINE_DOWN)
	{
		tox = pRect->x;
		toy = pRect->y + pRect->h;
	}
	DrawLine(x, y, tox, toy, Color);
}

void CRenderTools::DrawUIRect4(const CUIRect *r, vec4 ColorTopLeft, vec4 ColorTopRight, vec4 ColorBottomLeft, vec4 ColorBottomRight, int Corners, float Rounding)
{
	Graphics()->TextureClear();

	Graphics()->QuadsBegin();
	DrawRoundRectExt4(r->x,r->y,r->w,r->h,ColorTopLeft,ColorTopRight,ColorBottomLeft,ColorBottomRight,Rounding, Corners);
	Graphics()->QuadsEnd();
}

void CRenderTools::RenderTee(CAnimState* pAnim, const CTeeRenderInfo* pInfo, int Emote, vec2 Dir, vec2 Pos)
{
	vec2 Direction = Dir;
	vec2 Position = Pos;
	bool IsBot = pInfo->m_BotTexture.IsValid();

	// first pass we draw the outline
	// second pass we draw the filling
	for(int p = 0; p < 2; p++)
	{
		bool OutLine = p==0;

		for(int f = 0; f < 2; f++)
		{
			float AnimScale = pInfo->m_Size * 1.0f/64.0f;
			float BaseSize = pInfo->m_Size;
			if(f == 1)
			{
				vec2 BodyPos = Position + vec2(pAnim->GetBody()->m_X, pAnim->GetBody()->m_Y)*AnimScale;
				IGraphics::CQuadItem BodyItem(BodyPos.x, BodyPos.y, BaseSize, BaseSize);
				IGraphics::CQuadItem BotItem(BodyPos.x+(2.f/3.f)*AnimScale, BodyPos.y+(-16+2.f/3.f)*AnimScale, BaseSize, BaseSize); // x+0.66, y+0.66 to correct some rendering bug
				IGraphics::CQuadItem Item;

				// draw bot visuals (background)
				if(IsBot && !OutLine)
				{
					Graphics()->TextureSet(pInfo->m_BotTexture);
					Graphics()->QuadsBegin();
					Graphics()->SetColor(1.0f, 1.0f, 1.0f, 1.0f);
					SelectSprite(SPRITE_TEE_BOT_BACKGROUND, 0, 0, 0);
					Item = BotItem;
					Graphics()->QuadsDraw(&Item, 1);
					Graphics()->QuadsEnd();
				}

				// draw bot visuals (foreground)
				if(IsBot && !OutLine)
				{
					Graphics()->TextureSet(pInfo->m_BotTexture);
					Graphics()->QuadsBegin();
					Graphics()->SetColor(1.0f, 1.0f, 1.0f, 1.0f);
					SelectSprite(SPRITE_TEE_BOT_FOREGROUND, 0, 0, 0);
					Item = BotItem;
					Graphics()->QuadsDraw(&Item, 1);
					Graphics()->SetColor(pInfo->m_BotColor.r, pInfo->m_BotColor.g, pInfo->m_BotColor.b, pInfo->m_BotColor.a);
					SelectSprite(SPRITE_TEE_BOT_GLOW, 0, 0, 0);
					Item = BotItem;
					Graphics()->QuadsDraw(&Item, 1);
					Graphics()->QuadsEnd();
				}

				// draw decoration
				if(pInfo->m_aTextures[SKINPART_DECORATION].IsValid())
				{
					Graphics()->TextureSet(pInfo->m_aTextures[2]);
					Graphics()->QuadsBegin();
					Graphics()->QuadsSetRotation(pAnim->GetBody()->m_Angle*pi*2);
					Graphics()->SetColor(pInfo->m_aColors[SKINPART_DECORATION].r, pInfo->m_aColors[SKINPART_DECORATION].g, pInfo->m_aColors[SKINPART_DECORATION].b, pInfo->m_aColors[SKINPART_DECORATION].a);
					SelectSprite(OutLine?SPRITE_TEE_DECORATION_OUTLINE:SPRITE_TEE_DECORATION, 0, 0, 0);
					Item = BodyItem;
					Graphics()->QuadsDraw(&Item, 1);
					Graphics()->QuadsEnd();
				}

				// draw body (behind marking)
				Graphics()->TextureSet(pInfo->m_aTextures[SKINPART_BODY]);
				Graphics()->QuadsBegin();
				Graphics()->QuadsSetRotation(pAnim->GetBody()->m_Angle*pi*2);
				if(OutLine)
				{
					Graphics()->SetColor(1.0f, 1.0f, 1.0f, 1.0f);
					SelectSprite(SPRITE_TEE_BODY_OUTLINE, 0, 0, 0);
				}
				else
				{
					Graphics()->SetColor(pInfo->m_aColors[SKINPART_BODY].r, pInfo->m_aColors[SKINPART_BODY].g, pInfo->m_aColors[SKINPART_BODY].b, pInfo->m_aColors[SKINPART_BODY].a);
					SelectSprite(SPRITE_TEE_BODY, 0, 0, 0);
				}
				Item = BodyItem;
				Graphics()->QuadsDraw(&Item, 1);
				Graphics()->QuadsEnd();

				// draw marking
				if(pInfo->m_aTextures[SKINPART_MARKING].IsValid() && !OutLine)
				{
					Graphics()->TextureSet(pInfo->m_aTextures[SKINPART_MARKING]);
					Graphics()->QuadsBegin();
					Graphics()->QuadsSetRotation(pAnim->GetBody()->m_Angle*pi*2);
					Graphics()->SetColor(pInfo->m_aColors[SKINPART_MARKING].r*pInfo->m_aColors[SKINPART_MARKING].a, pInfo->m_aColors[SKINPART_MARKING].g*pInfo->m_aColors[SKINPART_MARKING].a,
						pInfo->m_aColors[SKINPART_MARKING].b*pInfo->m_aColors[SKINPART_MARKING].a, pInfo->m_aColors[SKINPART_MARKING].a);
					SelectSprite(SPRITE_TEE_MARKING, 0, 0, 0);
					Item = BodyItem;
					Graphics()->QuadsDraw(&Item, 1);
					Graphics()->QuadsEnd();
				}

				// draw body (in front of marking)
				if(!OutLine)
				{
					Graphics()->TextureSet(pInfo->m_aTextures[SKINPART_BODY]);
					Graphics()->QuadsBegin();
					Graphics()->QuadsSetRotation(pAnim->GetBody()->m_Angle*pi*2);
					Graphics()->SetColor(1.0f, 1.0f, 1.0f, 1.0f);
					for(int t = 0; t < 2; t++)
					{
						SelectSprite(t==0?SPRITE_TEE_BODY_SHADOW:SPRITE_TEE_BODY_UPPER_OUTLINE, 0, 0, 0);
						Item = BodyItem;
						Graphics()->QuadsDraw(&Item, 1);
					}
					Graphics()->QuadsEnd();
				}

				// draw eyes
				Graphics()->TextureSet(pInfo->m_aTextures[SKINPART_EYES]);
				Graphics()->QuadsBegin();
				Graphics()->QuadsSetRotation(pAnim->GetBody()->m_Angle*pi*2);
				if(IsBot)
				{
					Graphics()->SetColor(pInfo->m_BotColor.r, pInfo->m_BotColor.g, pInfo->m_BotColor.b, pInfo->m_BotColor.a);
					Emote = EMOTE_SURPRISE;
				}
				else
					Graphics()->SetColor(pInfo->m_aColors[SKINPART_EYES].r, pInfo->m_aColors[SKINPART_EYES].g, pInfo->m_aColors[SKINPART_EYES].b, pInfo->m_aColors[SKINPART_EYES].a);
				if(p == 1)
				{
					switch (Emote)
					{
						case EMOTE_PAIN:
							SelectSprite(SPRITE_TEE_EYES_PAIN, 0, 0, 0);
							break;
						case EMOTE_HAPPY:
							SelectSprite(SPRITE_TEE_EYES_HAPPY, 0, 0, 0);
							break;
						case EMOTE_SURPRISE:
							SelectSprite(SPRITE_TEE_EYES_SURPRISE, 0, 0, 0);
							break;
						case EMOTE_ANGRY:
							SelectSprite(SPRITE_TEE_EYES_ANGRY, 0, 0, 0);
							break;
						default:
							SelectSprite(SPRITE_TEE_EYES_NORMAL, 0, 0, 0);
							break;
					}

					float EyeScale = BaseSize*0.60f;
					float h = Emote == EMOTE_BLINK ? BaseSize*0.15f/2.0f : EyeScale/2.0f;
					vec2 Offset = vec2(Direction.x*0.125f, -0.05f+Direction.y*0.10f)*BaseSize;
					IGraphics::CQuadItem QuadItem(BodyPos.x+Offset.x, BodyPos.y+Offset.y, EyeScale, h);
					Graphics()->QuadsDraw(&QuadItem, 1);
				}
				Graphics()->QuadsEnd();

				// draw xmas hat
				if(!OutLine && pInfo->m_HatTexture.IsValid())
				{
					Graphics()->TextureSet(pInfo->m_HatTexture);
					Graphics()->QuadsBegin();
					Graphics()->QuadsSetRotation(pAnim->GetBody()->m_Angle*pi * 2);
					Graphics()->SetColor(1.0f, 1.0f, 1.0f, 1.0f);
					int Flag = Direction.x < 0.0f ? SPRITE_FLAG_FLIP_X : 0;
					switch(pInfo->m_HatSpriteIndex)
					{
					case 0:
						SelectSprite(SPRITE_TEE_HATS_TOP1, Flag, 0, 0);
						break;
					case 1:
						SelectSprite(SPRITE_TEE_HATS_TOP2, Flag, 0, 0);
						break;
					case 2:
						SelectSprite(SPRITE_TEE_HATS_SIDE1, Flag, 0, 0);
						break;
					case 3:
						SelectSprite(SPRITE_TEE_HATS_SIDE2, Flag, 0, 0);
					}
					Item = BodyItem;
					Graphics()->QuadsDraw(&Item, 1);
					Graphics()->QuadsEnd();
				}
			}

			// draw feet
			Graphics()->TextureSet(pInfo->m_aTextures[SKINPART_FEET]);
			Graphics()->QuadsBegin();
			CAnimKeyframe *pFoot = f ? pAnim->GetFrontFoot() : pAnim->GetBackFoot();

			float w = BaseSize/2.1f;
			float h = w;

			Graphics()->QuadsSetRotation(pFoot->m_Angle*pi*2);

			if(OutLine)
			{
				Graphics()->SetColor(1.0f, 1.0f, 1.0f, 1.0f);
				SelectSprite(SPRITE_TEE_FOOT_OUTLINE, 0, 0, 0);
			}
			else
			{
				bool Indicate = !pInfo->m_GotAirJump;
				float cs = 1.0f; // color scale
				if(Indicate)
					cs = 0.5f;
				Graphics()->SetColor(pInfo->m_aColors[SKINPART_FEET].r*cs, pInfo->m_aColors[SKINPART_FEET].g*cs, pInfo->m_aColors[SKINPART_FEET].b*cs, pInfo->m_aColors[SKINPART_FEET].a);
				SelectSprite(SPRITE_TEE_FOOT, 0, 0, 0);
			}

			IGraphics::CQuadItem QuadItem(Position.x+pFoot->m_X*AnimScale, Position.y+pFoot->m_Y*AnimScale, w, h);
			Graphics()->QuadsDraw(&QuadItem, 1);
			Graphics()->QuadsEnd();
		}
	}
}

void CRenderTools::RenderTeeHand(const CTeeRenderInfo *pInfo, vec2 CenterPos, vec2 Dir, float AngleOffset,
								 vec2 PostRotOffset)
{
	// in-game hand size is 15 when tee size is 64
	float BaseSize = 15.0f * (pInfo->m_Size / 64.0f);

	vec2 HandPos = CenterPos + Dir;
	float Angle = angle(Dir);
	if(Dir.x < 0)
		Angle -= AngleOffset;
	else
		Angle += AngleOffset;

	vec2 DirX = Dir;
	vec2 DirY(-Dir.y,Dir.x);

	if(Dir.x < 0)
		DirY = -DirY;

	HandPos += DirX * PostRotOffset.x;
	HandPos += DirY * PostRotOffset.y;

	const vec4 Color = pInfo->m_aColors[SKINPART_HANDS];
	IGraphics::CQuadItem QuadOutline(HandPos.x, HandPos.y, 2*BaseSize, 2*BaseSize);
	IGraphics::CQuadItem QuadHand = QuadOutline;

	Graphics()->TextureSet(pInfo->m_aTextures[SKINPART_HANDS]);
	Graphics()->QuadsBegin();
	Graphics()->SetColor(Color.r, Color.g, Color.b, Color.a);
	Graphics()->QuadsSetRotation(Angle);

	SelectSprite(SPRITE_TEE_HAND_OUTLINE, 0, 0, 0);
	Graphics()->QuadsDraw(&QuadOutline, 1);
	SelectSprite(SPRITE_TEE_HAND, 0, 0, 0);
	Graphics()->QuadsDraw(&QuadHand, 1);

	Graphics()->QuadsSetRotation(0);
	Graphics()->QuadsEnd();
}

static void CalcScreenParams(float Amount, float WMax, float HMax, float Aspect, float *w, float *h)
{
	float f = sqrtf(Amount) / sqrtf(Aspect);
	*w = f*Aspect;
	*h = f;

	// limit the view
	if(*w > WMax)
	{
		*w = WMax;
		*h = *w/Aspect;
	}

	if(*h > HMax)
	{
		*h = HMax;
		*w = *h*Aspect;
	}
}

void CRenderTools::MapScreenToWorld(float CenterX, float CenterY, float ParallaxX, float ParallaxY,
	float OffsetX, float OffsetY, float Aspect, float Zoom, float aPoints[4])
{
	float Width, Height;
	CalcScreenParams(1150*1000, 1500, 1050, Aspect, &Width, &Height);
	CenterX *= ParallaxX;
	CenterY *= ParallaxY;
	Width *= Zoom;
	Height *= Zoom;

	aPoints[0] = OffsetX+CenterX-Width/2;
	aPoints[1] = OffsetY+CenterY-Height/2;
	aPoints[2] = aPoints[0]+Width;
	aPoints[3] = aPoints[1]+Height;
}

void CRenderTools::MapScreenToGroup(float CenterX, float CenterY, CMapItemGroup *pGroup, float Zoom)
{
	float aPoints[4];
	MapScreenToWorld(CenterX, CenterY, pGroup->m_ParallaxX/100.0f, pGroup->m_ParallaxY/100.0f,
		pGroup->m_OffsetX, pGroup->m_OffsetY, Graphics()->ScreenAspect(), Zoom, aPoints);
	Graphics()->MapScreen(aPoints[0], aPoints[1], aPoints[2], aPoints[3]);
}

float CRenderTools::DrawClientID(ITextRender* pTextRender, float FontSize, vec2 CursorPosition, int ID,
	const vec4& BgColor, const vec4& TextColor)
{
	if(!g_Config.m_ClShowUserId) return 0;

	char aBuf[4];
	str_format(aBuf, sizeof(aBuf), "%d", ID);

	static CTextCursor s_Cursor;
	s_Cursor.Reset();
	s_Cursor.m_FontSize = FontSize;
	s_Cursor.m_Align = TEXTALIGN_CENTER;

	vec4 OldColor = pTextRender->GetColor();
	pTextRender->TextColor(TextColor);
	pTextRender->TextDeferred(&s_Cursor, aBuf, -1);
	pTextRender->TextColor(OldColor);

	const float LinebaseY = CursorPosition.y + s_Cursor.BaseLineY();
	const float Width = 1.4f * FontSize;

	CUIRect Rect;
	Rect.x = CursorPosition.x;
	Rect.y = LinebaseY - FontSize + 0.15f * FontSize;
	Rect.w = Width;
	Rect.h = FontSize;
	DrawRoundRect(&Rect, BgColor, 0.25f * FontSize);

	s_Cursor.MoveTo(Rect.x + Rect.w / 2.0f, CursorPosition.y);
	pTextRender->DrawTextPlain(&s_Cursor);

	return Width + 0.2f * FontSize;
}

float CRenderTools::GetClientIdRectSize(float FontSize)
{
	if(!g_Config.m_ClShowUserId) return 0;
	return 1.4f * FontSize + 0.2f * FontSize;
}

// mrpg client
float CRenderTools::DrawUIText(ITextRender* pTextRender, vec2 CursorPosition, const char* pText,
	const vec4& BgColor, const vec4& TextColor, float FontSize)
{
	static CTextCursor s_Cursor;
	s_Cursor.Reset();
	s_Cursor.m_FontSize = FontSize;
	s_Cursor.m_Align = TEXTALIGN_CENTER;

	vec4 OldColor = pTextRender->GetColor();
	pTextRender->TextColor(TextColor);
	pTextRender->TextDeferred(&s_Cursor, pText, -1);
	pTextRender->TextColor(OldColor);

	const float LinebaseY = CursorPosition.y + s_Cursor.BaseLineY();
	const float Width = 12.0f + pTextRender->TextWidth(FontSize, pText, -1);

	CUIRect Rect;
	Rect.x = CursorPosition.x;
	Rect.y = LinebaseY - FontSize + 0.15f * FontSize;
	Rect.w = Width;
	Rect.h = FontSize;
	DrawUIRectMonochromeGradient(&Rect, BgColor, CUI::CORNER_ALL, 0.25f * FontSize);

	s_Cursor.MoveTo(Rect.x + Rect.w / 2.0f, CursorPosition.y);
	pTextRender->DrawTextOutlined(&s_Cursor);
	return Rect.w + 4.0f;
}

void CRenderTools::DrawUIBar(ITextRender* pTextRender, CUIRect Rect, vec4 Color, int Num, int Max, const char* pText, const int Shares, const float Rounding, const float MarginSize, float FontOffset)
{
	float ScreenX0, ScreenY0, ScreenX1, ScreenY1;
	Graphics()->GetScreen(&ScreenX0, &ScreenY0, &ScreenX1, &ScreenY1);

	// Background colored progress
	CUIRect BackgroundProgress;
	Rect.Margin(MarginSize, &BackgroundProgress);

	// Processing and centralizing the text by bar
	const float FakeToScreenY = (Graphics()->ScreenHeight() / (ScreenY1 - ScreenY0));
	const float FontSize = ((float)(BackgroundProgress.h * FakeToScreenY) / FakeToScreenY) - FontOffset;
	const float TextWeidth = (float)pTextRender->TextWidth(FontSize, pText, -1);
	if (TextWeidth > Rect.w)
	{
		const float changesSize = TextWeidth - Rect.w;
		Rect.x -= changesSize / 2.0f;
		Rect.w = TextWeidth;

		BackgroundProgress.x = Rect.x;
		BackgroundProgress.w = Rect.w;
	}

	// Render bar
	const float Progress = clamp((float)Num * BackgroundProgress.w / (float)Max, 0.0f, Rect.w);
	DrawRoundRect(&Rect, vec4(0.5f, 0.5f, 0.5f, 0.3f), Rounding);
	BackgroundProgress.w = Progress;
	DrawUIRect4(&BackgroundProgress, Color, Color, Color / 1.1f, Color / 1.1f, 15, (Progress < Rounding ? Progress : Rounding));

	// Cursor
	static CTextCursor s_Cursor;
	const float ProgressCursorX = (Rect.x + Rect.w / 2.0f) - TextWeidth / 2.0f;

	s_Cursor.Reset();
	s_Cursor.m_FontSize = FontSize;
	s_Cursor.MoveTo(ProgressCursorX, Rect.y);
	pTextRender->TextDeferred(&s_Cursor, pText, -1);

	// Shares
	if (Shares)
	{
		const float BordourSize = Rect.w / Shares;
		const float FakeToScreenX = (Graphics()->ScreenWidth() / (ScreenX1 - ScreenX0));
		const float BordourWeidth = max((int)((MarginSize)*FakeToScreenX) / FakeToScreenX, (int)((FontSize / 8.0f) * FakeToScreenX) / FakeToScreenX);
		CUIRect Bordour = { Rect.x, BackgroundProgress.y, BordourWeidth, BackgroundProgress.h };
		for (int i = 0; i < Shares; i++)
		{
			if (BackgroundProgress.w < (BordourSize * (i+1)))
				continue;

			Bordour.x += BordourSize;
			DrawUIRect(&Bordour, Color / 4.0f, -1, 0);
		}
	}

	// Text
	pTextRender->TextColor(1, 1, 1, 0.85f);
	pTextRender->TextSecondaryColor(0.0f, 0.0f, 0.0f, 0.1f);
	pTextRender->DrawTextPlain(&s_Cursor);
	pTextRender->TextColor(1, 1, 1, 1);
	pTextRender->TextSecondaryColor(0.0f, 0.0f, 0.0f, 0.3f);
}

void CRenderTools::RenderWings(CAnimState* pAnim, int SpriteID, vec2 Dir, vec2 Pos, vec2 PosWings, vec2 Size)
{
	if (SpriteID <= 0)
		return;

	vec2 DirNormalizeX = normalize(Dir) * (Size.x / 2.0f);
	vec2 PosNormalizeX = normalize(Dir) * (Size.x / 4.0f);
	vec2 DirNormalizeY = normalize(Dir) * 5.0f;

	for (int p = 0; p < 2; p++)
	{
		for (int f = 0; f < 2; f++)
		{
			if (f == 1)
			{
				Graphics()->TextureSet(g_pData->m_aImages[SpriteID].m_Id);
				Graphics()->QuadsBegin();

				Graphics()->QuadsSetRotation(0 - pAnim->GetWings()->m_Angle * pi * 2);
				IGraphics::CQuadItem Quad2((Pos.x - PosWings.x) + (pAnim->GetWings()->m_X) - PosNormalizeX.x,
					Pos.y - (PosWings.y + pAnim->GetWings()->m_Y) - DirNormalizeY.y,
					Size.x + DirNormalizeX.x,
					Size.y);
				Graphics()->QuadsDrawTL(&Quad2, 1);
				Graphics()->QuadsSetRotation(0 + pAnim->GetWings()->m_Angle * pi * 2);
				IGraphics::CQuadItem Quad((Pos.x + PosWings.x) - (pAnim->GetWings()->m_X) - PosNormalizeX.x,
					Pos.y - (PosWings.y + pAnim->GetWings()->m_Y) - DirNormalizeY.y,
					-Size.x + DirNormalizeX.x,
					Size.y);
				Graphics()->QuadsDrawTL(&Quad, 1);

				Graphics()->QuadsEnd();
			}
		}
	}
}

void CRenderTools::RenderCursor(int ImageID, float CenterX, float CenterY, float Size)
{
	Graphics()->TextureSet(g_pData->m_aImages[ImageID].m_Id);
	Graphics()->WrapClamp();
	Graphics()->QuadsBegin();
	Graphics()->SetColor(1.0f, 1.0f, 1.0f, 1.0f);
	IGraphics::CQuadItem QuadItem(CenterX, CenterY, Size, Size);
	Graphics()->QuadsDrawTL(&QuadItem, 1);
	Graphics()->QuadsEnd();
	Graphics()->WrapNormal();
}