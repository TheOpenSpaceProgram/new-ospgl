#pragma once 
#include "../GUISkin.h"

// A simple skin used in most interfaces, inspired
// by the look of FreeOrion
class SimpleSkin : public GUISkin 
{
private:
	
	float titlebar_height = 18.0f;
	float titlebar_margin = 5.0f;

	float window_margins = 10.0f;
	float window_edge_size = 1.0f;
	
	float window_icon_size = 11.0f;

	AssetHandle<Image> window_close;
	AssetHandle<Image> window_min;
	AssetHandle<Image> window_pin;
	AssetHandle<BitmapFont> window_title_font;


public:
	

	virtual void draw_button(NVGcontext* v, glm::ivec2 pos, glm::ivec2 size, const std::string& text,
		ButtonState state, ButtonStyle style = ButtonStyle::NORMAL) override;

	virtual void draw_dropdown_header(NVGcontext* vg, glm::ivec2 pos, glm::ivec2 size, const std::string& text,
									  bool is_open, bool default_text, ButtonState st) override;

	virtual void draw_dropdown_body(NVGcontext* vg, glm::ivec2 pos, glm::ivec2 size,
									const std::vector<std::string> w) override;

	virtual bool can_drag_window(GUIWindow* window, glm::ivec2 mpos) override;
	virtual bool can_close_window(GUIWindow* window, glm::ivec2 mpos) override;
	virtual bool can_minimize_window(GUIWindow* window, glm::ivec2 mpos) override;
	virtual bool can_pin_window(GUIWindow* window, glm::ivec2 mpos) override;
	virtual ResizePoint can_resize_window(GUIWindow* window, glm::ivec2 mpos) override;
	virtual glm::ivec4 get_window_aabb(GUIWindow* window) override;

	virtual void draw_window(NVGcontext* vg, GUIWindow* window) override;

	virtual void draw_link(NVGcontext* vg, glm::ivec2 link_start, glm::ivec2 win_pos) override;
	virtual bool can_cut_link(NVGcontext* vg, glm::ivec2 link_start, glm::ivec2 win_pos) override;

	NVGcolor get_background_color(bool bright = false) override;
	NVGcolor get_stroke_color(ButtonState state) override;
	NVGcolor get_fill_color(ButtonState state) override;
	NVGcolor get_error_color() override;
	NVGcolor get_highlight_color() override;
	NVGcolor get_foreground_color(bool soft = false) override;
	NVGcolor get_button_color(ButtonState state) override;

	int get_scrollbar_width() override;
	NVGcolor get_scrollbar_color() override;
	NVGcolor get_scrollbar_scroller_color() override;

	SimpleSkin();
};
