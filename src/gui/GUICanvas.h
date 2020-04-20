#pragma once
#include <vector>
#include "GUILayout.h"
#include <nanovg/nanovg.h>
#include <tuple>
#include "GUIInput.h"

// A canvas can contain more canvases
// A canvas with children cannot contain layouts
class GUICanvas
{
private:


public:

	// Is null if we are splitted. 
	// If one is present while splitting, child_1 will inherit it
	GUILayout* layout;

	// Only makes sense on splitted canvas, was the split 
	// horizontal or vertical?
	bool horizontal;

	// Negative numbers are ignored, set to a positive number
	// so child_0 gets exactly [x] pixels in the appropiate direction
	int child_0_pixels;

	// child_0 is the top or left split
	GUICanvas* child_0;
	// child_1 is the bottom or right split
	GUICanvas* child_1;

	float fac;
	glm::vec2 factor;
	glm::vec2 position;

	// Returns the children, first child_0, then child_1
	std::pair<GUICanvas*, GUICanvas*> divide_h(float factor);
	// Returns the children, first child_0, then child_1
	std::pair<GUICanvas*, GUICanvas*> divide_v(float factor);

	// Prepares all controls for the update, including resizing fixed-size canvases
	void prepare(glm::ivec2 pos, glm::ivec2 size, GUIInput* gui_input);

	// Resizes children space distribution appropiately
	void resize(float n_factor);

	void update_children();

	void debug(glm::ivec2 real_position, glm::ivec2 real_size, NVGcontext* vg);

	void draw(NVGcontext* vg, GUISkin* skin, glm::ivec4 def_scissor);

	GUICanvas()
	{
		position = glm::vec2(0.0f, 0.0f);
		factor = glm::vec2(1.0f, 1.0f);
		child_0 = nullptr;
		child_1 = nullptr;
		layout = nullptr;
		child_0_pixels = -1;
	}
};
