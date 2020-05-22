#include "GUICanvas.h"
#include <util/Logger.h>

std::pair<GUICanvas*, GUICanvas*> GUICanvas::divide_h(float nfac)
{
	logger->check(child_0 == nullptr && child_1 == nullptr, "Tried to split a non-leaf GUICanvas");

	fac = nfac;

	child_0 = new GUICanvas();
	child_1 = new GUICanvas();

	float n_canvas_size = nfac * factor.x;
	child_0->factor = glm::vec2(n_canvas_size, factor.y);
	child_0->position = position;

	child_1->position = position;
	child_1->factor = factor;	
	child_1->position.x += n_canvas_size;
	child_1->factor.x -= n_canvas_size;

	horizontal = true;

	if(layout != nullptr)
	{
		child_1->layout = layout;
		layout = nullptr;
	}

	return std::make_pair(child_0, child_1);
}

std::pair<GUICanvas*, GUICanvas*> GUICanvas::divide_v(float nfac)
{
	logger->check(child_0 == nullptr && child_1 == nullptr, "Tried to split a non-leaf GUICanvas");

	fac = nfac;

	child_0 = new GUICanvas();
	child_1 = new GUICanvas();

	float n_canvas_size = nfac * factor.y;
	child_0->factor = glm::vec2(factor.x, n_canvas_size);
	child_0->position = position;

	child_1->position = position;
	child_1->factor = factor;	
	child_1->position.y += n_canvas_size;
	child_1->factor.y -= n_canvas_size;

	horizontal = false;

	if(layout != nullptr)
	{
		child_1->layout = layout;
		layout = nullptr;
	}

	return std::make_pair(child_0, child_1);
}

void GUICanvas::prepare(glm::ivec2 pos, glm::ivec2 size, GUIInput* gui_input)
{
	// Apply pixel sizes
	if(child_0 || child_1)
	{
		if(child_0_pixels > 0)
		{
			float rsize;
			if(horizontal)
			{
				rsize = (float)size.x * factor.x;
			}
			else
			{
				rsize = (float)size.y * factor.y;
			}

			float nfac = (float)child_0_pixels / rsize;
			resize(nfac);
		}

		child_0->prepare(pos, size, gui_input);
		child_1->prepare(pos, size, gui_input);	
	}

	if(layout)
	{
		glm::ivec2 rpos = pos + glm::ivec2(position * glm::vec2(size));
		glm::ivec2 rsize = glm::ivec2(glm::vec2(size) * factor);

		layout->prepare_wrapper(rpos, rsize, gui_input);
	}

}

void GUICanvas::debug(glm::ivec2 real_pos, glm::ivec2 real_size, NVGcontext* vg)
{
	if(child_0 == nullptr && child_1 == nullptr)
	{

		float pos_x = real_pos.x + position.x * real_size.x;
		float pos_y = real_pos.y + position.y * real_size.y;
		float width = real_size.x * factor.x;
		float height = real_size.y * factor.y;

		nvgBeginPath(vg);
		nvgRect(vg, pos_x, pos_y, width, height);
		nvgStrokeColor(vg, nvgRGB(255, 255, 255));
		nvgStrokeWidth(vg, 1.0f);
		nvgStroke(vg);	

	}
	else
	{
		child_0->debug(real_pos, real_size, vg);
		child_1->debug(real_pos, real_size, vg);
	}

}

void GUICanvas::draw(NVGcontext* vg, GUISkin* skin, glm::ivec4 def_scissor)
{
	if(layout)
	{
		layout->draw(vg, skin);
		// Reset scissor test, the layout most likely used it
		nvgScissor(vg, def_scissor.x, def_scissor.y, def_scissor.z, def_scissor.w);	
	}

	// Draw children
	if(child_0 || child_1)
	{
		child_0->draw(vg, skin, def_scissor);
		child_1->draw(vg, skin, def_scissor);
	}
}


void GUICanvas::resize(float n_factor)
{
	if(child_0 == nullptr || child_1 == nullptr)
	{
		return;
	}

	if(horizontal)
	{
		float n_canvas_size = n_factor * factor.x;
		
		child_0->factor = glm::vec2(n_canvas_size, factor.y);
		child_0->position = position;

		child_1->position = position;
		child_1->factor = factor;	
		child_1->position.x += n_canvas_size;
		child_1->factor.x -= n_canvas_size;
	}
	else
	{

		float n_canvas_size = n_factor * factor.y;
		child_0->factor = glm::vec2(factor.x, n_canvas_size);
		child_0->position = position;

		child_1->position = position;
		child_1->factor = factor;	
		child_1->position.y += n_canvas_size;
		child_1->factor.y -= n_canvas_size;
	}

	fac = n_factor;

	child_0->update_children();
	child_1->update_children();

}

void GUICanvas::update_children()
{
	if(child_0 != nullptr && child_1 != nullptr)
	{
		resize(fac);
		child_0->update_children();
		child_1->update_children();	
	}
}

