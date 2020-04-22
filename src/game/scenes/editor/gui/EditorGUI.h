#pragma once
#include <nanovg/nanovg.h>
#include <gui/GUICanvas.h>
#include <gui/GUIInput.h>
#include <gui/widgets/GUIImageButton.h>
#include <gui/layouts/GUIListLayout.h>
#include <gui/layouts/GUIVerticalLayout.h>
#include <gui/skins/SimpleSkin.h>

#include <assets/PartPrototype.h>
#include <assets/Image.h>
#include <assets/AssetManager.h>

#include "PartIconRenderer.h"

class EditorScene;

struct EditorCategory
{
	AssetHandle<Image> icon;
	// Ownership of the AssetHandles is done in EditorGUI, so we hold
	// a raw pointer
	std::vector<PartPrototype*> parts;
	std::string name;
	std::string desc;
	std::string id;

	int priority;

	void load_from_path(const std::string& path);

};


struct EditorPart
{
	AssetHandle<PartPrototype> proto;
	GLuint icon;	
	double angle;
	void render(PartIconRenderer* renderer);
};

// The side-pane can display a lot of stuff, but usually it displays
// the C++ implemented part list and editor controls. Parts can draw a
// custom interface there, use their context menu, or pop up a full
// blown, lua controlled, gui
class EditorGUI
{
private:

	int prev_width, prev_height;
	std::vector<GUIImageButton*> buttons;

public:

	GUIImageButton* old_category_toggled;

	PartIconRenderer* icon_renderer;

	SimpleSkin skin;

	std::string current_category = "command";

	NVGcontext* vg;

	std::vector<EditorPart> all_parts;
	std::vector<EditorCategory> categories;

	glm::ivec2 part_icon_size = glm::ivec2(64, 84);
	int parts_per_row = 3;
	int part_margin = 8;
	int category_icon_size = 24;

	void do_gui(int width, int height, GUIInput* gui_input);
	
	GUICanvas def_panel;
	GUIListLayout* part_list;
	GUIVerticalLayout* category_list;

	void init(EditorScene* scene);

	// Called when chaning category
	void update_part_list();

};