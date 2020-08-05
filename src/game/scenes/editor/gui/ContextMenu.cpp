#include "ContextMenu.h"


std::pair<glm::ivec2, glm::ivec2> ContextMenu::prepare(glm::vec2 start_pos, 
		glm::vec2 size, glm::vec4 viewport)
{
	final_pos = glm::round(start_pos);
	final_size = glm::round(size);

	return std::make_pair(start_pos, size);
}

void ContextMenu::draw(NVGcontext* vg)
{
	nvgStrokeColor(vg, outline_color);
	// Draw title bar
	nvgBeginPath(vg);
	nvgRect(vg, final_pos.x - 3.5f, final_pos.y - 15.5f, final_size.x + 7.0f, 16.0f);
	nvgFillColor(vg, nvgRGB(85, 92, 100));
	nvgFill(vg);
	nvgStroke(vg);
	// Draw main body
	nvgBeginPath(vg);
	nvgRect(vg, final_pos.x + 0.5f, final_pos.y + 0.5f, final_size.x, final_size.y);
	nvgFillColor(vg, nvgRGB(40, 45, 50));
	nvgFill(vg);
	nvgStroke(vg);

	nvgFillColor(vg, nvgRGB(255, 255, 255));
	nvgBitmapText(vg, title_font.duplicate(), TextDrawer::LEFT, final_pos.x - 2.0f, final_pos.y - 12.0f, "Hello, world!");
}

ContextMenu::ContextMenu()
{
	title_font = AssetHandle<BitmapFont>("core:fonts/ProggySquare.fnt");
}
