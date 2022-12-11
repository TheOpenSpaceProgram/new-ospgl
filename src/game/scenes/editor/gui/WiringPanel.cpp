#include "WiringPanel.h"
#include "../EditorScene.h"
#include "EditorGUI.h"
#include <algorithm>

void WiringPanel::init(EditorScene *sc, NVGcontext *vg, GUISkin *skin)
{
	this->gui_skin = skin;
	this->vg = vg;
	this->edgui = &sc->gui;

}

void WiringPanel::prepare_gui(int width, int panel_width, int height, GUIInput *ginput)
{
	this->gui_input = ginput;
	panel.prepare(glm::ivec2(0, 0), glm::ivec2(panel_width, height), ginput);
}

void WiringPanel::do_gui(int width, int panel_width, int height)
{
	panel.draw(vg, gui_skin, glm::ivec4(0, 0, width, height));
}
