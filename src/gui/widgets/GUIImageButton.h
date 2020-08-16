#pragma once 
#include "../GUIWidget.h"
#include "GUIBaseButton.h"
#include <assets/AssetManager.h>
#include <assets/Image.h>

#include <string>
#include <nanovg/nanovg_gl.h>


class GUIImageButton : public GUIBaseButton
{
private:

	AssetHandle<Image> img_handle;

public:

	// NanoVG image handle
	int image = 0;
	std::string name;
	glm::ivec2 img_size;

	enum ImageMode
	{
		CENTER,
		STRETCH,
		TILE
	};

	// Use negative values to let that axis free, this changes the size of the button
	glm::ivec2 force_image_size;
	ImageMode img_mode;

	virtual glm::ivec2 prepare(glm::ivec2 wpos, glm::ivec2 wsize, glm::ivec4 viewport, GUIInput* ipt) override;

	virtual void draw(NVGcontext* vg, GUISkin* skin) override;

	void set_image(NVGcontext* vg, AssetHandle<Image>&& img);
	void set_image(NVGcontext* vg, GLuint handle, glm::ivec2 size);

	GUIImageButton()
	{
		force_image_size = glm::ivec2(-1, -1);
		img_mode = STRETCH;
		image = 0; 
	}

};
