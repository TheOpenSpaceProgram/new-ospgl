#pragma once 
#include <glm/glm.hpp>
#include <string>
#include <nanovg/nanovg.h>

class GUIWindow;

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

	// These functions are only called if the window allows said functionality,
	// so don't check it here. Return true if said action can be done, for example,
	// return true on can_drag_window if the mouse is over the titlebar
	virtual bool can_drag_window(GUIWindow* window, glm::ivec2 mpos) = 0;
	virtual bool can_close_window(GUIWindow* window, glm::ivec2 mpos) = 0;
	virtual bool can_minimize_window(GUIWindow* window, glm::ivec2 mpos) = 0;
	virtual bool can_pin_window(GUIWindow* window, glm::ivec2 mpos) = 0;
	virtual glm::ivec4 get_window_aabb(GUIWindow* window) = 0;

	virtual void draw_window(NVGcontext* vg, GUIWindow* window) = 0;

};
