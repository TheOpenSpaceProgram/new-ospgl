#include "SymmetryMode.h"
#include <game/scenes/editor/interfaces/ModifyInterface.h>
#include <game/scenes/editor/gui/ModifyPanel.h>
#include <game/scenes/editor/EditorVehicleInterface.h>
#include <game/scenes/editor/EditorScene.h>


void SymmetryMode::take_gui_control(ModifyPanel* panel, ModifyInterface* mod_int, EditorVehicleInterface* edveh_int)
{
	env["symmetry_panel"] = panel;
	env["modify_interface"] = mod_int;
	env["edveh_interface"] = edveh_int;
	LuaUtil::call_function_if_present(env["take_gui_control"]);
}

void SymmetryMode::leave_gui_control()
{
	LuaUtil::call_function_if_present(env["leave_gui_control"]);
	env["symmetry_panel"] = nullptr;
	env["modify_interface"] = nullptr;
}

void SymmetryMode::init(sol::state *in_state, EditorVehicle* in_vehicle, const std::string &in_pkg)
{
	this->edveh = in_vehicle;
	this->sc = in_vehicle->scene;

	auto[pkg, name] = osp->assets->get_package_and_name(script_path, in_pkg);

	env = sol::environment(*in_state, sol::create, in_state->globals());
	lua_core->load((sol::table&)env, pkg);
	env["symmetry_mode"] = this;
	env["vehicle"] = in_vehicle->veh;
	env["osp"] = osp;

	std::string full_path = osp->assets->res_path + pkg + "/" + name;
	auto result = (*in_state).safe_script_file(full_path, env);
	if(!result.valid())
	{
		sol::error err = result;
		logger->fatal("Lua Error loading symmetry mode:\n{}", err.what());
	}
}

std::vector<Piece*> SymmetryMode::make_clones(int count)
{
	Vehicle* veh = root->in_vehicle;
	// Remove all old clones, except first one (root)
	for(int i = 1; i < clones.size(); i++)
	{
		std::vector<Piece*> child = veh->get_children_of(clones[i]);
		for(Piece* p : child)
		{
			veh->remove_piece(p);
		}
		veh->remove_piece(clones[i]);
	}

	std::vector<Piece*> out;
	std::vector<Piece*> all_new_pieces;

	out.push_back(root);
	for(int i = 1; i < count; i++)
	{
		Piece* new_root = veh->duplicate(root, &sc->lua_state, &sc->piece_id, &sc->part_id);
		out.push_back(new_root);
		all_new_pieces.push_back(new_root);
		std::vector<Piece*> new_child = veh->get_children_of(new_root);
		all_new_pieces.insert(all_new_pieces.end(), new_child.begin(), new_child.end());
	}
	clones = out;

	for(Piece* p : all_new_pieces)
	{
		edveh->update_collider(p);
	}

	return out;
}

