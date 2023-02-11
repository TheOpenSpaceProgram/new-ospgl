#pragma once 
#include <assets/PartPrototype.h>
#include "PartIconRenderer.h"
#include "EditorPanel.h"

#include <gui/widgets/GUIImageButton.h>
#include <gui/widgets/GUITextField.h>
#include <gui/widgets/GUIDropDown.h>
#include <gui/layouts/GUIListLayout.h>
#include <gui/layouts/GUIVerticalLayout.h>
#include <gui/layouts/GUISingleLayout.h>
#include <gui/skins/SimpleSkin.h>
#include <gui/GUICanvas.h>
#include "GroupManager.h"

class EditorVehicleInterface;

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

class EditorPartList : public EditorPanel
{
private:

	std::vector<GUIImageButton*> buttons;

	NVGcontext* vg;
	EditorScene* scene;
	EditorVehicleInterface* edveh_int;
	GUIInput* gui_input;

	void create_part(AssetHandle<PartPrototype>& proto);
	std::shared_ptr<GUIDropDown> group_dropdown;
	void update_groups();

	GroupManager group_manager;

public:

	std::shared_ptr<GUIImageButton> old_category_toggled;
	PartIconRenderer* icon_renderer;

	std::string current_category = "command";

	std::vector<EditorPart> all_parts;
	std::vector<EditorCategory> categories;

	glm::ivec2 part_icon_size = glm::ivec2(64, 84);
	int parts_per_row = 3;
	int part_margin = 8;
	int category_icon_size = 24;
	
	GUICanvas def_panel;
	std::shared_ptr<GUIListLayout> part_list;
	std::shared_ptr<GUIVerticalLayout> category_list;

	std::shared_ptr<GUISingleLayout> group_selector;
	std::shared_ptr<GUISingleLayout> search_bar;

	// Called when changing category
	void update_part_list();

	void init(EditorScene* sc, NVGcontext* vg) override;

	void add_gui(int width, int panel_width, int height, GUIScreen* screen) override;

	int get_panel_width();
	
};

