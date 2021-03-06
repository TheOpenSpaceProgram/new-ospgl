#pragma once
#include "../GUIWidget.h"

// Base class for all buttons
class GUIBaseButton : public GUIWidget
{
protected:

	void set_hover(bool nhover);
	void set_down(int btn);
	void set_up(int btn);

public:

	bool hover = false;
	bool disabled = false;
	bool toggled = false;	//< If true button will always show the clicked variant

	// We only support left and right buttons for compatibility with laptops
	bool click[2] = {false, false};

	Signal<void()> on_enter_hover; 		//< Called the frame the mouse enters the button
	Signal<void()> during_hover; 			//< Called every frame the mouse is hovering
	Signal<void()> on_leave_hover;		//< Called the frame the mouse leaves the button
	
	Signal<void(int)> on_clicked;		//< Called the frame the button is clicked
	Signal<void(int)> during_click;		//< Called every frame the button is held
	Signal<void(int)> on_released;		//< Called the frame the button is released (or leaves)

	void do_button(glm::ivec2 pos, glm::ivec2 size, glm::ivec4 viewport, GUIInput* ipt);
	GUISkin::ButtonState get_button_state();

};
