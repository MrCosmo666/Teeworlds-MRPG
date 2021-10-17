#include <game/client/ui.h>

#include "elements.h"
#include "value.h"

#include <base/system.h>

CUIRect CValue::GetRect() const { return { m_Value.x, m_Value.y, m_Value.w, m_Value.h }; }

CValue::CValue()
{
	m_BackAnimation = false;
	m_Value = vec4(0, 0, 0, 0);
	m_NewValue = vec4(0, 0, 0, 0);
	m_OldValue = vec4(0, 0, 0, 0);
	m_AnimEnded = true;
	m_Callback = nullptr;
}

void CValue::Init(CUIRect *pRect)
{
	vec4 InitVector = vec4(pRect->x, pRect->y, pRect->w, pRect->h);
	Init(InitVector);
}

void CValue::Init(vec4 Value)
{
	m_Value = Value;
	m_NewValue = Value;
	m_OldValue = Value;
	m_AnimEnded = true;
	m_Callback = nullptr;
}

void CValue::Anim(CUIRect* pRect, float Time, int AnimType, bool BackAnimation, bool RepairAnimation)
{
	vec4 InitVector = vec4(pRect->x, pRect->y, pRect->w, pRect->h);
	Anim(InitVector, Time, AnimType, BackAnimation, RepairAnimation);
}

void CValue::Anim(vec4 Value, float Time, int AnimType, bool BackAnimation, bool RepairAnimation)
{
	if(Value == m_Value)
		return;

	m_NewValue = Value;
	m_OldValue = m_Value;

	const float AnimTime = time_freq() * Time;
	m_AnimEnded = false;
	m_Animation = AnimType;
	m_AnimTime = time_get();
	m_AnimEndTime = time_get() + (int)round(AnimTime);
	m_BackAnimation = BackAnimation;
	m_RepairAnimation = RepairAnimation;
	m_Callback = nullptr;
}

void CValue::SetAnimEndCallback(AnimCallback Callback)
{
	m_Callback = Callback;
}

void CValue::RepairAnimation()
{
	m_Value = m_NewValue;
	int BackAnimation = m_Animation;
	if(m_Animation % 4 == ANIMATION_TYPE::AnimIN)
		BackAnimation++;
	else if(m_Animation % 4 == ANIMATION_TYPE::AnimOUT)
		BackAnimation--;
	Anim(m_OldValue, (float)((m_AnimEndTime - m_AnimTime) / time_freq()), BackAnimation, false, m_RepairAnimation);
}

void CValue::Recalculate()
{
	float PassedTime = (float)((time_get() - m_AnimTime) / ((m_AnimEndTime - m_AnimTime) * 1.0f));
	m_Value = Animation(m_Animation, m_OldValue, m_NewValue, PassedTime);
}

void CValue::EndAnimation()
{
	m_AnimEnded = true;
	m_RepairAnimation = false;
	m_Value = m_NewValue;
	if(m_Callback != nullptr)
	{
		m_Callback();
		m_Callback = nullptr;
		return;
	}

	// skip back animation with callback
	if(m_BackAnimation)
		RepairAnimation();
}

void CValue::PreRender()
{
	if(m_AnimTime <= time_get() && time_get() <= m_AnimEndTime)
		Recalculate();
	else if(!m_AnimEnded)
	{
		if(m_RepairAnimation)
		{
			RepairAnimation();
			return;
		}
		EndAnimation();
	}
}

vec4 CValue::Animation(int AnimType, vec4 min, vec4 max, float time)
{
	float s = 0.0f;
	switch (AnimType)
	{
		case EaseIN:
			time = -cosf(time * pi / 2) + 1;
			break;
		case EaseOUT:
			time = sinf(time * pi / 2);
			break;
		case EaseINOUT:
			time = -0.5f * (cosf(pi * time) - 1);
			break;
		case EaseOUTIN:
			time *= 2;
			if (time < 1)
			{
				time = 0.5f * sinf(time * pi / 2);
			}
			else
			{
				time -= 1;
				time = -0.5f * cosf(time * pi / 2) + 1;
			}
			break;

		case EaseIN2:
			time = time * time;
			break;
		case EaseOUT2:
			time = -time * (time - 2);
			break;
		case EaseINOUT2:
			time *= 2;
			if (time < 1)
			{
				time = 0.5f * time * time;
			}
			else
			{
				time -= 1;
				time = -0.5f * (time * (time - 2) - 1);
			}
			break;
		case EaseOUTIN2:
			time *= 2;
			if (time < 1)
			{
				time = -0.5f * time * (time - 2);
			}
			else
			{
				time -= 1;
				time = 0.5f * time * time + 0.5f;
			}
			break;


		case EaseIN3:
			time = time * time * time;
			break;
		case EaseOUT3:
			time -= 1;
			time = time * time * time + 1;
			break;
		case EaseINOUT3:
			time *= 2;
			if (time < 1)
			{
				time = 0.5f * time * time * time;
			}
			else
			{
				time -= 2;
				time = 0.5f * (time * time * time + 2);
			}
			break;
		case EaseOUTIN3:
			time *= 2;
			if (time < 1)
			{
				time = 0.5f * (time * time * time + 2) - 1;
			}
			else
			{
				time -= 1;
				time = 0.5f * time * time * time + 0.5f;
			}
			break;

		case EaseIN4:
			time = time * time * time * time;
			break;
		case EaseOUT4:
			time -= 1;
			time = -(time * time * time * time - 1);
			break;
		case EaseINOUT4:
			time *= 2;
			if (time < 1)
			{
				time = 0.5f * time * time * time * time;
			}
			else
			{
				time -= 2;
				time = -0.5f * (time * time * time * time - 2);
			}
			break;
		case EaseOUTIN4:
			time *= 2;
			if (time < 1)
			{
				time = -0.5f * time * time * time * (time - 2);
			}
			else
			{
				time -= 1;
				time = 0.5f * time * time * time * time + 0.5f;
			}
			break;

		case EaseIN5:
			time = time * time * time * time * time;
			break;
		case EaseOUT5:
			time -= 1;
			time = time * time * time * time * time + 1;
			break;
		case EaseINOUT5:
			time *= 2;
			if (time < 1)
			{
				time = 0.5f * (time * time * time * time * time);
			}
			else
			{
				time -= 2;
				time = 0.5f * (time * time * time * time * time + 2);
			}
			break;
		case EaseOUTIN5:
			time *= 2;
			if (time < 1)
			{
				time = 0.5f * (time * time * time * time * time + 2) - 1;
			}
			else
			{
				time -= 1;
				time = 0.5f * time * time * time * time * time + 0.5f;
			}
			break;

		case EaseIN6:
			time = powf(2.0f, 10 * (time - 1));
			break;
		case EaseOUT6:
			time = -powf(2.0f, -10 * time) + 1;
			break;
		case EaseINOUT6:
			time *= 2;
			if (time < 1)
			{
				time = 0.5f * powf(2.0f, 10 * (time - 1));
			}
			else
			{
				time -= 1;
				time = 0.5f * (-powf(2.0f, -10 * time) + 2);
			}
			break;
		case EaseOUTIN6:
			time *= 2;
			if (time < 1)
			{
				time = -0.5f * time * time * time * time * time * (time - 2);
			}
			else
			{
				time -= 1;
				time = 0.5f * time * time * time * time * time * time + 0.5f;
			}
			break;

		case EaseINCirc:
			time = -(sqrtf(1 - time * time) - 1);
			break;
		case EaseOUTCirc:
			time -= 1;
			time = sqrtf(1 - time * time);
			break;
		case EaseINOUTCirc:
			time *= 2;
			if (time < 1)
			{
				time = -0.5f * (sqrtf(1 - time * time) - 1);
			}
			else
			{
				time -= 2;
				time = 0.5f * (sqrtf(1 - time * time) + 1);
			}
			break;
		case EaseOUTINCirc:
			time *= 2;
			if (time < 1)
			{
				time -= 1;
				time = 0.5f * sqrtf(1 - time * time);
			}
			else
			{
				time -= 1;
				time = - 0.5f * sqrtf(1 - time * time) + 1;
			}
			break;

		case EaseINBack:
			s = 1.70158f;
			time = time * time * ((s + 1) * time - s);
			break;
		case EaseOUTBack:
			s = 1.70158f;
			time -= 1;
			time = time * time * ((s + 1) * time + s) + 1;
			break;
		case EaseINOUTBack:
			s = 1.70158f * 1.525f;
			time *= 2;
			if (time < 1)
			{
				time = 0.5f * (time * time * ((s + 1) * time - s));
			}
			else
			{
				time -= 2;
				time = 0.5f * (time * time * ((s + 1) * time + s) + 2);
			}
			break;
		case EaseOUTINBack:
			s = 1.70158f * 1.525f;
			time *= 2;
			if (time < 1)
			{
				time -= 1;
				time = 0.5f * (time * time * ((s + 1) * time + s) + 1);
			}
			else
			{
				//time -= 2;
				time = 0.5f * (time * time * ((s + 1) * time - s));
			}
			break;

		case EaseINElastic:
			time = sinf(13 * pi / 2.0f * time) * powf(2.0f, 10 * (time - 1));
			break;
		case EaseOUTElastic:
			time = sinf(-13 * pi / 2.0f * (time + 1)) * powf(2.0f, -10 * time) + 1;
			break;
		case EaseINOUTElastic:
			if (time < 0.5)
			{
				time = 0.5f * sinf(13 * pi / 2.0f * (2 * time)) * powf(2.0f, 10 * ((2.0f * time) - 1));
			}
			else
			{
				time = 0.5f * (sinf(-13 * pi / 2.0f * ((2 * time - 1) + 1)) * powf(2.0f, -10 * (2.0f * time - 1)) + 2);
			}

			break;

		case EaseINBounce:
			time = 1 - BounceOut(1 - time);
			break;
		case EaseOUTBounce:
			time = BounceOut(time);
			break;
		case EaseINOUTBounce:
			if (time < 0.5f)
				time = 0.5f * (1 - BounceOut(1 - time * 2));
			else
				time = 0.5f * BounceOut(time * 2 - 1) + 0.5f;
			break;

		default:
			break;
	}
	return (min + ((max-min) * time));
}

float CValue::BounceOut(float time)
{
	if (time < 4 / 11.0f)
	{
		time = (121 * time * time) / 16.0f;
	}
	else if (time < 8 / 11.0f)
	{
		time = (363 / 40.0f * time * time) - (99 / 10.0f * time) + 17 / 5.0f;
	}
	else if (time < 9 / 10.0f)
	{
		time = (4356 / 361.0f * time * time) - (35442 / 1805.0f * time) + 16061 / 1805.0f;
	}
	else
	{
		time = (54 / 5.0f * time * time) - (513 / 25.0f * time) + 268 / 25.0f;
	}
	return time;
}
