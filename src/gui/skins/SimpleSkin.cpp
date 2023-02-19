#include "SimpleSkin.h"
#include "../GUIWindow.h"

void SimpleSkin::draw_button(NVGcontext* v, glm::ivec2 pos, glm::ivec2 size, const std::string& text,
	ButtonState state, ButtonStyle style)
{
	
	nvgBeginPath(v);	

	int bevel_pixels = 10;
	
	if(size.x <= 32 || size.y <= 32)
	{
		// Fallback to a rectangle, bevel looks wrong
		nvgRect(v, pos.x - 0.5f, pos.y - 0.5f, size.x + 1.0f, size.y + 1.0f);	
	}
	else
	{
	
		if(style == ButtonStyle::NORMAL)
		{
			nvgMoveTo(v, pos.x + 0.5f, pos.y + size.y + 0.5f);
			nvgLineTo(v, pos.x + size.x - bevel_pixels + 0.5f, pos.y + size.y + 0.5f);
			nvgLineTo(v, pos.x + size.x + 0.5f, pos.y + size.y - bevel_pixels + 0.5f);
			nvgLineTo(v, pos.x + size.x + 0.5f, pos.y + 0.5f);
			nvgLineTo(v, pos.x + bevel_pixels + 0.5f, pos.y + 0.5f);
			nvgLineTo(v, pos.x + 0.5f, pos.y + bevel_pixels + 0.5f);
			nvgLineTo(v, pos.x + 0.5f, pos.y + size.y + 0.5f);	
		}
		else if(style == ButtonStyle::SYMMETRIC)
		{
			// Similar to before but with "bevel" on all borders
			nvgMoveTo(v, pos.x + bevel_pixels + 0.5f, pos.y + 0.5f);
			nvgLineTo(v, pos.x + 0.5f, pos.y + bevel_pixels + 0.5f);
			nvgLineTo(v, pos.x + 0.5f, pos.y + size.y - bevel_pixels + 0.5f);
			nvgLineTo(v, pos.x + bevel_pixels + 0.5f, pos.y + size.y + 0.5f);
			nvgLineTo(v, pos.x + size.x - bevel_pixels + 0.5f, pos.y + size.y + 0.5f);
			nvgLineTo(v, pos.x + size.x + 0.5f, pos.y + size.y - bevel_pixels + 0.5f);
			nvgLineTo(v, pos.x + size.x + 0.5f, pos.y + bevel_pixels + 0.5f);
			nvgLineTo(v, pos.x + size.x - bevel_pixels + 0.5f, pos.y + 0.5f);
			nvgLineTo(v, pos.x + bevel_pixels + 0.5f, pos.y + 0.5f);	
		}
	}

	nvgStrokeWidth(v, 1.0f);

	if(style == ButtonStyle::GOOD)
	{
	}
	else if(style == ButtonStyle::BAD)
	{
		
	}
	else
	{
		nvgFillColor(v, get_fill_color(state));
		nvgStrokeColor(v, get_stroke_color(state));
	}

	nvgFill(v);
	nvgStroke(v);

	// Basic, nonconfigurable text rendering for quick-&-dirty text buttons
	if(!text.empty())
	{
		nvgFillColor(v, get_foreground_color());
		nvgTextAlign(v, NVG_ALIGN_CENTER | NVG_ALIGN_MIDDLE);
		nvgText(v, pos.x + size.x * 0.5f, pos.y + size.y * 0.5f, text.c_str(), nullptr);
	}

}

bool SimpleSkin::can_drag_window(GUIWindow* window, glm::ivec2 mpos) 
{
	if(window->is_pinned())
	{
		return false;
	}
	float side_size = window_margins + window_edge_size;
	glm::vec2 title_tl = glm::vec2(window->pos.x - side_size, window->pos.y - titlebar_height - side_size) + 0.5f;
	glm::vec2 title_br = glm::vec2(window->pos.x + window->size.x + side_size, window->pos.y - window_margins) + 0.5f;

	return mpos.x > title_tl.x && mpos.y > title_tl.y && mpos.x < title_br.x && mpos.y < title_br.y;
}

bool SimpleSkin::can_close_window(GUIWindow* window, glm::ivec2 mpos) 
{
	if(window->is_pinned())
	{
		return false;
	}

	float side_size = window_margins + window_edge_size;
	glm::vec2 button_pos = 
		glm::vec2(window->pos.x + window->size.x - window_icon_size, 
		window->pos.y - titlebar_height - side_size + window_icon_size * 0.5f) + 0.5f;

	return mpos.x > button_pos.x - 2.0f && mpos.y > button_pos.y - 2.0f && 
		mpos.x < button_pos.x + window_icon_size + 2.0f && mpos.y < button_pos.y + window_icon_size + 2.0f;
}

bool SimpleSkin::can_minimize_window(GUIWindow* window, glm::ivec2 mpos) 
{
	if(window->is_pinned())
	{
		return false;
	}

	float side_size = window_margins + window_edge_size;
	glm::vec2 button_pos = 
		glm::vec2(window->pos.x + window->size.x - window_icon_size, 
		window->pos.y - titlebar_height - side_size + window_icon_size * 0.5f) + 0.5f;
	if(window->closeable)
	{
		button_pos.x -= (window_icon_size + 2.0f);
	}

	return mpos.x > button_pos.x - 2.0f && mpos.y > button_pos.y - 2.0f && 
		mpos.x < button_pos.x + window_icon_size + 2.0f && mpos.y < button_pos.y + window_icon_size + 2.0f;
}

bool SimpleSkin::can_pin_window(GUIWindow* window, glm::ivec2 mpos) 
{
	float side_size = window_margins + window_edge_size;
	glm::vec2 button_pos = 
		glm::vec2(window->pos.x + window->size.x - window_icon_size, 
		window->pos.y - titlebar_height - side_size + window_icon_size * 0.5f) + 0.5f;
	if(window->closeable && !window->is_pinned())
	{
		button_pos.x -= (window_icon_size + 2.0f);
	}
	if(window->minimizable && !window->is_pinned())
	{
		button_pos.x -= (window_icon_size + 2.0f);
	}

	return mpos.x > button_pos.x - 2.0f && mpos.y > button_pos.y - 2.0f && 
		mpos.x < button_pos.x + window_icon_size + 2.0f && mpos.y < button_pos.y + window_icon_size + 2.0f;
}

SimpleSkin::ResizePoint SimpleSkin::can_resize_window(GUIWindow* window, glm::ivec2 mpos) 
{
	if(window->is_minimized() || window->is_pinned())
	{
		return SimpleSkin::NONE;
	}

	float margin = window_margins + window_edge_size;
	glm::ivec4 outer = get_window_aabb(window);
	glm::ivec4 inner = glm::ivec4(outer.x + margin, outer.y + margin * 0.5f, 
		outer.z - margin * 2.0f, outer.w - margin * 2.0f);

	bool in_outer = mpos.x > outer.x && mpos.y > outer.y && mpos.x < outer.x + outer.z && mpos.y < outer.y + outer.w;
	bool in_inner = mpos.x > inner.x && mpos.y > inner.y && mpos.x < inner.x + inner.z && mpos.y < inner.y + inner.w;

	if(in_outer && !in_inner)
	{
		// We can resize, check position
		bool left = mpos.x < inner.x && mpos.x > outer.x;
		bool right = mpos.x > inner.x + inner.z  && mpos.x < outer.x + outer.z;
		bool top = mpos.y < inner.y + titlebar_height && mpos.y > outer.y;
		bool bottom = mpos.y > inner.y + inner.w && mpos.y < outer.y + outer.w;

		if	(left && bottom && !top)	{ return BOTTOM_LEFT; }
		else if (right && bottom && !top)	{ return BOTTOM_RIGHT; }
		else if (right && !top)				{ return RIGHT; }
		else if (bottom && !top) 			{ return BOTTOM; }
		else if (left && !top) 				{ return LEFT; }

	}
	
	return NONE;
}

glm::ivec4 SimpleSkin::get_window_aabb(GUIWindow* window) 
{
	glm::ivec4 aabb;

	aabb.x = window->pos.x - window_margins - window_edge_size;
	aabb.y = window->pos.y - window_margins - window_edge_size - titlebar_height;
	aabb.z = window->size.x + (window_margins + window_edge_size) * 2;
	aabb.w = window->size.y + window_margins * 2 + window_edge_size + titlebar_height;
	if(window->is_minimized())
	{
		aabb.w = titlebar_height + window_edge_size;
	}
	

	return aabb;
}

void SimpleSkin::draw_window(NVGcontext* vg, GUIWindow* window) 
{
	nvgStrokeWidth(vg, 1.0);
	glm::vec2 pos = window->pos;
	glm::vec2 size = window->size;

	float side_size = window_margins + window_edge_size;

	NVGcolor body_background = nvgTransRGBA(get_background_color(), window->alpha * 255);
	NVGcolor title_background;
	if(window->drag_hovered)
	{
		title_background = nvgRGBA(71, 80, 89, window->alpha * 255);
	}
	else
	{
		title_background = nvgTransRGBA(get_background_color(true), window->alpha * 255);
	}
	NVGcolor outline_color;
	if(window->is_focused())
	{
		outline_color = nvgRGB(170, 170, 170);
	}
	else
	{
		outline_color = nvgRGB(61, 70, 79);
	}

	glm::vec2 title_tl = glm::vec2(pos.x - side_size, pos.y - titlebar_height - side_size) + 0.5f;
	glm::vec2 title_tr = glm::vec2(pos.x + size.x, pos.y - titlebar_height - side_size) + 0.5f;
	glm::vec2 title_trb = glm::vec2(pos.x + size.x + side_size, pos.y - titlebar_height) + 0.5f;
	glm::vec2 title_br = glm::vec2(pos.x + size.x + side_size, pos.y - window_margins) + 0.5f;
	glm::vec2 title_bl = glm::vec2(pos.x - side_size, pos.y - window_margins) + 0.5f;
	glm::vec2 win_br = title_br + glm::vec2(0.0f, size.y + side_size);
	glm::vec2 win_bbr = win_br + glm::vec2(-4.0f, 4.0f);
	glm::vec2 win_bl = win_br - glm::vec2(size.x + side_size * 2.0f, 0.0f);
	glm::vec2 win_bbl = win_bl + glm::vec2(4.0f, 4.0f);

	if (window->is_pinned())
	{
		// We ignore alpha and just use a fixed value for pinned windows
		NVGcolor body_background_pinned = nvgTransRGBA(get_background_color(), 180);

		glm::vec2 fsize = glm::vec2(size.x + side_size * 2.0f, size.y + side_size * 2.0f + titlebar_height);
		// We draw a simple rect with the titlebar space
		nvgBeginPath(vg);
		nvgRect(vg, title_tl.x, title_tl.y, fsize.x, fsize.y);
		nvgFillColor(vg, body_background_pinned);
		nvgStrokeColor(vg, outline_color);
		nvgFill(vg);
		nvgStroke(vg);

	}
	else
	{
		// Titlebar background is brighter
		nvgBeginPath(vg);
		nvgMoveTo(vg, title_tl.x, title_tl.y);
		nvgLineTo(vg, title_tr.x, title_tr.y);
		nvgLineTo(vg, title_trb.x, title_trb.y);
		nvgLineTo(vg, title_br.x, title_br.y);
		nvgLineTo(vg, title_bl.x, title_bl.y);
		nvgLineTo(vg, title_tl.x, title_tl.y);
		nvgFillColor(vg, title_background);
		nvgFill(vg);


		nvgStrokeColor(vg, outline_color);

		if (window->is_minimized())
		{
			// Only draw the titlebar outline
			nvgBeginPath(vg);
			nvgMoveTo(vg, title_tl.x, title_tl.y);
			nvgLineTo(vg, title_tr.x, title_tr.y);
			nvgLineTo(vg, title_trb.x, title_trb.y);
			nvgLineTo(vg, title_br.x, title_br.y);
			nvgLineTo(vg, title_bl.x, title_bl.y);
			nvgLineTo(vg, title_tl.x, title_tl.y);
			nvgStroke(vg);
		}
		else
		{
			// Window background is darker
			nvgBeginPath(vg);
			nvgMoveTo(vg, title_bl.x, title_bl.y - 0.5f);
			nvgLineTo(vg, title_br.x, title_br.y - 0.5f);
			nvgLineTo(vg, win_br.x, win_br.y);
			nvgLineTo(vg, win_bbr.x, win_bbr.y);
			nvgLineTo(vg, win_bbl.x, win_bbl.y);
			nvgLineTo(vg, win_bl.x, win_bl.y);
			nvgLineTo(vg, title_bl.x, title_bl.y - 0.5f);
			nvgFillColor(vg, body_background);
			nvgFill(vg);

			nvgBeginPath(vg);
			nvgMoveTo(vg, title_tl.x, title_tl.y);
			nvgLineTo(vg, title_tr.x, title_tr.y);
			nvgLineTo(vg, title_trb.x, title_trb.y);
			nvgLineTo(vg, title_br.x, title_br.y);
			nvgLineTo(vg, win_br.x, win_br.y);
			nvgLineTo(vg, win_bbr.x, win_bbr.y);
			nvgLineTo(vg, win_bbl.x, win_bbl.y);
			nvgLineTo(vg, win_bl.x, win_bl.y);
			nvgLineTo(vg, title_tl.x, title_tl.y);
			nvgStroke(vg);
		}
	}


	nvgFillColor(vg, nvgRGB(255, 255, 255));
	nvgBitmapText(vg, window_title_font.duplicate(), TextDrawer::LEFT,
				  title_tl.x + titlebar_margin + 0.5f, title_tl.y + titlebar_margin + 0.5f, window->title.c_str());

	bool bools[3] = {window->closeable, window->minimizable, window->pinable};
	if (window->is_pinned())
	{
		bools[0] = false;
		bools[1] = false;
	}
	bool hovered[3] = {window->close_hovered, window->minimize_hovered, window->pin_hovered};
	AssetHandle<Image> *imgs[3] = {&window_close, &window_min, &window_pin};
	float x_offset = 0.0f;
	for (int i = 0; i < 3; i++)
	{
		if (bools[i])
		{
			float alpha = hovered[i] ? 1.0f : 0.5f;
			glm::vec2 pos = title_tr + glm::vec2(-window_icon_size - x_offset, window_icon_size * 0.5f);
			x_offset += window_icon_size + 2.0f;
			NVGpaint pnt = nvgImagePattern(vg, pos.x, pos.y, window_icon_size, window_icon_size,
										   0.0f, (*imgs[i]).get_noconst()->get_nvg_image(vg), alpha);
			nvgBeginPath(vg);
			nvgRect(vg, pos.x, pos.y, window_icon_size, window_icon_size);
			nvgFillPaint(vg, pnt);
			nvgFill(vg);
		}

	}
}


void
SimpleSkin::draw_dropdown_header(NVGcontext *v, glm::ivec2 pos, glm::ivec2 size, const std::string &text, bool is_open,
								 bool default_text, ButtonState st)
{
	nvgBeginPath(v);
	// Stylized dropdown, maybe really ugly but i like it

	int bevel_pixels = 10;
	// Main box
	nvgMoveTo(v, pos.x + 0.5f, pos.y + size.y + 0.5f);
	nvgLineTo(v, pos.x + size.x - bevel_pixels + 0.5f, pos.y + size.y + 0.5f);
	nvgLineTo(v, pos.x + size.x + 0.5f, pos.y + size.y - bevel_pixels + 0.5f);
	nvgLineTo(v, pos.x + size.x + 0.5f, pos.y + 0.5f);
	nvgLineTo(v, pos.x + 0.5f, pos.y + 0.5f);
	nvgLineTo(v, pos.x + 0.5f, pos.y + bevel_pixels + 0.5f);
	nvgLineTo(v, pos.x + 0.5f, pos.y + size.y + 0.5f);

	nvgFillColor(v, get_fill_color(st));
	nvgStrokeColor(v, get_stroke_color(st));
	nvgFill(v);
	nvgStroke(v);

	// Right drop down arrow
	nvgBeginPath(v);
	nvgMoveTo(v, pos.x + size.x - bevel_pixels + 0.5f, pos.y + size.y + 0.5f);
	nvgLineTo(v, pos.x + size.x - bevel_pixels * 2.0f + 0.5f, pos.y + size.y - bevel_pixels + 0.5f);
	nvgLineTo(v, pos.x + size.x - bevel_pixels * 2.0f + 0.5f, pos.y + 0.5f);
	nvgLineTo(v, pos.x + size.x + 0.5f, pos.y + 0.5f);
	nvgLineTo(v, pos.x + size.x + 0.5f, pos.y + size.y - bevel_pixels + 0.5f);
	nvgLineTo(v, pos.x + size.x - bevel_pixels + 0.5f, pos.y + size.y + 0.5f);

	nvgFillColor(v, get_fill_color(ButtonState::CLICKED));
	nvgStrokeColor(v, get_stroke_color(ButtonState::CLICKED));
	nvgFill(v);
	nvgStroke(v);

	// Text
	nvgFillColor(v, get_foreground_color(default_text));
	nvgTextAlign(v, NVG_ALIGN_MIDDLE | NVG_ALIGN_LEFT);
	nvgText(v, pos.x + 3.5f, pos.y + size.y * 0.5f + 2.0f, text.c_str(), nullptr);

}

void
SimpleSkin::draw_dropdown_body(NVGcontext *vg, glm::ivec2 pos, glm::ivec2 size, const std::vector<std::string> elems)
{
	// The dropdown body takes its whole canvas
}

SimpleSkin::SimpleSkin()
{
	window_title_font = AssetHandle<BitmapFont>("core:fonts/ProggySquare.fnt");
	window_close = AssetHandle<Image>("core:gui/close.png");
	window_min = AssetHandle<Image>("core:gui/minimize.png");
	window_pin = AssetHandle<Image>("core:gui/pin.png");
}

NVGcolor SimpleSkin::get_background_color(bool bright)
{
	if(bright)
	{
		return nvgRGB(61, 70, 79);
	}
	else
	{
		return nvgRGB(40, 45, 50);
	}
}

NVGcolor SimpleSkin::get_fill_color(GUISkin::ButtonState state)
{
	if(state == ButtonState::CLICKED)
	{
		return nvgRGB(73, 73, 73);
	}
	else if(state == ButtonState::DISABLED)
	{
		return nvgRGB(21, 21, 21);
	}
	else if(state == ButtonState::HOVERED)
	{
		return nvgRGB(41, 41, 41);
	}
	else
	{
		return nvgRGB(21, 21, 21);
	}
}

NVGcolor SimpleSkin::get_stroke_color(GUISkin::ButtonState state)
{
	if(state == ButtonState::CLICKED)
	{
		return nvgRGB(142, 142, 142);
	}
	else if(state == ButtonState::DISABLED)
	{
		return nvgRGB(41, 41, 41);
	}
	else if(state == ButtonState::HOVERED)
	{
		return nvgRGB(73, 73, 73);
	}
	else
	{
		return nvgRGB(73, 73, 73);
	}
}

NVGcolor SimpleSkin::get_error_color()
{
	return nvgRGB(255, 70, 70);
}

NVGcolor SimpleSkin::get_highlight_color()
{
	return nvgRGB(70, 70, 255);
}

NVGcolor SimpleSkin::get_foreground_color(bool soft)
{
	if(soft)
	{
		return nvgRGB(200, 200, 230);
	}
	else
	{
		return nvgRGB(240, 240, 240);
	}
}

NVGcolor SimpleSkin::get_button_color(GUISkin::ButtonState state)
{
	switch(state)
	{
		case ButtonState::NORMAL:
			return nvgRGB(235, 235, 235);
			break;
		case ButtonState::DISABLED:
			return nvgRGB(200, 200, 200);
			break;
		case ButtonState::HOVERED:
			return nvgRGB(245, 245, 245);
			break;
		case ButtonState::CLICKED:
			return nvgRGB(255, 255, 255);
			break;
	}

	//logger->fatal("Wrong ButtonState {}", state);
	return nvgRGB(255, 0, 255);
}

void SimpleSkin::draw_link(NVGcontext *vg, glm::ivec2 link_start, glm::ivec2 win_pos, glm::ivec2 win_size,
						   glm::ivec2 mpos, bool cutable)
{
	float side_size = window_margins + window_edge_size;
	glm::vec2 title_tl = glm::vec2(win_pos.x - side_size, win_pos.y - titlebar_height - side_size) + 0.5f;
	glm::vec2 title_tr = glm::vec2(win_pos.x + win_size.x, win_pos.y - titlebar_height - side_size) + 0.5f;
	glm::vec2 title_trb = glm::vec2(win_pos.x + win_size.x + side_size, win_pos.y - titlebar_height) + 0.5f;

	nvgStrokeWidth(vg, 5.0);
	nvgStrokeColor(vg, get_foreground_color());
	nvgFillColor(vg, get_foreground_color());

	glm::ivec2 end_pos;
	bool left = false;
	if(link_start.x < title_tl.x)
	{
		end_pos = glm::ivec2(title_tl.x - 5, title_tl.y + 5);
		left = true;
	}
	else
	{
		end_pos = glm::ivec2(title_tr.x + 5, title_tr.y + 5);
	}

	if(cutable && can_cut_link(link_start, win_pos, win_size, mpos))
	{
		// Line in two halves
		float dist = glm::distance((glm::vec2)link_start, (glm::vec2)end_pos);
		float fac1 = (dist - 20.0f) / dist;
		glm::ivec2 half_way1 = (glm::vec2)link_start * (1.0f - fac1) + (glm::vec2)end_pos * fac1;
		nvgBeginPath(vg);
		nvgMoveTo(vg, link_start.x, link_start.y);
		nvgLineTo(vg, half_way1.x, half_way1.y);
		nvgStroke(vg);

		// Cut symbol
	}
	else
	{
		nvgBeginPath(vg);
		nvgMoveTo(vg, link_start.x, link_start.y);
		nvgLineTo(vg, end_pos.x, end_pos.y);
		nvgStroke(vg);
	}


	nvgBeginPath(vg);
	nvgRect(vg, link_start.x - 5.0, link_start.y - 5.0, 10.0, 10.0);
	nvgFill(vg);

	// Draw linkage point
	nvgBeginPath(vg);
	if(left)
	{
		nvgRect(vg, end_pos.x - 5.0, end_pos.y - 5.0, 10.0, 10.0);
	}
	else
	{
		nvgBeginPath(vg);
		nvgMoveTo(vg, title_tr.x, title_tr.y);
		nvgLineTo(vg, title_trb.x, title_tr.y);
		nvgLineTo(vg, title_trb.x, title_trb.y);
		nvgClosePath(vg);
	}
	nvgFill(vg);
}

bool SimpleSkin::can_cut_link(glm::ivec2 link_start, glm::ivec2 win_pos, glm::ivec2 win_size, glm::ivec2 mpos)
{
	float side_size = window_margins + window_edge_size;
	glm::vec2 title_tl = glm::vec2(win_pos.x - side_size, win_pos.y - titlebar_height - side_size) + 0.5f;
	glm::vec2 title_tr = glm::vec2(win_pos.x + win_size.x, win_pos.y - titlebar_height - side_size) + 0.5f;
	glm::vec2 title_trb = glm::vec2(win_pos.x + win_size.x + side_size, win_pos.y - titlebar_height) + 0.5f;

	glm::ivec2 end_pos;
	if(link_start.x < title_tl.x)
		end_pos = glm::ivec2(title_tl.x - 5, title_tl.y + 5);
	else
		end_pos = glm::ivec2(title_tr.x + 5, title_tr.y + 5);

	float dist = glm::distance((glm::vec2)link_start, (glm::vec2)end_pos);
	float fac1 = (dist - 20.0f) / dist;
	glm::ivec2 half_way1 = (glm::vec2)link_start * (1.0f - fac1) + (glm::vec2)end_pos * fac1;
	dist = glm::distance((glm::vec2)half_way1, (glm::vec2)mpos);

	return dist < 20.0f;
}

int SimpleSkin::get_scrollbar_width()
{
	return 2;
}

NVGcolor SimpleSkin::get_scrollbar_color()
{
	return nvgRGB(100, 100, 100);
}

NVGcolor SimpleSkin::get_scrollbar_scroller_color()
{
	return nvgRGB(180, 180, 180);
}

