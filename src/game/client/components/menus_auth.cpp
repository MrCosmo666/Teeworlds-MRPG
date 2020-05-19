/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#include <base/color.h>
#include <engine/graphics.h>
#include <engine/textrender.h>
#include <engine/shared/config.h>

#include <generated/client_data.h>

#include <game/version.h>
#include <game/client/render.h>
#include <game/client/ui.h>
#include "menus.h"

#include "scoreboard.h"

void CMenus::OnAuthMessage(int MsgType, void* pRawMsg)
{
	if(MsgType == NETMSGTYPE_SV_CLIENTPROGRESSAUTH)
	{
		CNetMsg_Sv_ClientProgressAuth* pMsg = (CNetMsg_Sv_ClientProgressAuth*)pRawMsg;
		switch(pMsg->m_Code)
		{
			// all codes states
			case AUTH_ALL_UNKNOWN:
			setAuthMessage("Unknow error (#0)", EAuthColorMessage::ERROR_MESSAGE);
			break;

			case AUTH_ALL_MUSTCHAR:
			setAuthMessage("Must contain 4-15 characters (#1)", EAuthColorMessage::ERROR_MESSAGE);
			break;

			case AUTH_REGISTER_GOOD:
			setAuthMessage("The was completed successfully", EAuthColorMessage::SUCCESS_MESSAGE);
			break;
			
			case AUTH_LOGIN_GOOD:
			SetAuthState(false);
			setAuthMessage("The was completed successfully", EAuthColorMessage::SUCCESS_MESSAGE);
			break;

			// login codes states
			case AUTH_LOGIN_ALREADY:
			setAuthMessage("Already authed (#3)", EAuthColorMessage::ERROR_MESSAGE);
			break;

			case AUTH_LOGIN_WRONG:
			setAuthMessage("Wrong login or password (#4)", EAuthColorMessage::ERROR_MESSAGE);
			break;

			case AUTH_LOGIN_NICKNAME:
			setAuthMessage("Wrong nickname (#5)", EAuthColorMessage::ERROR_MESSAGE);
			break;

			// register codes states
			case AUTH_REGISTER_ERROR_NICK:
			setAuthMessage("Change nick already registered (#6)", EAuthColorMessage::ERROR_MESSAGE);
			break;
		}
	}
}

void CMenus::RenderAuthWindow()
{
	if (m_pClient->m_pScoreboard->IsActive())
		return;

	m_MenuActiveID = EMenuState::AUTHSTATE;

	CUIRect MainView = *UI()->Screen();
	Graphics()->MapScreen(MainView.x, MainView.y, MainView.w, MainView.h);

	// устанавливаем стороны
	CUIRect Basic = MainView, BasicLeft, BasicRight, Label;
	Basic.VSplitMid(&BasicLeft, &BasicRight);
	BasicLeft.VSplitLeft(30.0f, 0, &BasicLeft);
	BasicRight.VSplitRight(30.0f, &BasicRight, 0);

	{ // рисуем фон лого
		CUIRect BackgroundLogo;
		Basic.VMargin(25.0f, &BackgroundLogo);
		BackgroundLogo.HMargin(100.0f, &BackgroundLogo);
		BackgroundLogo.HSplitTop(330.0f, &BackgroundLogo, 0);
		RenderTools()->DrawUIRect4(&BackgroundLogo,
			vec4(0.0f, 0.0f, 0.0f, 0.0f), vec4(0.0f, 0.0f, 0.0f, 0.0f),
			vec4(0.02f, 0.02f, 0.02f, 0.5f), vec4(0.02f, 0.02f, 0.02f, 0.5f), CUI::CORNER_B, 5.0f);
	}

	// --------------------- BACKGROUND --------------------------
	// -----------------------------------------------------------

	CUIRect MainBox;
	{ // main
		Basic.HMargin(170.0f, &MainBox);
		MainBox.VMargin(25.0f, &MainBox);
		RenderTools()->DrawUIRect(&MainBox, vec4(0.1f, 0.1f, 0.1f, 0.5f), CUI::CORNER_ALL, 5.0f);
	}

	CUIRect BackLeft = BasicLeft;
	{ // left
		BasicLeft.HMargin(180.0f, &BackLeft);
		BackLeft.Margin(5.0f, &BackLeft);
		RenderTools()->DrawUIRect(&BackLeft, vec4(0.3f, 0.3f, 0.3f, 0.3f), CUI::CORNER_ALL, 5.0f);

		BackLeft.HSplitTop(-32.0f, 0, &Label);
		UI()->DoLabel(&Label, Localize("Rules"), 24.0f, CUI::ALIGN_LEFT);
	}

	CUIRect BackRight = BasicRight;
	{ // right
		BasicRight.HMargin(180.0f, &BackRight);
		BackRight.Margin(5.0f, &BackRight);
		RenderTools()->DrawUIRect(&BackRight, vec4(0.3f, 0.3f, 0.3f, 0.3f), CUI::CORNER_ALL, 5.0f);

		BackRight.HSplitTop(-32.0f, 0, &Label);
		UI()->DoLabel(&Label, Localize("Account"), 24.0f, CUI::ALIGN_LEFT);
	}


	MainView.VMargin(5.0f, &MainView);

	CUIRect Message, Button;
	if (aAuthResultReason[0])
	{
		TextRender()->TextColor(aAuthResultColor.r, aAuthResultColor.g, aAuthResultColor.b, aAuthResultColor.a);

		MainBox.HSplitTop(250.f, 0, &Message);
		UI()->DoLabel(&Message, aAuthResultReason, 16.0f, CUI::ALIGN_CENTER, Message.w);
		TextRender()->TextColor(1.0f, 1.0f, 1.0f, 1.0f);
	}

	// --------------------- RULES SIDE --------------------------
	static bool s_PlayerAcceptRules = false;
	{
		// ----------------- BACKGROUND RULES ----------------
		CUIRect BackRules;
		BackLeft.Margin(5.0f, &BackRules);
		RenderTools()->DrawUIRect(&BackRules, vec4(0.2f, 0.2f, 0.2f, 0.75f), CUI::CORNER_ALL, 5.0f);

		// ------------------ RULES --------------------------

		// based
		BackRules.HSplitTop(10.0f, &Label, &BackRules);
		UI()->DoLabel(&Label, Localize("* This game mode will fully or partially support \nall vanilla players"), 14.0f, CUI::ALIGN_LEFT, -5.5f);

		// information
		BackRules.HSplitTop(20.0f, 0, &BackRules);
		BackRules.HSplitTop(10.0f, &Label, &BackRules);
		TextRender()->TextColor(0.6f, 1.0f, 0.6f, 1.0f);
		UI()->DoLabel(&Label, Localize("Information:"), 16.0f, CUI::ALIGN_LEFT, -5.5f);

		// rules
		BackRules.HSplitTop(50.0f, 0, &BackRules);
		BackRules.HSplitTop(10.0f, &Label, &BackRules);
		TextRender()->TextColor(1.0f, 0.6f, 0.6f, 1.0f);
		UI()->DoLabel(&Label, Localize("Rules on the server:"), 16.0f, CUI::ALIGN_LEFT, -5.5f);

		BackRules.HSplitTop(10.0f, 0, &BackRules);
		BackRules.HSplitTop(10.0f, &Label, &BackRules);
		TextRender()->TextColor(1.0f, 1.0f, 1.0f, 1.0f);
		UI()->DoLabel(&Label, Localize("- Don't use bugs\n"
										"- Don't use bots and other hack soft\n"
										"- Don't use dummy multi-account's\n"
										"- Don't share self account data (login, password)\n"
										"- Do not use ads, that is not part of the game"), 14.0f, CUI::ALIGN_LEFT);

		BackRules.HSplitTop(70.0f, 0, &BackRules);
		BackRules.HSplitTop(20.0f, &Button, &BackRules);
		static int s_AcceptedRules = 0;
		if(DoButton_CheckBox(&s_AcceptedRules, Localize("I read it and agree!"), s_PlayerAcceptRules, &Button))
			s_PlayerAcceptRules ^= true;
	}

	// --------------------- REGISTER / LOGIN SIDE --------------------------
	// -----------------------------------------------------------
	{
		// устанавливаем стороны
		CUIRect BasicLogin, BasicRegister;
		BackRight.VSplitMid(&BasicLogin, &BasicRegister);
		BackRight.Margin(10.0f, &BackRight);

		// ----------------- BACKGROUND LOGIN SIDE ----------------
		{ // left
			CUIRect BackLogin;
			BasicLogin.Margin(5.0f, &BackLogin);
			RenderTools()->DrawUIRect(&BackLogin, vec4(0.17f, 0.52f, 0.12f, 0.5f), CUI::CORNER_ALL, 5.0f);
		}

		{ // right
			CUIRect BackRegister; 
			BasicRegister.Margin(5.0f, &BackRegister);
			RenderTools()->DrawUIRect(&BackRegister, HexToRgba(0x147FF57), CUI::CORNER_ALL, 5.0f);
		}

		BasicLogin.Margin(6.0f, &BasicLogin);
		BasicRegister.Margin(6.0f, &BasicRegister);

		// ------------------------------------------------------
		// ----------------- LOGIN SIDE ( LEFT ) ----------------
		// ------------------------------------------------------
		{
			// - авторизация название
			BasicLogin.HSplitTop(25.0f, &Label, &BasicLogin);
			UI()->DoLabel(&Label, Localize("Log in to account"), 16.0f, CUI::ALIGN_LEFT);

			// - - - - - - - - - - - - - - - - - - ЛОГИН - - - - - - - - - - - - - - - //
			// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - //
			BasicLogin.HSplitTop(15.0f, &Label, &BasicLogin);
			UI()->DoLabel(&Label, Localize("Login"), 12.0f, CUI::ALIGN_LEFT);
			{
				static float s_OffsetUsername = 0.0f;
				static int s_boxAccountLogin = 0;
				BasicLogin.HSplitTop(25.0f, &Button, &BasicLogin);
				DoEditBox(&s_boxAccountLogin, &Button, g_Config.m_AccountMRPG, sizeof(g_Config.m_AccountMRPG), Button.h * ms_FontmodHeight * 0.8f, &s_OffsetUsername);
			}

			// - - - - - - - - - - - - - - - - - -ПАРОЛЬ - - - - - - - - - - - - - - - //
			// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - //
			BasicLogin.HSplitTop(10.0f, 0, &BasicLogin); // spacer
			BasicLogin.HSplitTop(15.0f, &Label, &BasicLogin);
			UI()->DoLabel(&Label, Localize("Password"), 12.0f, CUI::ALIGN_LEFT);
			{
				static float s_OffsetPassword = 0.0f;
				static int s_boxPasswordLogin = 0;
				BasicLogin.HSplitTop(25.0f, &Button, &BasicLogin);
				DoEditBox(&s_boxPasswordLogin, &Button, g_Config.m_PasswordMRPG, sizeof(g_Config.m_PasswordMRPG), Button.h* ms_FontmodHeight * 0.8f, &s_OffsetPassword, true);
			}

			// - - - - - - - - - - - - - - - - - -КНОПКИ - - - - - - - - - - - - - - - //
			// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - //
			BasicLogin.HSplitTop(60.0f, 0, &BasicLogin); // spacer
			BasicLogin.HSplitTop(25.0f, &Button, &BasicLogin);
			static CButtonContainer s_LoginButton;
			if(DoButton_Menu(&s_LoginButton, Localize("Join"), 0, &Button, 0, CUI::CORNER_ALL, 2.5f, 0.0f, vec4(1.0f, 1.0f, 1.0f, 0.75f), true))
				m_pClient->SendAuthPack(g_Config.m_AccountMRPG, g_Config.m_PasswordMRPG, false);
		}

		// ----------------------------------------------------------
		// ----------------- REGISTER SIDE ( RIGHT ) ----------------
		// ----------------------------------------------------------
		{
			// - регистрация инфомрация
			BasicRegister.HSplitTop(25.0f, &Label, &BasicRegister);
			UI()->DoLabel(&Label, Localize("Register account"), 16.0f, CUI::ALIGN_LEFT);
			
			// - переменные
			static char s_aAccount[64];
			static char s_aPassword[64];
			static char s_aRepeatPassword[64];

			// - - - - - - - - - - - - - - - - - - ЛОГИН - - - - - - - - - - - - - - - //
			// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - //
			BasicRegister.HSplitTop(15.0f, &Label, &BasicRegister);
			UI()->DoLabel(&Label, Localize("Login"), 12.0f, CUI::ALIGN_LEFT);
			BasicRegister.HSplitTop(25.0f, &Button, &BasicRegister);
			{
				static float s_OffsetUsername = 0.0f;
				static int s_boxAccountLogin = 0;
				DoEditBox(&s_boxAccountLogin, &Button, s_aAccount, sizeof(s_aAccount), Button.h * ms_FontmodHeight * 0.8f, &s_OffsetUsername);
			}

			// - - - - - - - - - - - - - - - - - -ПАРОЛЬ - - - - - - - - - - - - - - - //
			// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - //
			BasicRegister.HSplitTop(10.0f, 0, &BasicRegister); // spacer
			BasicRegister.HSplitTop(15.0f, &Label, &BasicRegister);
			UI()->DoLabel(&Label, Localize("Password"), 12.0f, CUI::ALIGN_LEFT);
			BasicRegister.HSplitTop(25.0f, &Button, &BasicRegister);
			{
				static float s_OffsetPassword = 0.0f;
				DoEditBox(&s_aPassword, &Button, s_aPassword, sizeof(s_aPassword), Button.h * ms_FontmodHeight * 0.8f, &s_OffsetPassword, true);
			}

			// - - - - - - - - - - - - - - ПОВТОР ПАРОЛЯ - - - - - - - - - - - - - - - //
			// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - //
			BasicRegister.HSplitTop(10.0f, 0, &BasicRegister); // spacer
			BasicRegister.HSplitTop(15.0f, &Label, &BasicRegister);
			UI()->DoLabel(&Label, Localize("Repeat password"), 12.0f, CUI::ALIGN_LEFT);
			BasicRegister.HSplitTop(25.0f, &Button, &BasicRegister);
			{
				static float s_OffsetRepeatPassword = 0.0f;
				DoEditBox(&s_aRepeatPassword, &Button, s_aRepeatPassword, sizeof(s_aRepeatPassword), Button.h * ms_FontmodHeight * 0.8f, &s_OffsetRepeatPassword, true);
			}

			// - - - - - - - - - - - - - - - - - -КНОПКИ - - - - - - - - - - - - - - - //
			// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - //
			BasicRegister.HSplitTop(10.0f, 0, &BasicRegister); // spacer
			BasicRegister.HSplitTop(25.0f, &Button, &BasicRegister);
			static CButtonContainer s_LoginButton;
			if(DoButton_Menu(&s_LoginButton, Localize("Register"), 0, &Button, 0, CUI::CORNER_ALL, 2.5f, 0.0f, vec4(1.0f, 1.0f, 1.0f, 0.75f), true))
			{
				if(str_comp(s_aPassword, s_aRepeatPassword) != 0)
				{
					setAuthMessage("Registration: the password does not match the repeated one!", EAuthColorMessage::ERROR_MESSAGE);
					mem_zero(s_aPassword, sizeof(s_aPassword));
					mem_zero(s_aRepeatPassword, sizeof(s_aRepeatPassword));
				}
				else if(s_PlayerAcceptRules == 0)
					setAuthMessage("Registration: you need to accept rules!", EAuthColorMessage::WARNING_MESSAGE);
				else
				{
					m_pClient->SendAuthPack(s_aAccount, s_aPassword, true);
					mem_zero(s_aPassword, sizeof(s_aPassword));
					mem_zero(s_aRepeatPassword, sizeof(s_aRepeatPassword));
				}
			}
		}
	}

	// render logo
	Graphics()->TextureSet(g_pData->m_aImages[IMAGE_MMOLOGO].m_Id);
	Graphics()->QuadsBegin();
	IGraphics::CQuadItem QuadItem(MainView.w / 2 - 240, MainView.h / 2 - 270, 513, 128);
	Graphics()->QuadsDrawTL(&QuadItem, 1);
	Graphics()->QuadsEnd();

	// render cursor
	RenderCursor(IMAGE_CURSOR, vec4(1.0f, 0.7f, 0.6f, 0.6f));
}

void CMenus::setAuthMessage(const char* Message, int EAuthColorMessage)
{
	str_copy(aAuthResultReason, Message, sizeof(aAuthResultReason));

	switch(EAuthColorMessage)
	{
		// red default
		default:
		case EAuthColorMessage::ERROR_MESSAGE:
		aAuthResultColor = vec4(1.0f, 0.5f, 0.5f, 1.0f);
		break;

		case EAuthColorMessage::WARNING_MESSAGE:
		aAuthResultColor = vec4(1.0f, 0.5f, 0.0f, 1.0f);
		break;

		case EAuthColorMessage::SUCCESS_MESSAGE:
		aAuthResultColor = vec4(0.5f, 1.0f, 0.5f, 1.0f);
		break;
	}
}