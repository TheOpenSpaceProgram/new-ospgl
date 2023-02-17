#pragma once
#include "glm/detail/type_vec.hpp"
#include <glm/glm.hpp>
#include <nanovg/nanovg.h>
#include "GUIInput.h"
#include "GUISkin.h"

class GUIScreen;

// Base class for GUIWidgets, which are contained
// (and managed) by a GUILayout
class GUIWidget
{
protected:

	// Remember to write these from prepare!
	glm::ivec2 pos;
	glm::ivec2 size;

public:

	// If the widget is allowed to be freely sized, what size should we use? (per_axis)
	glm::ivec2 default_size;

	// Set by the Layout during prepare
	bool is_visible;	

	// If the widget can block the mouse, this can disable that functionality
	bool blocks_mouse = true;

	glm::ivec2 default_position(glm::ivec2 wpos, glm::ivec2 wsize)
	{
		pos = wpos;
		size = wsize;
		if(size.x < 0)
		{
			size.x = default_size.x;
		}

		if(size.y < 0)
		{
			size.y = default_size.y;
		}

		return size;
	}

	// Return the size you used
	// 'wsize' may have negative coordinates, these mean you are free to take
	// as much space as required
	// Remember: Store widget_pos and used size for drawing, we provide the
	// pos and size variables, but you must write to them!
	// Called bottom-to-top, same as draw order
	virtual glm::ivec2 position(glm::ivec2 wpos, glm::ivec2 wsize, GUIScreen* screen) = 0;

	// This will be called two times, once with execute_user_actions = false where you must
	// block the 3D scene, and another with execute_user_actions = true where you may be blocekd
	// by the 3D scene. Finally, this is called top-to-bottom (opposite to draw order)
	virtual void prepare(glm::ivec4 viewport, GUIScreen* screen, GUIInput* gui_input) = 0;

	// This is called bottom-to-top (painter's algorithm)
	virtual void draw(NVGcontext* ctx, GUISkin* skin) = 0;

	// Generate any overlay canvases here. May not be needed.
	virtual void pre_prepare(GUIScreen* screen) {};

	virtual ~GUIWidget(){};

};
