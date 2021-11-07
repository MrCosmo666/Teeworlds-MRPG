#ifndef GAME_CLIENT_ANIM_UI_BLOCK_H
#define GAME_CLIENT_ANIM_UI_BLOCK_H

#include "elements.h"

class CBlock : public CAnimElementsUI
{
public:
    CBlock(class CAnimUI *pAnimUI, const char *Name);

    void Render() override;
    void Set(float RoundCorner, int CornerType, bool MonochromeGradient) override;

private:
	float m_RoundCorner;
	int m_CornerType;
	bool m_MonochromeGradient;
};


#endif