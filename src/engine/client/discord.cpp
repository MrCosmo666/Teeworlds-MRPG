/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#include <engine/client.h>
#include <engine/discord.h>
#include <engine/serverbrowser.h>

#include <generated/protocol.h>

#include <game/gamecore.h>

#include "discord.h"

#define APPLICATION_ID 753325967778381955

CDiscord::CDiscord()
{
	m_pClient = 0;
	m_ActivityData.m_LastChange = 0;
	m_ActivityData.m_LastStateChange = 0;
	m_ActivityData.m_LastState = -1;
	m_DiscordState.m_UserFound = false;

	discord::Core* pCore;
	discord::Result Result = discord::Core::Create(APPLICATION_ID, DiscordCreateFlags_NoRequireDiscord, &pCore);
	m_DiscordState.m_Core.reset(pCore);
	if (!m_DiscordState.m_Core)
	{
		dbg_msg("discord", "failed to instantiate discord core: %d", static_cast<int>(Result));
		return;
	}

	m_DiscordState.m_Core->SetLogHook(discord::LogLevel::Debug, [this](discord::LogLevel Level, const char* pMessage) {
		this->OnLog(Level, pMessage);
		});

	m_DiscordState.m_Core->UserManager().OnCurrentUserUpdate.Connect([this]() {
		this->OnCurrentUserUpdate();
		});

	m_DiscordState.m_Core->ActivityManager().OnActivityJoin.Connect([this](const char* pSecret) {
		this->OnActivityJoin(pSecret);
		});

	m_DiscordState.m_Core->ActivityManager().OnActivityInvite.Connect([this](discord::ActivityActionType ActivityActionType, discord::User const& User, discord::Activity const& Activity) {
		this->OnActivityInvite(ActivityActionType, User, Activity);
		});

	m_DiscordState.m_Core->ActivityManager().OnActivityJoinRequest.Connect([this](discord::User const& User) {
		this->OnActivityJoinRequest(User);
		});

	m_DiscordState.m_Core->ActivityManager().OnActivitySpectate.Connect([this](const char* pSecret) {
		this->OnActivitySpectate(pSecret);
		});
}

void CDiscord::Init()
{
	m_pClient = Kernel()->RequestInterface<IClient>();
	m_pGameClient = Kernel()->RequestInterface<IGameClient>();
}

void CDiscord::OnLog(discord::LogLevel Level, const char* pMessage)
{
	dbg_msg("discord", "[Log] (Level %d): %s", static_cast<uint32_t>(Level), pMessage);
}

void CDiscord::OnCurrentUserUpdate()
{
	m_DiscordState.m_UserFound = true;
	m_DiscordState.m_Core->UserManager().GetCurrentUser(&m_DiscordState.m_CurrentUser);

	dbg_msg("discord", "user updated: %s#%s", m_DiscordState.m_CurrentUser.GetUsername(), m_DiscordState.m_CurrentUser.GetDiscriminator());
}

void CDiscord::OnActivityJoin(const char* pSecret)
{
#ifdef CONF_DEBUG
	dbg_msg("discord", "OnActivityJoin fired: secret: %s", pSecret);
#endif

	// todo: add some checks, don't use ip as secret lol
	// Client()->Connect(pSecret);
}

void CDiscord::OnActivityInvite(discord::ActivityActionType ActivityActionType, discord::User const& User, discord::Activity const& Activity)
{
#ifdef CONF_DEBUG
	dbg_msg("discord", "OnActivityInvite fired: type: %s, user: %s#%s, activity: %s", ActivityActionType == discord::ActivityActionType::Join ? "Join" : "Spectate", User.GetUsername(), User.GetDiscriminator(), Activity.GetName());
#endif
}

void CDiscord::OnActivityJoinRequest(discord::User const& User)
{
#ifdef CONF_DEBUG
	dbg_msg("discord", "OnActivityJoinRequest fired: user: %s#%s", User.GetUsername(), User.GetDiscriminator());
#endif
}

void CDiscord::OnActivitySpectate(const char* pSecret)
{
#ifdef CONF_DEBUG
	dbg_msg("discord", "OnActivitySpectate fired: secret: %s", pSecret);
#endif
}

void CDiscord::HandleActivity()
{
	if (m_ActivityData.m_LastChange + time_freq() * 5 > time_get())
		return;

	// handle state changes
	if (m_ActivityData.m_LastState != Client()->State())
	{
		m_ActivityData.m_LastState = Client()->State();
		m_ActivityData.m_LastStateChange = time_get();
	}

	// update activity
	discord::Activity* pActivity = new discord::Activity();
	pActivity->GetAssets().SetLargeImage("mrpg_tee");
	pActivity->GetAssets().SetLargeText("Teeworlds MRPG");
	pActivity->GetTimestamps().SetStart(time(0) - ((time_get() - m_ActivityData.m_LastStateChange) / time_freq()));
	pActivity->SetInstance(false);

	if (m_ActivityData.m_LastState == IClient::STATE_ONLINE)
	{
		CServerInfo Info = { 0 };
		Client()->GetServerInfo(&Info);
		if (!Info.m_aName[0])
			return;

		const CNetObj_Mmo_ClientInfo* pInfo = GameClient()->GetMMOInfo();
		if (pInfo)
		{
			char aGold[32];
			IntsToStr(pInfo->m_Gold, 6, aGold);

			char aBuf[64];
			str_format(aBuf, sizeof(aBuf), "Lvl. %d | Gold: %s", pInfo->m_Level, aGold);
			pActivity->SetDetails(aBuf);

			str_format(aBuf, sizeof(aBuf), "Playing on %s", Client()->GetCurrentMapName());
			pActivity->SetState(aBuf);

			pActivity->GetParty().GetSize().SetCurrentSize(Info.m_NumClients);
			pActivity->GetParty().GetSize().SetMaxSize(Info.m_MaxClients);
		}
		else
		{
			// non mrpg server support
			char aDetails[64];
			str_format(aDetails, sizeof(aDetails), "%s on %s", Info.m_aGameType, Info.m_aMap);
			pActivity->SetDetails(aDetails);
			pActivity->SetState("In Game");
			pActivity->GetParty().GetSize().SetCurrentSize(Info.m_NumClients);
			pActivity->GetParty().GetSize().SetMaxSize(Info.m_MaxClients);
		}

		// invite test
		/*pActivity->GetParty().SetId(Info.m_aName);
		pActivity->GetSecrets().SetJoin(Info.m_aAddress);*/

		/*pActivity->GetAssets().SetSmallImage("nodes_icon");
		pActivity->GetAssets().SetSmallText("Nodes");*/
	}
	else if(m_ActivityData.m_LastState != IClient::STATE_LOADING && m_ActivityData.m_LastState != IClient::STATE_CONNECTING) // don't update while changing map
	{
		pActivity->SetState("In Main Menu");
	}

	m_DiscordState.m_Core->ActivityManager().UpdateActivity(*pActivity, [this](discord::Result Result) {
		if (Result != discord::Result::Ok)
			dbg_msg("discord", "failed to update activity: %d", static_cast<int>(Result));
		});

	m_ActivityData.m_LastChange = time_get();
}

void CDiscord::Update()
{
	if (!m_DiscordState.m_Core)
		return;

	HandleActivity();

	m_DiscordState.m_Core->RunCallbacks();
}

IDiscord* CreateDiscordWrapper() { return new CDiscord; }