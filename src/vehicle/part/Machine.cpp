#include "Machine.h"
#include "../../util/Logger.h"
#include "../../util/SerializeUtil.h"
#include "../../assets/AssetManager.h"

#include "../Vehicle.h"

void Machine::handle_lua_error(const sol::protected_function_result& pfr, const std::string& msg, bool fatal)
{
	if (!pfr.valid())
	{
		sol::error err = pfr;
		logger->error("'{}': {}", file_name, msg);
		logger->error("{}", err.what());
		if (error_handling.is_load_fatal)
		{
			logger->fatal("Error is set to be fatal. Check that the machine's code is correct");
		}
	}
}

void Machine::deserialize(const cpptoml::table& toml)
{
	sol::protected_function  load_ptr = lua_state["load"];
	if (load_ptr.get_type() == sol::type::function)
	{
		auto result = load_ptr(toml);

		handle_lua_error(result, "Error while loading", error_handling.is_load_fatal);
	}
}

Machine::Machine(std::shared_ptr<cpptoml::table> init_toml, Vehicle* in_vehicle, Part* in_part, World* in_world)
{	
	
	this->init_toml = init_toml;
	this->vehicle = vehicle;

	cpptoml::table& init_toml_p = *init_toml;

	std::string script_path;
	SAFE_TOML_GET_FROM(init_toml_p, script_path, "script_path", std::string);
	this->file_name = script_path;

	std::string script = assets->load_string(script_path);

	// Assign all global objects 
	lua_state["machine"] = &(*this);
	lua_state["part"] = &(*in_part);
	lua_state["world"] = &(*in_world);

	// Load cor elib

	auto result = lua_state.safe_script(script, sol::script_pass_on_error);
	handle_lua_error(result, "Error while initializing", error_handling.is_init_fatal);


}

Machine::~Machine()
{
}
