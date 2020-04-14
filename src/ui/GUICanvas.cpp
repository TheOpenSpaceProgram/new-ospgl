#include "GUICanvas.h"
#include <util/Logger.h>

GUICanvas* GUICanvas::divide_h(float nfac)
{
	logger->check_important(child_0 == nullptr && child_1 == nullptr, "Tried to split a non-leaf GUICanvas");

	fac = nfac;

	child_0 = new GUICanvas();
	child_1 = new GUICanvas();

	float n_canvas_size = nfac * factor.x;
	child_1->factor = glm::vec2(n_canvas_size, factor.y);
	child_1->position = position;

	child_0->position = position;
	child_0->factor = factor;	
	child_0->position.x += n_canvas_size;
	child_0->factor.x -= n_canvas_size;

	horizontal = true;

	return child_0;
}

GUICanvas* GUICanvas::divide_v(float nfac)
{
	logger->check_important(child_0 == nullptr && child_1 == nullptr, "Tried to split a non-leaf GUICanvas");

	fac = nfac;

	child_0 = new GUICanvas();
	child_1 = new GUICanvas();

	float n_canvas_size = nfac * factor.y;
	child_1->factor = glm::vec2(factor.x, n_canvas_size);
	child_1->position = position;

	child_0->position = position;
	child_0->factor = factor;	
	child_0->position.y += n_canvas_size;
	child_0->factor.y -= n_canvas_size;

	horizontal = false;

	return child_0;
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


void GUICanvas::resize(float n_factor)
{
	if(child_0 == nullptr || child_1 == nullptr)
	{
		return;
	}

	if(horizontal)
	{
		float n_canvas_size = n_factor * factor.x;
		
		child_1->factor = glm::vec2(n_canvas_size, factor.y);
		child_1->position = position;

		child_0->position = position;
		child_0->factor = factor;	
		child_0->position.x += n_canvas_size;
		child_0->factor.x -= n_canvas_size;
	}
	else
	{

		float n_canvas_size = n_factor * factor.y;
		child_1->factor = glm::vec2(factor.x, n_canvas_size);
		child_1->position = position;

		child_0->position = position;
		child_0->factor = factor;	
		child_0->position.y += n_canvas_size;
		child_0->factor.y -= n_canvas_size;
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

