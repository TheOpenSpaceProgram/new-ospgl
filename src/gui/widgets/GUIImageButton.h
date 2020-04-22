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

	virtual void draw(NVGcontext* vg, GUISkin* skin) override
	{
		skin->draw_button(vg, pos, size, "", get_button_state(), GUISkin::ButtonStyle::SYMMETRIC);

		if(image > 0)
		{
			
			NVGpaint img_fill = nvgImagePattern(vg, pos.x, pos.y, size.x, size.y, 0.0f, image, 1.0f);
			nvgBeginPath(vg);
			nvgFillPaint(vg, img_fill);
			nvgRect(vg, pos.x, pos.y, size.x, size.y);
			nvgFill(vg);
		}
	}

	void set_image(NVGcontext* vg, Image* to)
	{
		image = nvglCreateImageFromHandleGL3(vg, to->id, to->get_width(), to->get_height(), 0);
	}

	void set_image(NVGcontext* vg, GLuint handle, glm::ivec2 size)
	{
		image = nvglCreateImageFromHandleGL3(vg, handle, size.x, size.y, 0);
	}

};
