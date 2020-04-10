#pragma once
#include <unordered_map>
#include <vector>
#include <string>

class GLFWwindow;


// All invalid names will result in an error, not default values
// Once something is mapped it cannot be unmapped, you need to 
// recreate the context from scratch 
// (This is done automatically by the remapping UI, mods should not change
// keybinds using any other method)
// You can have as many mappings as you wish for any of the axes or actions
//
// Inputs are overriden by maximum absolute value. This means that the value of an axis
// is the maximum (or minimum) of all inputs, and actions are activated if any of the 
// mapped inputs are active
class InputContext
{
private:

	struct JoyAxisMapping
	{
		std::string to_axis;
		int joystick_id, axis_id;
		bool invert;
		double deadzone;
	};

	struct JoyActionMapping
	{
		std::string to_action;
		int joystick_id, button_id;
	};

	struct KeyAxisMapping
	{
		std::string to_axis;
		int plus_key, minus_key;
		double speed, attenuation;

		double cur_value;	
	};

	struct KeyActionMapping
	{
		std::string to_action;
		int key;
	};

	std::unordered_map<std::string, double> axes;
	std::unordered_map<std::string, bool> actions;
	std::unordered_map<std::string, bool> actions_previous;

	std::vector<JoyAxisMapping> joy_axis_mappings;
	std::vector<JoyActionMapping> joy_action_mappings;
	std::vector<KeyAxisMapping> key_axis_mappings;
	std::vector<KeyActionMapping> key_action_mappings;

	int get_joystick_button(int jid, int button);	

	struct JoystickState
	{
		bool is_present;
		int axes_count;
		int buttons_count;
		const float* axes;
		const unsigned char* buttons;
	};

	// Joysticks IDs start at 0, unlike GLFW where they start at 1!
	std::array<JoystickState, 16> joystick_states;

	void obtain_joystick_states();

public:

	double get_axis(const std::string& name);

	// Returns true if action is currently active
	bool get_action(const std::string& name);	
	// Returns true if action was pressed this frame
	bool get_action_down(const std::string& name);
	// Returns true if action was released this frame
	bool get_action_up(const std::string& name);

	// Invert controls if the input is inverted (+ -> -)
	// Deadzone sets a area around the origin where input is ignored (in both signs)
	void map_axis_to_joystick(const std::string& name, int joystick_id, int axis_id, bool invert, double deadzone);
	// Speed controls how fast the axis moves in units / second
	// Attenuation controls how fast the axis returns to the origin in units / second
	void map_axis_to_keys(const std::string& name, int plus_key, int minus_key, double speed, double attenuation);
	void map_action_to_key(const std::string& name, int key);
	void map_action_to_joybutton(const std::string& name, int joystick_id, int button_id);

	// Reads all inputs, make sure you call it before everything that needs inputs
	void update(GLFWwindow* window, double dt);
};
