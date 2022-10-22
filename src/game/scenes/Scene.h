#pragma once
#include <gui/GUIInput.h>
#include <string>

class OSP;

// Scenes handle user interaction with the universe, either via seeing
// and controlling vehicles or other task (creating vehicles, discovering technology...)
class Scene
{
public:

	GUIInput gui_input;

	// Called the frame the scene is loaded
	virtual void load() = 0;
	virtual void update() = 0;
	virtual void render() = 0;
	// Called the frame the scene is unloaded
	virtual void unload() = 0;
	virtual void do_imgui_debug() = 0;

	virtual std::string get_name() = 0;

	virtual ~Scene() {}
};
