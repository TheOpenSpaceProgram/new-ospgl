#pragma once
#include "PlumbingEditor.h"
#include <gui/GUIWidget.h>

class PlumbingEditorWidget : public GUIWidget
{
private:

	glm::vec4 span;

public:

	PlumbingEditor pb_editor;
	void prepare(glm::ivec4 viewport, GUIScreen* screen, GUIInput* gui_input) override;
	glm::ivec2 position(glm::ivec2 wpos, glm::ivec2 wsize, GUIScreen* screen) override;
	void draw(NVGcontext* ctx, GUISkin* skin) override;

	PlumbingEditorWidget();

};
