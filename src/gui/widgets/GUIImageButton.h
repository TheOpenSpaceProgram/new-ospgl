#pragma once 
#include "../GUIWidget.h"
#include "GUIBaseButton.h"
#include <assets/Image.h>

#include <string>
#include <nanovg/nanovg_gl.h>

class GUIImageButton : public GUIBaseButton
{
public:
	
	// NanoVG image handle
	int image = -1;
	std::string name;

	// Use negative values to let that axis free
	glm::ivec2 force_image_size;

	// TODO: Write these properly and add the image!
	virtual glm::ivec2 prepare(glm::ivec2 wpos, glm::ivec2 wsize, GUIInput* ipt) override
	{	
		pos = wpos;
		size = force_image_size;

		do_button(pos, size, ipt);

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

		if(image > 0)
		{
			
			NVGpaint img_fill = nvgImagePattern(vg, x, y, size.x, size.y, 0.0f, image, 1.0f);
			nvgBeginPath(vg);
			nvgFillPaint(vg, img_fill);
			nvgRect(vg, x, y, w, h);
			nvgFill(vg);
		}
		else
		{
			nvgFontSize(vg, 14.0f);
			nvgFontFace(vg, "bold");
			nvgTextAlign(vg, NVG_ALIGN_CENTER | NVG_ALIGN_MIDDLE);
			nvgFillColor(vg, nvgRGB(0, 0, 0));
			nvgText(vg, x + floor(w / 2), y + floor(h / 2), name.c_str(), nullptr);
		}
	}

	void set_image(NVGcontext* vg, Image* to)
	{
		image = nvglCreateImageFromHandleGL3(vg, to->id, to->get_width(), to->get_height(), 0);
	}

};
