#pragma once
#include "../GUIWidget.h"
#include "../../universe/Events.h"

// Base class for all buttons
// Emits the following events:
// on_enter_hover
// during_hover
// on_leave_hover
// on_clicked
// during_click
// on_released
class GUIBaseButton : public GUIWidget, public EventEmitter
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

	void do_button(glm::ivec2 pos, glm::ivec2 size, glm::ivec4 viewport, GUIInput* ipt);
	GUISkin::ButtonState get_button_state();

};
