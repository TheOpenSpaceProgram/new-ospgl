#pragma once 
#include <glm/glm.hpp>
#include <string>
#include <nanovg/nanovg.h>

// Widgets call GUISkin::draw_x to draw different skinned primitives,
// this way multiple GUI skins are possible.
class GUISkin
{
public:

	enum class ButtonState
	{	
		NORMAL,
		DISABLED,
		HOVERED,
		CLICKED
	};

	enum class ButtonStyle
	{
		NORMAL,		//< Wathever the skin wants to do
		SYMMETRIC,	//< The button must be symmetric on all axis, usually used in tiling
		GOOD,		//< Normal but with GOOD motives (green color, checkmark...)
		BAD    		//< Normal but with BAD motives (red color, cross...)
	};

	virtual void draw_button(NVGcontext* vg, glm::ivec2 pos, glm::ivec2 size, const std::string& text, 
			ButtonState state, ButtonStyle style = ButtonStyle::NORMAL) = 0;

};
