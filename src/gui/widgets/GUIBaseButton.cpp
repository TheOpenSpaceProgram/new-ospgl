#include "GUIBaseButton.h"


void GUIBaseButton::set_hover(bool nhover)
{
	if(nhover)
	{
		if(!hover)
		{
			on_enter_hover();
		}	
	}
	else
	{
		if(hover)
		{
			on_leave_hover();
		}
	}

	hover = nhover;
}

void GUIBaseButton::set_click(int btn, bool value)	
{
	if(value)
	{
		if(!click[btn])
		{
			on_clicked((int)btn);
		}
	}
	else
	{
		if(click[btn])
		{
			on_released((int)btn);
		}
	}

	click[btn] = value;
}

void GUIBaseButton::do_button(glm::ivec2 pos, glm::ivec2 size, GUIInput* ipt)
{
	if(!ipt->ext_mouse_blocked)
	{
		if(ipt->mouse_inside(pos, size))
		{
			if(blocks_mouse)
			{
				ipt->mouse_blocked = true;
			}

			if(disabled)
			{
				set_click(0, false);
				set_click(1, false);
			}
			else
			{
				set_click(0, ipt->mouse_pressed(0));
				set_click(1, ipt->mouse_pressed(1));
			}

			set_hover(true);
			during_hover();
		}
		else
		{
			set_click(0, false);
			set_click(1, false);

			set_hover(false);
		}
	}
}


GUISkin::ButtonState GUIBaseButton::get_button_state()
{
	if(toggled)
	{
		return GUISkin::ButtonState::CLICKED;
	}

	if(disabled)
	{
		return GUISkin::ButtonState::DISABLED;
	}

	if(click[0] || click[1])
	{
		return GUISkin::ButtonState::CLICKED;
	}

	if(hover)
	{
		return GUISkin::ButtonState::HOVERED;
	}

	return GUISkin::ButtonState::NORMAL;	
}
