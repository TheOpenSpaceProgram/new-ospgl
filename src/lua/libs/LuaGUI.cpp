#include "LuaGUI.h"
#include <gui/GUISkin.h>

void LuaGUI::load_to(sol::table &table)
{
	table.new_usertype<GUISkin>("gui_skin",
		"get_foreground_color", &GUISkin::get_foreground_color,
		"get_background_color", &GUISkin::get_background_color
	);
}

LuaGUI::LuaGUI()
{

}

LuaGUI::~LuaGUI()
{

}
