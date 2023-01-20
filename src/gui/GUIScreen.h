#pragma once
#include <vector>
#include "GUICanvas.h"
#include "GUIWindowManager.h"

// A GUIScreen is needed to draw canvases, and handles off-canvas elements
// such as dropdowns
// It also includes a GUIWindowManager
class GUIScreen
{
private:

	using CanvasPosSize = std::pair<GUICanvas*, std::pair<glm::ivec2, glm::ivec2>>;

	// Canvas to prepare/draw after everything else is finished (example: dropdowns)
	std::vector<CanvasPosSize> post_canvas;
	std::vector<CanvasPosSize> canvas;

public:
	// Space in the actual game screen to use for this GUIScreen
	// (x, y, width, height)
	glm::ivec4 screen;
	// We hold a shared_ptr for lua automated gargabe collection
	std::shared_ptr<GUISkin> skin;
	// We hold a weak reference to gui_input which is managed externally
	// (No need to use a weak_ptr, gui_input will always outlive screens!)
	GUIInput* gui_input;
	GUIWindowManager win_manager;


	void new_frame(glm::ivec4 screen);
	void new_frame();

	// Called by widgets (or you if you need) to add overlay canvas
	void add_post_canvas(GUICanvas* canvas, glm::ivec2 pos, glm::ivec2 size);
	// Call on each canvas to do them this frame
	void add_canvas(GUICanvas* canvas, glm::ivec2 pos, glm::ivec2 size);

	// Call before any input handling by the 3D game, so the GUI can block the game
	void prepare_pass();
	// Call after input handling by the 3D game, so the 3D game can block the GUI
	void input_pass();
	// Uses NVG to draw everything
	void draw();

	void init(std::shared_ptr<GUISkin> skin, GUIInput* gui_input);

};
