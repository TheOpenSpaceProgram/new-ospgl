#pragma once
#include "../GUILayout.h"


class GUIListLayout : public GUILayout 
{
private:

	int element_vmargin;
	int element_hmargin;

public:


	void position(glm::ivec2 pos, glm::ivec2 size, GUIScreen* screen) override;
	void prepare(GUIInput* gui_input, GUIScreen* screen) override;
	void pre_prepare(GUIScreen* screen) override;

	GUIListLayout(int element_vmargin = 4, int element_hmargin = 4)
	{
		this->element_vmargin = element_vmargin;
		this->element_hmargin = element_hmargin;

		// Set some sane defaults
		vscrollbar.color = nvgRGB(100, 100, 100);
		vscrollbar.scroller_color = nvgRGB(180, 180, 180);
		vscrollbar.scroller_sel_color = nvgRGB(240, 240, 240);
		vscrollbar.width = 4;
		vscrollbar.positive_pos = true;
		vscrollbar.scroll = 0.0;
		vscrollbar.draw = true;
		vscrollbar.enabled = true;
	}
};
