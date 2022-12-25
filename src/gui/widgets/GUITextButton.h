#pragma once
#include "GUIBaseButton.h"

class GUITextButton : public GUIBaseButton
{
private:
	bool uses_bitmap;
	std::string ft_font;
	float ft_size;
	AssetHandle<BitmapFont> bm_font;
public:
	// If the button is allowed to be freely sized, what size should we use on each axis?
	glm::ivec2 button_size;

	// If set to true, the button will ignore skin colors and use given color
	bool override_color = false;
	NVGcolor color;

	std::string text;
	bool center_vertical = true;
	bool center_horizontal = true;

	bool force_to_text_size;

	void prepare(glm::ivec4 viewport, GUIScreen* screen, GUIInput* gui_input) override;
	glm::ivec2 position(glm::ivec2 wpos, glm::ivec2 wsize, GUIScreen* screen) override;
	virtual void draw(NVGcontext* ctx, GUISkin* skin) override;

	GUITextButton(std::string text, std::string font = "regular", float size = 16.0f)
	{
		this->text = text;
		uses_bitmap = false;
		ft_font = font;
		ft_size = size;
		button_size = glm::ivec2(400, 20);
	}

	// Warning: bitmap text doesn't support line breaking!
	GUITextButton(std::string text, AssetHandle<BitmapFont>&& font)
	{
		this->text = text;
		uses_bitmap = true;
		bm_font = std::move(font);
		button_size = glm::ivec2(400, 20);
	}
};


