#include "EditorScene.h"
#include <OSP.h>

void EditorScene::load()
{
	get_osp()->game_state.universe.paused = true;
	
	// The GUI takes a big portion of the screen and it's opaque
	get_osp()->renderer->override_viewport = glm::dvec4(0.25, 0.0, 1.0, 1.0);
	get_osp()->renderer->cam = &cam;	

	gui.vg = get_osp()->renderer->vg;

	
}

void EditorScene::update()
{
	do_gui();
}

void EditorScene::render()
{
	get_osp()->renderer->render(nullptr);
}

void EditorScene::do_gui()
{
	float width = get_osp()->renderer->get_width(true);
	float height = get_osp()->renderer->get_height(true);

	gui.do_gui(width, height);	
}

void EditorScene::unload()
{

	get_osp()->renderer->override_viewport = glm::dvec4(0.0, 0.0, 1.0, 1.0);
	get_osp()->game_state.universe.paused = false;	
}
