#include "EditorScene.h"
#include <util/SerializeUtil.h>
#include <assets/AssetManager.h>
#include <OSP.h>

void EditorScene::load()
{
	// Load the different models
	std::string model_path = *SerializeUtil::load_file(assets->resolve_path("core:meshes/editor_attachment.toml"))
		->get_as<std::string>("model");

	AssetHandle<Model> model = AssetHandle<Model>(model_path);

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
}

double t = 0.0;

void EditorScene::update()
{
	gui_input.update();
	cam.update(get_osp()->game_dt, &gui_input);
	gui_input.ext_mouse_blocked |= cam.blocked;
	gui_input.ext_keyboard_blocked |= cam.blocked;
	
	do_gui();
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
}


