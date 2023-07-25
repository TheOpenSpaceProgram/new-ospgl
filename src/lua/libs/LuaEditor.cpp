#include "LuaEditor.h"
#include "game/scenes/editor/EditorScene.h"

#include "LuaEvents.h"

void LuaEditor::load_to(sol::table& table)
{
	table.new_usertype<EditorScene>("editor_scene", sol::no_constructor,
		"veh", sol::property([](EditorScene* slf){ return slf->vehicle->veh; }),
		"edveh", sol::property([](EditorScene* slf){ return slf->vehicle.get(); }),
		"int", sol::property([](EditorScene* slf){ return &slf->vehicle_int; }),
		"gui_screen", &EditorScene::gui_screen,
		"cam", &EditorScene::cam,
		"get_viewports", [](EditorScene* s)
		{
			auto[r, viewport, real_screen_size, gui_vport] = s->get_viewports();
			return std::make_tuple((glm::dvec2)r, (glm::dvec4)viewport, (glm::dvec2)real_screen_size, (glm::dvec4)gui_vport);
		},
		EVENT_EMITTER_SIGN_UP(EditorScene));

	table.new_usertype<EditorVehicle>("editor_vehicle", sol::no_constructor,
		  "update_collider", &EditorVehicle::update_collider,
		  "update_collider_hierarchy", &EditorVehicle::update_collider_hierarchy,
		  "attach", &EditorVehicle::attach,
		  "set_piece_highlight", [](EditorVehicle* edveh, Piece* p, glm::dvec3 h)
		  {
			edveh->piece_meta[p].highlight = h;
		  });
	table.new_usertype<EditorCamera>("editor_camera", sol::no_constructor,
		 "get_camera_uniforms", &EditorCamera::get_camera_uniforms);

	table.new_usertype<EditorVehicleInterface>("editor_vehicle_interface", sol::no_constructor,
	EVENT_EMITTER_SIGN_UP(EditorVehicleInterface));

	table.new_usertype<ModifyPanel>("modify_panel", sol::no_constructor,
			"set_canvas", &ModifyPanel::set_symmetry_canvas);

	table.new_usertype<ModifyInterface>("modify_interface", sol::no_constructor,
				"start_picking_piece", &ModifyInterface::start_picking_piece);


}
