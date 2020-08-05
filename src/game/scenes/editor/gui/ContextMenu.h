#pragma once
#include <glm/glm.hpp>
#include <nanovg/nanovg.h>
#include <gui/GUICanvas.h>
#include <assets/AssetManager.h>
#include <assets/BitmapFont.h>

// Draws the frame for right-click menus. GUI is separate
// Can be customized for different situations, but, overall, it
// will avoid getting off-screen at all costs
class ContextMenu
{
private:

	glm::vec2 final_pos;
	glm::vec2 final_size;

	AssetHandle<BitmapFont> title_font;

public:

	enum Mode
	{
		// Shows right next to the wanted start position, clamping
		// to screen edges if neccesary, but not showing any line
		ON_TOP,
		// Shows a little indicator line that moves from the start position
		// to an appropiate position for the context menu
		SEPARATED
	};

	Mode mode;
	// This is actually a horizontal or vertical wanted offset, in either direction
	glm::vec2 wanted_offset;
	NVGcolor outline_color = nvgRGB(218, 174, 104);


	// Returns the pos and size that was obtained at the end, for preparing a canvas
	std::pair<glm::ivec2, glm::ivec2> prepare(glm::vec2 start_pos, glm::vec2 size, glm::vec4 viewport);
	void draw(NVGcontext* vg);

	ContextMenu();
};
