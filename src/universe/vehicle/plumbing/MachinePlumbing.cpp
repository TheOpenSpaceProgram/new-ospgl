#include "MachinePlumbing.h"
#include "../part/Machine.h"
#include <util/LuaUtil.h>
#include <util/serializers/glm.h>

float MachinePlumbing::get_pressure(const std::string& port)
{
	logger->check(has_lua_plumbing(), "Cannot use plumbing functions on machines without plumbing");

	auto result = LuaUtil::safe_call_function(get_lua_plumbing()["get_pressure"], port);
	logger->check(result.valid(), "get_pressure failed, this is fatal");

	return result.get<float>();
}

float MachinePlumbing::get_free_volume(const std::string& port)
{
	logger->check(has_lua_plumbing(), "Cannot use plumbing functions on machines without plumbing");

	auto result = LuaUtil::safe_call_function(get_lua_plumbing()["get_free_volume"], port);
	logger->check(result.valid(), "get_free_volume failed, this is fatal");

	return result.get<float>();
}

StoredFluids MachinePlumbing::in_flow(std::string port, const StoredFluids &in)
{
	return StoredFluids();
}

StoredFluids MachinePlumbing::out_flow(std::string port, float volume)
{
	return StoredFluids();
}

// TODO: We could cache this too
glm::ivec2 MachinePlumbing::get_editor_size(bool expand, bool rotate) const
{
	logger->check(has_lua_plumbing(), "Cannot use plumbing functions on machines without plumbing");
	glm::ivec2 ret = base_size;

	if(expand)
	{
		ret += glm::ivec2(2, 2);
	}

	// Rotation
	if(rotate)
	{
		if (editor_rotation == 1 || editor_rotation == 3)
		{
			std::swap(ret.x, ret.y);
		}
	}

	return ret;
}

void MachinePlumbing::draw_diagram(void *vg)
{
	logger->check(has_lua_plumbing(), "Cannot use plumbing functions on machines without plumbing");
	LuaUtil::safe_call_function(get_lua_plumbing()["draw_diagram"], vg);
}

sol::table MachinePlumbing::get_lua_plumbing(bool silent_fail)
{
	auto value = machine->env["plumbing"];
	if(value.valid() && value.get_type() == sol::type::table)
	{
		return value.get<sol::table>();
	}
	else
	{
		if(!silent_fail)
		{
			logger->fatal("Couldn't find plumbing table on a machine with plumbing");
		}
		return sol::table();
	}
}

void MachinePlumbing::init(const cpptoml::table& init)
{
	// std::cout << init << std::endl;
	// Read plumbing_pos and rot from init if present
	bool found_pos = false;
	auto pos_table = init.get_table("plumbing_pos");
	if(pos_table)
	{
		deserialize(editor_position, *pos_table);
		found_pos = true;
	}
	editor_rotation = init.get_as<int>("plumbing_rot").value_or(editor_rotation);

	if(!found_pos)
	{
		glm::dvec2 offset = glm::dvec2(0.0);
		auto fluid_offset = init.get_table("fluid_offset");
		if (fluid_offset)
		{
			SerializeUtil::read_to(*fluid_offset, offset);
		}
		editor_position = glm::round(offset);
		if (editor_position.x < 0 || editor_position.y < 0)
		{
			logger->warn("Machines cannot have fluid_offsets below (0,0). Clamping!");
			editor_position = glm::ivec2(0, 0);
		}
	}


	has_lua = false;
	auto value = machine->env["plumbing"];
	if(value.valid() && value.get_type() == sol::type::table)
	{
		has_lua = true;
	}

	if(has_lua_plumbing())
	{

		can_add_ports = true;
		auto result = LuaUtil::safe_call_function_if_present(get_lua_plumbing()["init"]);
		if(result.has_value())
		{
			if(result->get_type() == sol::type::userdata)
			{
				glm::dvec2 dvec = result->get<glm::dvec2>();
				base_size = glm::ivec2(glm::round(dvec));
			}
			else
			{
				// It's a pair of numbers
				std::pair<int, int> pair = result->get<std::pair<int, int>>();
				base_size = glm::ivec2(pair.first, pair.second);
			}
		}
		can_add_ports = false;
	}
}

void MachinePlumbing::create_port(std::string id, std::string marker, std::string name, float x, float y)
{
	logger->check(can_add_ports, "Cannot add port ({}) currently, machine is already init", id);

	// Check that ID is available
	for(const FluidPort& port : fluid_ports)
	{
		if(port.id == id)
		{
			logger->fatal("Cannot add ports with duplicate id: {}", id);
		}
	}

	FluidPort port;
	port.pos = glm::vec2(x, y);
	port.marker = marker;
	port.gui_name = name;
	port.id = id;

	fluid_ports.push_back(port);

}

glm::vec2 MachinePlumbing::get_port_position(std::string id)
{
	for(const FluidPort& port : fluid_ports)
	{
		if(port.id == id)
		{
			glm::vec2 pos = port.pos;

			glm::ivec2 size = get_editor_size();
			if(editor_rotation == 1)
			{
				std::swap(pos.x, pos.y);
				pos.x = size.x - pos.x;
			}
			else if(editor_rotation == 2)
			{
				pos.x = (float)size.x - pos.x;
				pos.y = (float)size.y - pos.y;
			}
			else if(editor_rotation == 3)
			{
				std::swap(pos.x, pos.y);
				pos.y = (float)size.y - pos.y;
			}

			pos += editor_position;

			return pos;
		}
	}

	logger->fatal("Unknown port id={}", id);
	return glm::vec2(0, 0);
}



