#include "SymmetryMode.h"
#include <game/scenes/editor/interfaces/ModifyInterface.h>
#include <game/scenes/editor/gui/ModifyPanel.h>
#include <game/scenes/editor/EditorVehicleInterface.h>

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

void SymmetryMode::init(sol::state *in_state, const std::string &in_pkg)
{
	auto[pkg, name] = osp->assets->get_package_and_name(script_path, in_pkg);

	env = sol::environment(*in_state, sol::create, in_state->globals());
	lua_core->load((sol::table&)env, pkg);
	env["symmetry_mode"] = this;
	env["osp"] = osp;

	std::string full_path = osp->assets->res_path + pkg + "/" + name;
	auto result = (*in_state).safe_script_file(full_path, env);
	if(!result.valid())
	{
		sol::error err = result;
		logger->fatal("Lua Error loading symmetry mode:\n{}", err.what());
	}
}

