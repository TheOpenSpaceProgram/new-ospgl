#include "EditorScene.h"
#include <util/SerializeUtil.h>
#include <assets/AssetManager.h>
#include <OSP.h>
#include <renderer/Renderer.h>
#include <renderer/lighting/EnvMap.h>
#include <game/GameState.h>

void EditorScene::load()
{

	debug_drawer->debug_enabled = true;
	
	
	gui.vg = osp->renderer->vg;
	gui.init(this);

	osp->game_state->universe.paused = true;
	
	Renderer* r = osp->renderer;

	r->cam = &cam;


	r->add_drawable(&vehicle);

	r->add_drawable(&sky);
	sky.cubemap->generate_ibl_irradiance(32, 32);
	r->set_ibl_source(sky.cubemap.data);

	sun = SunLight(0, r->quality.sun_shadow_size);
	sun.color = glm::vec3(0.0);
	sun.position = glm::dvec3(1000.0, 0.0, 400.0);
	sun.ambient_color = glm::vec3(0.4, 0.4, 0.4);
	sun.near_shadow_span = 10.0;

	r->add_light(&sun);

	EnvMap* env = new EnvMap();
	r->add_light(env);

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
	if(cam.blocked)
	{
		gui_input.ext_mouse_blocked = true;
		gui_input.ext_keyboard_blocked = true;
	}
	do_gui();

	vehicle.update(osp->game_dt);
	vehicle_int.update(osp->game_dt);

	double rw = osp->renderer->get_width(true) - (double)gui.get_panel_width();
	double w = (double)gui.get_panel_width() / (double)osp->renderer->get_width(true);
	glm::dvec4 viewport = glm::dvec4(w, 0.0, 1.0, 1.0);
	glm::dvec2 real_screen_size = 
		glm::dvec2(osp->renderer->get_width(true), osp->renderer->get_height(true));

	if(!gui_input.mouse_blocked)
	{
		glm::vec4 vport = glm::vec4(w, 0, 1.0, 1.0);
		float gw = (float)gui.get_panel_width();
		glm::vec4 gui_vport = glm::vec4(gw, 0, real_screen_size.x - gw, real_screen_size.y); 
		vehicle_int.do_interface(cam.get_camera_uniforms(rw, real_screen_size.y), 
			vport, gui_vport, real_screen_size, osp->renderer->vg, &gui_input, &gui.skin);
	}
	
	cam.update(osp->game_dt, &gui_input);
	gui_input.ext_mouse_blocked |= cam.blocked;
	gui_input.ext_keyboard_blocked |= cam.blocked;
	
	bt_world->updateAabbs();

	//bt_world->debugDrawWorld();
}

void EditorScene::render()
{
	// The GUI takes a big portion of the screen and it's opaque
	double w = (double)gui.get_panel_width() / (double)osp->renderer->get_width(true);
	osp->renderer->override_viewport = glm::dvec4(w, 0.0, 1.0, 1.0);
	t += osp->game_dt;

	osp->renderer->render(nullptr);

}

void EditorScene::do_gui()
{
	float width = osp->renderer->get_width(true);
	float height = osp->renderer->get_height(true);


	gui.do_gui(width, height, &gui_input);	
}

void EditorScene::do_edveh_gui() 
{
}

void EditorScene::unload()
{
	osp->renderer->remove_drawable(&vehicle);
	osp->renderer->override_viewport = glm::dvec4(0.0, 0.0, 1.0, 1.0);
	osp->game_state->universe.paused = false;

	// Destroy the bullet stuff
	delete bt_world;
	delete bt_brf_interface;
	delete bt_dispatcher;
	delete bt_collision_config;
}

EditorScene::EditorScene() : vehicle(this), vehicle_int(&vehicle, &cam),
	sky(std::move(AssetHandle<Cubemap>("debug_system:skybox.hdr")))
{
}


