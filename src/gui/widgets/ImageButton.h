#pragma once 
#include "../GUIWidget.h"

class ImageButton : public GUIWidget
{
public:

	bool hlight = false;

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
	}

};
