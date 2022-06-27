#include <engine/graphics.h>
#include "anim_ui/block.h"
#include "anim_ui.h"

CAnimUI::CAnimUI(class CGameClient *Client)
{
	m_pClient = Client;
}

CAnimElementsUI *CAnimUI::New(const char *Name)
{
	CAnimElementsUI *Element = new CBlock(this, Name);
	m_aAnimElements.add(Element);
	return Element;
}

CAnimElementsUI *CAnimUI::Get(const char *Name)
{
	CAnimElementsUI *pResultElement = SearchElement(Name);
	if (pResultElement == NULL)
		pResultElement = New(Name);
	return pResultElement;
}

CAnimElementsUI *CAnimUI::SearchElement(char const *Name)
{
	for(int i = 0; i < m_aAnimElements.size(); i++)
	{
		if(str_comp_nocase(m_aAnimElements[i]->m_aName, Name) == 0)
			return m_aAnimElements[i];
	}
	return NULL;
}

int CAnimUI::SearchElementIndex(char const *Name)
{
	for(int i = 0; i < m_aAnimElements.size(); i++)
	{
		if(str_comp_nocase(m_aAnimElements[i]->m_aName, Name) == 0)
			return i;
	}
	return -1;
}

void CAnimUI::DeleteElement(const char* Name)
{
	int index = SearchElementIndex(Name);
	if(index != -1)
	{
		delete m_aAnimElements[index];
		m_aAnimElements.remove_index(index);
	}
}
