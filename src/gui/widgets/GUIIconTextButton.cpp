#include "GUIIconTextButton.h"

void GUIIconTextButton::set_image(NVGcontext *vg, AssetHandle<Image> &&to)
{
	image = to.get_noconst()->get_nvg_image(vg);
	img_handle = std::move(to);
	img_size = img_handle->get_size();
}

void GUIIconTextButton::draw(NVGcontext *vg, GUISkin *skin)
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

	nvgFontSize(vg, ft_size);
	nvgFontFace(vg, ft_font.c_str());
	nvgTextAlign(vg, NVG_ALIGN_TOP | NVG_ALIGN_LEFT);
	float asc, desc, lh, lwidth = size.x - 10.0f - img_size.x;
	nvgTextMetrics(vg, &asc, &desc, &lh);
	float bounds[4];
	pos.x += img_size.x + image_text_margin;
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

	pos.x -= img_size.x + image_text_margin;
	pos.y += -img_size.y / 2 + lh * 0.5f;
	NVGpaint img_fill = nvgImagePattern(vg, pos.x, pos.y, img_size.x, img_size.y, 0.0f, image, 1.0f);
	nvgBeginPath(vg);
	nvgFillPaint(vg, img_fill);
	nvgRect(vg, pos.x, pos.y, img_size.x, img_size.y);
	nvgFill(vg);

}

glm::ivec2 GUIIconTextButton::position(glm::ivec2 wpos, glm::ivec2 wsize, GUIScreen *screen)
{
	return default_position(wpos, wsize);
}

void GUIIconTextButton::prepare(glm::ivec4 viewport, GUIScreen *screen, GUIInput *gui_input)
{
	do_button(pos, size, viewport, gui_input);
}
