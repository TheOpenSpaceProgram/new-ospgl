#include "WiringPanel.h"
#include "../EditorScene.h"
#include "EditorGUI.h"
#include <algorithm>

void WiringPanel::init(EditorScene *sc, NVGcontext *vg)
{
	this->scene = sc;
	this->vg = vg;
	this->edgui = &sc->gui;

	group_dropdown = std::make_shared<GUIDropDown>();
	update_logical_groups();
	group_dropdown->item = 0;
	scene->vehicle_int.wire_interface.editing = scene->vehicle->veh->logical_groups[group_dropdown->options[0].first];



	// Group selector
	panel.divide_v(0.05);
	panel.child_pixels = 32;
	panel.pixels_for_child_1 = false;

	panel.child_0->layout = std::make_shared<GUISingleLayout>();
	panel.child_0->layout->add_widget(group_dropdown);

}

void WiringPanel::add_gui(int width, int panel_width, int height, GUIScreen *screen)
{
	screen->add_canvas(&panel, glm::ivec2(0, 0), glm::ivec2(panel_width, height));
}

void WiringPanel::update_logical_groups()
{
	for(auto& p : scene->vehicle->veh->logical_groups)
	{
		group_dropdown->options.emplace_back(p.first, p.second->display_str);
	}
	// Last option to allow the user to create logical groups
	group_dropdown->options.emplace_back("manage", "Manager user logical groups");
	group_dropdown->update_options();
}
