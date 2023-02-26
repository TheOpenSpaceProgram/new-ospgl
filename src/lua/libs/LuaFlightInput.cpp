#include "LuaFlightInput.h"
#include <game/scenes/flight/InputContext.h>
#include <assets/AssetManager.h>

void LuaFlightInput::load_to(sol::table& table)
{	
	table.new_usertype<InputContext>("context",
		"new", [](const std::string& resource_path, sol::this_environment this_env)
		{
			sol::environment& st = this_env;
			std::shared_ptr<InputContext> out = std::make_shared<InputContext>();

			// We load an asset, so set the current package to the script's package
			std::string old_pkg = osp->assets->get_current_package();
			osp->assets->set_current_package(st["__pkg"]);
			out->load_from_file(resource_path);
			osp->assets->set_current_package(old_pkg);

			return out;
		},
		"get_axis", &InputContext::get_axis,
		"get_action", &InputContext::get_action,
		"get_action_down", &InputContext::get_action_down,
		"get_action_up", &InputContext::get_action_up,
		"set_axis", &InputContext::set_axis,
		"update", &InputContext::update,
		"is_same_as", &InputContext::is_same_as);
}
