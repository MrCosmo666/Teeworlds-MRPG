#ifndef GAME_CLIENT_ANIM_UI_H
#define GAME_CLIENT_ANIM_UI_H

#include <game/client/anim_ui/elements.h>

#include <base/tl/array.h>

// TDTW
class CAnimUI
{
public:
	CAnimUI(class CGameClient *Client);

	void DeleteElement(const char *Name);
	CAnimElementsUI *New(const char *Name);
	CAnimElementsUI *Get(const char *Name);

	CAnimElementsUI *Get(int Index) { return m_aAnimElements[Index]; }
	int GetSize() const { return m_aAnimElements.size(); }

	class CGameClient *Client() const { return m_pClient; }
private:
	class CGameClient* m_pClient;
	array<CAnimElementsUI*> m_aAnimElements;

	CAnimElementsUI *SearchElement(char const *Name);
	int SearchElementIndex(char const *Name);
};

#endif
