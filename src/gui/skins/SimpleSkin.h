#pragma once 
#include "../GUISkin.h"

// A simple skin used in most interfaces, inspired
// by the look of FreeOrion
class SimpleSkin : public GUISkin 
{
public:

	virtual void draw_button(NVGcontext* v, glm::ivec2 pos, glm::ivec2 size, const std::string& text, 
		ButtonState state, ButtonStyle style = ButtonStyle::NORMAL) override;

};
