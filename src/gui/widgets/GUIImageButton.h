#pragma once 
#include "../GUIWidget.h"
#include "GUIBaseButton.h"

#include <string>

class GUIImageButton : public GUIBaseButton
{
public:
	

	std::string name;

	// Use negative values to let that axis free
	glm::ivec2 force_image_size;

	// TODO: Write these properly and add the image!
	virtual glm::ivec2 prepare(glm::ivec2 wpos, glm::ivec2 wsize, GUIInput* ipt) override
	{	
		pos = wpos;
		size = force_image_size;

		if(!ipt->ext_mouse_blocked)
		{
			if(ipt->is_mouse_inside(pos, size))
			{
				ipt->mouse_blocked = true;

				set_hover(true);
				during_hover();
			}
			else
			{
				set_hover(false);
			}
		}

		return size;
	}

	virtual void draw(NVGcontext* vg) override
	{
		float x = (float)pos.x;
		float y = (float)pos.y;
		float w = (float)size.x;
		float h = (float)size.y;

		nvgBeginPath(vg);
		nvgRect(vg, x, y, w, h);
		if(hover)
		{
			nvgFillColor(vg, nvgRGB(128, 255, 128));
		}
		else
		{
			nvgFillColor(vg, nvgRGB(64, 128, 64));
		}
		nvgFill(vg);

		nvgFontSize(vg, 14.0f);
		nvgFontFace(vg, "bold");
		nvgTextAlign(vg, NVG_ALIGN_CENTER | NVG_ALIGN_MIDDLE);
		nvgFillColor(vg, nvgRGB(0, 0, 0));
		nvgText(vg, x + floor(w / 2), y + floor(h / 2), name.c_str(), nullptr);
	}

};
