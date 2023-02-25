#include "LuaEditor.h"
#include "game/scenes/editor/EditorScene.h"

#include "LuaEvents.h"

void LuaEditor::load_to(sol::table& table)
{
	table.new_usertype<EditorScene>("editor_scene", sol::no_constructor,
		"veh", sol::property([](EditorScene* slf){ return slf->vehicle->veh; }),
		"edveh", sol::property([](EditorScene* slf){ return slf->vehicle.get(); }),
		"int", sol::property([](EditorScene* slf){ return &slf->vehicle_int; }));

	table.new_usertype<EditorVehicle>("editor_vehicle", sol::no_constructor);

	table.new_usertype<EditorVehicleInterface>("editor_vehicle_interface", sol::no_constructor,
	EVENT_EMITTER_SIGN_UP(EditorVehicleInterface));

}
