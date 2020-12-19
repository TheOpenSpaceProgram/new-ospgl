#include "InputContext.h"
#include <GLFW/glfw3.h>
#include <util/Logger.h>
#include "FlightInput.h"
#include <cpptoml.h>
#include <util/SerializeUtil.h>
#include <assets/AssetManager.h>
#include <assets/Config.h>
#include <filesystem>
#include <unordered_set>

double InputContext::get_axis(const std::string& name)
{
	auto it = axes.find(name);
	logger->check(it != axes.end(), "Could not find axis '{}'", name);
	return it->second;
}

bool InputContext::get_action(const std::string& name)
{
	auto it = actions.find(name);
	logger->check(it != actions.end(), "Could not find action '{}'", name);
	return it->second;
}

bool InputContext::get_action_down(const std::string& name)
{
	auto it = actions.find(name);
	logger->check(it != actions.end(), "Could not find action '{}'", name);
	auto it_prev = actions_previous.find(name);
	// If the first is present, this one is too
	return it->second && !it_prev->second;	
}

bool InputContext::get_action_up(const std::string& name)
{
	auto it = actions.find(name);
	logger->check(it != actions.end(), "Could not find action '{}'", name);
	auto it_prev = actions_previous.find(name);
	// If the first is present, this one is too
	return !it->second && it_prev->second;	

}

void InputContext::map_action_to_key(const std::string& name, int key)
{
	KeyActionMapping map;
	map.to_action = name;
	map.key = key;
	key_action_mappings.push_back(map);

	actions[name] = false;
	actions_previous[name] = false;
}

void InputContext::map_action_to_joybutton(const std::string& name, int joystick_id, int button_id)
{
	JoyActionMapping map;
	map.to_action = name;
	map.joystick_id = joystick_id;
	map.button_id = button_id;
	joy_action_mappings.push_back(map);

	actions[name] = false;
	actions_previous[name] = false;
}

void InputContext::map_axis_to_keys(const std::string& name, int plus_key, int minus_key, double speed, double attenuation, double origin)
{
	KeyAxisMapping map;
	map.to_axis = name;
	map.plus_key = plus_key;
	map.minus_key = minus_key;
	map.speed = speed;
	map.attenuation = attenuation;
	map.origin = origin;
	map.cur_value = origin;

	key_axis_mappings.push_back(map);

	axes[name] = 0.0;
}

void InputContext::map_axis_to_joystick(const std::string& name, int joystick_id, int axis_id, bool invert, double deadzone, double origin)
{
	JoyAxisMapping map;
	map.to_axis = name;
	map.joystick_id = joystick_id;
	map.axis_id = axis_id;
	map.invert = invert;
	map.deadzone = deadzone;
	map.origin = origin;
	joy_axis_mappings.push_back(map);

	axes[name] = 0.0;
}

void InputContext::obtain_joystick_states()
{
	for(int jid = 0; jid < 16; jid++)
	{
		if(glfwJoystickPresent(jid))
		{
			joystick_states[jid].is_present = true;
			joystick_states[jid].axes = glfwGetJoystickAxes(jid, &joystick_states[jid].axes_count);
		   	joystick_states[jid].buttons = glfwGetJoystickButtons(jid, &joystick_states[jid].buttons_count);	
		}
		else
		{
			joystick_states[jid].is_present = false;
		}
	}	
}

int InputContext::get_joystick_button(int jid, int button)
{
	if(jid > 15 || jid < 0)
	{
		return GLFW_RELEASE;
	}

	// Search a specific joystick
	if(joystick_states[jid].is_present)
	{
		if(button < joystick_states[jid].buttons_count)
		{
			return (int)joystick_states[jid].buttons[button];
		}
		else
		{
			return GLFW_RELEASE;
		}
	}
	else
	{
		return GLFW_RELEASE;
	}

}

double InputContext::get_joystick_axis(int jid, int axis)
{
	if(jid > 15 || jid < 0)
	{
		return -2.0;
	}

	if(joystick_states[jid].is_present)
	{
		if(axis < joystick_states[jid].axes_count)
		{
			return (double)joystick_states[jid].axes[axis];
		}
		else
		{
			return -2.0;
		}
	}
	else
	{
		return -2.0;
	}

}


void InputContext::update(GLFWwindow* window, double dt)
{
	obtain_joystick_states();

	for(auto pair : actions)
	{
		actions_previous[pair.first] = pair.second;
		actions[pair.first] = false;
	}
	
	if(!input->keyboard_blocked)
	{	
		for(auto& map : key_action_mappings)
		{
			if(glfwGetKey(window, map.key) == GLFW_PRESS)
			{
				actions[map.to_action] = true;
			}
		}
	}

	for(auto& map : joy_action_mappings)
	{
		if(get_joystick_button(map.joystick_id, map.button_id) == GLFW_PRESS)
		{
			actions[map.to_action] = true;
		}	
	}

	// Axes are a bit more complicated as they need post processing
	// Joysticks override keyboard IFF the joystick is present
	// TODO: Think of another method for this stuff, it feels like it lacks customization
	
	std::unordered_set<std::string> written_key_to;
	std::unordered_set<std::string> written_joy_to;

	for(auto& map : key_axis_mappings)
	{
		if(written_key_to.find(map.to_axis) != written_key_to.end())
		{
			logger->warn("Multiple keyboard inputs for the same axis. This is invalid.");
			continue;
		}

		// Update current value
		double change = 0.0;

		if(!input->keyboard_blocked)
		{
			if(glfwGetKey(window, map.plus_key) == GLFW_PRESS)
			{
				change += 1.0;
			}

			if(glfwGetKey(window, map.minus_key) == GLFW_PRESS)
			{
				change -= 1.0;
			}
		}

		change *= map.speed;

		// TODO: Maybe check instead for any key pressed?
		if(change == 0.0)
		{
			// Attenuation
			double sign = glm::sign(map.origin - map.cur_value);
			
			double next = map.cur_value + sign * map.attenuation * dt;
			double nsign = glm::sign(map.origin - next);

			if(nsign != sign)
			{
				// Overshoot
				map.cur_value = map.origin;
			}
			else
			{
				map.cur_value = next;
			}
		}
		else
		{
			map.cur_value += change * dt;
		}

		if(map.cur_value > 1.0)
		{
			map.cur_value = 1.0;
		}

		if(map.cur_value < -1.0)
		{
			map.cur_value = -1.0;
		}

		axes[map.to_axis] = map.cur_value;

		written_key_to.insert(map.to_axis);	
	}

	for(auto& map : joy_axis_mappings)
	{
		double value = get_joystick_axis(map.joystick_id, map.axis_id);
		if(value < -1.5)
		{
			continue; // Joystick is not present
		}

		if(written_joy_to.find(map.to_axis) != written_joy_to.end())
		{
			logger->warn("Multiple joystick inputs for the same axis. This is invalid.");
			continue;
		}

		// Postprocess
		double fvalue = value * (map.invert ? -1 : 1);
		double forigin = map.origin; // < Origin is not inverted
		double distance_from_origin = fvalue - forigin;

		if(glm::abs(distance_from_origin) < map.deadzone)
		{
			fvalue = forigin;
		}

		
		auto it_b = axis_blocks.find(map.to_axis);
		if(it_b != axis_blocks.end())
		{
			double distance = it_b->second.value - fvalue;
			if(glm::abs(distance) <= it_b->second.epsilon)
			{
				// Remove the axis block, we are close enough
				axis_blocks.erase(it_b);
			}
			else
			{
				// Block the joystick
				fvalue = it_b->second.value;
			}
		}	
		
		axes[map.to_axis] = fvalue;
	}

}

void InputContext::set_axis(const std::string& name, double value, double epsilon)
{
	if(value > 1.0 || value < -1.0)
	{
		logger->warn("Tried to set axis '{}' to a value '{}' which is out of bounds.", name, value);
		return;
	}

	// Create an AxisBlock or modify existing
	auto it = axis_blocks.find(name);
	if(it == axis_blocks.end())
	{
		AxisBlock n_block;
		axis_blocks.insert(std::make_pair(name, n_block));
		it = axis_blocks.find(name);
	}

	it->second.value = value;
	it->second.epsilon = epsilon;

	// Apply to all keyboard mappings to this axis
	
	for(auto& mapping : key_axis_mappings)
	{
		if(mapping.to_axis == name)
		{
			mapping.cur_value = value;
		}
	}
}


InputContext::~InputContext()
{
	if(input != nullptr)
	{
		input->set_ctx(nullptr);
	}
}

void InputContext::load_from_file(const std::string& path)
{
	using table_ptr = std::shared_ptr<cpptoml::table>;
	Config* cfg = osp->assets->get_from_path<Config>(path);

	// All input configs are stored in udata/input/ so it's 
	// easy to find them
	std::string config_path = osp->assets->udata_path + "input/" + *cfg->root->get_as<std::string>("config");

	if(!std::filesystem::exists(config_path))
	{
		// Copy the file
		std::string real_path = cfg->real_path;
		std::string dir_path = config_path.substr(0, config_path.find_last_of('/') + 1);
		std::filesystem::create_directories(dir_path);
		logger->info("Copying input config from {} to {} (in dir: {})", real_path, config_path, dir_path);
		std::filesystem::copy(real_path, config_path);
	}

	// Read the config file
	table_ptr root = SerializeUtil::load_file(config_path);

	auto inputs = root->get_table_array("input");
	if(inputs)
	{
		for(auto input : *inputs)
		{
			std::string type = *input->get_as<std::string>("type");
			std::string name = *input->get_as<std::string>("name");
			auto mappings = input->get_table_array("mapping");
			if(mappings)
			{
				for(auto mapping : *mappings)
				{
					std::string map_type = *mapping->get_as<std::string>("type");

					if(type == "axis")
					{
						if(map_type == "key")
						{
							int up = (int)*mapping->get_as<int64_t>("up");	
							int down = (int)*mapping->get_as<int64_t>("down");
							double speed = *mapping->get_as<double>("speed");
							double attenuation = *mapping->get_as<double>("attenuation");
							double origin = *mapping->get_as<double>("origin");

							map_axis_to_keys(name, up, down, speed, attenuation, origin);
						}
						else if(map_type == "joy")
						{
							int joy_id = (int)*mapping->get_as<int64_t>("joystick");
							int axis = (int)*mapping->get_as<int64_t>("axis");
							bool invert = *mapping->get_as<bool>("invert");
							double deadzone = *mapping->get_as<double>("deadzone");
							double origin = *mapping->get_as<double>("origin");		

							map_axis_to_joystick(name, joy_id, axis, invert, deadzone, origin);
						}
						else
						{
							logger->warn("Unknown mapping type '{}' for axis '{}'. Ignoring", map_type, name);
						}
					}
					else if(type == "action")
					{
						if(map_type == "key")
						{
							int key = (int)*mapping->get_as<int64_t>("key");	
							
							map_action_to_key(name, key);
						}
						else if(map_type == "joy")
						{
							int joystick = (int)*mapping->get_as<int64_t>("joystick");
							int button = (int)*mapping->get_as<int64_t>("button");
						
							map_action_to_joybutton(name, joystick, button);	
						}
						else
						{
							logger->warn("Unknown mapping type '{}' for action '{}'. Ignoring", map_type, name);
						}

					}
					else
					{
						logger->warn("Unknown input type '{}' for input '{}'. Ignoring", type, name);
					}
				}
			}
		}
	}
}

