#ifndef GAME_CLIENT_ANIM_UI_VALUE_H
#define GAME_CLIENT_ANIM_UI_VALUE_H

#include <base/vmath.h>

#include <functional>

class CValue
{
	typedef std::function<void()> AnimCallback;
	AnimCallback m_Callback;

public:
	bool m_AnimEnded;

	CValue();
	void Init(class CUIRect* pRect);
	void Anim(class CUIRect* pRect, float Time, int AnimType, bool BackAnimation = false, bool RepairAnimation = false);
	void Init(vec4 Value);
	void Anim(vec4 Value, float Time, int AnimType, bool BackAnimation = false, bool RepairAnimation = false);
	void SetAnimEndCallback(AnimCallback Callback);
	void RepairAnimation();
	void Recalculate();
	void EndAnimation();
	void PreRender();

	vec4 GetValue() const { return m_Value; }
	class CUIRect GetRect() const;

private:
	bool m_RepairAnimation;
	bool m_BackAnimation;
	double m_AnimTime;
	double m_AnimEndTime;
	int m_Animation;
	vec4 m_Value, m_NewValue, m_OldValue;
	vec4 Animation(int AnimType, vec4 min, vec4 max, float time);

	float BounceOut(float time);
};

#endif