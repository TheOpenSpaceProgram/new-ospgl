#pragma once
#include "../GUIWidget.h"

// Base class for all buttons
class GUIBaseButton : public GUIWidget
{
protected:

	void set_hover(bool nhover)
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

public:

	bool hover = false;
	bool lclick = false;
	bool rclick = false;

	Signal<void()> on_enter_hover; 		//< Called the frame the mouse enters the button
	Signal<void()> during_hover; 			//< Called every frame the mouse is hovering
	Signal<void()> on_leave_hover;		//< Called the frame the mouse leaves the button
	
	Signal<void(int)> on_cliked;		//< Called the frame the button is clicked with a mousebutton
	Signal<void(int)> during_click;		//< Called every frame the button is held with a mousebutton
	Signal<void(int)> on_released;		//< Called the frame the button is released (with a mousebutton)


};
