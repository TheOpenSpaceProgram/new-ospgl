#include "GUICanvas.h"
#include <util/Logger.h>

std::pair<std::shared_ptr<GUICanvas>, std::shared_ptr<GUICanvas>> GUICanvas::divide_h(float nfac)
{
	logger->check(child_0 == nullptr && child_1 == nullptr, "Tried to split a non-leaf GUICanvas");

	child_0 = std::make_shared<GUICanvas>();
	child_1 = std::make_shared<GUICanvas>();

	horizontal = true;
	factor = nfac;

	if(layout != nullptr)
	{
		child_1->layout = layout;
		layout = nullptr;
	}

	return std::make_pair(child_0, child_1);
}

std::pair<std::shared_ptr<GUICanvas>, std::shared_ptr<GUICanvas>> GUICanvas::divide_v(float nfac)
{
	logger->check(child_0 == nullptr && child_1 == nullptr, "Tried to split a non-leaf GUICanvas");

	child_0 = std::make_shared<GUICanvas>();
	child_1 = std::make_shared<GUICanvas>();

	horizontal = false;
	factor = nfac;

	if(layout != nullptr)
	{
		child_1->layout = layout;
		layout = nullptr;
	}

	return std::make_pair(child_0, child_1);
}

void GUICanvas::position_widgets(glm::ivec2 pos, glm::ivec2 size, GUIScreen* screen)
{
	// We first position our children (which will do the same)
	// so the result is the canvas are prepared bottom-to-top
	if(child_0 || child_1)
	{
		// Apply pixel sizes
		if(child_pixels > 0)
		{
			float rsize;
			if(horizontal)
				rsize = (float)size.x;
			else
				rsize = (float)size.y;

			float nfac = (float)child_pixels / rsize;
			if(pixels_for_child_1)
				nfac = 1.0f - nfac;
			factor = nfac;
		}

		glm::ivec2 size0;
		glm::ivec2 pos1;
		glm::ivec2 size1;
		if(horizontal)
		{
			size0 = glm::ivec2(size.x * factor, size.y);
			pos1 = pos + glm::ivec2(size0.x, 0);
			size1 = size - glm::ivec2(size0.x, 0);
		}
		else
		{
			size0 = glm::ivec2(size.x, size.y * factor);
			pos1 = pos + glm::ivec2(0, size0.y);
			size1 = size - glm::ivec2(0, size0.y);
		}
		child_0->position_widgets(pos, size0, screen);
		child_1->position_widgets(pos1, size1, screen);
	}

	if(layout)
	{
		glm::ivec2 rpos = pos;
		glm::ivec2 rsize = size;

		layout->position_wrapper(rpos, rsize, screen);
	}

}

void GUICanvas::prepare(GUIScreen* screen, GUIInput* gui_input) const
{
	// This is done top to bottom (although order is not really vital here!)
	if(layout)
	{
		layout->prepare_wrapper(screen, gui_input);
	}

	if(child_0 || child_1)
	{
		child_0->prepare(screen, gui_input);
		child_1->prepare(screen, gui_input);
	}
}

void GUICanvas::pre_prepare(GUIScreen *screen) const
{
	if(child_0 || child_1)
	{
		child_0->pre_prepare(screen);
		child_1->pre_prepare(screen);
	}

	if(layout)
	{
		layout->pre_prepare(screen);
	}
}

void GUICanvas::draw(NVGcontext* vg, GUISkin* skin, glm::ivec4 def_scissor) const
{
	if(layout)
	{
		layout->draw(vg, skin);
		// Reset scissor test, the layout most likely used it
		if(def_scissor == glm::ivec4(0, 0, 0, 0))
		{
			nvgResetScissor(vg);
		}
		else
		{
			nvgScissor(vg, def_scissor.x, def_scissor.y, def_scissor.z, def_scissor.w);
		}
	}

	// Draw children
	if(child_0 || child_1)
	{
		child_0->draw(vg, skin, def_scissor);
		child_1->draw(vg, skin, def_scissor);
	}
}

GUICanvas::~GUICanvas()
{
}

void GUICanvas::set_layout(std::shared_ptr<GUILayout> nlayout)
{
	child_0 = nullptr;
	child_1 = nullptr;
	layout = nlayout;
}


