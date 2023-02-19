#pragma once
#include "../GUIWidget.h"

class GUILabel : public GUIWidget
{
private:
	std::string ft_font;
	float ft_size;
public:

	enum LabelStyle
	{
		TITLE,
		SEPARATOR
	};

	bool override_color = false;
	NVGcolor color;

	std::string text;
	bool center_vertical = true;
	bool center_horizontal = true;
	LabelStyle style = LabelStyle::TITLE;

	void prepare(glm::ivec4 viewport, GUIScreen* screen, GUIInput* gui_input) override;
	glm::ivec2 position(glm::ivec2 wpos, glm::ivec2 wsize, GUIScreen* screen) override;
	virtual void draw(NVGcontext* ctx, GUISkin* skin) override;

	GUILabel(std::string text, std::string font = "regular", float size = 16.0f)
	{
		this->text = text;
		ft_font = font;
		ft_size = size;
		default_size = glm::ivec2(400, 24);
	}
};
