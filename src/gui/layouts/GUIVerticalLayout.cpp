#include "GUIVerticalLayout.h"

#include <util/Logger.h>

void GUIVerticalLayout::position(glm::ivec2 vpos, glm::ivec2 vsize, GUIScreen *screen)
{
	linear_helper(vpos, vsize, screen, true);
}

void GUIVerticalLayout::prepare(GUIInput* gui_input, GUIScreen* screen)
{
	for(auto widget : widgets)
	{
		widget->prepare(get_aabb(), screen, gui_input);
	}

	if(gui_input->mouse_inside(get_aabb()))
	{
		gui_input->scroll_blocked = true;
	}

}

void GUIVerticalLayout::pre_prepare(GUIScreen *screen)
{
	for(auto widget : widgets)
	{
		widget->pre_prepare(screen);
	}
}
