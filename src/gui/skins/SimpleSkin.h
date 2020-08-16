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
	

	NVGcolor background_color = nvgRGB(40, 45, 50);

	virtual void draw_button(NVGcontext* v, glm::ivec2 pos, glm::ivec2 size, const std::string& text, 
		ButtonState state, ButtonStyle style = ButtonStyle::NORMAL) override;
	
	virtual bool can_drag_window(GUIWindow* window, glm::ivec2 mpos) override;
	virtual bool can_close_window(GUIWindow* window, glm::ivec2 mpos) override;
	virtual bool can_minimize_window(GUIWindow* window, glm::ivec2 mpos) override;
	virtual bool can_pin_window(GUIWindow* window, glm::ivec2 mpos) override;
	virtual ResizePoint can_resize_window(GUIWindow* window, glm::ivec2 mpos) override;
	virtual glm::ivec4 get_window_aabb(GUIWindow* window) override;
	
	virtual void draw_window(NVGcontext* vg, GUIWindow* window) override;

	SimpleSkin();
};
