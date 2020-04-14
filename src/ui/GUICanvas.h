#pragma once
#include <vector>
#include "GUILayout.h"
#include <nanovg/nanovg.h>

// A canvas can contain more canvases
// A canvas with children cannot contain layouts
class GUICanvas
{
private:


public:

	// Only makes sense on splitted canvas, was the split 
	// horizontal or vertical?
	bool horizontal;

	// child_0 is the right or bottom split
	GUICanvas* child_0;
	// child_1 is the top or left split
	GUICanvas* child_1;

	float fac;
	glm::vec2 factor;
	glm::vec2 position;

	// Returns the right / bottom child
	GUICanvas* divide_h(float factor);
	// Returns the right / bottom child
	GUICanvas* divide_v(float factor);

	// Resizes children space distribution appropiately
	void resize(float n_factor);

	void update_children();

	void debug(glm::ivec2 real_position, glm::ivec2 real_size, NVGcontext* vg);

	GUICanvas()
	{
		position = glm::vec2(0.0f, 0.0f);
		factor = glm::vec2(1.0f, 1.0f);
		child_0 = nullptr;
		child_1 = nullptr;
	}
};
