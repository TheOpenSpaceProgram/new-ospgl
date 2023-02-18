#pragma once
#include "GUILinearLayout.h"


class GUIVerticalLayout : public GUILinearLayout
{
public:


	void position(glm::ivec2 pos, glm::ivec2 size, GUIScreen* screen) override;
	void prepare(GUIInput* gui_input, GUIScreen* screen) override;
	void pre_prepare(GUIScreen* screen) override;

	GUIVerticalLayout(int element_margin = 4)
	{
		this->element_margin = element_margin;
		// Set some sane defaults
		vscrollbar.positive_pos = true;
		vscrollbar.scroll = 0.0;
		vscrollbar.draw = true;
		vscrollbar.enabled = true;
	}
};
