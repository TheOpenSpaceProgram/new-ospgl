#include "PlumbingMachine.h"
#include "../part/Machine.h"
#include <util/LuaUtil.h>
#include <util/serializers/glm.h>
#include <gui/GUISkin.h>

float PlumbingMachine::get_pressure(const std::string& port)
{
	logger->check(has_lua_plumbing(), "Cannot use plumbing functions on machines without plumbing");

	auto result = LuaUtil::safe_call_function(get_lua_plumbing()["get_pressure"], port);
	logger->check(result.valid(), "get_pressure failed, this is fatal");

	return result.get<float>();
}

StoredFluids PlumbingMachine::in_flow(std::string port, const StoredFluids &in, bool do_flow)
{
	logger->check(has_lua_plumbing(), "Cannot use plumbing functions on machines without plumbing");

	auto result = LuaUtil::safe_call_function(get_lua_plumbing()["in_flow"], port, in, do_flow);
	logger->check(result.valid(), "in_flow failed, this is fatal");

	return std::move(result.get<StoredFluids>());
}

StoredFluids PlumbingMachine::out_flow(std::string port, float volume, bool do_flow)
{
	logger->check(has_lua_plumbing(), "Cannot use plumbing functions on machines without plumbing");

	auto result = LuaUtil::safe_call_function(get_lua_plumbing()["out_flow"], port, volume, do_flow);
	logger->check(result.valid(), "out_flow failed, this is fatal");

	return std::move(result.get<StoredFluids>());
}

void PlumbingMachine::draw_diagram(void* vg, GUISkin* gui_skin)
{
	logger->check(has_lua_plumbing(), "Cannot use plumbing functions on machines without plumbing");
	LuaUtil::safe_call_function(get_lua_plumbing()["draw_diagram"], vg, gui_skin);
}

sol::table PlumbingMachine::get_lua_plumbing(bool silent_fail)
{
	auto value = in_machine->env["plumbing"];
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

void PlumbingMachine::init(const cpptoml::table& init)
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
	auto value = in_machine->env["plumbing"];
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

void PlumbingMachine::create_port(std::string id, std::string marker, std::string name, bool is_flow_port, float x, float y)
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
	port.in_machine = this;
	port.pos = glm::vec2(x, y);
	port.marker = marker;
	port.gui_name = name;
	port.id = id;
	port.is_flow_port = is_flow_port;

	fluid_ports.push_back(port);

}

FluidPort* PlumbingMachine::get_port_by_id(const std::string &name)
{
	for(size_t i = 0; i < fluid_ports.size(); i++)
	{
		if(fluid_ports[i].id == name)
		{
			return &fluid_ports[i];
		}
	}

	logger->fatal("Could not find port with id: {}", name);
	return nullptr;
}

glm::vec2 PlumbingMachine::correct_editor_pos(glm::vec2 local_pos)
{
	glm::vec2 pos = local_pos;

	glm::ivec2 size = get_size();
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


glm::ivec2 PlumbingMachine::get_size(bool expand, bool rotate)
{
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

std::vector<FluidPort *> PlumbingMachine::get_connected_ports(const std::string &port)
{
	return std::vector<FluidPort *>();
}

float PlumbingMachine::get_pressure_drop(const std::string &from, const std::string &to, float cur_P)
{
	logger->check(has_lua_plumbing(), "Cannot use plumbing functions on machines without plumbing");

	auto result = LuaUtil::safe_call_function(get_lua_plumbing()["get_pressure_drop"], from, to, cur_P);
	logger->check(result.valid(), "get_pressure_drop failed, this is fatal");

	return result.get<float>();
}

