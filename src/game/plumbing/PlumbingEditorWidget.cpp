#include "PlumbingEditorWidget.h"

glm::ivec2 PlumbingEditorWidget::prepare(glm::ivec2 wpos, glm::ivec2 wsize, glm::ivec4 viewport, GUIInput *gui_input)
{
	glm::vec2 size = glm::vec2(wsize.x, wsize.y);
	span = glm::ivec4(wpos.x, wpos.y, size.x, size.y);
	pb_editor.prepare(gui_input, span);
	return size;
}

void PlumbingEditorWidget::draw(NVGcontext* ctx, GUISkin* skin)
{
	pb_editor.do_editor(ctx, span, skin);
}
