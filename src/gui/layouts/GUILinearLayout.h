#pragma once
#include "../GUILayout.h"

class GUILinearLayout : public GUILayout
{
protected:
	int element_margin;
	std::vector<bool> same_line;

public:

	// Next added widget will be on the same line as previous one
	void mark_same_line();

	void linear_helper(glm::ivec2 vpos, glm::ivec2 vsize, GUIScreen* screen, bool vertical);

	// TO preserve same-line
	void on_add_widget(GUIWidget* widget) override;
	void on_remove_widget(GUIWidget* widget) override;
	void on_move_widget(GUIWidget* widget, int new_index) override;

};
