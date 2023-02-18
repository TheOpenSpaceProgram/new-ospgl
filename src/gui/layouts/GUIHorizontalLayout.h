#pragma once
#include "../GUILayout.h"
#include "GUILinearLayout.h"


class GUIHorizontalLayout : public GUILinearLayout
{
public:


	void position(glm::ivec2 pos, glm::ivec2 size, GUIScreen* screen) override;
	void prepare(GUIInput* gui_input, GUIScreen* screen) override;
	void pre_prepare(GUIScreen* screen) override;


	GUIHorizontalLayout(int element_margin = 4)
	{
		this->element_margin = element_margin;
		// Set some sane defaults
		hscrollbar.positive_pos = true;
		hscrollbar.draw = true;
		hscrollbar.enabled = true;
	}
};
