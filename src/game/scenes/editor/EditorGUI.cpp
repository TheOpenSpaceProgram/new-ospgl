#include "EditorGUI.h"
#include <glm/glm.hpp>

#include <gui/layouts/GUIVerticalLayout.h>
#include <gui/widgets/GUIImageButton.h>
#include "EditorScene.h"
#include <OSP.h>

void EditorGUI::do_gui(int width, int height, GUIInput* gui_input)
{
	float w = (float)width; float h = (float)height;

	// Draw the side pane
	nvgBeginPath(vg);
	float swidth = glm::max(side_width * w, (float)minimum_side);
	nvgRect(vg, 0.0f, 0.0f, swidth, 1.0f * h);
	nvgFillColor(vg, nvgRGB(30, 35, 40));
	nvgFill(vg);

	def_panel.prepare(glm::ivec2(0, 0), glm::ivec2(swidth, height), gui_input);	
	def_panel.draw(vg, glm::ivec4(0, 0, width, height));
	//def_panel.debug(glm::ivec2(0, 0), glm::ivec2(swidth, height), vg);

	prev_width = width;
	prev_height = height;
}



void EditorGUI::init(EditorScene* sc)
{
	prev_width = 0;
	prev_height = 0;

	def_panel.divide_v(0.05);
	def_panel.child_0_pixels = 32;
	def_panel.child_1->layout = new GUIVerticalLayout();

	GameDatabase* gdb = &sc->get_osp()->game_database;

	for(int i = 0; i < gdb->parts.size(); i++)
	{
		GUIImageButton* btn = new GUIImageButton();
		def_panel.child_1->layout->add_widget(btn);
		btn->name = gdb->parts[i];
	}
}

