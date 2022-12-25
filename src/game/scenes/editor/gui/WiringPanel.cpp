#include "WiringPanel.h"
#include "../EditorScene.h"
#include "EditorGUI.h"
#include <algorithm>

void WiringPanel::init(EditorScene *sc, NVGcontext *vg)
{
	this->scene = sc;
	this->vg = vg;
	this->edgui = &sc->gui;

}

void WiringPanel::add_gui(int width, int panel_width, int height, GUIScreen *screen)
{
	screen->add_canvas(&panel, glm::ivec2(0, 0), glm::ivec2(panel_width, height));
}
