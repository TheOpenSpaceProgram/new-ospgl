#include "PlumbingEditorWidget.h"

glm::ivec2 PlumbingEditorWidget::position(glm::ivec2 wpos, glm::ivec2 wsize, GUIScreen* screen)
{
	glm::vec2 size = glm::vec2(wsize.x, wsize.y);
	span = glm::ivec4(wpos.x, wpos.y, size.x, size.y);
	return size;
}

void PlumbingEditorWidget::prepare(glm::ivec4 viewport, GUIScreen *screen, GUIInput *gui_input)
{
	pb_editor.prepare(gui_input, span);
}

void PlumbingEditorWidget::draw(NVGcontext* ctx, GUISkin* skin)
{
	pb_editor.do_editor(ctx, span, skin);
}

PlumbingEditorWidget::PlumbingEditorWidget()
{
}
