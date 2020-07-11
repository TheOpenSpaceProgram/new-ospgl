#include "LuaFlightInput.h"
#include <game/scenes/flight/FlightInput.h>
#include <assets/AssetManager.h>

void LuaFlightInput::load_to(sol::table& table)
{	
	table.new_usertype<InputContext>("context",
		"new", [](const std::string& resource_path, sol::this_environment this_env)
		{
			sol::environment& st = this_env;
			InputContext out = InputContext();

			// We load an asset, so set the current package to the script's package
			std::string old_pkg = assets->get_current_package();
			assets->set_current_package(st["__pkg"]);
			out.load_from_file(resource_path);
			assets->set_current_package(old_pkg);

			return out;
		},
		"get_axis", &InputContext::get_axis,
		"get_action", &InputContext::get_action,
		"get_action_down", &InputContext::get_action_down,
		"get_action_up", &InputContext::get_action_up,
		"set_axis", &InputContext::set_axis);	
}
