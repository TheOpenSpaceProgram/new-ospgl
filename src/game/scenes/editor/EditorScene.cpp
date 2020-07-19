#include "EditorScene.h"
#include <util/SerializeUtil.h>
#include <assets/AssetManager.h>
#include <OSP.h>

void EditorScene::load()
{

	debug_drawer->debug_enabled = true;
	
	
	gui.vg = get_osp()->renderer->vg;
	gui.init(this);

	get_osp()->game_state.universe.paused = true;
	
	Renderer* r = get_osp()->renderer;

	r->cam = &cam;


	r->add_drawable(&vehicle);

	sun = SunLight(0, r->quality.sun_shadow_size);
	sun.position = glm::dvec3(1000.0, 0.0, 400.0);
	sun.ambient_color = glm::vec3(0.4, 0.4, 0.4);
	sun.near_shadow_span = 10.0;

	r->add_light(&sun);	

	// Create the bullet physics stuff
	bt_brf_interface = new btDbvtBroadphase();
	bt_collision_config = new btDefaultCollisionConfiguration();
	bt_dispatcher = new btCollisionDispatcher(bt_collision_config);
	bt_world = new btCollisionWorld(bt_dispatcher, bt_brf_interface, bt_collision_config);	
	debug_draw = new BulletDebugDrawer();
	debug_draw->setDebugMode(
		btIDebugDraw::DBG_DrawConstraints |
		btIDebugDraw::DBG_DrawWireframe |
		btIDebugDraw::DBG_DrawFrames |
		btIDebugDraw::DBG_DrawConstraintLimits |
		btIDebugDraw::DBG_DrawAabb);
	bt_world->setDebugDrawer(debug_draw);

	lua_core->load(lua_state, "__UNDEFINED__");

	vehicle.init(&lua_state);
}

double t = 0.0;

void EditorScene::update()
{
	gui_input.update();
	cam.update(get_osp()->game_dt, &gui_input);
	gui_input.ext_mouse_blocked |= cam.blocked;
	gui_input.ext_keyboard_blocked |= cam.blocked;
	do_gui();

	vehicle.update(get_osp()->game_dt);
	vehicle_int.update(get_osp()->game_dt);

	double rw = get_osp()->renderer->get_width(true) - (double)gui.get_panel_width();
	double w = (double)gui.get_panel_width() / (double)get_osp()->renderer->get_width(true);
	glm::dvec4 viewport = glm::dvec4(w, 0.0, 1.0, 1.0);
	glm::dvec2 real_screen_size = 
		glm::dvec2(get_osp()->renderer->get_width(true), get_osp()->renderer->get_height(true));

	if(!gui_input.mouse_blocked)
	{
		vehicle_int.handle_input(
			cam.get_camera_uniforms(rw, get_osp()->renderer->get_height(true)), 
			viewport, real_screen_size,
			&gui_input); 
	}
	
	bt_world->updateAabbs();

	//bt_world->debugDrawWorld();
}

void EditorScene::render()
{
	// The GUI takes a big portion of the screen and it's opaque
	double w = (double)gui.get_panel_width() / (double)get_osp()->renderer->get_width(true);
	get_osp()->renderer->override_viewport = glm::dvec4(w, 0.0, 1.0, 1.0);
	t += get_osp()->game_dt;

	get_osp()->renderer->render(nullptr);

}

void EditorScene::do_gui()
{
	float width = get_osp()->renderer->get_width(true);
	float height = get_osp()->renderer->get_height(true);

	gui.do_gui(width, height, &gui_input);	
}

void EditorScene::unload()
{
	get_osp()->renderer->remove_drawable(&vehicle);
	get_osp()->renderer->override_viewport = glm::dvec4(0.0, 0.0, 1.0, 1.0);
	get_osp()->game_state.universe.paused = false;	

	// Destroy the bullet stuff
	delete bt_world;
	delete bt_brf_interface;
	delete bt_dispatcher;
	delete bt_collision_config;
}

EditorScene::EditorScene() : vehicle(this), vehicle_int(&vehicle, &cam)
{
}


