/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#include <game/server/gamecontext.h>

#include "SkillPlayer.h"

#include <game/server/mmocore/GameEntities/Skills/healthturret/healer-health.h>
#include <game/server/mmocore/GameEntities/Skills/sleepygravity/sleepygravity.h>
#include <game/server/mmocore/GameEntities/Skills/noctislucisteleport/noctis_teleport.h>

CSkillInformation& CSkillPlayer::Info() const 
{ 
	return SkillsJob::ms_aSkillsData[m_SkillID]; 
}

void CSkillPlayer::SetSkillOwner(CPlayer* pPlayer)
{
	m_pPlayer = pPlayer;
	m_pGS = pPlayer->GS();
}

void CSkillPlayer::SelectNextControlEmote()
{
	if(!m_pPlayer || !m_pPlayer->IsAuthed())
		return;

	m_SelectedEmoticion++;
	if(m_SelectedEmoticion >= NUM_EMOTICONS)
		m_SelectedEmoticion = -1;

	SJK.UD("tw_accounts_skills", "SelectedEmoticion = '%d' WHERE SkillID = '%d' AND OwnerID = '%d'", m_SelectedEmoticion, m_SkillID, m_pPlayer->Acc().m_AuthID);
}

bool CSkillPlayer::Use()
{
	if(!m_pPlayer || !m_pPlayer->IsAuthed() || !m_pPlayer->GetCharacter() || m_Level <= 0)
		return false;

	// mana check
	CCharacter* pChr = m_pPlayer->GetCharacter();
	const int PriceMana = kurosio::translate_to_procent_rest(m_pPlayer->GetStartMana(), Info().m_ManaProcent);
	if(PriceMana > 0 && pChr->CheckFailMana(PriceMana))
		return false;

	const vec2 PlayerPosition = pChr->GetPos();
	const int ClientID = m_pPlayer->GetCID();
	if(m_SkillID == Skill::SkillHeartTurret)
	{
		for(CHealthHealer* pHh = (CHealthHealer*)GS()->m_World.FindFirst(CGameWorld::ENTYPE_SKILLTURRETHEART); pHh; pHh = (CHealthHealer*)pHh->TypeNext())
		{
			if(pHh->m_pPlayer->GetCID() != ClientID)
				continue;

			pHh->Reset();
			break;
		}
		const int PowerLevel = PriceMana;
		new CHealthHealer(&GS()->m_World, m_pPlayer, GetBonus(), PowerLevel, PlayerPosition);
		return true;
	}

	if(m_SkillID == Skill::SkillSleepyGravity)
	{
		for(CSleepyGravity* pHh = (CSleepyGravity*)GS()->m_World.FindFirst(CGameWorld::ENTYPE_SLEEPYGRAVITY); pHh; pHh = (CSleepyGravity*)pHh->TypeNext())
		{
			if(pHh->m_pPlayer->GetCID() != ClientID)
				continue;

			pHh->Reset();
			break;
		}
		const int PowerLevel = PriceMana;
		new CSleepyGravity(&GS()->m_World, m_pPlayer, GetBonus(), PowerLevel, PlayerPosition);
		return true;
	}

	if(m_SkillID == Skill::SkillNoctisTeleport)
	{
		new CNoctisTeleport(&GS()->m_World, PlayerPosition, pChr, GetBonus());
		return true;
	}

	if(m_SkillID == Skill::SkillBlessingGodWar)
	{
		for(int i = 0; i < MAX_PLAYERS; i++)
		{
			CPlayer* pPlayerSearch = GS()->GetPlayer(i, true, true);
			if(!pPlayerSearch || GS()->Server()->GetWorldID(i) != GS()->GetWorldID() || distance(PlayerPosition, pPlayerSearch->GetCharacter()->GetPos()) > 800
				|| (pPlayerSearch->GetCharacter()->IsAllowedPVP(ClientID) && i != ClientID))
				continue;

			const int RealAmmo = 10 + pPlayerSearch->GetAttributeCount(Stats::StAmmo);
			const int RestoreAmmo = kurosio::translate_to_procent_rest(RealAmmo, min(GetBonus(), 100));
			for(int i = WEAPON_GUN; i <= WEAPON_LASER; i++)
			{
				pPlayerSearch->GetCharacter()->GiveWeapon(i, RestoreAmmo);
				GS()->CreateSound(PlayerPosition, SOUND_CTF_GRAB_PL);
				GS()->CreateDeath(PlayerPosition, i);
			}
		}

		GS()->CreateText(NULL, false, vec2(PlayerPosition.x, PlayerPosition.y - 96.0f), vec2(0, 0), 40, "RECOVERY AMMO");
		return true;
	}
	return false;
}

bool CSkillPlayer::Upgrade()
{
	if(!m_pPlayer || !m_pPlayer->IsAuthed() || m_Level >= Info().m_MaxLevel)
		return false;

	const int ClientID = m_pPlayer->GetCID();
	std::shared_ptr<ResultSet> RES(SJK.SD("*", "tw_accounts_skills", "WHERE SkillID = '%d' AND OwnerID = '%d'", m_SkillID, m_pPlayer->Acc().m_AuthID));
	if(RES->next())
	{
		if(m_pPlayer->CheckFailMoney(Info().m_PriceSP, itSkillPoint))
			return false;

		m_Level++;
		SJK.UD("tw_accounts_skills", "SkillLevel = '%d' WHERE SkillID = '%d' AND OwnerID = '%d'", m_Level, m_SkillID, m_pPlayer->Acc().m_AuthID);
		GS()->Chat(ClientID, "Increased the skill [{STR} level to {INT}]", Info().m_aName, &m_Level);
		return true;
	}

	if(m_pPlayer->CheckFailMoney(Info().m_PriceSP, itSkillPoint))
		return false;

	m_Level = 1;
	m_SelectedEmoticion = -1;
	SJK.ID("tw_accounts_skills", "(SkillID, OwnerID, SkillLevel) VALUES ('%d', '%d', '1');", m_SkillID, m_pPlayer->Acc().m_AuthID);
	GS()->Chat(ClientID, "Learned a new skill [{STR}]", Info().m_aName);
	return true;
}