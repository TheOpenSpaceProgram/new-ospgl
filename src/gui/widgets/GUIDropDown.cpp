#include "GUIDropDown.h"
#include <gui/GUICanvas.h>
#include <gui/layouts/GUIVerticalLayout.h>
#include <gui/widgets/GUITextButton.h>
#include <gui/GUIScreen.h>

void GUIDropDown::pre_prepare(GUIScreen *screen)
{
	if(open)
	{
		glm::ivec2 cpos;
		glm::ivec2 csize;
		glm::ivec4 sbounds = screen->screen;
		bool drop_up = may_drop_upwards && pos.y > (sbounds.y + sbounds.w) / 2;
		if(drop_up && false)
		{
			// TODO!
		}
		else
		{
			cpos = pos + glm::ivec2(0, size.y);
			int dsize = max_dropdown_pixels;
			if(max_dropdown_pixels < 0)
			{
				dsize = (sbounds.y + sbounds.w) - pos.y + size.y - 5;
			}
			csize = glm::ivec2(size.x, dsize);
		}
		screen->add_post_canvas(chooser_canvas, cpos, csize);
	}
}

void GUIDropDown::prepare(glm::ivec4 viewport, GUIScreen *screen, GUIInput *gui_input)
{
	do_button(pos, size, viewport, gui_input);
}

glm::ivec2 GUIDropDown::position(glm::ivec2 wpos, glm::ivec2 wsize, GUIScreen* screen)
{
	return default_position(wpos, wsize);
}

void GUIDropDown::draw(NVGcontext *ctx, GUISkin *skin)
{
	auto st = get_button_state();
	if(open)
	{
		st = GUISkin::ButtonState::CLICKED;
	}

	if(item >= 0)
	{
		skin->draw_dropdown_header(ctx, pos, size, options[item].second, open, false, st);
	}
	else
	{
		skin->draw_dropdown_header(ctx, pos, size, not_chosen_string, open, true, st);
	}

}

GUIDropDown::GUIDropDown()
{
	default_size = glm::ivec2(400, 24);

	item = -1;
	open = false;
	not_chosen_string = "";
	chooser_canvas = new GUICanvas();
	chooser_layout = new GUIVerticalLayout(0);
	chooser_canvas->layout = chooser_layout;

	may_drop_upwards = true;
	max_dropdown_pixels = -1;

	on_clicked.add_handler([this](int _)
						   {
								this->open = !this->open;
						   });
}

GUIDropDown::~GUIDropDown()
{
	delete chooser_canvas;
	delete chooser_layout;
}

void GUIDropDown::update_options()
{
	chooser_layout->remove_all_widgets();
	for(size_t i = 0; i < options.size(); i++)
	{
		GUITextButton* btn = new GUITextButton(options[i].second, "medium");
		btn->default_size.y = item_size;
		btn->on_clicked.add_handler([this, i](int _)
		{
			this->choose(i);
		});
		btn->on_enter_hover.add_handler([this, i]()
		{
			this->on_item_enter_hover.call(this->options[i]);
		});
		btn->on_leave_hover.add_handler([this, i]()
		{
			this->on_item_leave_hover.call(this->options[i]);
		});
		chooser_layout->add_widget(btn);
	}

}

void GUIDropDown::choose(size_t i)
{
	open = false;
	int old_item = item;
	item = (int)i;
	auto opt = options[i];
	on_item_chosen.call(opt);
	if(old_item != item)
	{
		if(old_item >= 0)
		{
			on_item_change.call(opt, options[old_item]);
		}
		else
		{
			on_item_change.call(opt, {"", ""});
		}
	}
}

void GUIDropDown::select(std::string id)
{
	for(int i = 0; i < options.size(); i++)
	{
		if(options[i].first == id)
		{
			item = i;
			break;
		}
	}

}

