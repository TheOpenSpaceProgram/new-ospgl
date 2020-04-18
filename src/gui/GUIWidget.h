#pragma once
#include "glm/detail/type_vec.hpp"
#include <glm/glm.hpp>
#include <nanovg/nanovg.h>
#include "GUIInput.h"

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

	// Return the size you used
	// 'size' may have negative coordinates, these mean you are free to take
	// as much space as required
	// Remember: Store widget_pos and used size for drawing, we provide the
	// pos and size variables, but you must write to them!
	virtual glm::ivec2 prepare(glm::ivec2 wpos, glm::ivec2 wsize, GUIInput* gui_input) = 0;
	virtual void draw(NVGcontext* ctx) = 0;

};
