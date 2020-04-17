#include "GUILayout.h"
#include <util/Logger.h>

void GUILayout::add_widget(GUIWidget* widget)
{
	widgets.push_back(widget);

	on_add_widget(widget);
}

void GUILayout::remove_widget(GUIWidget* widget)
{
	for(auto it = widgets.begin(); it != widgets.end(); it++)
	{
		if(*it == widget)
		{
			widgets.erase(it);
			on_remove_widget(widget);
			return;
		}
	}

	logger->warn("Tried to remove a widget which was not present");
}

size_t GUILayout::get_widget_count()
{
	return widgets.size();
}

void GUILayout::draw(NVGcontext* ctx)
{
	glm::ivec2 pos = get_pos();
	glm::ivec2 size = get_size();

	nvgScissor(ctx, pos.x, pos.y, size.x, size.y);	

	for(auto widget : widgets)
	{
		widget->draw(ctx);
	}
}

void GUILayout::prepare_wrapper(glm::ivec2 pos, glm::ivec2 size, GUIInput* gui_input)
{
	this->pos = pos;
	this->size = size;
	prepare(gui_input);
}
