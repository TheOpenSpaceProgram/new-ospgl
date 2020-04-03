#include "Machine.h"
#include "../../util/Logger.h"
#include "../../util/SerializeUtil.h"
#include "../../assets/AssetManager.h"

#include "../Vehicle.h"
#include "sol.hpp"


Machine::Machine(std::shared_ptr<cpptoml::table> init_toml, std::string cur_pkg)
{	
	this->init_toml = init_toml;
	logger->check_important(init_toml != nullptr, "Malformed init_toml");
	cpptoml::table& init_toml_p = *init_toml;

	std::string script_path;
	SAFE_TOML_GET_FROM(init_toml_p, script_path, "script", std::string);

	lua_state["machine"] = this;

	std::string old = assets->get_current_package();
	assets->set_current_package(cur_pkg);
	assets->load_script_to(lua_state, script_path);
	assets->set_current_package(old);

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

	this->in_part = in_part;	
}

void Machine::define_ports()
{
	is_defining_ports = true;

	LuaUtil::call_function(lua_state, "define_ports", "machine define_ports");

	while(new_ports.size() > 0)
	{
		// We pop from the front until the array is empty
		PortDefinition port_def = new_ports.front();
		new_ports.pop_back();

		bool found = false;
		bool same_type;
		for(auto port_it = ports.begin(); port_it != ports.end(); )
		{
			Port* port = *port_it;
			// Find a port with the same name and that is the same IO type
			if(	port->name == port_def.name &&
				port->is_output == port_def.output)
			{
				// Remove the port
				ports.erase(port_it);

				Vehicle* in_vehicle = in_part->vehicle;
				std::vector<Port*>& ports = in_vehicle->all_ports;

				// Remove ourselves from the port list
				// TODO: Think of using an unordered_set for this
				for(auto port_it = ports.begin(); port_it != ports.end(); port_it++)
				{
					Port* iport = *port_it;
					// Remove ourselves from any port that outputs to us
					if (!port->is_output && iport->is_output)
					{
						std::remove_if(port->to.begin(), port->to.end(), [port](Port* aval)
						{
							return aval == port;
						});
					}

					if (iport == port)
					{
						port_it = ports.erase(port_it);
					}
					else
					{
						port_it++;
					}
				}

				delete port;	
				found = true;
				same_type = port->type == PortValue::get_type(port_def.type);
				break;
			}
			else
			{
				port_it++;
			}
		}

		bool add_new = false;

		if(found)
		{
			if(same_type)
			{
				// Do nothing
			}
			else
			{
				add_new = true;
			}
		}	
		else
		{
			// Add new port
			add_new = true;
		}

		if(add_new)
		{
			Port* port_new = new Port();
			port_new->in_machine = this;
			port_new->is_output = port_def.output;
			port_new->name = port_def.name;
			port_new->callback = port_def.callback;

			if (port_new->is_output)
			{
				outputs[port_new->name] = port_new;
			}

			in_part->vehicle->all_ports.push_back(port_new);
			ports.push_back(port_new);
		}

	}


	new_ports.clear();
	is_defining_ports = false;
}

void Machine::add_input_port(const std::string & name, const std::string & type, sol::safe_function callback)
{
	logger->check_important(is_defining_ports, "Tried to change a port while the machine was not changing ports");

	PortDefinition port_def = PortDefinition();
	port_def.name = name;
	port_def.type = type;
	port_def.output = false;
	port_def.callback = callback;

	new_ports.push_back(port_def);
}

void Machine::add_output_port(const std::string & name, const std::string & type)
{
	logger->check_important(is_defining_ports, "Tried to change a port while the machine was not changing ports");

	PortDefinition port_def = PortDefinition();
	port_def.name = name;
	port_def.type = type;
	port_def.output = true;
	port_def.callback = sol::nil;

	new_ports.push_back(port_def);
}


Port* Machine::get_input_port(const std::string& name)
{
	for(Port* p : ports)
	{
		if(p->name == name && p->is_output == false)
		{
			return p;
		}
	}

	logger->fatal("Tried to get a input port named '{}' which did not exist", name);
	return nullptr;
}

Port* Machine::get_output_port(const std::string& name)
{
	for(Port* p : ports)
	{
		if(p->name == name && p->is_output == true)
		{
			return p;
		}
	}

	logger->fatal("Tried to get a output port named '{}' which did not exist", name);
	return nullptr;
}

PortResult Machine::write_to_port(const std::string& name, PortValue val)
{
	PortResult out; out.result = PortResult::GOOD;

	auto it = outputs.find(name);
	if (it == outputs.end())
	{
		out.result = PortResult::PORT_NOT_FOUND;
		return out;
	}

	if (it->second->blocked)
	{
		out.result = PortResult::PORT_BLOCKED;
		return out;
	}

	if (it->second->type != val.type)
	{
		out.result = PortResult::INVALID_TYPE;
		return out;
	}

	// Write the value and block the port
	for (Port* o : it->second->to)
	{
		o->receive(val);
	}
	
	it->second->blocked = true;
	

	return out;
}


Machine::~Machine()
{
}
