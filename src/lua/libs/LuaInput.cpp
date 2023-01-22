#include "LuaInput.h"
#include <util/InputUtil.h>

void LuaInput::load_to(sol::table &table)
{
	table.new_usertype<InputUtil>("input_util_ut",
		  "prev_mouse_pos", &InputUtil::prev_mouse_pos,
		  "mouse_pos", &InputUtil::mouse_pos,
		  "mouse_delta", &InputUtil::mouse_delta,
		  "mouse_scroll_delta", &InputUtil::mouse_scroll_delta,
		  "prev_mouse_scroll", &InputUtil::prev_mouse_scroll,
		  "mouse_scroll", &InputUtil::mouse_scroll,
		  "key_pressed", &InputUtil::key_pressed,
		  "key_down", &InputUtil::key_down,
		  "key_down_or_repeating", &InputUtil::key_down_or_repeating,
		  "key_up", &InputUtil::key_up,
		  "mouse_pressed", &InputUtil::mouse_pressed,
		  "mouse_down", &InputUtil::mouse_down,
		  "mouse_up", &InputUtil::mouse_up,
		  "set_cursor", &InputUtil::set_cursor,
		  "get_input_text", &InputUtil::get_input_text);

	table["cursor"]["normal"] = InputUtil::Cursor::NORMAL;
	table["cursor"]["ibeam"] = InputUtil::Cursor::IBEAM;
	table["cursor"]["crosshair"] = InputUtil::Cursor::CROSSHAIR;
	table["cursor"]["hand"] = InputUtil::Cursor::HAND;
	table["cursor"]["resize_ew"] = InputUtil::Cursor::RESIZE_EW;
	table["cursor"]["resize_ns"] = InputUtil::Cursor::RESIZE_NS;
	table["cursor"]["resize_nwse"] = InputUtil::Cursor::RESIZE_NWSE;
	table["cursor"]["resize_nesw"] = InputUtil::Cursor::RESIZE_NESW;
	table["cursor"]["resize_all"] = InputUtil::Cursor::RESIZE_ALL;
	table["cursor"]["not_allowed"] = InputUtil::Cursor::NOT_ALLOWED;

	table["btn"]["left"] = GLFW_MOUSE_BUTTON_LEFT;
	table["btn"]["right"] = GLFW_MOUSE_BUTTON_RIGHT;
	table["btn"]["middle"] = GLFW_MOUSE_BUTTON_MIDDLE;

	// Assign the global input_util
	table["input_util"] = input;
	// Assign __index to input_util so you can directly call the functions
	table[sol::metatable_key]["__index"] = table["input_util"];

	// Globals like key names, etc...
#define SET_KEY(id, glfw_id) table["key_" id] = glfw_id
	SET_KEY("unknown", GLFW_KEY_UNKNOWN);
	SET_KEY("space", GLFW_KEY_SPACE);
	SET_KEY("apostrophe", GLFW_KEY_APOSTROPHE);
	SET_KEY("comma", GLFW_KEY_COMMA);
	SET_KEY("minus", GLFW_KEY_MINUS);
	SET_KEY("period", GLFW_KEY_PERIOD);
	SET_KEY("slash", GLFW_KEY_SLASH);
	SET_KEY("0", GLFW_KEY_0);
	SET_KEY("1", GLFW_KEY_1);
	SET_KEY("2", GLFW_KEY_2);
	SET_KEY("3", GLFW_KEY_3);
	SET_KEY("4", GLFW_KEY_4);
	SET_KEY("5", GLFW_KEY_5);
	SET_KEY("6", GLFW_KEY_6);
	SET_KEY("7", GLFW_KEY_7);
	SET_KEY("8", GLFW_KEY_8);
	SET_KEY("9", GLFW_KEY_9);
	SET_KEY("semicolon", GLFW_KEY_SEMICOLON);
	SET_KEY("equal", GLFW_KEY_EQUAL);
	SET_KEY("a", GLFW_KEY_A);
	SET_KEY("b", GLFW_KEY_B);
	SET_KEY("c", GLFW_KEY_C);
	SET_KEY("d", GLFW_KEY_D);
	SET_KEY("e", GLFW_KEY_E);
	SET_KEY("f", GLFW_KEY_F);
	SET_KEY("g", GLFW_KEY_G);
	SET_KEY("h", GLFW_KEY_H);
	SET_KEY("i", GLFW_KEY_I);
	SET_KEY("j", GLFW_KEY_J);
	SET_KEY("k", GLFW_KEY_K);
	SET_KEY("l", GLFW_KEY_L);
	SET_KEY("m", GLFW_KEY_M);
	SET_KEY("n", GLFW_KEY_N);
	SET_KEY("o", GLFW_KEY_O);
	SET_KEY("p", GLFW_KEY_P);
	SET_KEY("q", GLFW_KEY_Q);
	SET_KEY("r", GLFW_KEY_R);
	SET_KEY("s", GLFW_KEY_S);
	SET_KEY("t", GLFW_KEY_T);
	SET_KEY("u", GLFW_KEY_U);
	SET_KEY("v", GLFW_KEY_V);
	SET_KEY("w", GLFW_KEY_W);
	SET_KEY("x", GLFW_KEY_X);
	SET_KEY("y", GLFW_KEY_Y);
	SET_KEY("z", GLFW_KEY_Z);
	SET_KEY("left_bracket", GLFW_KEY_LEFT_BRACKET);
	SET_KEY("backslash", GLFW_KEY_BACKSLASH);
	SET_KEY("right_bracket", GLFW_KEY_RIGHT_BRACKET);
	SET_KEY("grave_accent", GLFW_KEY_GRAVE_ACCENT);
	SET_KEY("world_1", GLFW_KEY_WORLD_1);
	SET_KEY("world_2", GLFW_KEY_WORLD_2);
	SET_KEY("escape", GLFW_KEY_ESCAPE);
	SET_KEY("enter", GLFW_KEY_ENTER);
	SET_KEY("tab", GLFW_KEY_TAB);
	SET_KEY("backspace", GLFW_KEY_BACKSPACE);
	SET_KEY("insert", GLFW_KEY_INSERT);
	SET_KEY("delete", GLFW_KEY_DELETE);
	SET_KEY("right", GLFW_KEY_RIGHT);
	SET_KEY("left", GLFW_KEY_LEFT);
	SET_KEY("down", GLFW_KEY_DOWN);
	SET_KEY("up", GLFW_KEY_UP);
	SET_KEY("page_up", GLFW_KEY_PAGE_UP);
	SET_KEY("page_down", GLFW_KEY_PAGE_DOWN);
	SET_KEY("home", GLFW_KEY_HOME);
	SET_KEY("end", GLFW_KEY_END);
	SET_KEY("caps_lock", GLFW_KEY_CAPS_LOCK);
	SET_KEY("scroll_lock", GLFW_KEY_SCROLL_LOCK);
	SET_KEY("num_lock", GLFW_KEY_NUM_LOCK);
	SET_KEY("print_screen", GLFW_KEY_PRINT_SCREEN);
	SET_KEY("pause", GLFW_KEY_PAUSE);
	SET_KEY("f1", GLFW_KEY_F1);
	SET_KEY("f2", GLFW_KEY_F2);
	SET_KEY("f3", GLFW_KEY_F3);
	SET_KEY("f4", GLFW_KEY_F4);
	SET_KEY("f5", GLFW_KEY_F5);
	SET_KEY("f6", GLFW_KEY_F6);
	SET_KEY("f7", GLFW_KEY_F7);
	SET_KEY("f8", GLFW_KEY_F8);
	SET_KEY("f9", GLFW_KEY_F9);
	SET_KEY("f10", GLFW_KEY_F10);
	SET_KEY("f11", GLFW_KEY_F11);
	SET_KEY("f12", GLFW_KEY_F12);
	SET_KEY("f13", GLFW_KEY_F13);
	SET_KEY("f14", GLFW_KEY_F14);
	SET_KEY("f15", GLFW_KEY_F15);
	SET_KEY("f16", GLFW_KEY_F16);
	SET_KEY("f17", GLFW_KEY_F17);
	SET_KEY("f18", GLFW_KEY_F18);
	SET_KEY("f19", GLFW_KEY_F19);
	SET_KEY("f20", GLFW_KEY_F20);
	SET_KEY("f21", GLFW_KEY_F21);
	SET_KEY("f22", GLFW_KEY_F22);
	SET_KEY("f23", GLFW_KEY_F23);
	SET_KEY("f24", GLFW_KEY_F24);
	SET_KEY("f25", GLFW_KEY_F25);
	SET_KEY("kp_0", GLFW_KEY_KP_0);
	SET_KEY("kp_1", GLFW_KEY_KP_1);
	SET_KEY("kp_2", GLFW_KEY_KP_2);
	SET_KEY("kp_3", GLFW_KEY_KP_3);
	SET_KEY("kp_4", GLFW_KEY_KP_4);
	SET_KEY("kp_5", GLFW_KEY_KP_5);
	SET_KEY("kp_6", GLFW_KEY_KP_6);
	SET_KEY("kp_7", GLFW_KEY_KP_7);
	SET_KEY("kp_8", GLFW_KEY_KP_8);
	SET_KEY("kp_9", GLFW_KEY_KP_9);
	SET_KEY("kp_decimal", GLFW_KEY_KP_DECIMAL);
	SET_KEY("kp_divide", GLFW_KEY_KP_DIVIDE);
	SET_KEY("kp_multiply", GLFW_KEY_KP_MULTIPLY);
	SET_KEY("kp_substract", GLFW_KEY_KP_SUBTRACT);
	SET_KEY("kp_add", GLFW_KEY_KP_ADD);
	SET_KEY("kp_enter", GLFW_KEY_KP_ENTER);
	SET_KEY("kp_equal", GLFW_KEY_KP_EQUAL);
	SET_KEY("left_shift", GLFW_KEY_LEFT_SHIFT);
	SET_KEY("left_control", GLFW_KEY_LEFT_CONTROL);
	SET_KEY("left_alt", GLFW_KEY_LEFT_ALT);
	SET_KEY("left_super", GLFW_KEY_LEFT_SUPER);
	SET_KEY("right_shift", GLFW_KEY_RIGHT_SHIFT);
	SET_KEY("right_control", GLFW_KEY_RIGHT_CONTROL);
	SET_KEY("right_alt", GLFW_KEY_RIGHT_ALT);
	SET_KEY("right_super", GLFW_KEY_RIGHT_SUPER);
	SET_KEY("menu", GLFW_KEY_MENU);

}
