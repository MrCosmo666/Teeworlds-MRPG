/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#include <engine/demo.h>
#include <engine/engine.h>

#include <engine/shared/config.h>

#include <generated/client_data.h>

#include <game/client/components/particles.h>
#include <game/client/components/skins.h>
#include <game/client/components/flow.h>
#include <game/client/components/damageind.h>
#include <game/client/components/sounds.h>
#include <game/client/gameclient.h>

#include "effects.h"

#include "game/game_context.h"

inline vec2 RandomDir() { return normalize(vec2(frandom()-0.5f, frandom()-0.5f)); }

// TODO: read and optimize code

CEffects::CEffects()
{
	m_Add50hz = false;
	m_Add100hz = false;
	m_DamageTaken = 0;
	m_DamageTakenTick = 0;
}

void CEffects::AirJump(vec2 Pos)
{
	CParticle p;
	p.SetDefault();
	p.m_Spr = SPRITE_PART_AIRJUMP;
	p.m_Pos = Pos + vec2(-6.0f, 16.0f);
	p.m_Vel = vec2(0, -200);
	p.m_LifeSpan = 0.5f;
	p.m_StartSize = 48.0f;
	p.m_EndSize = 0;
	p.m_Rot = frandom()*pi*2;
	p.m_Rotspeed = pi*2;
	p.m_Gravity = 500;
	p.m_Friction = 0.7f;
	p.m_FlowAffected = 0.0f;
	m_pClient->m_pParticles->Add(CParticles::GROUP_GENERAL, &p);

	p.m_Pos = Pos + vec2(6.0f, 16.0f);
	m_pClient->m_pParticles->Add(CParticles::GROUP_GENERAL, &p);

	m_pClient->m_pSounds->PlayAt(CSounds::CHN_WORLD, SOUND_PLAYER_AIRJUMP, 1.0f, Pos);
}

void CEffects::DamageIndicator(vec2 Pos, int Amount)
{
	// ignore if there is no damage
	if(Amount == 0)
		return;

	m_DamageTaken++;
	float Angle;
	// create healthmod indicator
	if(Client()->LocalTime() < m_DamageTakenTick+0.5f)
	{
		// make sure that the damage indicators don't group together
		Angle = m_DamageTaken*0.25f;
	}
	else
	{
		m_DamageTaken = 0;
		Angle = 0;
	}

	float a = 3*pi/2 + Angle;
	float s = a-pi/3;
	float e = a+pi/3;
	for(int i = 0; i < Amount; i++)
	{
		float f = mix(s, e, float(i+1)/float(Amount+2));
		m_pClient->m_pDamageind->Create(vec2(Pos.x, Pos.y), direction(f));
	}

	m_DamageTakenTick = Client()->LocalTime();
}

void CEffects::PowerupShine(vec2 Pos, vec2 size)
{
	if(!m_Add50hz)
		return;

	CParticle p;
	p.SetDefault();
	p.m_Spr = SPRITE_PART_SLICE;
	p.m_Pos = Pos + vec2((frandom()-0.5f)*size.x, (frandom()-0.5f)*size.y);
	p.m_Vel = vec2(0, 0);
	p.m_LifeSpan = 0.5f;
	p.m_StartSize = 16.0f;
	p.m_EndSize = 0;
	p.m_Rot = frandom()*pi*2;
	p.m_Rotspeed = pi*2;
	p.m_Gravity = 500;
	p.m_Friction = 0.9f;
	p.m_FlowAffected = 0.0f;
	m_pClient->m_pParticles->Add(CParticles::GROUP_GENERAL, &p);
}

void CEffects::SmokeTrail(vec2 Pos, vec2 Vel)
{
	if(!m_Add50hz)
		return;

	CParticle p;
	p.SetDefault();
	p.m_Spr = SPRITE_PART_SMOKE;
	p.m_Pos = Pos;
	p.m_Vel = Vel + RandomDir()*50.0f;
	p.m_LifeSpan = 0.5f + frandom()*0.5f;
	p.m_StartSize = 12.0f + frandom()*8;
	p.m_EndSize = 0;
	p.m_Friction = 0.7f;
	p.m_Gravity = frandom()*-500.0f;
	m_pClient->m_pParticles->Add(CParticles::GROUP_PROJECTILE_TRAIL, &p);
}


void CEffects::SkidTrail(vec2 Pos, vec2 Vel)
{
	if(!m_Add100hz)
		return;

	CParticle p;
	p.SetDefault();
	p.m_Spr = SPRITE_PART_SMOKE;
	p.m_Pos = Pos;
	p.m_Vel = Vel + RandomDir()*50.0f;
	p.m_LifeSpan = 0.5f + frandom()*0.5f;
	p.m_StartSize = 24.0f + frandom()*12;
	p.m_EndSize = 0;
	p.m_Friction = 0.7f;
	p.m_Gravity = frandom()*-500.0f;
	p.m_Color = vec4(0.75f,0.75f,0.75f,1.0f);
	m_pClient->m_pParticles->Add(CParticles::GROUP_GENERAL, &p);
}

void CEffects::BulletTrail(vec2 Pos)
{
	if(!m_Add100hz)
		return;

	CParticle p;
	p.SetDefault();
	p.m_Spr = SPRITE_PART_BALL;
	p.m_Pos = Pos;
	p.m_LifeSpan = 0.25f + frandom()*0.25f;
	p.m_StartSize = 8.0f;
	p.m_EndSize = 0;
	p.m_Friction = 0.7f;
	m_pClient->m_pParticles->Add(CParticles::GROUP_PROJECTILE_TRAIL, &p);
}

void CEffects::PlayerSpawn(vec2 Pos)
{
	for(int i = 0; i < 32; i++)
	{
		CParticle p;
		p.SetDefault();
		p.m_Spr = SPRITE_PART_SHELL;
		p.m_Pos = Pos;
		p.m_Vel = RandomDir() * (powf(frandom(), 3)*600.0f);
		p.m_LifeSpan = 0.3f + frandom()*0.3f;
		p.m_StartSize = 64.0f + frandom()*32;
		p.m_EndSize = 0;
		p.m_Rot = frandom()*pi*2;
		p.m_Rotspeed = frandom();
		p.m_Gravity = frandom()*-400.0f;
		p.m_Friction = 0.7f;
		p.m_Color = vec4(0xb5/255.0f, 0x50/255.0f, 0xcb/255.0f, 1.0f);
		m_pClient->m_pParticles->Add(CParticles::GROUP_GENERAL, &p);

	}
	m_pClient->m_pSounds->PlayAt(CSounds::CHN_WORLD, SOUND_PLAYER_SPAWN, 1.0f, Pos);
}

void CEffects::PlayerDeath(vec2 Pos, int ClientID)
{
	vec3 BloodColor(1.0f,1.0f,1.0f);

	if(ClientID >= 0)
	{
		if(m_pClient->m_GameInfo.m_GameFlags&GAMEFLAG_TEAMS)
		{
			int ColorVal = m_pClient->m_pSkins->GetTeamColor(m_pClient->m_aClients[ClientID].m_aUseCustomColors[SKINPART_BODY], m_pClient->m_aClients[ClientID].m_aSkinPartColors[SKINPART_BODY],
																m_pClient->m_aClients[ClientID].m_Team, SKINPART_BODY);
			BloodColor = m_pClient->m_pSkins->GetColorV3(ColorVal);
		}
		else
		{
			if(m_pClient->m_aClients[ClientID].m_aUseCustomColors[SKINPART_BODY])
				BloodColor = m_pClient->m_pSkins->GetColorV3(m_pClient->m_aClients[ClientID].m_aSkinPartColors[SKINPART_BODY]);
			else
			{
				const CSkins::CSkinPart *s = m_pClient->m_pSkins->GetSkinPart(SKINPART_BODY, m_pClient->m_aClients[ClientID].m_SkinPartIDs[SKINPART_BODY]);
				if(s)
					BloodColor = s->m_BloodColor;
			}
		}
	}

	for(int i = 0; i < 64; i++)
	{
		CParticle p;
		p.SetDefault();
		p.m_Spr = SPRITE_PART_SPLAT01 + (random_int()%3);
		p.m_Pos = Pos;
		p.m_Vel = RandomDir() * ((frandom()+0.1f)*900.0f);
		p.m_LifeSpan = 0.3f + frandom()*0.3f;
		p.m_StartSize = 24.0f + frandom()*16;
		p.m_EndSize = 0;
		p.m_Rot = frandom()*pi*2;
		p.m_Rotspeed = (frandom()-0.5f) * pi;
		p.m_Gravity = 800.0f;
		p.m_Friction = 0.8f;
		vec3 c = BloodColor * (0.75f + frandom()*0.25f);
		p.m_Color = vec4(c.r, c.g, c.b, 0.75f);
		m_pClient->m_pParticles->Add(CParticles::GROUP_GENERAL, &p);
	}
}


void CEffects::Explosion(vec2 Pos)
{
	// add to flow
	for(int y = -8; y <= 8; y++)
		for(int x = -8; x <= 8; x++)
		{
			if(x == 0 && y == 0)
				continue;

			float a = 1 - (length(vec2(x,y)) / length(vec2(8,8)));
			m_pClient->m_pFlow->Add(Pos+vec2(x,y)*16, normalize(vec2(x,y))*5000.0f*a, 10.0f);
		}

	// add the explosion
	CParticle p;
	p.SetDefault();
	p.m_Spr = SPRITE_PART_EXPL01;
	p.m_Pos = Pos;
	p.m_LifeSpan = 0.4f;
	p.m_StartSize = 150.0f;
	p.m_EndSize = 0;
	p.m_Rot = frandom()*pi*2;
	m_pClient->m_pParticles->Add(CParticles::GROUP_EXPLOSIONS, &p);

	// add the smoke
	for(int i = 0; i < 24; i++)
	{
		CParticle p;
		p.SetDefault();
		p.m_Spr = SPRITE_PART_SMOKE;
		p.m_Pos = Pos;
		p.m_Vel = RandomDir() * ((1.0f + frandom()*0.2f) * 1000.0f);
		p.m_LifeSpan = 0.5f + frandom()*0.4f;
		p.m_StartSize = 32.0f + frandom()*8;
		p.m_EndSize = 0;
		p.m_Gravity = frandom()*-800.0f;
		p.m_Friction = 0.4f;
		p.m_Color = mix(vec4(0.75f,0.75f,0.75f,1.0f), vec4(0.5f,0.5f,0.5f,1.0f), frandom());
		m_pClient->m_pParticles->Add(CParticles::GROUP_GENERAL, &p);
	}
}

void CEffects::HammerHit(vec2 Pos)
{
	CParticle p;
	p.SetDefault();
	p.m_Spr = SPRITE_PART_HIT01;
	p.m_Pos = Pos;
	p.m_LifeSpan = 0.3f;
	p.m_StartSize = 120.0f;
	p.m_EndSize = 0;
	p.m_Rot = frandom()*pi*2;
	m_pClient->m_pParticles->Add(CParticles::GROUP_EXPLOSIONS, &p);
	m_pClient->m_pSounds->PlayAt(CSounds::CHN_WORLD, SOUND_HAMMER_HIT, 1.0f, Pos);
}


// mmotee
void CEffects::MmoEffects(vec2 Pos, int EffectID)
{
	CParticle p;
	p.SetDefault();
	if (EffectID == EFFECT_SPASALON)
	{
		p.m_Spr = rand() % 2 == 0 ? (int)(SPRITE_RELAX_EYES) : (int)(SPRITE_RELAX_HEART);
		p.m_Pos = Pos;
		p.m_LifeSpan = 2.5f;

		p.m_Gravity = -1000.0f;
		p.m_Friction = 0.4f;
		p.m_Frames = 1;
		p.m_StartSize = 60.0f + rand() % 10;
		p.m_EndSize = 0;
		p.m_Rot = 0 + rand() % 2 - rand() % 4;
		p.m_Color = vec4(100.0f, 100.0f, 100.0f, 0.01f);

		m_pClient->m_pParticles->Add(CParticles::GROUP_MMOEFFECTS, &p);
		return;
	}

	p.m_Spr = SPRITE_TELEPORT1;
	p.m_Frames = 8;
	p.m_Pos = Pos;
	p.m_LifeSpan = 0.4f;
	p.m_Gravity = -1000.0f;
	p.m_Friction = 0.4f;
	p.m_StartSize = 220;
	p.m_EndSize = 220;
	p.m_Rot = 0;
	p.m_Color = vec4(100.0f, 100.0f, 100.0f, 0.10f);
	m_pClient->m_pParticles->Add(CParticles::GROUP_TELEPORT, &p);
}

void CEffects::MmoTextEffect(vec2 Pos, const char* pText, int Flag)
{
	CParticle p;
	p.SetDefault();
	p.m_TextBuf[0] = '\0';

	// potion effects / basic text
	if(Flag & TEXTEFFECT_FLAG_POTION || Flag & TEXTEFFECT_FLAG_BASIC)
	{
		p.m_Pos = Vec2Range(&Pos, 60);
		p.m_LifeSpan = 0.7f;
		p.m_Gravity = -1000.0f;
		p.m_StartSize = 21.0f;
		p.m_EndSize = 21.0f;

		if(str_find(pText, "Health") != nullptr)
			p.m_Color = vec4(1.0f, 0.80f, 1.0f, 1.0f);
		if(str_find(pText, "Mana") != nullptr)
			p.m_Color = vec4(0.80f, 0.80f, 1.0f, 1.0f);
		if(str_find(pText, "Poison") != nullptr)
			p.m_Color = vec4(0.40f, 0.80f, 0.0f, 1.0f);
		if(str_find(pText, "Slowdown") != nullptr)
			p.m_Color = vec4(0.50f, 1.00f, 0.50f, 1.0f);
		if(str_find(pText, "Fire") != nullptr)
			p.m_Color = vec4(1.0f, 0.65f, 0.0f, 1.0f);
		if(str_find(pText, "Ice") != nullptr)
			p.m_Color = vec4(0.0f, 0.50f, 1.0f, 1.0f);
	}
	
	// damage effect
	else if(Flag & TEXTEFFECT_FLAG_DAMAGE || Flag & TEXTEFFECT_FLAG_CRIT_DAMAGE)
	{
		const int DamageValue = str_toint(pText);
		const float IncreaseSize = min((float)DamageValue, 28.0f);
		
		p.m_Pos = Pos;
		p.m_Vel = vec2(-8 + rand() % 10, -8 + rand() % 10) * 50 * (40 * 0.025f);
		p.m_LifeSpan = 1.5f;
		p.m_Gravity = 0.7f;
		p.m_FlowAffected = 0.5f;
		p.m_Friction = 0.8f;

		if(Flag & TEXTEFFECT_FLAG_CRIT_DAMAGE)
		{
			p.m_StartSize = 22.0f + IncreaseSize * 0.5f;
			p.m_EndSize = 22.0f + IncreaseSize * 0.5f;
			p.m_Color = vec4(1.0f, 0.80f, 0.3f, 1.0f);
		}
		else if(Flag & TEXTEFFECT_FLAG_DAMAGE)
		{
			p.m_StartSize = 18.0f + IncreaseSize * 0.5f;
			p.m_EndSize = 18.0f + IncreaseSize * 0.3f;
			p.m_Color = vec4(1.0f, 1.0f, 1.0f, 1.0f);
		}
	}

	// miss
	else if(Flag & TEXTEFFECT_FLAG_MISS)
	{
		p.m_Pos = Pos;
		p.m_StartSize = 28.0f;
		p.m_EndSize = 28.0f;
		p.m_Gravity = 0.7f;
		p.m_FlowAffected = 0.5f;
		p.m_Friction = 0.8f;
		p.m_Vel = vec2(-8 + rand() % 10, -8 + rand() % 10) * 50 * (40 * 0.025f);
		p.m_LifeSpan = 1.5f;
		p.m_Color = vec4(0.6f, 0.7f, 1.0f, 1.0f);
	}
	
	// adding
	if(Flag & TEXTEFFECT_FLAG_ADDING)
		str_append(p.m_TextBuf, "+", sizeof(p.m_TextBuf));
	// removing
	if(Flag & TEXTEFFECT_FLAG_REMOVING)
		str_append(p.m_TextBuf, "-", sizeof(p.m_TextBuf));

	str_append(p.m_TextBuf, pText, sizeof(p.m_TextBuf));
	m_pClient->m_pParticles->Add(CParticles::GROUP_DAMAGEMMO, &p);
}

void CEffects::EnchantEffect(vec2 Pos, vec2 Vel, vec4 Color, float RandomSize)
{
	if (!m_Add50hz || g_Config.m_ClShowMEffects == 3)
		return;

	vec2 PosRandom = vec2(Pos.x + 40.0f - frandom() * 80.0f, Pos.y + 40.0f - frandom() * 80.0f);
	if (length(PosRandom) < 40.0f)
		return;

	CParticle p;
	p.SetDefault();
	p.m_Spr = SPRITE_MMO_GAME_EFFECT;
	p.m_Pos = PosRandom;
	p.m_Vel = Vel + RandomDir() * 200.0f;
	p.m_LifeSpan = 0.5f + frandom() * 0.5f;
	p.m_StartSize = 5.0f + frandom() * 20.0f;
	p.m_EndSize = 0;
	p.m_Friction = 0.8f;
	p.m_Gravity = frandom() * -500.0f;

	if (RandomSize > 0.0f)
		p.m_Color = vec4(Color.r + frandom() * RandomSize, Color.g + frandom() * RandomSize, Color.b + frandom() * RandomSize, Color.a);
	else
		p.m_Color = Color;

	m_pClient->m_pParticles->Add(CParticles::GROUP_MMOPROJ, &p);
}

/*void CEffects::BubbleEffect(vec2 Pos, vec2 Vel)
{
	if (!m_Add50hz)
		return;

	CParticle p;
	p.SetDefault();
	p.m_Spr = SPRITE_MMO_GAME_BUBBLE_FIRE;
	p.m_Pos = Pos;
	p.m_Vel = Vel + RandomDir() * 200.0f;
	p.m_LifeSpan = 0.5f + frandom() * 0.5f;
	p.m_StartSize = 16.0f + frandom() * 18;
	p.m_EndSize = 0;
	p.m_Friction = 0.6f;
	p.m_Gravity = frandom() * -500.0f;
	m_pClient->m_pParticles->Add(CParticles::GROUP_MMOPROJ, &p);
}*/

void CEffects::OnRender()
{
	static int64 s_LastUpdate100hz = 0;
	static int64 s_LastUpdate50hz = 0;

	const float Speed = GetEffectsSpeed();
	const int64 Now = time_get();
	const int64 Freq = time_freq();

	if (Now - s_LastUpdate100hz > Freq / (100 * Speed))
	{
		m_Add100hz = true;
		s_LastUpdate100hz = Now;
	}
	else
		m_Add100hz = false;

	if (Now - s_LastUpdate50hz > Freq / (50 * Speed))
	{
		m_Add50hz = true;
		s_LastUpdate50hz = Now;
	}
	else
		m_Add50hz = false;

	if (m_Add50hz)
		m_pClient->m_pFlow->Update();
}

float CEffects::GetEffectsSpeed()
{
	if (Client()->State() == IClient::STATE_DEMOPLAYBACK)
		return DemoPlayer()->BaseInfo()->m_Speed;
	return 1.0f;
}