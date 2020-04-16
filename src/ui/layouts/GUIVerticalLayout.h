#include "../GUILayout.h"

class GUIVerticalLayout : public GUILayout
{
public:

	unsigned char r, g, b;

	virtual void draw(glm::ivec2 pos, glm::ivec2 size, NVGcontext* vg) override;

	GUIVerticalLayout(unsigned char r, unsigned char g, unsigned char b)
	{
		this->r = r;
		this->g = g;
		this->b = b;
	}
};
