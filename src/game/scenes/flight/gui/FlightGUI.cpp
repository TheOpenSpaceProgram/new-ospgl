#include "FlightGUI.h"
#include <game/scenes/flight/FlightScene.h>
#include <universe/Universe.h>
#include <universe/entity/entities/VehicleEntity.h>
#include <gui/layouts/GUISingleLayout.h>

void FlightGUI::init(FlightScene* scene)
{
	plumbing = window_manager.create_window();
	plumbing->pos = glm::ivec2(20, 30);
	plumbing->size = glm::ivec2(400, 400);
	plumbing->title = "Plumbing";

	// Add the plumbing editor
	pb_editor = new PlumbingEditorWidget();
	VehicleEntity* v_ent =  scene->universe->get_entity_as<VehicleEntity>(2);
	pb_editor->pb_editor.veh = v_ent->vehicle;
	plumbing->canvas.layout = new GUISingleLayout();
	plumbing->canvas.layout->add_widget(pb_editor);

}

void FlightGUI::prepare_gui(int width, int height, GUIInput *gui_input)
{
	window_manager.prepare(gui_input, &skin);
}

void FlightGUI::do_gui(int width, int height)
{
	window_manager.viewport = glm::ivec4(0, 0, width, height);
	window_manager.draw(vg, &skin);
}
