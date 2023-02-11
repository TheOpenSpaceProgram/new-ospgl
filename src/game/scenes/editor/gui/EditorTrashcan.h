#pragma once
#include "EditorPanel.h"
#include <gui/GUICanvas.h>
#include <gui/layouts/GUISingleLayout.h>
#include <gui/widgets/GUIImageButton.h>
#include <assets/AssetManager.h>
#include <assets/Image.h>

class EditorGUI;
class EditorScene;

// We allow dropping in stuff to be deleted, and also handle creation of subassemblies
// TODO: Deleted stuff is kept in a buffer for later recovery, but up to a limit
// * Could be done by simply storing all the pieces which are trashed (keeping them
// * alive in memory and keeping the links between them)
class EditorTrashcan : public EditorPanel
{
private:
	NVGcontext* vg;
	EditorScene* scene;

	AssetHandle<Image> trash_image;

	GUICanvas panel;
	std::shared_ptr<GUISingleLayout> trash_area_layout;
	std::shared_ptr<GUIImageButton> trash_button;

	EditorGUI* edgui;
	GUIInput* gui_input;
	
	void on_trash(int button);

public:
	void init(EditorScene* sc, NVGcontext* vg) override;
	void add_gui(int width, int panel_width, int height, GUIScreen* screen) override;
};