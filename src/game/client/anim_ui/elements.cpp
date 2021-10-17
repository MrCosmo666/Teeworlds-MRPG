#include "elements.h"

#include <engine/graphics.h>
#include <game/client/gameclient.h>
#include <game/client/anim_ui.h>


CAnimElementsUI::CAnimElementsUI(class CAnimUI *pAnimUI, const char *Name)
{
	m_pAnimUI = pAnimUI;
	m_pPos = new CValue();
	m_pColor = new CValue();
	str_copy(m_aName, Name, sizeof(m_aName));
}

CAnimElementsUI::~CAnimElementsUI()
{
	delete m_pPos;
	delete m_pColor;
}

void CAnimElementsUI::Draw()
{
	PreRender();
	Render();
	PostRender();
}

void CAnimElementsUI::PreRender()
{
	m_pPos->PreRender();
	m_pColor->PreRender();
}

class IClient *CAnimElementsUI::Client() const{ return m_pAnimUI->Client()->Client();}
class IGraphics *CAnimElementsUI::Graphics() const{ return m_pAnimUI->Client()->Graphics();}
class ITextRender *CAnimElementsUI::TextRender() const{ return m_pAnimUI->Client()->TextRender();}
class CRenderTools *CAnimElementsUI::RenderTools() const{ return m_pAnimUI->Client()->RenderTools();}