#include "EditorGUI.h"
#include <glm/glm.hpp>

void EditorGUI::do_gui(int width, int height)
{
	float w = (float)width; float h = (float)height;

	// Draw the side pane
	nvgBeginPath(vg);
	float swidth = glm::max(side_width * w, (float)minimum_side);
	nvgRect(vg, 0.0f, 0.0f, swidth, 1.0f * h);
	nvgFillColor(vg, nvgRGB(30, 35, 40));
	nvgFill(vg);

	/*if(width != prev_width || height != prev_height)
	{*/
		// We use some fixed size layouts (the search bar) so this is needed
		prepare_def_panel(width, height);
	//}

	def_panel.debug(glm::ivec2(0, 0), glm::ivec2(swidth, height), vg);

	prev_width = width;
	prev_height = height;
}

static float t = 0.0f;

void EditorGUI::prepare_def_panel(int width, int height)
{
	def_panel.resize((sin(t) + 1.0f) * 0.5);

	t += 0.01f;
}

EditorGUI::EditorGUI()
{
	prev_width = 0;
	prev_height = 0;

	GUICanvas* sub = def_panel.divide_v(0.5f);
	sub->divide_v(0.5f);
	GUICanvas* subsub = def_panel.child_1->divide_v(0.5f);
	subsub->divide_h(0.5f);

}

