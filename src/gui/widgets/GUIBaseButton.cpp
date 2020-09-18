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

void GUIBaseButton::set_down(int btn)	
{
	if(!click[btn])
	{
		on_clicked(std::move(btn));
	}
	click[btn] = true;
}

void GUIBaseButton::set_up(int btn) 
{
	if(click[btn])
	{
		on_released(std::move(btn));
	}
	click[btn] = false;
}

void GUIBaseButton::do_button(glm::ivec2 pos, glm::ivec2 size, glm::ivec4 viewport, GUIInput* ipt)
{
	bool unset = false;
	if(!ipt->ext_mouse_blocked)
	{
		if(ipt->mouse_inside(pos, size) && ipt->mouse_inside(viewport))
		{
			if(blocks_mouse)
			{
				ipt->mouse_blocked = true;
			}

			if(ipt->mouse_down(0)){ set_down(0); }
			if(ipt->mouse_down(1)){ set_down(1); }
			if(ipt->mouse_up(0)){ set_up(0); }
			if(ipt->mouse_up(1)){ set_up(1); }

			set_hover(true);
			during_hover();
		}
		else
		{
			unset = true;
		}
	}
	else
	{
		unset = true;
	}

	if(unset)
	{
		set_up(0);
		set_up(1);
		set_hover(false);
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
