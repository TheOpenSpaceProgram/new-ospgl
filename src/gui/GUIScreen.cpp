#include "GUIScreen.h"
#include <renderer/Renderer.h>

void GUIScreen::init(std::shared_ptr<GUISkin> nskin, GUIInput* ngui_input)
{
	skin = nskin;
	gui_input = ngui_input;
}

void GUIScreen::prepare_pass()
{
	post_canvas.clear();

	// First position pass
	win_manager.position(this);
	for(auto o : canvas)
	{
		o.first->position_widgets(o.second.first, o.second.second, this);
	}

	// pre-prepare generates post_canvas, which will correctly be placed
	for(auto c : canvas)
	{
		c.first->pre_prepare(this);
	}

	win_manager.pre_prepare(this);

	// Post canvas position pass, these may not create further post-canvases!
	for(auto o : post_canvas)
	{
		o.first->position_widgets(o.second.first, o.second.second, this);
	}

	// first prepare without user actions, top to down
	gui_input->execute_user_actions = false;
	win_manager.prepare(gui_input, this);
	for(auto o : post_canvas)
	{
		o.first->prepare(this, gui_input);
	}
	for(auto o : canvas)
	{
		o.first->prepare(this, gui_input);
	}
	//gui_input->debug();

}

void GUIScreen::input_pass()
{
	// Reset GUI block status so it doesn't block itself from the previous pass
	gui_input->mouse_blocked = false;
	gui_input->keyboard_blocked = false;
	gui_input->scroll_blocked = false;

	gui_input->execute_user_actions = true;
	for(auto o : post_canvas)
	{
		o.first->prepare(this, gui_input);
	}
	win_manager.prepare(gui_input, this);
	for(auto o : canvas)
	{
		o.first->prepare(this, gui_input);
	}
}

void GUIScreen::draw()
{
	for(auto o : canvas)
	{
		o.first->draw(osp->renderer->vg, skin.get(), viewport);
	}
	win_manager.draw(osp->renderer->vg, this);
	for(auto o : post_canvas)
	{
		o.first->draw(osp->renderer->vg, skin.get(), viewport);
	}
}

void GUIScreen::add_canvas(std::shared_ptr<GUICanvas> acanvas, glm::ivec2 pos, glm::ivec2 size)
{
	canvas.emplace_back(acanvas, std::make_pair(pos, size));
}

void GUIScreen::add_post_canvas(std::shared_ptr<GUICanvas> acanvas, glm::ivec2 pos, glm::ivec2 size)
{
	post_canvas.emplace_back(acanvas, std::make_pair(pos, size));
}

void GUIScreen::new_frame(glm::ivec4 nscreen)
{
	viewport = nscreen;
	post_canvas.clear();
	canvas.clear();
}

void GUIScreen::new_frame()
{
	new_frame(glm::ivec4(0, 0, osp->renderer->get_width(true), osp->renderer->get_height(true)));
}

void GUIScreen::add_canvas(GUICanvas* canvas, glm::ivec2 pos, glm::ivec2 size)
{
	// Create a dummy shared ptr that does no deletion
	auto canvas_ptr = std::shared_ptr<GUICanvas>(canvas, null_deleter<GUICanvas>);
	add_canvas(canvas_ptr, pos, size);
}

void GUIScreen::add_post_canvas(GUICanvas* canvas, glm::ivec2 pos, glm::ivec2 size)
{
	// Create a dummy shared ptr that does no deletion
	auto canvas_ptr = std::shared_ptr<GUICanvas>(canvas, null_deleter<GUICanvas>);
	add_post_canvas(canvas_ptr, pos, size);
}
