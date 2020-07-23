#pragma once
#include "../Scene.h"
#include <nanovg/nanovg.h>
#include "EditorCamera.h"
#include "gui/EditorGUI.h"
#include "EditorVehicle.h"
#include "EditorVehicleInterface.h"

#include <renderer/lighting/SunLight.h>

#include <assets/Model.h>

#pragma warning(push, 0)
#include <btBulletDynamicsCommon.h>
#include <BulletDynamics/ConstraintSolver/btNNCGConstraintSolver.h>
#pragma warning(pop)
#include <physics/debug/BulletDebugDrawer.h>

class EditorScene : public Scene
{
private:

	EditorCamera cam;
	EditorVehicle vehicle;
	EditorVehicleInterface vehicle_int;


	// We need a world for the very simple colliders, but we have no 
	// dynamics, links, or anything like that
	btDefaultCollisionConfiguration* bt_collision_config;
	btBroadphaseInterface* bt_brf_interface;
	btCollisionDispatcher* bt_dispatcher;
	BulletDebugDrawer* debug_draw;

	SunLight sun;

	void do_gui();
	void do_edveh_gui();

public:

	friend class EditorGUI;
	EditorGUI gui;
	
	// We hold a non-universe lua_state for all machines to interact
	sol::state lua_state;
	
	btCollisionWorld* bt_world;

	virtual void load() override;
	virtual void update() override;
	virtual void render() override;
	virtual void unload() override;

	EditorScene();

};
