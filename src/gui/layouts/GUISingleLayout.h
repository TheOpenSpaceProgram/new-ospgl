#pragma once
#include "../GUILayout.h"


class GUISingleLayout : public GUILayout 
{
private:

public:


	void position(glm::ivec2 pos, glm::ivec2 size, GUIScreen* screen) override;
	void prepare(GUIInput* gui_input, GUIScreen* screen) override;
	void pre_prepare(GUIScreen* screen) override;

	GUISingleLayout()
	{
	}
};
