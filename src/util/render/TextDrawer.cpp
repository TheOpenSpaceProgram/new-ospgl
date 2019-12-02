#include "TextDrawer.h"

TextDrawer* text_drawer;

TextDrawer::TextDrawer()
{

}


TextDrawer::~TextDrawer()
{
}

void create_global_text_drawer()
{
	text_drawer = new TextDrawer();
}

void destroy_global_text_drawer()
{
	delete text_drawer;
}
