#include "LuaGUI.h"
#include <gui/GUISkin.h>
#include <gui/skins/SimpleSkin.h>
#include <gui/GUIScreen.h>

void LuaGUI::load_to(sol::table &table)
{
	table.new_usertype<GUISkin>("skin",
		"get_foreground_color", &GUISkin::get_foreground_color,
		"get_background_color", &GUISkin::get_background_color,
		"get_default_skin", []()
		{
			return (std::shared_ptr<GUISkin>)std::make_shared<SimpleSkin>();
		}
	);

	table.new_usertype<GUIScreen>("screen",
		  // Returns shared pointer for garbage collection
		  "new", [](std::shared_ptr<GUISkin> skin, GUIInput* gui_input)
		  {
			auto ptr = std::make_shared<GUIScreen>();
			ptr->init(skin, gui_input);
			return ptr;
		  },
		  "new_frame", sol::overload(
				  [](GUIScreen* screen, glm::vec4 screens)
				  {
					  screen->new_frame(screens);
				  },
				  [](GUIScreen* screen, int sx, int sy, int sw, int sh)
				  {
					  screen->new_frame(glm::ivec4(sx, sy, sw, sh));
				  },
				  sol::resolve<void()>(&GUIScreen::new_frame)),
		  "add_canvas", sol::overload(
				  [](GUIScreen* screen, GUICanvas* canvas, glm::vec2 pos, glm::vec2 size)
				  {
					  screen->add_canvas(canvas, pos, size);
				  },
				  [](GUIScreen* screen, GUICanvas* canvas, int x, int y, int w, int h)
				  {
					  screen->add_canvas(canvas, glm::ivec2(x, y), glm::ivec2(w, h));
				  }
				  ),
		  "add_post_canvas", sol::overload(
				[](GUIScreen* screen, GUICanvas* canvas, glm::vec2 pos, glm::vec2 size)
				{
					screen->add_post_canvas(canvas, pos, size);
				},
				[](GUIScreen* screen, GUICanvas* canvas, int x, int y, int w, int h)
				{
					screen->add_post_canvas(canvas, glm::ivec2(x, y), glm::ivec2(w, h));
				}
			),
		  "prepare_pass", &GUIScreen::prepare_pass,
		  "input_pass", &GUIScreen::input_pass,
		  "draw", &GUIScreen::draw
	);

	// We only expose what's needed! For 3D input use the normal input system, not GUIInput
	table.new_usertype<GUIInput>("input",
		 "ext_mouse_blocked", &GUIInput::ext_mouse_blocked,
		 "ext_scroll_blocked", &GUIInput::ext_scroll_blocked,
		 "ext_keyboard_blocked", &GUIInput::ext_keyboard_blocked,
		 "mouse_blocked", &GUIInput::mouse_blocked,
		 "scroll_blocked", &GUIInput::scroll_blocked,
		 "keyboard_blocked", &GUIInput::keyboard_blocked,
		 "debug", &GUIInput::debug
		 );

}

LuaGUI::LuaGUI()
{

}

LuaGUI::~LuaGUI()
{

}
