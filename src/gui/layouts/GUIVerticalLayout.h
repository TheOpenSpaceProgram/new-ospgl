#include "../GUILayout.h"

class GUIVerticalLayout : public GUILayout
{
private:

	int element_margin;

public:


	virtual void prepare(GUIInput* gui_input) override;

	GUIVerticalLayout(int element_margin = 4)
	{
		this->element_margin = element_margin;
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
