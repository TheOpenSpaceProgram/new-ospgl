#include "GUISingleLayout.h"

void GUISingleLayout::position(glm::ivec2 vpos, glm::ivec2 vsize, GUIScreen *screen)
{
	if(widgets.size() == 0)
		return;

	pos = vpos;
	size = vsize;

	vpos += glm::ivec2(margins.x, margins.z);
	vsize -= glm::ivec2(margins.y, margins.w) * 2; //< *2 because left and right are taken

	widgets[0]->is_visible = true;
	widgets[0]->position(glm::ivec2(vpos.x, vpos.y), glm::ivec2(vsize.x, vsize.y), screen);

	vscrollbar.draw = false;

}

void GUISingleLayout::prepare(GUIInput* gui_input, GUIScreen* screen)
{
	if(widgets.size() == 0)
		return;

	widgets[0]->prepare(get_aabb(), screen, gui_input);

}

void GUISingleLayout::pre_prepare(GUIScreen *screen)
{
	widgets[0]->pre_prepare(screen);
}
