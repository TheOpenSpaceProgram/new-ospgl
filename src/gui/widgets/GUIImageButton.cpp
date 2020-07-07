#include "GUIImageButton.h"

glm::ivec2 GUIImageButton::prepare(glm::ivec2 wpos, glm::ivec2 wsize, GUIInput* ipt)
{	
	pos = wpos;
	size = force_image_size;
	if(size.x < 0)
	{
		size.x = wsize.x;
	}

	if(size.y < 0)
	{
		size.y = wsize.y;
	}

	do_button(pos, size, ipt);

	return size;
}

void GUIImageButton::draw(NVGcontext* vg, GUISkin* skin)
{
	skin->draw_button(vg, pos, size, "", get_button_state(), GUISkin::ButtonStyle::SYMMETRIC);

	if(image > 0)
	{
		NVGpaint img_fill;
		if(img_mode == STRETCH)
		{
			img_fill = nvgImagePattern(vg, pos.x, pos.y, size.x, size.y, 0.0f, image, 1.0f);
			nvgBeginPath(vg);
			nvgFillPaint(vg, img_fill);
			nvgRect(vg, pos.x, pos.y, size.x, size.y);
			nvgFill(vg);
		}
		else if(img_mode == CENTER)
		{
			glm::ivec2 imgpos = pos + size / 2 - img_size / 2;
			img_fill = nvgImagePattern(vg, imgpos.y, imgpos.y, img_size.x, img_size.y, 0.0f, image, 1.0f);
			// We just have to draw the center rectangle
			nvgBeginPath(vg);
			nvgFillPaint(vg, img_fill);
			nvgRect(vg, imgpos.x, imgpos.y, img_size.x, img_size.y);
			nvgFill(vg);
		}
		else if(img_mode == TILE)
		{
			img_fill = nvgImagePattern(vg, pos.x, pos.y, img_size.x, img_size.y, 0.0f, image, 1.0f);
			nvgBeginPath(vg);
			nvgFillPaint(vg, img_fill);
			nvgRect(vg, pos.x, pos.y, size.x, size.y);
			nvgFill(vg);
		}

	}
}

void GUIImageButton::set_image(NVGcontext* vg, Image* to)
{
	image = nvglCreateImageFromHandleGL3(vg, to->id, to->get_width(), to->get_height(), 0);
	img_size = to->get_size();
}

void GUIImageButton::set_image(NVGcontext* vg, GLuint handle, glm::ivec2 size)
{
	image = nvglCreateImageFromHandleGL3(vg, handle, size.x, size.y, 0);
	img_size = size;
}