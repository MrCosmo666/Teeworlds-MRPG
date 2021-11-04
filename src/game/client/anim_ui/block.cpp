#include "block.h"

#include <game/client/render.h>

CBlock::CBlock(class CAnimUI* pAnimUI, const char *Name) : CAnimElementsUI(pAnimUI, Name)
{
	m_CornerType = CUI::CORNER_ALL;
	m_RoundCorner = 0.0f;
	m_MonochromeGradient = false;
}

void CBlock::Render()
{
	CUIRect Rect = { m_pPos->GetValue().x, m_pPos->GetValue().y, m_pPos->GetValue().w, m_pPos->GetValue().h };
	if(!m_MonochromeGradient)
		RenderTools()->DrawUIRect(&Rect, m_pColor->GetValue(), (int)m_CornerType, m_RoundCorner);
	else
		RenderTools()->DrawUIRectMonochromeGradient(&Rect, m_pColor->GetValue(), (int)m_CornerType, m_RoundCorner);
}

void CBlock::Set(float RoundCorner, int CornerType, bool MonochromeGradient)
{
	m_RoundCorner = RoundCorner;
	m_CornerType = CornerType;
	m_MonochromeGradient = MonochromeGradient;
}


