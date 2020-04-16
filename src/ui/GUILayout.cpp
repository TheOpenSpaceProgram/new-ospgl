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
