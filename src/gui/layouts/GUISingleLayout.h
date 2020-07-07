#pragma once
#include "../GUILayout.h"


class GUISingleLayout : public GUILayout 
{
private:

public:


	virtual void prepare(GUIInput* gui_input) override;

	GUISingleLayout()
	{
	}
};
