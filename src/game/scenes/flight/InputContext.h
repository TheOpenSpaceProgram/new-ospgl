#pragma once
#include <unordered_map>
#include <vector>
#include <string>
#include <cpptoml.h>

class GLFWwindow;
class FlightInput;

// All invalid names will result in an error, not default values
// Once something is mapped it cannot be unmapped, you need to 
// recreate the context from scratch 
// (This is done automatically by the remapping UI, mods should not change
// keybinds using any other method)
// You can have as many mappings as you wish for any of the axes or actions
//
// Conflicting actions are easy, if any of the mapped inputs for that action are true,
// then the action is true.
//
// Axes are different, joysticks always override keyboard value, but having multiple
// keys mapped to a single axis, or multiple joy axis mapped will result in only one of
// these inputs working (the others get overriden). You will get a warning if this is the case!
//
// The lua code can force an axis to some value, keyboard controlled axes will work fine, but for joysticks
// sadly this is not as simple. The joystick axis will be ignored until its value
// is EXACTLY the target value (this only works on -1 or 1) or close enough (for not -1 or 1 values, set epsilon).
// An example of this behaviour is a throttle cut button, if using a joystick, the user will need to manually
// return the throttle lever to 0 before it's useful again.
// TODO: Maybe rethink the previous paragraph? There could be another way to do this?
class InputContext
{
private:

	struct JoyAxisMapping
	{
		std::string to_axis;
		int joystick_id, axis_id;
		bool invert;
		double deadzone; // (deadzone is applied around the origin)
		double origin;
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
		double speed, attenuation, origin;

		double cur_value;	
	};

	// Maps an axis value to an action
	struct KeyAxisActionMapping
	{
		std::string to_axis;
		double axis_value;
		int key;
	};

	struct KeyActionMapping
	{
		std::string to_action;
		int key;
	};

	std::unordered_map<std::string, double> axes;
	std::unordered_map<std::string, bool> actions;
	std::unordered_map<std::string, bool> actions_previous;
	
	struct AxisBlock
	{
		double value;
		double epsilon;		
	};

	std::unordered_map<std::string, AxisBlock> axis_blocks;

	std::vector<JoyAxisMapping> joy_axis_mappings;
	std::vector<JoyActionMapping> joy_action_mappings;
	std::vector<KeyAxisMapping> key_axis_mappings;
	std::vector<KeyActionMapping> key_action_mappings;

	int get_joystick_button(int jid, int button);
	// Doesn't do any postprocessing! Returns -2.0 (impossible value) if not found.
	double get_joystick_axis(int jid, int axis);

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
	
	void init_config(cpptoml::table& base, cpptoml::table& target);

	FlightInput* input;


public:

	friend class FlightInput;

	double get_axis(const std::string& name);

	// Returns true if action is currently active
	bool get_action(const std::string& name);	
	// Returns true if action was pressed this frame
	bool get_action_down(const std::string& name);
	// Returns true if action was released this frame
	bool get_action_up(const std::string& name);

	void set_axis(const std::string& name, double value, double epsilon = 0.0);

	// Invert controls if the input is inverted (+ -> -)
	// Deadzone sets a area around the origin where input is ignored (in both signs)
	void map_axis_to_joystick(const std::string& name, int joystick_id, int axis_id, bool invert, double deadzone, double origin);
	// Speed controls how fast the axis moves in units / second
	// Attenuation controls how fast the axis returns to the origin in units / second
	void map_axis_to_keys(const std::string& name, int plus_key, int minus_key, double speed, double attenuation, double origin);
	void map_action_to_key(const std::string& name, int key);
	void map_action_to_joybutton(const std::string& name, int joystick_id, int button_id);

	// Reads all inputs, make sure you call it before everything that needs inputs
	void update(GLFWwindow* window, double dt);

	bool is_active(){ return input != nullptr; }

	void load_from_file(const std::string& path);

	InputContext() { input = nullptr; } 
	~InputContext();

};
