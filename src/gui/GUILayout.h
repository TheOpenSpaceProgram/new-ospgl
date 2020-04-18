#pragma once 
#include <glm/glm.hpp>
#include "GUIWidget.h"

#include <glm/glm.hpp>
#include <vector>
#include <nanovg/nanovg.h>
#include "GUIInput.h"

// A layout contains widgets, which are rendered
// in appropiate positions
// This is the base class for all types
// (We manage the lifetime of all added widgets)
// Widgets are ordered in the same way the vector is 
// ordered, first added is first to appear on wathever
// layout is used
// Layouts are free to add scrollbars or wathever it's neccesary,
// that's their responsability
class GUILayout
{
protected:

	std::vector<GUIWidget*> widgets;
	
	glm::ivec2 pos, size;

	// To disable a scrollbar set max_scroll to 0 or a negative number
	// Scrollbars will otherwise always show, even if there is nowhere
	// to scroll
	// draw Disables the graphics of the scrollbar, but not function
	// enabled Disables the function and graphics
	struct Scrollbar
	{
		bool draw;
		bool enabled;

		int scroll;
		int max_scroll;
		// Positive pos changes the scrollbar from the left or top to the right or bottom
		bool positive_pos; 
		int width;
		NVGcolor color = nvgRGBA(100, 100, 100, 255);
		NVGcolor scroller_color = nvgRGBA(170, 170, 170, 255);
		NVGcolor scroller_sel_color = nvgRGBA(255, 255, 255, 255);

	};

	Scrollbar vscrollbar;

	// (left, right, top, down)
	glm::ivec4 margins = glm::ivec4(4.0, 6.0, 4.0, 4.0);

public:

	// Blocks the mouse if it's over the layout
	bool block_mouse;

	virtual void on_add_widget(GUIWidget* widget) {}

	virtual void on_remove_widget(GUIWidget* widget) {}

	virtual void on_move_widget(GUIWidget* widget, int new_index) {}

	// We will automatically delete the widget if we are 
	// destroyed, so heap-allocate it
	void add_widget(GUIWidget* widget);
	
	// We will NOT delete the widget, feel free to move it
	// to another layout
	// Will report an error if a not present widget is given
	void remove_widget(GUIWidget* widget);

	void draw_vscrollbar(NVGcontext* vg);
	void prepare_vscrollbar(GUIInput* gui_input);

	// By default, the layout will glScissor its area
	// and draw scrollbars if needed
	virtual void draw(NVGcontext* vg);
	void prepare_wrapper(glm::ivec2 pos, glm::ivec2 size, GUIInput* gui_input);
	virtual void prepare(GUIInput* gui_input) = 0;

	// These two are meant to be called from "draw" or "prepare"
	glm::ivec2 get_pos(){ return pos; }
	glm::ivec2 get_size(){ return size; }

	size_t get_widget_count();

	GUILayout();
};
