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
		nvgTextAlign(vg, NVG_ALIGN_TOP | NVG_ALIGN_LEFT);
		float asc, desc, lh, lwidth = size.x - 10.0f;
		nvgTextMetrics(vg, &asc, &desc, &lh);
		float bounds[4];
		if(center_horizontal || center_vertical)
		{
			nvgTextBoxBounds(vg, pos.x + 6.0f, pos.y, lwidth, text.c_str(), nullptr, bounds);
		}
		if(center_horizontal)
		{
			float free_h = lwidth - (bounds[2] - bounds[0]);
			pos.x += free_h * 0.5f;
		}
		if(center_vertical)
		{
			float free_v = (size.y - 10.0f) - (bounds[3] - bounds[1]);
			pos.y += free_v * 0.5f + lh * 0.5f;
		}
		pos.x = glm::round(pos.x);
		pos.y = glm::round(pos.y);
		nvgTextBox(vg, pos.x + 6.0f, pos.y, lwidth, text.c_str(), nullptr);
	}
}

void GUITextButton::prepare(glm::ivec4 viewport, GUIScreen *screen, GUIInput *gui_input)
{
	do_button(pos, size, viewport, gui_input);
}

glm::ivec2 GUITextButton::position(glm::ivec2 wpos, glm::ivec2 wsize, GUIScreen* screen)
{
	return default_position(wpos, wsize);
}
