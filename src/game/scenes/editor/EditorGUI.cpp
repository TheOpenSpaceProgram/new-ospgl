#include "EditorGUI.h"
#include <glm/glm.hpp>

#include <gui/layouts/GUIVerticalLayout.h>
#include <gui/layouts/GUIListLayout.h>
#include <gui/widgets/GUIImageButton.h>
#include "EditorScene.h"
#include <OSP.h>


void EditorCategory::load_from_path(const std::string& path)
{
	using table_ptr = std::shared_ptr<cpptoml::table>;
	std::string pkg = assets->get_package_and_name(path, "core").first;
	
	table_ptr root = SerializeUtil::load_file(assets->resolve_path(path));
	name = root->get_as<std::string>("name").value_or("[NAME NOT SET]");
	desc = root->get_as<std::string>("description").value_or("");
	priority = root->get_as<int64_t>("priority").value_or(0);
	id = root->get_as<std::string>("id").value_or("");

	std::string icon_path = root->get_as<std::string>("icon").value_or("core:notex.png");

	std::string prev_pkg = assets->get_current_package();
	assets->set_current_package(pkg);
	icon = AssetHandle<Image>(icon_path);
	assets->set_current_package(prev_pkg);
}

void EditorGUI::do_gui(int width, int height, GUIInput* gui_input)
{
	float w = (float)width; float h = (float)height;

	// Draw the side pane
	float swidth = float(
		(part_icon_size + part_margin) * parts_per_row + 	// < Icons take one margin per part
		category_icon_size + part_margin * 2 + 3);			// < Margins of the part list + category margins


	nvgBeginPath(vg);
	nvgRect(vg, 0.0f, 0.0f, swidth, 1.0f * h);
	nvgFillColor(vg, nvgRGB(30, 35, 40));
	nvgFill(vg);


	def_panel.prepare(glm::ivec2(0, 0), glm::ivec2(swidth, height), gui_input);	
	def_panel.draw(vg, glm::ivec4(0, 0, width, height));
	//def_panel.debug(glm::ivec2(0, 0), glm::ivec2(swidth, height), vg);

	prev_width = width;
	prev_height = height;
}



void EditorGUI::init(EditorScene* sc)
{
	prev_width = 0;
	prev_height = 0;

	// Load all exposed via the database parts
	for(const std::string& path : sc->get_osp()->game_database.parts)
	{
		all_parts.emplace_back(path);
	}	

	// Load categories
	for(const std::string& cat : sc->get_osp()->game_database.part_categories)
	{
		categories.emplace_back();
		EditorCategory& n = categories[categories.size() - 1];
		n.load_from_path(cat);
	}

	std::sort(categories.begin(), categories.end(), [](const EditorCategory& a, const EditorCategory& b)
	{
		return a.priority < b.priority;
	});

	// Top search bar
	def_panel.divide_v(0.05);
	def_panel.child_0_pixels = 32;
	def_panel.child_1->divide_h(0.03);
	def_panel.child_1->child_0_pixels = category_icon_size + 4;

	part_list = new GUIListLayout(part_margin, part_margin);
	part_list->margins = glm::ivec4(part_margin, part_margin, part_margin, part_margin);	
	def_panel.child_1->child_1->layout = part_list;

	category_list = new GUIVerticalLayout(part_margin);
	category_list->vscrollbar.width = 1;
	category_list->margins = glm::ivec4(1, 2, part_margin, 0);
	def_panel.child_1->child_0->layout = category_list;

	GameDatabase* gdb = &sc->get_osp()->game_database;

	for(int i = 0; i < categories.size(); i++)
	{
		GUIImageButton* btn = new GUIImageButton();
		btn->force_image_size = glm::ivec2(category_icon_size);
		btn->set_image(vg, categories[i].icon.data);
		category_list->add_widget(btn);
		std::string id = categories[i].id;

		btn->on_clicked.add_handler([id, this](int btn)
		{
			std::string old_id = this->current_category;
			this->current_category = id;
			if(old_id != id)
			{
				this->update_part_list();
			}
			logger->info("Clicked with id: {}", id);
		});
	}

	current_category = "command";
	update_part_list();

}


void EditorGUI::update_part_list()
{
	part_list->remove_all_widgets();

	for(AssetHandle<PartPrototype>& proto : all_parts)
	{
		auto it = std::find(proto->categories.begin(), proto->categories.end(), current_category);
		if(it != proto->categories.end())
		{
			GUIImageButton* btn = new GUIImageButton();
			btn->force_image_size = glm::ivec2(part_icon_size);
			btn->name = proto->name;
			part_list->add_widget(btn);
		}	
	}
}
