#include <base/color.h>

#include <engine/storage.h>
#include <engine/shared/config.h>
#include <engine/textrender.h>
#include <engine/graphics.h>

#include <game/version.h>
#include <game/client/ui.h>
#include <game/client/render.h>
#include <game/client/gameclient.h>

#include <generated/client_data.h>

#include <game/client/components/binds.h>
#include <game/client/components/menus.h>
#include <game/client/components/items.h>
#include <game/client/components/skins.h>
#include <game/client/components/skinscursors.h>
#include <game/client/components/skinsemotes.h>
#include <game/client/components/skinsgame.h>
#include <game/client/components/skinsprojectile.h>
#include <game/client/components/skinsentities.h>

void CMenus::RenderSettingsMmo(CUIRect MainView)
{
	CUIRect Label, Button, Tabbar;

	static int s_SettingsPage = 0;
	MainView.HSplitBottom(80.0f, &MainView, 0);
	MainView.HSplitTop(18.0f, &Tabbar, &MainView);

	// рисуем меню вкладок
	const char* Tabs[4] = { "Visual", "General", "Effects", "Credits" };
	for (int i = 0; i < 4; i++)
	{
		Tabbar.VSplitLeft(182.5f, &Button, &Tabbar);

		static CButtonContainer s_Buttons[4];
		if (DoButton_MenuTabTop(&s_Buttons[i], Tabs[i], Client()->State() == IClient::STATE_OFFLINE && s_SettingsPage == i, &Button, s_SettingsPage == i ? 1.0f : 1.5f, 1.0f, 0))
			s_SettingsPage = i;
	}

	// длина фона и место для текста
	MainView.HSplitTop(400.0f, &MainView, &Label);

	// рисуем текст информации
	const char* Information[4] = { "Setting up the visual part of the client", "Setting up the general part of the client", "Setting up the effects part of the client", "Information & Credits" };
	UI()->DoLabel(&Label, Information[s_SettingsPage], 14.0f, CUI::ALIGN_CENTER);

	// рисуем меню выбранной страницы
	RenderSettingsMmoGeneral(MainView, s_SettingsPage);
}

void CMenus::RenderSettingsMmoGeneral(CUIRect MainView, int Page)
{
	CUIRect Button;
	RenderTools()->DrawUIRect4(&MainView, vec4(0.0f, 0.0f, 0.0f, g_Config.m_ClMenuAlpha / 50.0f), vec4(0.0f, 0.0f, 0.0f, g_Config.m_ClMenuAlpha / 50.0f), vec4(0.0f, 0.0f, 0.0f, 0.0f), vec4(0.0f, 0.0f, 0.0f, 0.0f), 0, 5.0f);

	// render MmoSet menu background
	float ButtonHeight = 20.0f;
	float Spacing = 2.0f;

	// Визуальные настройки
	if (Page == 0)
		RenderMmoSettingsTexture(MainView, MainView);
	
	// генеральные настройки
	else if (Page == 1)
	{
		// базовые настройки
		MainView.HSplitTop(ButtonHeight, &Button, &MainView);
		UI()->DoLabel(&Button, "Basic settings", 14.0f, CUI::ALIGN_CENTER);
	
		// устанавливаем стороны
		CUIRect Basic = MainView, BasicLeft, BasicRight;
		Basic.VSplitMid(&BasicLeft, &BasicRight);
		BasicLeft.VSplitRight(Spacing * 0.5f, &BasicLeft, 0);
		BasicRight.VSplitLeft(Spacing * 0.5f, 0, &BasicRight);

		// левая сторона
		// vanilla damage ind
		BasicLeft.HSplitTop(Spacing, 0, &BasicLeft);
		BasicLeft.HSplitTop(ButtonHeight, &Button, &BasicLeft);
		static int s_ButtonDmgInd = 0;
		if (DoButton_CheckBox(&s_ButtonDmgInd, Localize("Vanila Damage Ind (Vanilla)"), g_Config.m_ClMmoDamageInd, &Button))
			g_Config.m_ClMmoDamageInd ^= 1;

		// правая сторона
		// show colored
		BasicRight.HSplitTop(Spacing, 0, &BasicRight);
		BasicRight.HSplitTop(ButtonHeight, &Button, &BasicRight);
		static int s_ButtonColorVote = 0;
		if (DoButton_CheckBox(&s_ButtonColorVote, Localize("Show Colored Vote (Mmo Server)"), g_Config.m_ClShowColoreVote, &Button))
			g_Config.m_ClShowColoreVote ^= 1;
	}

	// эффекты настройки
	else if (Page == 2)
	{
		MainView.HSplitTop(ButtonHeight, &Button, &MainView);
		UI()->DoLabel(&Button, "General effects", 14.0f, CUI::ALIGN_CENTER);

		// buttons
		MainView.HSplitTop(ButtonHeight, &Button, &MainView);
		Button.VMargin(ButtonHeight, &Button);

		const char* Name[4] = { "Effects: All", "Effects: Enchant", "Effects: Items", "Effects: Disable" };
		DoScrollbarOption(&g_Config.m_ClShowMEffects, &g_Config.m_ClShowMEffects, &Button, Name[g_Config.m_ClShowMEffects], 0, 3);
	}
}

void CMenus::RenderMmoSettingsTexture(CUIRect MainView, CUIRect Background)
{
	CUIRect Button, TabBar;

	{ // меню выбора скинов
		MainView.HSplitTop(20.0f, &TabBar, &MainView);

		TabBar.VSplitLeft(TabBar.w / 6, &Button, &TabBar);
		static CButtonContainer s_ButtonGameSkins;
		if (DoButton_MenuTab(&s_ButtonGameSkins, Localize("Gameskin"), g_Config.m_Texture == 0, &Button, 0))
			g_Config.m_Texture = 0;

		TabBar.VSplitLeft(TabBar.w / 5, &Button, &TabBar);
		static CButtonContainer s_ButtonEmoticons;
		if (DoButton_MenuTab(&s_ButtonEmoticons, Localize("Emoticons"), g_Config.m_Texture == 1, &Button, 0))
			g_Config.m_Texture = 1;

		TabBar.VSplitLeft(TabBar.w / 4, &Button, &TabBar);
		static CButtonContainer s_ButtonCursors;
		if (DoButton_MenuTab(&s_ButtonCursors, Localize("Cursor"), g_Config.m_Texture == 2, &Button, 0))
			g_Config.m_Texture = 2;

		TabBar.VSplitLeft(TabBar.w / 3, &Button, &TabBar);
		static CButtonContainer s_ButtonParticles;
		if (DoButton_MenuTab(&s_ButtonParticles, Localize("Particles"), g_Config.m_Texture == 3, &Button, 0))
			g_Config.m_Texture = 3;

		TabBar.VSplitLeft(TabBar.w / 2, &Button, &TabBar);
		static CButtonContainer s_ButtonEntities;
		if (DoButton_MenuTab(&s_ButtonEntities, Localize("Entities"), g_Config.m_Texture == 4, &Button, 0))
			g_Config.m_Texture = 4;

		static CButtonContainer s_ButtonFonts;
		if (DoButton_MenuTab(&s_ButtonFonts, Localize("Fonts"), g_Config.m_Texture == 5, &TabBar, 0))
			g_Config.m_Texture = 5;
	}

	// замена скинов game.png
	if (g_Config.m_Texture == 0)
	{
		// инициализация
		if (!m_pClient->m_pGameSkins->IsLoading())
			m_pClient->m_pGameSkins->IntitilizeSelectSkin();

		// получение и вывод списков
		static sorted_array<const CGameSkins::CGameSkin*> s_GameSkinList;
		static bool m_RefreshSkinSelector = true;
		if (m_RefreshSkinSelector)
		{
			s_GameSkinList.clear();
			for (int i = 0; i < m_pClient->m_pGameSkins->Num(); ++i)
			{
				const CGameSkins::CGameSkin* s = m_pClient->m_pGameSkins->Get(i);
				s_GameSkinList.add(s);
			}
			m_RefreshSkinSelector = false;
		}

		int OldSelected = -1;
		static CListBoxState s_ListBoxState;
		UiDoListboxHeader(&s_ListBoxState, &MainView, Localize("Game"), 20.0f, 2.0f);
		UiDoListboxStart(&s_ListBoxState, &m_RefreshSkinSelector, 160.0f, 0, s_GameSkinList.size(), 3, OldSelected);
		for (int i = 0; i < s_GameSkinList.size(); ++i)
		{
			// если текстура равна 0 пропускаем
			const CGameSkins::CGameSkin* s = s_GameSkinList[i];
			if (s == 0) 
				continue;

			// если текстура равна выбранной текстуре то устанавливаем выбор на ней
			if (str_comp(s->m_aName, g_Config.m_GameTexture) == 0)
				OldSelected = i;

			// добавляем в бокс новый предмет
			CListboxItem Item = UiDoListboxNextItem(&s_ListBoxState, &s_GameSkinList[i], OldSelected == i);
			if (Item.m_Visible)
			{
				CUIRect Label;
				Item.m_Rect.Margin(5.0f, &Item.m_Rect);
				Item.m_Rect.HSplitBottom(5.0f, &Item.m_Rect, &Label);
				Item.m_Rect.HSplitTop(5.0f, 0, &Item.m_Rect); // some margin from the top
				UI()->DoLabel(&Item.m_Rect, s->m_aName, 10.0f, CUI::ALIGN_LEFT);

				Graphics()->TextureSet(s_GameSkinList[i]->m_Texture);
				Graphics()->QuadsBegin();
				IGraphics::CQuadItem QuadItem(Item.m_Rect.x + Item.m_Rect.w / 2 - 120.0f, Item.m_Rect.y + Item.m_Rect.h / 2 - 60.0f, 240.0f, 120.0f);
				Graphics()->QuadsDrawTL(&QuadItem, 1);
				Graphics()->QuadsEnd();
			}
		}

		// показываем новый выбор
		const int NewSelected = UiDoListboxEnd(&s_ListBoxState, 0);
		if (NewSelected != -1 && NewSelected != OldSelected)
		{
			// устанавливаем новый скин
			mem_copy(g_Config.m_GameTexture, s_GameSkinList[NewSelected]->m_aName, sizeof(g_Config.m_GameTexture));
			g_pData->m_aImages[IMAGE_GAME].m_Id = s_GameSkinList[NewSelected]->m_Texture;

			// старый скин ставим под новый
			OldSelected = NewSelected;
		}
	}

	// замена скинов emoticion
	else if (g_Config.m_Texture == 1)
	{
		// инициализация
		if (!m_pClient->m_pEmoticonsSkins->IsLoading())
			m_pClient->m_pEmoticonsSkins->IntitilizeSelectSkin();

		// продолжение 
		static sorted_array<const CEmoticonsSkins::CEmoticonsSkin*> s_EmoticionSkinList;
		static bool m_RefreshSkinSelector = true;
		if (m_RefreshSkinSelector)
		{
			s_EmoticionSkinList.clear();
			for (int i = 0; i < m_pClient->m_pEmoticonsSkins->Num(); ++i)
			{
				const CEmoticonsSkins::CEmoticonsSkin* s = m_pClient->m_pEmoticonsSkins->Get(i);
				s_EmoticionSkinList.add(s);
			}
			m_RefreshSkinSelector = false;
		}

		int OldSelected = -1;
		static CListBoxState s_ListBoxState;
		UiDoListboxHeader(&s_ListBoxState, &MainView, Localize("Emoticions"), 20.0f, 2.0f);
		UiDoListboxStart(&s_ListBoxState, &m_RefreshSkinSelector, 160.0f, 0, s_EmoticionSkinList.size(), 3, OldSelected);

		for (int i = 0; i < s_EmoticionSkinList.size(); ++i)
		{
			// если текстура равна 0 пропускаем
			const CEmoticonsSkins::CEmoticonsSkin* s = s_EmoticionSkinList[i];
			if (s == 0) 
				continue;

			// если текстура равна выбранной текстуре то устанавливаем выбор на ней
			if (str_comp(s->m_aName, g_Config.m_GameEmoticons) == 0)
				OldSelected = i;

			// добавляем в бокс новый предмет
			CListboxItem Item = UiDoListboxNextItem(&s_ListBoxState, &s_EmoticionSkinList[i], OldSelected == i);
			if (Item.m_Visible)
			{
				CUIRect Label;
				Item.m_Rect.Margin(5.0f, &Item.m_Rect);
				Item.m_Rect.HSplitBottom(5.0f, &Item.m_Rect, &Label);
				Item.m_Rect.HSplitTop(5.0f, 0, &Item.m_Rect); // some margin from the top
				UI()->DoLabel(&Item.m_Rect, s->m_aName, 10.0f, CUI::ALIGN_LEFT);

				Graphics()->TextureSet(s_EmoticionSkinList[i]->m_Texture);
				Graphics()->QuadsBegin();
				IGraphics::CQuadItem QuadItem(Item.m_Rect.x + Item.m_Rect.w / 2 - 60.0f, Item.m_Rect.y + Item.m_Rect.h / 2 - 60.0f, 120.0f, 120.0f);
				Graphics()->QuadsDrawTL(&QuadItem, 1);
				Graphics()->QuadsEnd();
			}
		}

		// показываем новый выбор
		const int NewSelected = UiDoListboxEnd(&s_ListBoxState, 0);
		if (NewSelected != -1 && NewSelected != OldSelected)
		{
			// устанавливаем новый скин
			mem_copy(g_Config.m_GameEmoticons, s_EmoticionSkinList[NewSelected]->m_aName, sizeof(g_Config.m_GameEmoticons));
			g_pData->m_aImages[IMAGE_EMOTICONS].m_Id = s_EmoticionSkinList[NewSelected]->m_Texture;

			// старый скин ставим под новый
			OldSelected = NewSelected;
		}
	}

	// замена скинов cursors
	else if (g_Config.m_Texture == 2)
	{
		// инициализация
		if (!m_pClient->m_pCursorsSkins->IsLoading())
			m_pClient->m_pCursorsSkins->IntitilizeSelectSkin();

		// продолжение
		static sorted_array<const CCursorsSkins::CCursorsSkin*> s_paSkinList;
		static CListBoxState s_ListBoxState;
		static bool m_RefreshSkinSelector = true;
		if (m_RefreshSkinSelector)
		{
			s_paSkinList.clear();
			for (int i = 0; i < m_pClient->m_pCursorsSkins->Num(); ++i)
			{
				const CCursorsSkins::CCursorsSkin* s = m_pClient->m_pCursorsSkins->Get(i);
				s_paSkinList.add(s);
			}
			m_RefreshSkinSelector = false;
		}

		int OldSelected = -1;
		UiDoListboxHeader(&s_ListBoxState, &MainView, Localize("Cursors"), 20.0f, 2.0f);
		UiDoListboxStart(&s_ListBoxState, &m_RefreshSkinSelector, 160.0f, 0, s_paSkinList.size(), 3, OldSelected);

		for (int i = 0; i < s_paSkinList.size(); ++i)
		{
			// если текстура равна 0 пропускаем
			const CCursorsSkins::CCursorsSkin* s = s_paSkinList[i];
			if (s == 0) 
				continue;

			// если текстура равна выбранной текстуре то устанавливаем выбор на ней
			if (str_comp(s->m_aName, g_Config.m_GameCursor) == 0)
				OldSelected = i;

			// добавляем в бокс новый предмет
			CListboxItem Item = UiDoListboxNextItem(&s_ListBoxState, &s_paSkinList[i], OldSelected == i);
			if (Item.m_Visible)
			{
				CUIRect Label;
				Item.m_Rect.Margin(5.0f, &Item.m_Rect);
				Item.m_Rect.HSplitBottom(5.0f, &Item.m_Rect, &Label);
				Item.m_Rect.HSplitTop(5.0f, 0, &Item.m_Rect); // some margin from the top
				UI()->DoLabel(&Item.m_Rect, s->m_aName, 10.0f, CUI::ALIGN_LEFT);

				Graphics()->TextureSet(s_paSkinList[i]->m_Texture);
				Graphics()->QuadsBegin();
				IGraphics::CQuadItem QuadItem(Item.m_Rect.x + Item.m_Rect.w / 2 - 60.0f, Item.m_Rect.y + Item.m_Rect.h / 2 - 60.0f, 120.0f, 120.0f);
				Graphics()->QuadsDrawTL(&QuadItem, 1);
				Graphics()->QuadsEnd();
			}
		}

		// показываем новый выбор
		const int NewSelected = UiDoListboxEnd(&s_ListBoxState, 0);
		if (NewSelected != -1 && NewSelected != OldSelected)
		{
			// устанавливаем новый скин
			mem_copy(g_Config.m_GameCursor, s_paSkinList[NewSelected]->m_aName, sizeof(g_Config.m_GameCursor));
			g_pData->m_aImages[IMAGE_CURSOR].m_Id = s_paSkinList[NewSelected]->m_Texture;

			// старый скин ставим под новый
			OldSelected = NewSelected;
		}
	}

	// замена скинов particles
	else if (g_Config.m_Texture == 3)
	{
		// инициализация
		if(!m_pClient->m_pParticlesSkins->IsLoading())
			m_pClient->m_pParticlesSkins->IntitilizeSelectSkin();

		// загружаем список статично чтобы не грузить постоянно его по 20 раз
		static sorted_array<const CParticlesSkins::CParticlesSkin*> s_ParticlesSkinList;
		static bool m_RefreshSkinSelector = true;
		if (m_RefreshSkinSelector)
		{
			s_ParticlesSkinList.clear();
			for (int i = 0; i < m_pClient->m_pParticlesSkins->Num(); ++i)
			{
				const CParticlesSkins::CParticlesSkin* s = m_pClient->m_pParticlesSkins->Get(i);
				s_ParticlesSkinList.add(s);
			}
			m_RefreshSkinSelector = false;
		}

		int OldSelected = -1;
		static CListBoxState s_ListBoxState;
		UiDoListboxHeader(&s_ListBoxState, &MainView, Localize("Particles"), 20.0f, 2.0f);
		UiDoListboxStart(&s_ListBoxState, &m_RefreshSkinSelector, 160.0f, 0, s_ParticlesSkinList.size(), 3, OldSelected);

		for (int i = 0; i < s_ParticlesSkinList.size(); ++i)
		{
			// если текстура равна 0 пропускаем
			const CParticlesSkins::CParticlesSkin* s = s_ParticlesSkinList[i];
			if (s == 0) 
				continue;

			// если текстура равна выбранной текстуре то устанавливаем выбор на ней
			if (str_comp(s->m_aName, g_Config.m_GameParticles) == 0)
				OldSelected = i;

			// добавляем в бокс новый предмет
			CListboxItem Item = UiDoListboxNextItem(&s_ListBoxState, &s_ParticlesSkinList[i], OldSelected == i);
			if (Item.m_Visible)
			{
				CUIRect Label;
				Item.m_Rect.Margin(5.0f, &Item.m_Rect);
				Item.m_Rect.HSplitBottom(5.0f, &Item.m_Rect, &Label);
				Item.m_Rect.HSplitTop(5.0f, 0, &Item.m_Rect); // some margin from the top
				UI()->DoLabel(&Item.m_Rect, s->m_aName, 10.0f, CUI::ALIGN_LEFT);

				Graphics()->TextureSet(s_ParticlesSkinList[i]->m_Texture);
				Graphics()->QuadsBegin();
				IGraphics::CQuadItem QuadItem(Item.m_Rect.x + Item.m_Rect.w / 2 - 60.0f, Item.m_Rect.y + Item.m_Rect.h / 2 - 60.0f, 120.0f, 120.0f);
				Graphics()->QuadsDrawTL(&QuadItem, 1);
				Graphics()->QuadsEnd();
			}
		}

		// показываем новый выбор
		const int NewSelected = UiDoListboxEnd(&s_ListBoxState, 0);
		if (NewSelected != -1 && NewSelected != OldSelected)
		{
			// устанавливаем новый скин
			mem_copy(g_Config.m_GameParticles, s_ParticlesSkinList[NewSelected]->m_aName, sizeof(g_Config.m_GameParticles));
			g_pData->m_aImages[IMAGE_PARTICLES].m_Id = s_ParticlesSkinList[NewSelected]->m_Texture;

			// старый скин ставим под новый
			OldSelected = NewSelected;
		}
	}

	// замена скинов entities
	else if (g_Config.m_Texture == 4)
	{
		// инициализация
		if(!m_pClient->m_pEntitiesSkins->IsLoading())
			m_pClient->m_pEntitiesSkins->IntitilizeSelectSkin();

		static sorted_array<const CEntitiesSkins::CEntitiesSkin*> s_EntitiesSkinList;
		static CListBoxState s_ListBoxState;
		static bool m_RefreshSkinSelector = true;
		if (m_RefreshSkinSelector)
		{
			s_EntitiesSkinList.clear();
			for (int i = 0; i < m_pClient->m_pEntitiesSkins->Num(); ++i)
			{
				const CEntitiesSkins::CEntitiesSkin* s = m_pClient->m_pEntitiesSkins->Get(i);
				s_EntitiesSkinList.add(s);
			}
			m_RefreshSkinSelector = false;
		}

		int OldSelected = -1;
		UiDoListboxHeader(&s_ListBoxState, &MainView, Localize("Entities"), 20.0f, 2.0f);
		UiDoListboxStart(&s_ListBoxState, &m_RefreshSkinSelector, 160.0f, 0, s_EntitiesSkinList.size(), 3, OldSelected);

		for (int i = 0; i < s_EntitiesSkinList.size(); ++i)
		{
			// если текстура равна 0 пропускаем
			const CEntitiesSkins::CEntitiesSkin* s = s_EntitiesSkinList[i];
			if (s == 0)
				continue;

			// если текстура равна выбранной текстуре то устанавливаем выбор на ней
			if (str_comp(s->m_aName, g_Config.m_GameEntities) == 0)
				OldSelected = i;

			// добавляем в бокс новый предмет
			CListboxItem Item = UiDoListboxNextItem(&s_ListBoxState, &s_EntitiesSkinList[i], OldSelected == i);
			if (Item.m_Visible)
			{
				CUIRect Label;
				Item.m_Rect.Margin(5.0f, &Item.m_Rect);
				Item.m_Rect.HSplitBottom(5.0f, &Item.m_Rect, &Label);
				Item.m_Rect.HSplitTop(5.0f, 0, &Item.m_Rect); // some margin from the top
				UI()->DoLabel(&Item.m_Rect, s->m_aName, 10.0f, CUI::ALIGN_LEFT);

				Graphics()->TextureSet(s_EntitiesSkinList[i]->m_Texture);
				Graphics()->QuadsBegin();
				IGraphics::CQuadItem QuadItem(Item.m_Rect.x + Item.m_Rect.w / 2 - 60.0f, Item.m_Rect.y + Item.m_Rect.h / 2 - 60.0f, 120.0f, 120.0f);
				Graphics()->QuadsDrawTL(&QuadItem, 1);
				Graphics()->QuadsEnd();
			}
		}

		// показываем новый выбор
		const int NewSelected = UiDoListboxEnd(&s_ListBoxState, 0);
		if(NewSelected != -1 && NewSelected != OldSelected)
		{
			// устанавливаем новый скин
			mem_copy(g_Config.m_GameEntities, s_EntitiesSkinList[NewSelected]->m_aName, sizeof(g_Config.m_GameEntities));
	
			// старый скин ставим под новый
			OldSelected = NewSelected;
		}
	}
	else if (g_Config.m_Texture == 5)
	{
		RenderFontSelection(MainView);
	}
}

class CFontFile
{
public:
	CFontFile() {}
	CFontFile(const char *n, const char *f) : m_Name(n), m_FileName(f) {}

	string m_Name;
	string m_FileName;
	bool operator<(const CFontFile &Other) { return m_Name < Other.m_Name; }
};

int GatherFonts(const char *pFileName, int IsDir, int Type, void *pUser)
{
	const char *pSuffix = str_endswith(pFileName, ".ttf");
	if (IsDir || !pSuffix) return 0;

	sorted_array<CFontFile> &Fonts = *((sorted_array<CFontFile> *)pUser);
	char aFontName[128];
	str_truncate(aFontName, sizeof(aFontName), pFileName, pSuffix - pFileName);
	for (int i = 0; i < Fonts.size(); i++)
	{
		if (!str_comp(Fonts[i].m_Name, aFontName))
			return 0;
	}
	Fonts.add(CFontFile(aFontName, pFileName));
	return 0;
}

void CMenus::RenderFontSelection(CUIRect MainView)
{
	static CListBoxState s_FontList;
	static int s_SelectedFont = 0;
	static sorted_array<CFontFile> s_Fonts;

	if (s_Fonts.size() == 0)
	{
		Storage()->ListDirectory(IStorage::TYPE_ALL, "fonts", GatherFonts, &s_Fonts);
		for (int i = 0; i < s_Fonts.size(); i++)
		{
			if (str_comp(s_Fonts[i].m_FileName, g_Config.m_ClFontfile) == 0)
			{
				s_SelectedFont = i;
				break;
			}
		}
	}

	int OldSelectedFont = s_SelectedFont;
	static float s_Fade[2] = { 0 };
	UiDoListboxHeader(&s_FontList, &MainView, Localize(""), 20.0f, 2.0f);
	UiDoListboxStart(&s_FontList, &s_Fade[0], 20.0f, Localize("Fonts"), s_Fonts.size(), 1, s_SelectedFont);
	for (sorted_array<CFontFile>::range r = s_Fonts.all(); !r.empty(); r.pop_front())
	{
		CListboxItem Item = UiDoListboxNextItem(&s_FontList, &r.front());
		if (Item.m_Visible)
		{
			Item.m_Rect.VMargin(5.0f, &Item.m_Rect);
			Item.m_Rect.y += 2.0f;
			UI()->DoLabel(&Item.m_Rect, r.front().m_Name, Item.m_Rect.h*ms_FontmodHeight*0.8f, CUI::ALIGN_LEFT);
		}
	}

	s_SelectedFont = UiDoListboxEnd(&s_FontList, 0);
	if (OldSelectedFont != s_SelectedFont)
	{
		str_copy(g_Config.m_ClFontfile, s_Fonts[s_SelectedFont].m_FileName, sizeof(g_Config.m_ClFontfile));
		char aRelFontPath[512];

		str_format(aRelFontPath, sizeof(aRelFontPath), "fonts/%s", g_Config.m_ClFontfile);
		char aFontPath[512];

		IOHANDLE File = Storage()->OpenFile(aRelFontPath, IOFLAG_READ, IStorage::TYPE_ALL, aFontPath, sizeof(aFontPath));
		if (File)
			io_close(File);

		TextRender()->SetDefaultFont(TextRender()->LoadFont(aFontPath));
	}
}