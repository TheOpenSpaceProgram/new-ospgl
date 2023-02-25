#pragma once
#include "../Scene.h"
#include <nanovg/nanovg.h>
#include "EditorCamera.h"
#include "gui/EditorGUI.h"
#include "EditorVehicle.h"
#include "EditorVehicleInterface.h"

#include <assets/BitmapFont.h>

#include <renderer/lighting/SunLight.h>

#include <assets/Model.h>

#pragma warning(push, 0)
#include <btBulletDynamicsCommon.h>
#include <BulletDynamics/ConstraintSolver/btNNCGConstraintSolver.h>
#pragma warning(pop)
#include <physics/debug/BulletDebugDrawer.h>
#include <renderer/util/Skybox.h>

// Emits the events:
// on_editor_update(dt) - Right before editor_update is called on machines
// post_editor_update(dt) - Right after editor_update is called on machines and interface has updated
// on_gui_prepare() - Called just before editor GUI is prepared
// post_gui_prepare() - Called just after editor GUI is prepared
// on_gui_input() - Called just before GUI handles inputs
// post_gui_input() - Called just after GUI handles inputs, just before drawing
// post_gui_draw() - Called just after GUI draws
class EditorScene : public Scene, public EventEmitter
{
public:	
	friend class EditorGUI;
	EditorGUI gui;
	std::shared_ptr<Skybox> sky;

	glm::dvec4 get_viewport();

	GUIScreen gui_screen;
private:
	SimpleSkin skin;



	// We need a world for the very simple colliders, but we have no 
	// dynamics, links, or anything like that
	btDefaultCollisionConfiguration* bt_collision_config;
	btBroadphaseInterface* bt_brf_interface;
	btCollisionDispatcher* bt_dispatcher;
	BulletDebugDrawer* debug_draw;

	std::shared_ptr<SunLight> sun;

	void do_gui();
	void do_edveh_gui();

public:
	EditorCamera cam;
	std::shared_ptr<EditorVehicle> vehicle;
	EditorVehicleInterface vehicle_int;

	// We hold a non-universe lua_state for all machines to interact
	sol::state lua_state;
	// It's very important for envs to be below lua_state to prevent SEGFAULT on close
	std::vector<sol::environment> envs;

	btCollisionWorld* bt_world;

	virtual void load() override;
	virtual void pre_update() override;
	virtual void update() override;
	virtual void render() override;
	virtual void unload() override;
	virtual void physics_update(double bdt) override {}
	std::string get_name() override
	{
		return "editor";
	}
	void do_imgui_debug() override
	{

	}

	// (rw, rh), viewport, real_screen_size, gui_vport
	std::tuple<glm::ivec2, glm::vec4, glm::dvec2, glm::vec4> get_viewports();

	EditorScene();

};
