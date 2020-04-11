#include "InputContext.h"
#include <GLFW/glfw3.h>
#include <util/Logger.h>
#include "Input.h"
#include <cpptoml.h>
#include <util/SerializeUtil.h>
#include <assets/AssetManager.h>
#include <assets/Config.h>
#include <filesystem>

double InputContext::get_axis(const std::string& name)
{
	auto it = axes.find(name);
	logger->check_important(it != axes.end(), "Could not find axis");
	return it->second;
}

bool InputContext::get_action(const std::string& name)
{
	auto it = actions.find(name);
	logger->check_important(it != actions.end(), "Could not find action");
	return it->second;
}

bool InputContext::get_action_down(const std::string& name)
{
	auto it = actions.find(name);
	logger->check_important(it != actions.end(), "Could not find action");
	auto it_prev = actions_previous.find(name);
	// If the first is present, this one is too
	return it->second && !it_prev->second;	
}

bool InputContext::get_action_up(const std::string& name)
{
	auto it = actions.find(name);
	logger->check_important(it != actions.end(), "Could not find action");
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

void InputContext::map_axis_to_keys(const std::string& name, int plus_key, int minus_key, double speed, double attenuation)
{
	KeyAxisMapping map;
	map.to_axis = name;
	map.plus_key = plus_key;
	map.minus_key = minus_key;
	map.speed = speed;
	map.attenuation = attenuation;
	map.cur_value = 0.0;

	key_axis_mappings.push_back(map);

	axes[name] = 0.0;
}

void InputContext::map_axis_to_joystick(const std::string& name, int joystick_id, int axis_id, bool invert, double deadzone)
{
	JoyAxisMapping map;
	map.to_axis = name;
	map.joystick_id = joystick_id;
	map.axis_id = axis_id;
	map.invert = invert;
	map.deadzone = deadzone;
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


void InputContext::update(GLFWwindow* window, double dt)
{
	obtain_joystick_states();

	for(auto pair : actions)
	{
		actions_previous[pair.first] = pair.second;
		actions[pair.first] = false;
	}
		
	for(auto& map : key_action_mappings)
	{
		if(glfwGetKey(window, map.key) == GLFW_PRESS)
		{
			actions[map.to_action] = true;
		}
	}

	for(auto& map : joy_action_mappings)
	{
		if(get_joystick_button(map.joystick_id, map.button_id) == GLFW_PRESS)
		{
			actions[map.to_action] = true;
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
	Config* cfg = assets->get_from_path<Config>(path);

	// All input configs are stored in udata/input/ so it's 
	// easy to find them
	std::string config_path = assets->udata_path + "input/" + *cfg->root->get_as<std::string>("config");

	if(!std::filesystem::exists(config_path))
	{
		// Copy the file
		std::string real_path = assets->res_path + cfg->in_package + "/" + cfg->path;
		//std::filesystem::copy(real_path, config_path);
		std::string dir_path = config_path.substr(config_path.find_last_of('/'));
		std::filesystem::create_directories(dir_path);
		logger->info("Copying input config from {} to {}", real_path, config_path);
	}

	// Read the config file
	table_ptr root = SerializeUtil::load_file(config_path);

}

