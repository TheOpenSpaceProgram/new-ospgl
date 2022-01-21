#include "GUITextButton.h"

void GUITextButton::draw(NVGcontext *vg, GUISkin *skin)
{
	skin->draw_button(vg, pos, size, "", get_button_state(), GUISkin::ButtonStyle::SYMMETRIC);

	nvgResetScissor(vg);
	if(override_color)
	{
		nvgFillColor(vg, color);
	}
	else
	{
		nvgFillColor(vg, skin->get_button_color(get_button_state()));
	}
	if(uses_bitmap)
	{
		AssetHandle<BitmapFont> bfont = AssetHandle<BitmapFont>("core:fonts/ProggySquare.fnt");
		nvgBitmapText(vg, bfont.duplicate(), 0, pos.x, pos.y, text.c_str());
	}
	else
	{
		nvgFontSize(vg, ft_size);
		nvgFontFace(vg, ft_font.c_str());
		float asc, desc, lh, lwidth = size.x - 10.0f;
		nvgTextMetrics(vg, &asc, &desc, &lh);
		float bounds[4];
		if(center_horizontal || center_vertical)
		{
			nvgTextBoxBounds(vg, pos.x + 6.0f, pos.y + lh + 8.0f, lwidth, text.c_str(), nullptr, bounds);
		}
		if(center_horizontal)
		{
			float free_h = lwidth - (bounds[2] - bounds[0]);
			pos.x += free_h * 0.5f;
		}
		if(center_vertical)
		{
			float free_v = (size.y - 10.0f) - (bounds[3] - bounds[1]);
			pos.y += free_v * 0.5f - lh * 0.25f;
		}
		pos.x = glm::round(pos.x);
		pos.y = glm::round(pos.y);
		nvgTextBox(vg, pos.x + 6.0f, pos.y + lh + 8.0f, lwidth, text.c_str(), nullptr);
	}
}

glm::ivec2 GUITextButton::prepare(glm::ivec2 wpos, glm::ivec2 wsize, glm::ivec4 viewport, GUIInput *ipt)
{
	pos = wpos;
	size = wsize;
	if(size.x < 0)
	{
		size.x = wsize.x;
	}

	if(size.y < 0)
	{
		size.y = wsize.y;
	}

	do_button(pos, size, viewport, ipt);

	return size;
}
