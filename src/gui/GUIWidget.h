#pragma once
#include "glm/detail/type_vec.hpp"
#include <glm/glm.hpp>
#include <nanovg/nanovg.h>
#include "GUIInput.h"
#include "util/Signal.h"
#include "GUISkin.h"

// Base class for GUIWidgets, which are contained
// (and managed) by a GUILayout
class GUIWidget
{
protected:

	// Remember to write these from prepare!
	glm::ivec2 pos;
	glm::ivec2 size;

public:

	// Set by the Layout during prepare
	bool is_visible;	

	// If the widget can block the mouse, this can disable that functionality
	bool blocks_mouse = true;

	

	// Return the size you used
	// 'wsize' may have negative coordinates, these mean you are free to take
	// as much space as required
	// Remember: Store widget_pos and used size for drawing, we provide the
	// pos and size variables, but you must write to them!
	// This may be called multiple times in the same frame on some cases! For example,
	// if an element needs to be blocked and block at the same time (good example is the plumbing editor)
	virtual glm::ivec2 prepare(glm::ivec2 wpos, glm::ivec2 wsize, glm::ivec4 viewport, GUIInput* gui_input) = 0;
	virtual void draw(NVGcontext* ctx, GUISkin* skin) = 0;

	virtual ~GUIWidget(){};

};
