#include "../GUILayout.h"

class GUIVerticalLayout : public GUILayout
{
private:

	glm::ivec4 margins;
	int element_margin;

public:


	virtual void prepare(GUIInput* gui_input) override;

	// Margins are in pixels and are (left, right, up, down)
	GUIVerticalLayout(int element_margin = 4, glm::ivec4 margins = glm::ivec4(4, 4, 4, 4))
	{
		this->margins = margins;
		this->element_margin = element_margin;
	}
};
