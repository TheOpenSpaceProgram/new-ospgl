#include "GUIScreen.h"
#include <renderer/Renderer.h>

void GUIScreen::init(glm::ivec4 nscreen, GUISkin *nskin, GUIInput* ngui_input)
{
	screen = nscreen;
	skin = nskin;
	gui_input = ngui_input;
}

void GUIScreen::prepare_pass()
{
	post_canvas.clear();

	// pre-prepare generates post_canvas
	for(auto c : canvas)
	{
		c.first->pre_prepare(this);
	}

	// position_pass
	for(auto o : post_canvas)
	{
		o.first->position_widgets(o.second.first, o.second.second, this);
	}
	for(auto o : canvas)
	{
		o.first->position_widgets(o.second.first, o.second.second, this);
	}

	// first prepare without user actions, top to down
	gui_input->execute_user_actions = false;
	for(auto o : post_canvas)
	{
		o.first->prepare(this, gui_input);
	}
	for(auto o : canvas)
	{
		o.first->prepare(this, gui_input);
	}

}

void GUIScreen::input_pass()
{
	// first prepare without user actions, top to down
	gui_input->execute_user_actions = true;
	for(auto o : post_canvas)
	{
		o.first->prepare(this, gui_input);
	}
	for(auto o : canvas)
	{
		o.first->prepare(this, gui_input);
	}
}

void GUIScreen::draw()
{
	for(auto o : canvas)
	{
		o.first->draw(osp->renderer->vg, skin, screen);
	}
	for(auto o : post_canvas)
	{
		o.first->draw(osp->renderer->vg, skin, screen);
	}
}

void GUIScreen::add_canvas(GUICanvas *acanvas, glm::ivec2 pos, glm::ivec2 size)
{
	canvas.emplace_back(acanvas, std::make_pair(pos, size));
}

void GUIScreen::add_post_canvas(GUICanvas *acanvas, glm::ivec2 pos, glm::ivec2 size)
{
	post_canvas.emplace_back(acanvas, std::make_pair(pos, size));
}

void GUIScreen::new_frame(glm::ivec4 nscreen)
{
	screen = nscreen;
	post_canvas.clear();
	canvas.clear();
}
