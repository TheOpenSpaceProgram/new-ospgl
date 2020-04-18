#pragma once 
#include "../GUIWidget.h"

#include <string>

class GUIImageButton : public GUIWidget
{
public:
	

	bool hlight = false;
	std::string name;

	virtual glm::ivec2 prepare(glm::ivec2 wpos, glm::ivec2 wsize, GUIInput* ipt) override
	{	
		hlight = false;
		pos = wpos;
		size = glm::ivec2(wsize.x, 32);

		if(!ipt->ext_mouse_blocked)
		{
			if(ipt->is_mouse_inside(pos, size))
			{
				ipt->mouse_blocked = true;
				hlight = true;
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
		if(hlight)
		{
			nvgFillColor(vg, nvgRGB(128, 255, 128));
		}
		else
		{
			nvgFillColor(vg, nvgRGB(64, 128, 64));
		}
		nvgFill(vg);

		nvgFontSize(vg, 12.0f);
		nvgFontFace(vg, "regular");
		nvgTextAlign(vg, NVG_ALIGN_CENTER | NVG_ALIGN_MIDDLE);
		nvgFillColor(vg, nvgRGB(0, 0, 0));
		nvgText(vg, x + floor(w / 2), y + floor(h / 2), name.c_str(), nullptr);
	}

};
