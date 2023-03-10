#include "SymmetryMode.h"
#include <game/scenes/editor/interfaces/ModifyInterface.h>

void SymmetryMode::do_gui(std::shared_ptr<GUICanvas> to_canvas, ModifyInterface *mod_int)
{
	LuaUtil::call_function_if_present(env["do_gui"], to_canvas, mod_int);
}

void SymmetryMode::init(sol::state *in_state, const std::string &in_pkg)
{
	auto[pkg, name] = osp->assets->get_package_and_name(script_path, in_pkg);

	env = sol::environment(*in_state, sol::create, in_state->globals());
	lua_core->load((sol::table&)env, pkg);
	env["symmetric_group"] = this;
	env["osp"] = osp;

	std::string full_path = osp->assets->res_path + pkg + "/" + name;
	auto result = (*in_state).safe_script_file(full_path, env);
	if(!result.valid())
	{
		sol::error err = result;
		logger->fatal("Lua Error loading symmetry mode:\n{}", err.what());
	}
}

void SymmetryMode::on_disconnect(Piece *piece)
{
	LuaUtil::call_function_if_present(env["on_disconnect"], piece);

}

void SymmetryMode::on_dirty(Piece *piece)
{
	LuaUtil::call_function_if_present(env["on_dirty"], piece);
}
