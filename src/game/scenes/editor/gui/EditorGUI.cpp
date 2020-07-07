#include "EditorGUI.h"
#include <glm/glm.hpp>

#include <gui/layouts/GUIVerticalLayout.h>
#include <gui/layouts/GUIListLayout.h>
#include <gui/widgets/GUIImageButton.h>
#include "../EditorScene.h"
#include <OSP.h>


int EditorGUI::get_panel_width()
{
	return part_list.get_panel_width();
}

void EditorGUI::do_gui(int width, int height, GUIInput* gui_input)
{
	float w = (float)width; float h = (float)height;

	// Draw the side pane
	float swidth = (float)get_panel_width();

	nvgBeginPath(vg);
	nvgRect(vg, 0.0f, 0.0f, swidth, 1.0f * h);
	nvgFillColor(vg, nvgRGB(30, 35, 40));
	nvgFill(vg);

	
	if(edveh_int->selected == nullptr)
	{
		part_list.do_gui(width, get_panel_width(), height, gui_input);
	}
	else
	{
		trashcan.do_gui(width, get_panel_width(), height, gui_input);
	}

	prev_width = width;
	prev_height = height;
}



void EditorGUI::init(EditorScene* sc)
{	
	prev_width = 0;
	prev_height = 0;
	// We hold a pointer to interact with the 3D editor
	this->edveh_int = &sc->vehicle_int;

	part_list.init(sc, vg, &skin);
	trashcan.init(sc, vg, &skin);

}


