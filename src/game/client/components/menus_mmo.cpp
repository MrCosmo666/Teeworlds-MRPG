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
	CUIRect Label, Button, Tabbar, BottomView;

	static int s_SettingsPage = 0;
	MainView.HSplitBottom(80.0f, &MainView, &BottomView);
	MainView.HSplitTop(18.0f, &Tabbar, &MainView);

	const char* Tabs[4] = { "Visual", "General", "Effects", "Credits" };
	const char* Information[4] = { "Setting up the visual part of the client", "Setting up the general part of the client", "Setting up the effects part of the client", "Information & Credits" };
	for (int i = 0; i < 4; i++)
	{
		Tabbar.VSplitLeft(182.5f, &Button, &Tabbar);

		static CButtonContainer s_Buttons[4] = { 0 };
		if (DoButton_MenuTabTop(&s_Buttons[i], Tabs[i], Client()->State() == IClient::STATE_OFFLINE && s_SettingsPage == i, &Button,
			s_SettingsPage == i ? 1.0f : 1.5f, 1.0f, 0))
			s_SettingsPage = i;
	}

	MainView.HSplitTop(-10.0f, &Label, &MainView);

	if (s_SettingsPage == 0)
		RenderSettingsMmoGeneral(MainView, 0);
	else if (s_SettingsPage == 1)
		RenderSettingsMmoGeneral(MainView, 1);
	else if (s_SettingsPage == 2)
		RenderSettingsMmoGeneral(MainView, 2);
	else if (s_SettingsPage == 3)
		RenderSettingsMmoGeneral(MainView, 3);

	Label.y += 1.0f;
	UI()->DoLabel(&Label, Tabs[s_SettingsPage], 20.0f, CUI::ALIGN_CENTER);
	Label.y += 16.0f;
	UI()->DoLabel(&Label, Information[s_SettingsPage], 14.0f, CUI::ALIGN_CENTER);
}

void CMenus::RenderSettingsMmoGeneral(CUIRect MainView, int Page)
{
	CUIRect Button, MmoSet, BottomView, Background, MainSave = MainView;

	// render MmoSet menu background
	float ButtonHeight = 20.0f;
	float Spacing = 2.0f;

	// visual
	if (Page == 0)
	{
		int NumOptions = 3;
		float BackgroundHeight = (float)(NumOptions + 3) * ButtonHeight + (float)NumOptions * Spacing;
		float TotalHeight = BackgroundHeight;

		// background
		MainView.HSplitBottom(MainView.h - TotalHeight + 40.0f, &MainView, &BottomView);
		MainView.HSplitTop(10.0f, 0, &Background);
		MainView.HSplitTop(10.0f, 0, &MainView);
		MainView.HSplitTop(BackgroundHeight, &MmoSet, &MainView);

		RenderMmoSettingsTexture(MainSave, Background);
	}

	if (Page == 1)
	{
		int NumOptions = 4;
		float BackgroundHeight = (float)(NumOptions + 3) * ButtonHeight + (float)NumOptions * Spacing;
		float TotalHeight = BackgroundHeight;
		
		// background
		MainView.HSplitBottom(MainView.h - TotalHeight + 40.0f, &MainView, &BottomView);
		MainView.HSplitTop(10.0f, 0, &Background);
		RenderTools()->DrawUIRect(&Background, vec4(0.0f, 0.0f, 0.0f, 0.8f), CUI::ALIGN_CENTER, 0.0f);
		MainView.HSplitTop(10.0f, 0, &MainView);
		MainView.HSplitTop(BackgroundHeight, &MmoSet, &MainView);

		// buttons
		MmoSet.HSplitTop(40.0f, 0, &MmoSet);
		MmoSet.HSplitTop(ButtonHeight, &Button, &MmoSet);
		Button.VMargin(ButtonHeight, &Button);

		static int s_ButtonDmgInd = 0;
		if (DoButton_CheckBox(&s_ButtonDmgInd, Localize("Vanila Damage Ind (Vanilla)"), g_Config.m_ClMmoDamageInd, &Button))
			g_Config.m_ClMmoDamageInd ^= 1;

		MmoSet.HSplitTop(ButtonHeight, &Button, &MmoSet);
		Button.VMargin(ButtonHeight, &Button);
		static int s_ButtonColorVote = 0;
		if (DoButton_CheckBox(&s_ButtonColorVote, Localize("Show Colored Vote (Mmo Server)"), g_Config.m_ClShowColoreVote, &Button))
			g_Config.m_ClShowColoreVote ^= 1;
	}

	if (Page == 2)
	{
		int NumOptions = 3;
		float BackgroundHeight = (float)(NumOptions + 3) * ButtonHeight + (float)NumOptions * Spacing;
		float TotalHeight = BackgroundHeight;

		// background
		MainView.HSplitBottom(MainView.h - TotalHeight + 40.0f, &MainView, &BottomView);
		MainView.HSplitTop(10.0f, 0, &Background);
		RenderTools()->DrawUIRect(&Background, vec4(0.0f, 0.0f, 0.0f, 0.8f), CUI::ALIGN_CENTER, 0.0f);
		MainView.HSplitTop(10.0f, 0, &MainView);
		MainView.HSplitTop(BackgroundHeight, &MmoSet, &MainView);

		// buttons
		MmoSet.HSplitTop(40.0f, 0, &MmoSet);
		MmoSet.HSplitTop(ButtonHeight, &Button, &MmoSet);
		Button.VMargin(ButtonHeight, &Button);

		const char* Name[4] = { "Effects: All", "Effects: Enchant", "Effects: Items", "Effects: Disable" };
		DoScrollbarOption(&g_Config.m_ClShowMEffects, &g_Config.m_ClShowMEffects, &Button, Name[g_Config.m_ClShowMEffects], 0, 3);
	}
}

void CMenus::RenderMmoSettingsTexture(CUIRect MainView, CUIRect Background)
{
	CUIRect Button, TabBar;

	// render page
	static int s_ControlPage = 0;
	if (s_ControlPage == 0) g_Config.m_Texture = 0;
	else if (s_ControlPage == 1) g_Config.m_Texture = 1;
	else if (s_ControlPage == 2) g_Config.m_Texture = 2;
	else if (s_ControlPage == 3) g_Config.m_Texture = 3;
	else if (s_ControlPage == 4) g_Config.m_Texture = 4;
	else if (s_ControlPage == 5) g_Config.m_Texture = 5;

	// render game menu backgrounds
	MainView.HSplitTop(50.0f, 0, &MainView);
	MainView.HSplitBottom(80.0f, &MainView, 0);
	MainView.HSplitTop(20.0f, &TabBar, &MainView);
	MainView.Margin(10.0f, &MainView);
	RenderTools()->DrawUIRect(&Background, vec4(0.0f, 0.0f, 0.0f, 0.8f), CUI::ALIGN_CENTER, 0.0f);

	// tab bar
	{
		TabBar.VSplitLeft(TabBar.w / 6, &Button, &TabBar);
		static CButtonContainer s_Button0;
		if (DoButton_MenuTab(&s_Button0, Localize("Gameskin"), s_ControlPage == 0, &Button, 0))
			s_ControlPage = 0;

		TabBar.VSplitLeft(TabBar.w / 5, &Button, &TabBar);
		static CButtonContainer s_Button1;
		if (DoButton_MenuTab(&s_Button1, Localize("Emoticons"), s_ControlPage == 1, &Button, 0))
			s_ControlPage = 1;

		TabBar.VSplitLeft(TabBar.w / 4, &Button, &TabBar);
		static CButtonContainer s_Button2;
		if (DoButton_MenuTab(&s_Button2, Localize("Cursor"), s_ControlPage == 2, &Button, 0))
			s_ControlPage = 2;

		TabBar.VSplitLeft(TabBar.w / 3, &Button, &TabBar);
		static CButtonContainer s_Button3;
		if (DoButton_MenuTab(&s_Button3, Localize("Particles"), s_ControlPage == 3, &Button, 0))
			s_ControlPage = 3;

		TabBar.VSplitLeft(TabBar.w / 2, &Button, &TabBar);
		static CButtonContainer s_Button4;
		if (DoButton_MenuTab(&s_Button4, Localize("Entities"), s_ControlPage == 4, &Button, 0))
			s_ControlPage = 4;

		static CButtonContainer s_Button5;
		if (DoButton_MenuTab(&s_Button5, Localize("Fonts"), s_ControlPage == 5, &TabBar, 0))
			s_ControlPage = 5;
	}

	if (g_Config.m_Texture == 0)
	{
		static sorted_array<const CgSkins::CgSkin*> s_paSkinList;
		static CListBoxState s_ListBoxState;
		static bool m_RefreshSkinSelector = true;
		if (m_RefreshSkinSelector)
		{
			s_paSkinList.clear();
			for (int i = 0; i < m_pClient->m_pgSkins->Num(); ++i)
			{
				const CgSkins::CgSkin* s = m_pClient->m_pgSkins->Get(i);
				// no special skins
				if (s->m_aName[0] == 'x' && s->m_aName[1] == '_')
					continue;

				s_paSkinList.add(s);
			}
			m_RefreshSkinSelector = false;
		}

		m_pSelectedSkin = 0;
		int OldSelected = -1;
		UiDoListboxHeader(&s_ListBoxState, &MainView, Localize(""), 20.0f, 2.0f);
		UiDoListboxStart(&s_ListBoxState, &m_RefreshSkinSelector, 160.0f, 0, s_paSkinList.size(), 3, OldSelected);

		for (int i = 0; i < s_paSkinList.size(); ++i)
		{
			const CgSkins::CgSkin* s = s_paSkinList[i];
			if (s == 0)
				continue;
			if (str_comp(s->m_aName, g_Config.m_GameTexture) == 0)
				OldSelected = i;

			CListboxItem Item = UiDoListboxNextItem(&s_ListBoxState, &s_paSkinList[i], OldSelected == i);
			if (Item.m_Visible)
			{
				CUIRect Label;
				Item.m_Rect.Margin(5.0f, &Item.m_Rect);
				Item.m_Rect.HSplitBottom(10.0f, &Item.m_Rect, &Label);
				Item.m_Rect.HSplitTop(5.0f, 0, &Item.m_Rect); // some margin from the top

				Graphics()->TextureSet(s_paSkinList[i]->m_Texture);
				Graphics()->QuadsBegin();
				IGraphics::CQuadItem QuadItem(Item.m_Rect.x + Item.m_Rect.w / 2 - 120.0f, Item.m_Rect.y + Item.m_Rect.h / 2 - 60.0f, 240.0f, 120.0f);
				Graphics()->QuadsDrawTL(&QuadItem, 1);
				Graphics()->QuadsEnd();
			}
		}

		const int NewSelected = UiDoListboxEnd(&s_ListBoxState, 0);
		if (NewSelected != -1)
		{
			if (NewSelected != OldSelected)
			{
				mem_copy(g_Config.m_GameTexture, s_paSkinList[NewSelected]->m_aName, sizeof(g_Config.m_GameTexture));
				g_pData->m_aImages[IMAGE_GAME].m_Id = s_paSkinList[NewSelected]->m_Texture;
			}
		}
		OldSelected = NewSelected;
	}

	else if (g_Config.m_Texture == 1)
	{
		static sorted_array<const CeSkins::CeSkin*> s_paSkinList;
		static CListBoxState s_ListBoxState;
		static bool m_RefreshSkinSelector = true;
		if (m_RefreshSkinSelector)
		{
			s_paSkinList.clear();
			for (int i = 0; i < m_pClient->m_peSkins->Num(); ++i)
			{
				const CeSkins::CeSkin* s = m_pClient->m_peSkins->Get(i);
				// no special skins
				if (s->m_aName[0] == 'x' && s->m_aName[1] == '_')
					continue;

				s_paSkinList.add(s);
			}
			m_RefreshSkinSelector = false;
		}

		m_pSelectedSkin = 0;
		int OldSelected = -1;
		UiDoListboxHeader(&s_ListBoxState, &MainView, Localize(""), 20.0f, 2.0f);
		UiDoListboxStart(&s_ListBoxState, &m_RefreshSkinSelector, 160.0f, 0, s_paSkinList.size(), 3, OldSelected);

		for (int i = 0; i < s_paSkinList.size(); ++i)
		{
			const CeSkins::CeSkin* s = s_paSkinList[i];
			if (s == 0)
				continue;
			if (str_comp(s->m_aName, g_Config.m_GameEmoticons) == 0)
				OldSelected = i;

			CListboxItem Item = UiDoListboxNextItem(&s_ListBoxState, &s_paSkinList[i], OldSelected == i);
			if (Item.m_Visible)
			{
				CUIRect Label;
				Item.m_Rect.Margin(5.0f, &Item.m_Rect);
				Item.m_Rect.HSplitBottom(10.0f, &Item.m_Rect, &Label);
				Item.m_Rect.HSplitTop(5.0f, 0, &Item.m_Rect); // some margin from the top

				Graphics()->TextureSet(s_paSkinList[i]->m_Texture);
				Graphics()->QuadsBegin();
				IGraphics::CQuadItem QuadItem(Item.m_Rect.x + Item.m_Rect.w / 2 - 60.0f, Item.m_Rect.y + Item.m_Rect.h / 2 - 60.0f, 120.0f, 120.0f);
				Graphics()->QuadsDrawTL(&QuadItem, 1);
				Graphics()->QuadsEnd();
			}
		}

		const int NewSelected = UiDoListboxEnd(&s_ListBoxState, 0);
		if (NewSelected != -1)
		{
			if (NewSelected != OldSelected)
			{
				mem_copy(g_Config.m_GameEmoticons, s_paSkinList[NewSelected]->m_aName, sizeof(g_Config.m_GameEmoticons));
				g_pData->m_aImages[IMAGE_EMOTICONS].m_Id = s_paSkinList[NewSelected]->m_Texture;
			}
		}
		OldSelected = NewSelected;
	}

	else if (g_Config.m_Texture == 2)
	{
		static sorted_array<const CcSkins::CcSkin*> s_paSkinList;
		static CListBoxState s_ListBoxState;
		static bool m_RefreshSkinSelector = true;
		if (m_RefreshSkinSelector)
		{
			s_paSkinList.clear();
			for (int i = 0; i < m_pClient->m_pcSkins->Num(); ++i)
			{
				const CcSkins::CcSkin* s = m_pClient->m_pcSkins->Get(i);
				// no special skins
				if (s->m_aName[0] == 'x' && s->m_aName[1] == '_')
					continue;

				s_paSkinList.add(s);
			}
			m_RefreshSkinSelector = false;
		}

		m_pSelectedSkin = 0;
		int OldSelected = -1;
		UiDoListboxHeader(&s_ListBoxState, &MainView, Localize(""), 20.0f, 2.0f);
		UiDoListboxStart(&s_ListBoxState, &m_RefreshSkinSelector, 160.0f, 0, s_paSkinList.size(), 3, OldSelected);

		for (int i = 0; i < s_paSkinList.size(); ++i)
		{
			const CcSkins::CcSkin* s = s_paSkinList[i];
			if (s == 0)
				continue;
			if (str_comp(s->m_aName, g_Config.m_GameCursor) == 0)
				OldSelected = i;

			CListboxItem Item = UiDoListboxNextItem(&s_ListBoxState, &s_paSkinList[i], OldSelected == i);
			if (Item.m_Visible)
			{
				CUIRect Label;
				Item.m_Rect.Margin(5.0f, &Item.m_Rect);
				Item.m_Rect.HSplitBottom(10.0f, &Item.m_Rect, &Label);
				Item.m_Rect.HSplitTop(5.0f, 0, &Item.m_Rect); // some margin from the top

				Graphics()->TextureSet(s_paSkinList[i]->m_Texture);
				Graphics()->QuadsBegin();
				IGraphics::CQuadItem QuadItem(Item.m_Rect.x + Item.m_Rect.w / 2 - 60.0f, Item.m_Rect.y + Item.m_Rect.h / 2 - 60.0f, 120.0f, 120.0f);
				Graphics()->QuadsDrawTL(&QuadItem, 1);
				Graphics()->QuadsEnd();
			}
		}

		const int NewSelected = UiDoListboxEnd(&s_ListBoxState, 0);
		if (NewSelected != -1)
		{
			if (NewSelected != OldSelected)
			{
				mem_copy(g_Config.m_GameCursor, s_paSkinList[NewSelected]->m_aName, sizeof(g_Config.m_GameCursor));
				g_pData->m_aImages[IMAGE_CURSOR].m_Id = s_paSkinList[NewSelected]->m_Texture;
			}
		}
		OldSelected = NewSelected;
	}

	else if (g_Config.m_Texture == 3)
	{
		static sorted_array<const CpSkins::CpSkin*> s_paSkinList;
		static CListBoxState s_ListBoxState;
		static bool m_RefreshSkinSelector = true;
		if (m_RefreshSkinSelector)
		{
			s_paSkinList.clear();
			for (int i = 0; i < m_pClient->m_ppSkins->Num(); ++i)
			{
				const CpSkins::CpSkin* s = m_pClient->m_ppSkins->Get(i);
				// no special skins
				if (s->m_aName[0] == 'x' && s->m_aName[1] == '_')
					continue;

				s_paSkinList.add(s);
			}
			m_RefreshSkinSelector = false;
		}

		m_pSelectedSkin = 0;
		int OldSelected = -1;
		UiDoListboxHeader(&s_ListBoxState, &MainView, Localize(""), 20.0f, 2.0f);
		UiDoListboxStart(&s_ListBoxState, &m_RefreshSkinSelector, 160.0f, 0, s_paSkinList.size(), 3, OldSelected);

		for (int i = 0; i < s_paSkinList.size(); ++i)
		{
			const CpSkins::CpSkin* s = s_paSkinList[i];
			if (s == 0)
				continue;
			if (str_comp(s->m_aName, g_Config.m_GameParticles) == 0)
				OldSelected = i;

			CListboxItem Item = UiDoListboxNextItem(&s_ListBoxState, &s_paSkinList[i], OldSelected == i);
			if (Item.m_Visible)
			{
				CUIRect Label;
				Item.m_Rect.Margin(5.0f, &Item.m_Rect);
				Item.m_Rect.HSplitBottom(10.0f, &Item.m_Rect, &Label);
				Item.m_Rect.HSplitTop(5.0f, 0, &Item.m_Rect); // some margin from the top

				Graphics()->TextureSet(s_paSkinList[i]->m_Texture);
				Graphics()->QuadsBegin();
				IGraphics::CQuadItem QuadItem(Item.m_Rect.x + Item.m_Rect.w / 2 - 60.0f, Item.m_Rect.y + Item.m_Rect.h / 2 - 60.0f, 120.0f, 120.0f);
				Graphics()->QuadsDrawTL(&QuadItem, 1);
				Graphics()->QuadsEnd();
			}
		}

		const int NewSelected = UiDoListboxEnd(&s_ListBoxState, 0);
		if (NewSelected != -1)
		{
			if (NewSelected != OldSelected)
			{
				mem_copy(g_Config.m_GameParticles, s_paSkinList[NewSelected]->m_aName, sizeof(g_Config.m_GameParticles));
				g_pData->m_aImages[IMAGE_PARTICLES].m_Id = s_paSkinList[NewSelected]->m_Texture;
			}
		}
		OldSelected = NewSelected;
	}

	else if (g_Config.m_Texture == 4)
	{
		static sorted_array<const CEnSkins::CEnSkin*> s_paSkinList;
		static CListBoxState s_ListBoxState;
		static bool m_RefreshSkinSelector = true;
		if (m_RefreshSkinSelector)
		{
			s_paSkinList.clear();
			for (int i = 0; i < m_pClient->m_penSkins->Num(); ++i)
			{
				const CEnSkins::CEnSkin* s = m_pClient->m_penSkins->Get(i);
				// no special skins
				if (s->m_aName[0] == 'x' && s->m_aName[1] == '_')
					continue;

				s_paSkinList.add(s);
			}
			m_RefreshSkinSelector = false;
		}

		m_pSelectedSkin = 0;
		int OldSelected = -1;
		UiDoListboxHeader(&s_ListBoxState, &MainView, Localize(""), 20.0f, 2.0f);
		UiDoListboxStart(&s_ListBoxState, &m_RefreshSkinSelector, 160.0f, 0, s_paSkinList.size(), 3, OldSelected);

		for (int i = 0; i < s_paSkinList.size(); ++i)
		{
			const CEnSkins::CEnSkin* s = s_paSkinList[i];
			if (s == 0)
				continue;
			if (str_comp(s->m_aName, g_Config.m_GameEntities) == 0)
				OldSelected = i;

			CListboxItem Item = UiDoListboxNextItem(&s_ListBoxState, &s_paSkinList[i], OldSelected == i);
			if (Item.m_Visible)
			{
				CUIRect Label;
				Item.m_Rect.Margin(5.0f, &Item.m_Rect);
				Item.m_Rect.HSplitBottom(10.0f, &Item.m_Rect, &Label);
				Item.m_Rect.HSplitTop(5.0f, 0, &Item.m_Rect); // some margin from the top

				Graphics()->TextureSet(s_paSkinList[i]->m_Texture);
				Graphics()->QuadsBegin();
				IGraphics::CQuadItem QuadItem(Item.m_Rect.x + Item.m_Rect.w / 2 - 60.0f, Item.m_Rect.y + Item.m_Rect.h / 2 - 60.0f, 120.0f, 120.0f);
				Graphics()->QuadsDrawTL(&QuadItem, 1);
				Graphics()->QuadsEnd();
			}
		}

		const int NewSelected = UiDoListboxEnd(&s_ListBoxState, 0);
		if (NewSelected != -1)
		{
			if (NewSelected != OldSelected)
				mem_copy(g_Config.m_GameEntities, s_paSkinList[NewSelected]->m_aName, sizeof(g_Config.m_GameEntities));
		}
		OldSelected = NewSelected;
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
	const int PathLength = str_length(pFileName);
	if (IsDir || PathLength <= 4 || pFileName[PathLength - 4] != '.' || str_comp_nocase(pFileName + PathLength - 3, "ttf") || pFileName[0] == '.')
		return 0;

	sorted_array<CFontFile> &Fonts = *((sorted_array<CFontFile> *)pUser);
	char aNiceName[128];
	str_copy(aNiceName, pFileName, PathLength - 3);
	aNiceName[0] = str_uppercase(aNiceName[0]);	// check if the font was already added	

	for (int i = 0; i < Fonts.size(); i++)
		if (!str_comp(Fonts[i].m_Name, aNiceName))
			return 0;

	Fonts.add(CFontFile(aNiceName, pFileName));
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
			if (str_comp(s_Fonts[i].m_FileName, g_Config.m_ClFontfile) == 0)
			{
				s_SelectedFont = i;
				break;
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