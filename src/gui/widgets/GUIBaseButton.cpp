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
		if(ipt->is_mouse_inside(pos, size))
		{
			ipt->mouse_blocked = true;

			set_click(0, ipt->is_mouse_clicked(0));
			set_click(1, ipt->is_mouse_clicked(1));

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
