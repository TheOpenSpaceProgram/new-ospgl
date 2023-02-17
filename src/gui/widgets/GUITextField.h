#pragma once
#include "../GUIWidget.h"
#include "../../universe/Events.h"

// A simple one-line only text field
// Emits the following events:
// on_intro(text: string) - called when the enter key is clicked and the text field selected, takes text written
// on_exit(text: string) - called when the escape key is clicked and the text field selected, takes text written
// on_change(text: string) - called everytime the string changes, takes text written
class GUITextField : public GUIWidget, EventEmitter
{
private:
	bool focused;
	float last_offset;
public:

	bool disabled;

	std::string string;
	std::string default_string;
	float ft_size;
	std::string ft_font;
	bool uses_bitmap;

	bool escape_clears;
	bool reselect_clears;
	void clear();

	int cursor_pos;
	std::vector<NVGglyphPosition> glyph_pos;

	void prepare(glm::ivec4 viewport, GUIScreen* screen, GUIInput* gui_input) override;
	glm::ivec2 position(glm::ivec2 wpos, glm::ivec2 wsize, GUIScreen* screen) override;
	virtual void draw(NVGcontext* ctx, GUISkin* skin) override;

	GUITextField(std::string font = "regular", float size = 16.0f);
};
