#include "EditorPartList.h"
#include "../EditorScene.h"
#include "../EditorVehicleInterface.h"
#include <game/database/GameDatabase.h>
#include <OSP.h>

void EditorCategory::load_from_path(const std::string& path)
{
	using table_ptr = std::shared_ptr<cpptoml::table>;
	std::string pkg = osp->assets->get_package_and_name(path, "core").first;
	
	table_ptr root = osp->assets->load_toml(path);
	name = root->get_as<std::string>("name").value_or("[NAME NOT SET]");
	desc = root->get_as<std::string>("description").value_or("");
	priority = root->get_as<int64_t>("priority").value_or(0);
	id = root->get_as<std::string>("id").value_or("");

	std::string icon_path = root->get_as<std::string>("icon").value_or("core:notex.png");

	std::string prev_pkg = osp->assets->get_current_package();
	osp->assets->set_current_package(pkg);
	icon = AssetHandle<Image>(icon_path);
	osp->assets->set_current_package(prev_pkg);
}

void EditorPartList::update_part_list()
{
	part_list->remove_all_widgets();

	for(auto& p : all_parts)
	{
		PartPrototype* proto = p.proto.data;
		auto it = std::find(proto->categories.begin(), proto->categories.end(), current_category);
		if(it != proto->categories.end())
		{
			GUIImageButton* btn = new GUIImageButton();
			btn->force_image_size = part_icon_size;
			btn->name = proto->name;
			btn->set_image(vg, p.icon, icon_renderer->size);
			part_list->add_widget(btn);

			btn->during_hover.add_handler([&p, this]()
			{
				p.angle += 0.04;
				p.render(this->icon_renderer);
			});

			btn->on_leave_hover.add_handler([&p, this]()
			{
				p.angle = 0.0;
				p.render(this->icon_renderer);
			});

			btn->on_clicked.add_handler([&p, this](int btn)
			{
				if(btn == GUI_LEFT_BUTTON)
				{
					create_part(p.proto);
				}
			});
		}	
	}
}

void EditorPart::render(PartIconRenderer* render)
{
	if(icon == 0)
	{
		icon = render->create_texture();
	}

	render->render(proto.data, angle, icon);
}


void EditorPartList::create_part(AssetHandle<PartPrototype>& proto) 
{
	Vehicle* veh = edveh_int->edveh->veh;
	EditorVehicle* edveh = edveh_int->edveh;
	
	Part* n_part = new Part(proto, nullptr);
	n_part->vehicle = veh;
	std::vector<Piece*> n_pieces = n_part->create_pieces();
	n_part->init(&scene->lua_state, veh);

	// Find a space for plumbing and put it there IF IT HAS PLUMBING
	// All machines are added at once following the toml offsets
	// so that everything remains coherent spatially
	glm::ivec2 part_pb_size = veh->plumbing.get_plumbing_size_of(n_part);
	if(part_pb_size != glm::ivec2(0, 0))
	{
		glm::ivec2 pos = veh->plumbing.find_free_space(part_pb_size);
		for(const auto& pair : n_part->machines)
		{
			PlumbingMachine& pb = pair.second->plumbing;
			if(pb.has_lua_plumbing())
			{
				pb.editor_position += pos;
			}
		}
	}



	// Create the pieces and parts
	veh->all_pieces.insert(veh->all_pieces.end(), n_pieces.begin(), n_pieces.end());
	veh->parts.push_back(n_part);
	if(veh->root == nullptr)
	{
		veh->root = n_part->pieces["p_root"];
	}

	// Create the editor vehicle stuff
	for(Piece* p : n_pieces)
	{
		edveh->update_collider(p);
	}


	edveh_int->attach_interface.selected = n_part->pieces["p_root"];
	edveh_int->attach_interface.on_selection_change();

	gui_input->block_mouse[GUI_LEFT_BUTTON] = true;

}

void EditorPartList::add_gui(int width, int panel_width, int height, GUIScreen *screen)
{
	screen->add_canvas(&def_panel, glm::ivec2(0, 0), glm::ivec2(panel_width, height));
	// Add for later because we need to block the double click on adding a part
	this->gui_input = screen->gui_input;
}

void EditorPartList::init(EditorScene* sc, NVGcontext* vg)
{
	this->vg = vg;
	this->edveh_int = sc->gui.edveh_int;
	this->scene = sc;

	icon_renderer = new PartIconRenderer(part_icon_size);
	// Load all exposed via the database parts
	for(const std::string& path : osp->game_database->parts)
	{
		all_parts.emplace_back();
		all_parts.back().proto = AssetHandle<PartPrototype>(path);
		all_parts.back().angle = 0.0;
		all_parts.back().icon = 0;
		all_parts.back().render(icon_renderer);		
	}	

	// Load categories
	for(const std::string& cat : osp->game_database->part_categories)
	{
		categories.emplace_back();
		EditorCategory& n = categories[categories.size() - 1];
		n.load_from_path(cat);
	}

	std::sort(categories.begin(), categories.end(), [](const EditorCategory& a, const EditorCategory& b)
	{
		return a.priority < b.priority;
	});

	// Group selector and search bar
	def_panel.divide_v(0.05);
	def_panel.child_0_pixels = 64;
	def_panel.child_0->divide_v(0.5);

	group_manager = nullptr;
	group_selector = new GUISingleLayout();
	group_dropdown = new GUIDropDown();
	update_groups();
	group_dropdown->on_item_change.add_handler(
		[this](const GUIDropDown::IDNamePair& new_opt, const GUIDropDown::IDNamePair& old_opt)
		{
			if(new_opt.first == "manage")
			{
				group_dropdown->select(old_opt.first);
				try_show_group_manager();
			}
			else
			{

			}
		});
	group_dropdown->item = 0;
	group_selector->add_widget(group_dropdown);

	search_bar = new GUISingleLayout();
	GUITextField* tfield = new GUITextField();
	tfield->default_string = osp->game_database->get_string("core:editor_search_parts");
	tfield->reselect_clears = true;
	tfield->escape_clears = true;
	search_bar->add_widget(tfield);

	def_panel.child_0->child_0->layout = group_selector;
	def_panel.child_0->child_1->layout = search_bar;

	def_panel.child_1->divide_h(0.03);
	def_panel.child_1->child_0_pixels = category_icon_size + 4;

	part_list = new GUIListLayout(part_margin, part_margin);
	part_list->margins = glm::ivec4(part_margin, part_margin, part_margin, part_margin);	
	def_panel.child_1->child_1->layout = part_list;

	category_list = new GUIVerticalLayout(part_margin);
	category_list->vscrollbar.width = 1;
	category_list->margins = glm::ivec4(1, 2, part_margin, 0);
	def_panel.child_1->child_0->layout = category_list;

	GameDatabase* gdb = osp->game_database;

	// Category buttons
	for(int i = 0; i < categories.size(); i++)
	{
		GUIImageButton* btn = new GUIImageButton();
		if(i == 0)
		{
			btn->toggled = true;
			old_category_toggled = btn;
		}

		btn->force_image_size = glm::ivec2(category_icon_size);
		btn->set_image(vg, categories[i].icon.duplicate());
		category_list->add_widget(btn);
		std::string id = categories[i].id;

		btn->on_clicked.add_handler([id, this, btn](int _)
		{
			std::string old_id = this->current_category;
			this->current_category = id;

			// Toggle off every other button
			this->old_category_toggled->toggled = false;
			this->old_category_toggled = btn;
			btn->toggled = true;

			if(old_id != id)
			{
				this->update_part_list();
			}

		});
	}

	current_category = "command";
	update_part_list();
}

int EditorPartList::get_panel_width()
{
	return
		(part_icon_size.x + part_margin) * parts_per_row + 	// < Icons take one margin per part
		category_icon_size + part_margin * 2 + 3;			// < Margins of the part list + category margins
}

void EditorPartList::update_groups()
{
	Vehicle* veh = edveh_int->edveh->veh;
	for(size_t id = 0; id < veh->group_names.size(); id++)
	{
		group_dropdown->options.emplace_back(std::to_string(id), veh->group_names[id]);
	}
	group_dropdown->options.emplace_back("manage", "Manage groups");
	group_dropdown->update_options();

	logger->check(group_dropdown->options.size() > 1, "Vehicle doesn't have any groups, there should be atleast one!");
}


void EditorPartList::try_show_group_manager()
{
	if(group_manager != nullptr)
	{
		return;
	}

	Vehicle* veh = edveh_int->edveh->veh;
	group_manager = scene->gui_screen.win_manager.create_window(&group_manager,
																glm::ivec2(-1, -1),
																glm::ivec2(400, 200));
	group_manager->title = "Group Manager";

	group_manager->canvas.divide_h(0.25f);

	// Left child: Group list and at the bottom new group button
	group_manager->canvas.child_0->divide_v(0.8f);
	GUIVerticalLayout* list_layout = new GUIVerticalLayout();
	for(size_t id = 0; id < veh->group_names.size(); id++)
	{
		GUITextButton* btn = new GUITextButton(veh->group_names[id]);
		list_layout->add_widget(btn);
	}

	GUISingleLayout* new_btn_layout = new GUISingleLayout();
	GUITextButton* new_btn = new GUITextButton("Create new");
	new_btn_layout->add_widget(new_btn);
	group_manager->canvas.child_0->child_0->layout = list_layout;
	group_manager->canvas.child_0->child_1->layout = new_btn_layout;
	// Right child: Group options
	GUIVerticalLayout* opts_layout = new GUIVerticalLayout();
	group_manager->canvas.child_1->layout = opts_layout;

}



