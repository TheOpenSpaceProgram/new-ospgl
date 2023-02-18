#pragma once 
#include <glm/glm.hpp>
#include "GUIWidget.h"

#include <glm/glm.hpp>
#include <vector>
#include <nanovg/nanovg.h>
#include "GUIInput.h"
#include "GUIScrollbar.h"

class GUIScreen;

// A layout contains widgets, which are rendered
// in appropiate positions
// This is the base class for all types
// Widgets are ordered in the same way the vector is
// ordered, first added is first to appear on wathever
// layout is used
// Layouts are free to add scrollbars or wathever it's neccesary,
// that's their responsability
class GUILayout
{
protected:

	std::vector<std::shared_ptr<GUIWidget>> widgets;
	
	glm::ivec2 pos, size;



public:

	GUIScrollbar vscrollbar;
	GUIScrollbar hscrollbar;

	// (left, right, top, down)
	glm::ivec4 margins = glm::ivec4(4.0, 4.0, 4.0, 4.0);

	// Blocks the mouse if it's over the layout
	bool block_mouse;

	// Called with the widget already added
	virtual void on_add_widget(GUIWidget* widget) {}

	// Called before the widget is removed
	virtual void on_remove_widget(GUIWidget* widget) {}

	virtual void on_move_widget(GUIWidget* widget, int new_index) {}

	void add_widget(std::shared_ptr<GUIWidget> widget);
	// Note: This will find a matching widget and remove it from the layout
	void remove_widget(GUIWidget* widget);
	template<typename T>
	void remove_widget_lua(std::shared_ptr<T> widget)
	{
		remove_widget(widget.get());
	}
	template<typename T>
	void add_widget_lua(std::shared_ptr<T> d){ add_widget(d); }

	void remove_all_widgets();

	void prepare_scrollbar(GUIInput* gui_input, GUIScrollbar* scroll, bool v);
	void draw_vscrollbar(NVGcontext* vg, GUISkin* skin);
	void draw_hscrollbar(NVGcontext* vg, GUISkin* skin);

	// By default, the layout will glScissor its area and draw scrollbars if needed
	void draw(NVGcontext* vg, GUISkin* skin);
	void position_wrapper(glm::ivec2 pos, glm::ivec2 size, GUIScreen* screen);
	virtual void position(glm::ivec2 pos, glm::ivec2 size, GUIScreen* screen) = 0;
	virtual void prepare(GUIInput* gui_input, GUIScreen* screen) = 0;
	void prepare_wrapper(GUIScreen* screen, GUIInput* gui_input);
	virtual void pre_prepare(GUIScreen* screen) = 0;

	// These two are meant to be called from "draw" or "prepare"
	glm::ivec2 get_pos(){ return pos; }
	glm::ivec2 get_size(){ return size; }
	glm::ivec4 get_aabb() { return {pos.x, pos.y, size.x, size.y}; }

	size_t get_widget_count();

	GUILayout();
	virtual ~GUILayout();
};
