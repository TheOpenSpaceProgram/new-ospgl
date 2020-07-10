#include "Machine.h"
#include <util/Logger.h>
#include <util/SerializeUtil.h>
#include <assets/AssetManager.h>

#include "../Vehicle.h"
#include "Part.h"
#include "sol.hpp"


Machine::Machine(std::shared_ptr<cpptoml::table> init_toml, std::string cur_pkg)
{	
	this->init_toml = init_toml;
	logger->check(init_toml != nullptr, "Malformed init_toml");
	cpptoml::table& init_toml_p = *init_toml;

	std::string script_path;
	SAFE_TOML_GET_FROM(init_toml_p, script_path, "script", std::string);

	lua_state["machine"] = this;

	std::string old = assets->get_current_package();
	assets->set_current_package(cur_pkg);
	assets->load_script_to(lua_state, script_path);
	assets->set_current_package(old);

}

void Machine::load_interface(const std::string& name, sol::table n_table) 
{
	auto it = interfaces.find(name);	
	// We do this check anyway to try to keep the code consistent and avoid
	// multiple interface loads (of the same interface) in the same machine
	if(it != interfaces.end())
	{
		logger->fatal("Tried to load an interface which was already loaded ({})", name);
	}

	interfaces.insert(std::make_pair(name, n_table));
}

void Machine::pre_update(double dt)
{
	LuaUtil::call_function_if_present(lua_state, "pre_update", "machine pre_update", dt);
}

void Machine::update(double dt)
{
	LuaUtil::call_function_if_present(lua_state, "update", "machine update", dt);
}

void Machine::editor_update(double dt)
{
	LuaUtil::call_function_if_present(lua_state, "editor_update", "machine editor_update", dt);
}


void Machine::init(Part* in_part, Universe* in_universe)
{
	lua_state["part"] = in_part;
	lua_state["universe"] = in_universe;
	lua_state["vehicle"] = in_part->vehicle;

	this->in_part = in_part;

	LuaUtil::call_function_if_present(lua_state, "init", "machine init");
	
}

std::vector<Machine*> Machine::get_all_connected()
{
	// TODO: We could cache this? Could be a small perfomance gain
	return get_connected_if([](Machine* m){ return true; });
}

std::vector<Machine*> Machine::get_connected_with(const std::vector<std::string>& interfaces)
{
	return get_connected_if([interfaces](Machine* m)
	{
		for(const std::string& a : interfaces)
		{
			if(m->interfaces.find(a) != m->interfaces.end())
			{
				return true;
			}
		}

		return false;
	});
}


sol::table Machine::get_interface(const std::string& name) 
{
	auto it = interfaces.find(name);
	if(it == interfaces.end())
	{
		return sol::nil;
	}
	else
	{
		return it->second;
	}
}

Machine::~Machine()
{
	logger->info("Ending machine");
	lua_state.collect_garbage();
}

std::vector<Machine*> Machine::get_connected_if(std::function<bool(Machine*)> fnc) 
{
	std::vector<Machine*> out;

	auto range = in_part->vehicle->wires.equal_range(this);
	for(auto it = range.first; it != range.second; it++)
	{
		if(fnc(it->second))
		{
			out.push_back(it->second);
		}
	}

	return out;
}
