#pragma once
#include <gui/GUIInput.h>
#include <gui/GUIScreen.h>
#include <string>

class OSP;

// Scenes handle user interaction with the universe, either via seeing
// and controlling vehicles or other task (creating vehicles, discovering technology...)
class Scene
{
public:

	// A global GUI input is required for all screens (so they are compatible with each other and debug tools)
	GUIInput gui_input;

	// Called the frame the scene is loaded
	virtual void load() = 0;
	// It's your responsability to call universe->update(dt) (or not)
	virtual void pre_update() = 0;
	// Called after pre_update, so entities have already run 1 update! (If you called universe->update)
	virtual void update() = 0;
	virtual void render() = 0;
	// Called the frame the scene is unloaded
	virtual void unload() = 0;
	virtual void do_imgui_debug() = 0;

	virtual std::string get_name() = 0;
	virtual ~Scene() {}
};
