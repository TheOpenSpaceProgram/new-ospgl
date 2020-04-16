#include "GUIVerticalLayout.h"

#include <util/Logger.h>

void GUIVerticalLayout::draw(glm::ivec2 pos, glm::ivec2 size, NVGcontext* vg)
{
	logger->info("Pos: {} {} Size: {} {}", pos.x, pos.y, size.x, size.y);

	nvgBeginPath(vg);
	nvgRect(vg, pos.x, pos.y, size.x, size.y);
	nvgFillColor(vg, nvgRGB(r, g, b));
	nvgFill(vg);	

}
