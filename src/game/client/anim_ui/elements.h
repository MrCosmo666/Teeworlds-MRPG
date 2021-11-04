#ifndef GAME_CLIENT_ANIM_UI_ELEMENTS_H
#define GAME_CLIENT_ANIM_UI_ELEMENTS_H

#include "value.h"

class CAnimElementsUI
{
protected:
	virtual void Render() = 0;
	virtual void PreRender();
	virtual void PostRender() {};

public:
	char m_aName[256];

    CAnimElementsUI(class CAnimUI* pAnimUI, const char *Name);
	virtual ~CAnimElementsUI();

	class IClient* Client() const;
	class IGraphics* Graphics() const;
	class ITextRender* TextRender() const;
	class CRenderTools* RenderTools() const;

	void Draw();
	virtual void Set(float RoundCorner, int CornerType, bool MonochromeGradient){};

	CValue *GetPos() const { return m_pPos; }
	CValue *GetColor() const { return m_pColor; }

	bool IsAnimated() const { return (m_pPos->m_AnimEnded || m_pColor->m_AnimEnded); }
	void EndAnimations() const
	{ m_pPos->EndAnimation(); m_pColor->EndAnimation(); }

protected:
    CValue *m_pPos;
    CValue *m_pColor;
    class CAnimUI *m_pAnimUI;
};

#endif
