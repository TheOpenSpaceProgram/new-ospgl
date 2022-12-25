#pragma once
#include "../widgets/GUIBaseButton.h"

class GUICanvas;
class GUIVerticalLayout;

// This widget is slightly tricky as it needs to draw outside its canvas if choosing
class GUIDropDown : public GUIBaseButton
{

public:
	// Is the user choosing?
	bool open;

	// Vertical size of each item (horizontal matches dropdown)
	int item_size = 20;
	// How big may the dropdown be? negative for no limit (within screen)
	int max_dropdown_pixels;
	// Can the dropdown go up?
	bool may_drop_upwards;

	GUICanvas* chooser_canvas;
	GUIVerticalLayout* chooser_layout;

	std::string not_chosen_string;
	bool allow_no_selection;
	// index in the array, negative means none selected (if possible, otherwise it will be set to the first element)
	int item;
	// be careful when modifying manually as item may be invalidated!
	std::vector<std::string> options;

	// Called when an item is chosen with the item
	Signal<void(std::string)> on_item_chosen;

	// Called when an item is chosen, which was not previously, with the item
	Signal<void(std::string)> on_item_change;

	// Call after modifying options so that the layout contains the new items
	void update_options();

	void prepare(glm::ivec4 viewport, GUIScreen* screen, GUIInput* gui_input) override;
	glm::ivec2 position(glm::ivec2 wpos, glm::ivec2 wsize, GUIScreen* screen) override;
	virtual void pre_prepare(GUIScreen* screen) override;
	virtual void draw(NVGcontext* ctx, GUISkin* skin) override;

	GUIDropDown();
	~GUIDropDown();
};
