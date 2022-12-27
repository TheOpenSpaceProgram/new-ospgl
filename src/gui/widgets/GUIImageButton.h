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

	ImageMode img_mode;

	void prepare(glm::ivec4 viewport, GUIScreen* screen, GUIInput* gui_input) override;
	glm::ivec2 position(glm::ivec2 wpos, glm::ivec2 wsize, GUIScreen* screen) override;

	virtual void draw(NVGcontext* vg, GUISkin* skin) override;

	void set_image(NVGcontext* vg, AssetHandle<Image>&& img);
	void set_image(NVGcontext* vg, GLuint handle, glm::ivec2 size);

	GUIImageButton()
	{
		default_size = glm::ivec2(-1, -1);
		img_mode = STRETCH;
		image = 0; 
	}

};
