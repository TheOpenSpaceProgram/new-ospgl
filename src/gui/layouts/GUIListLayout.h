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
		vscrollbar.positive_pos = true;
		vscrollbar.draw = true;
		vscrollbar.enabled = true;
	}
};
