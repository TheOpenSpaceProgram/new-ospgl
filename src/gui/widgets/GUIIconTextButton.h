#pragma once
#include "GUIBaseButton.h"

// Shows an icon and text next to it. The icon behaves
// pretty much like the "first" character of the text
class GUIIconTextButton : public GUIBaseButton
{
private:

	std::string ft_font;
	float ft_size;
	AssetHandle<Image> img_handle;
	int image;

public:
	bool override_color = false;
	NVGcolor color;

	std::string text;
	bool center_vertical = true;
	bool center_horizontal = true;

	int image_text_margin;
	glm::ivec2 img_size;

	void prepare(glm::ivec4 viewport, GUIScreen* screen, GUIInput* gui_input) override;
	glm::ivec2 position(glm::ivec2 wpos, glm::ivec2 wsize, GUIScreen* screen) override;

	virtual void draw(NVGcontext* vg, GUISkin* skin) override;

	void set_image(NVGcontext* vg, AssetHandle<Image>&& img);

	GUIIconTextButton(std::string text, std::string font = "regular", float size = 16.0f)
	{
		this->text = text;
		ft_font = font;
		ft_size = size;
		default_size = glm::ivec2(400, 24);
		image_text_margin = 4;
	}
};
