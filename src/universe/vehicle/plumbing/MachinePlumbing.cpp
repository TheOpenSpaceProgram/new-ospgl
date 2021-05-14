#include "MachinePlumbing.h"
#include "../part/Machine.h"
#include <util/LuaUtil.h>
#include <util/serializers/glm.h>

float MachinePlumbing::get_pressure(std::string port)
{
	return 1.0f;
}

glm::ivec2 MachinePlumbing::get_editor_size()
{
	logger->check(has_plumbing(), "Cannot use plumbing functions on machines without plumbing");

	auto result = LuaUtil::safe_call_function(get_lua_plumbing()["get_editor_size"]);
	if(result.valid())
	{
		if(result.get_type() == sol::type::userdata)
		{
			glm::dvec2 dvec = result.get<glm::dvec2>();
			return glm::ivec2(glm::round(dvec));
		}
		else
		{
			// It's a pair of numbers
			std::pair<int, int> pair = result.get<std::pair<int, int>>();
			return glm::ivec2(pair.first, pair.second);
		}
	}
	else
	{
		logger->warn("Could not find function get_editor_size in plumbing");
		return glm::ivec2(0,0);
	}
}

void MachinePlumbing::draw_diagram(void *vg)
{
	logger->check(has_plumbing(), "Cannot use plumbing functions on machines without plumbing");
	LuaUtil::safe_call_function(get_lua_plumbing()["draw_diagram"], vg);
}

sol::table MachinePlumbing::get_lua_plumbing()
{
	logger->check(has_plumbing(), "Cannot use plumbing functions on machines without plumbing");

	auto value = machine->env["plumbing"];
	if(value.valid() && value.get_type() == sol::type::table)
	{
		return value.get<sol::table>();
	}
	else
	{
		logger->fatal("Couldn't find plumbing table on a machine with plumbing");
		return sol::table();
	}
}

void MachinePlumbing::init(const cpptoml::table& init)
{
	auto fluid_ports_toml = init.get_table_array("fluid_port");
	if(fluid_ports_toml)
	{
		for(auto& port : *fluid_ports_toml)
		{
			FluidPort n_port;
			SAFE_TOML_GET_FROM(*port, n_port.id, "id", std::string);
			SAFE_TOML_GET_FROM(*port, n_port.marker, "marker", std::string);


			// Check that the name is not present already
			bool found = false;
			for(auto& fluid_port : fluid_ports)
			{
				if(fluid_port.id == n_port.id)
				{
					logger->error("Duplicated port name '{}', ignoring", n_port.id);
					found = true;
					break;
				}
			}

			if(!found)
			{
				fluid_ports.push_back(n_port);
			}
		}
	}

	glm::dvec2 offset = glm::dvec2(0.0);
	auto fluid_offset = init.get_table("fluid_offset");
	if(fluid_offset)
	{
		SerializeUtil::read_to(*fluid_offset, offset);
	}
	editor_position = glm::round(offset);
	if(editor_position.x < 0 || editor_position.y < 0)
	{
		logger->warn("Machines cannot have fluid_offsets below (0,0). Clamping!");
		editor_position = glm::ivec2(0, 0);
	}


	if(has_plumbing())
	{
		// Let machine init, it should check and store ports
		// We create a sol table for convenience to the lua code (so it can be copied over without trouble)
		// indices are fluid_ports ids and they point to the marker
		sol::table tb = sol::state_view(machine->env.lua_state()).create_table();

		for(auto& fluid_port : fluid_ports)
		{
			tb[fluid_port.id] = fluid_port.marker;
		}

		LuaUtil::call_function_if_present(get_lua_plumbing()["init"], tb);
	}
}
