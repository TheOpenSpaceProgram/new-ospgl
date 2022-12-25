#pragma once
#include "../GUIWidget.h"

// A simple one-line only text field
class GUITextField : public GUIWidget
{
private:
	bool focused;
	float last_offset;
public:

	// called when the enter key is clicked and the text field selected
	Signal<void(std::string)> on_intro;
	// called when the escape key is clicked and the text field selected
	Signal<void(std::string)> on_exit;
	// called when the string changes
	Signal<void(std::string)> on_change;

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
