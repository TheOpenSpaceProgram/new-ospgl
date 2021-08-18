#pragma once
#include "PlumbingEditor.h"
#include <gui/GUIWidget.h>

class PlumbingEditorWidget : public GUIWidget
{
private:

	glm::vec4 span;

public:

	PlumbingEditor pb_editor;
	glm::ivec2 prepare(glm::ivec2 wpos, glm::ivec2 wsize, glm::ivec4 viewport, GUIInput* gui_input) override;
	void draw(NVGcontext* ctx, GUISkin* skin) override;

};
