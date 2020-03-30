#include "Machine.h"
#include "../../util/Logger.h"
#include "../../util/SerializeUtil.h"
#include "../../assets/AssetManager.h"

#include "../Vehicle.h"


Machine::Machine(std::shared_ptr<cpptoml::table> init_toml)
{		
	cpptoml::table& init_toml_p = *init_toml;

	std::string script_path;
	SAFE_TOML_GET_FROM(init_toml_p, script_path, "script", std::string);

	lua_state = assets->load_script(script_path);

	// Assign all global objects 
	lua_state["machine"] = &(*this);

}

void Machine::update(double dt)
{
	LuaUtil::call_function(lua_state, "update", "machine update", dt);
}

void Machine::init(Part* in_part, Universe* in_universe)
{
	lua_state["part"] = in_part;
	lua_state["universe"] = in_part;
	lua_state["vehicle"] = in_part->vehicle;	
}

Machine::~Machine()
{
}
