#pragma once
#include "EditorPanel.h"
#include <gui/GUICanvas.h>
#include <gui/layouts/GUISingleLayout.h>
#include <gui/widgets/GUIImageButton.h>
#include <assets/AssetManager.h>
#include <assets/Image.h>

class EditorGUI;

// We allow dropping in stuff to be deleted, and also handle creation of subassemblies
// TODO: Deleted stuff is kept in a buffer for later recovery, but up to a limit
// * Could be done by simply storing all the pieces which are trashed (keeping them
// * alive in memory and keeping the links between them)
class EditorTrashcan : public EditorPanel
{
private:
	GUISkin* gui_skin;
	NVGcontext* vg;

	AssetHandle<Image> trash_image;

	GUICanvas panel;
	GUISingleLayout* trash_area_layout;
	GUIImageButton trash_button;

	EditorGUI* edgui;
	GUIInput* gui_input;
	
	void on_trash(int button);

public:
	void init(EditorScene* sc, NVGcontext* vg, GUISkin* skin) override;
	void prepare_gui(int width, int panel_width, int height, GUIInput* gui_input) override;
	void do_gui(int width, int panel_width, int height) override;
};