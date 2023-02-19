#include "GUILabel.h"

void GUILabel::prepare(glm::ivec4 viewport, GUIScreen *screen, GUIInput *gui_input)
{
	 // Do nothing
}

glm::ivec2 GUILabel::position(glm::ivec2 wpos, glm::ivec2 wsize, GUIScreen* screen)
{
	return default_position(wpos, wsize);
}

void GUILabel::draw(NVGcontext* vg, GUISkin* skin)
{
	glm::ivec2 opos = pos;
	glm::ivec2 osize = size;

	if(override_color)
	{
		nvgFillColor(vg, color);
	}
	else
	{
		nvgFillColor(vg, skin->get_foreground_color());
	}

	nvgFontSize(vg, ft_size);
	nvgFontFace(vg, ft_font.c_str());
	nvgTextAlign(vg, NVG_ALIGN_TOP | NVG_ALIGN_LEFT);
	float asc, desc, lh, lwidth = size.x - 10.0f;
	nvgTextMetrics(vg, &asc, &desc, &lh);
	float bounds[4];
	if(center_horizontal || center_vertical || style == LabelStyle::SEPARATOR)
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

	if(style == LabelStyle::SEPARATOR)
	{
		// Draw lines around the text
		nvgBeginPath(vg);
		nvgStrokeWidth(vg, 5.0f);
		nvgStrokeColor(vg, skin->get_foreground_color());
		float line_y = pos.y + 0.5f * (bounds[3] - bounds[1]);
		nvgMoveTo(vg, opos.x, line_y);
		nvgLineTo(vg, pos.x - 5.0f,  line_y);
		nvgMoveTo(vg, opos.x + osize.x, line_y);
		float line_x = pos.x + (bounds[2] - bounds[0]) + 12.0f + 5.0f;
		nvgLineTo(vg, line_x, line_y);
		nvgStroke(vg);
	}

}
