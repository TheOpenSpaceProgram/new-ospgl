#include "LuaGUI.h"
#include <gui/GUISkin.h>
#include <gui/skins/SimpleSkin.h>
#include <gui/GUIScreen.h>

void LuaGUI::load_to(sol::table &table)
{
	table.new_usertype<GUISkin>("skin",
		"get_foreground_color", &GUISkin::get_foreground_color,
		"get_background_color", &GUISkin::get_background_color,
		"draw_link", [](GUISkin* self, void* vg, glm::dvec2 start_pos, glm::dvec2 win_pos){
			self->draw_link((NVGcontext*)vg, start_pos, win_pos);},
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
				  [](GUIScreen* screen, glm::dvec4 screens)
				  {
					  screen->new_frame(screens);
				  },
				  [](GUIScreen* screen, int sx, int sy, int sw, int sh)
				  {
					  screen->new_frame(glm::ivec4(sx, sy, sw, sh));
				  },
				  sol::resolve<void()>(&GUIScreen::new_frame)),
		  "add_canvas", sol::overload(
				  [](GUIScreen* screen, std::shared_ptr<GUICanvas> canvas, glm::dvec2 pos, glm::dvec2 size)
				  {
					  screen->add_canvas(canvas, pos, size);
				  },
				  [](GUIScreen* screen, std::shared_ptr<GUICanvas> canvas, int x, int y, int w, int h)
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
		  "draw", &GUIScreen::draw,
		  "input", &GUIScreen::gui_input,
		  "skin", &GUIScreen::skin,
		  "viewport", sol::property([](GUIScreen* screen){return (glm::dvec4)screen->viewport;},
									[](GUIScreen* screen, glm::dvec4 v){ screen->viewport = v;}),
		  "win_manager", &GUIScreen::win_manager

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

	table.new_usertype<GUICanvas>("canvas",
		  "new", [](){
			return std::make_shared<GUICanvas>();
		},
		"divide_h", [](GUICanvas* self, float fac)
		{
		return self->divide_h(fac);
		},
		"divide_v", [](GUICanvas* self, float fac)
		{
			return self->divide_v(fac);
	  	},
	    // Utility functions
	    "divide_v_pixels", [](GUICanvas* self, int pixels, sol::optional<bool> for_bottom)
	    {
			auto ret = self->divide_v(0.5);
			self->child_pixels = pixels;
			self->pixels_for_child_1 = for_bottom.value_or(false);
			return ret;
		},
		"divide_h_pixels", [](GUICanvas* self, int pixels, sol::optional<bool> for_right)
		{
			auto ret = self->divide_h(0.5);
			self->child_pixels = pixels;
			self->pixels_for_child_1 = for_right.value_or(false);
			return ret;
		},
		"child_pixels", &GUICanvas::child_pixels,
		"pixels_for_child_1", &GUICanvas::pixels_for_child_1
		);

	table.new_usertype<GUIWindowManager>("win_manager", sol::no_constructor,
		 "create_window", [](GUIWindowManager* self, sol::optional<glm::dvec2> pos, sol::optional<glm::dvec2> size)
			 {
				glm::ivec2 posi = pos.value_or(glm::dvec2(-1.0, -1.0));
				glm::ivec2 sizei = size.value_or(glm::dvec2(-1.0, -1.0));
				auto win = self->create_window(posi, sizei);
				// We always use shared_ptr in lua!
				return win.lock();
			 });

	table.new_usertype<GUIWindow>("window", sol::no_constructor,
			  "canvas", &GUIWindow::canvas,
			  "is_open", &GUIWindow::is_open,
			  "closeable", &GUIWindow::closeable,
			  "pinable", &GUIWindow::pinable,
			  "pos", sol::property([](GUIWindow* self){return (glm::dvec2)self->pos;},
								   [](GUIWindow* self, glm::dvec2 val){ self->pos = val; }),
		      "size", sol::property([](GUIWindow* self){return (glm::dvec2)self->size;},
					   			  [](GUIWindow* self, glm::dvec2 val){ self->size = val; }),
			  "minimizable", &GUIWindow::minimizable,
			  "resizeable", &GUIWindow::resizeable,
			  "moveable", &GUIWindow::moveable,
			  "pin_passthrough", &GUIWindow::pin_passthrough,
			  "title", &GUIWindow::title,
			  "style", &GUIWindow::style,
			  "alpha", &GUIWindow::alpha,
			  "close", &GUIWindow::close);

	table.new_enum("window_style",
				   "normal", GUISkin::WindowStyle::NORMAL,
				   "linked", GUISkin::WindowStyle::LINKED);

}

LuaGUI::LuaGUI()
{

}

LuaGUI::~LuaGUI()
{

}
