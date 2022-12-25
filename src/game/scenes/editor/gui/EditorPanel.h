#pragma once
#include <nanovg/nanovg.h>
#include <gui/GUISkin.h>
#include <gui/GUIInput.h>
#include <gui/GUIScreen.h>

class EditorScene;

// Base class for all editor panels
class EditorPanel
{
public:


	virtual void init(EditorScene* sc, NVGcontext* vg) = 0;

	virtual void add_gui(int width, int panel_width, int height, GUIScreen* screen) = 0;
};