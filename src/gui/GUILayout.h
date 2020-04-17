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

public:

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

	// By default, the layout will glScissor its area
	virtual void draw(NVGcontext* vg);
	void prepare_wrapper(glm::ivec2 pos, glm::ivec2 size, GUIInput* gui_input);
	virtual void prepare(GUIInput* gui_input) = 0;

	// These two are meant to be called from "draw" or "prepare"
	glm::ivec2 get_pos(){ return pos; }
	glm::ivec2 get_size(){ return size; }

	size_t get_widget_count();
};
