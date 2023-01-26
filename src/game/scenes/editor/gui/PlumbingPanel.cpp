#include "PlumbingPanel.h"
#include "../EditorScene.h"
#include "../EditorVehicleInterface.h"
#include <game/database/GameDatabase.h>
#include <OSP.h>
#include <universe/Universe.h>
#include <game/GameState.h>

void PlumbingPanel::init(EditorScene *sc, NVGcontext *vg)
{
	this->vg = vg;
	this->edveh_int = sc->gui.edveh_int;
	this->scene = sc;

	part_list = new GUIListLayout(part_margin, part_margin);
	part_list->margins = glm::ivec4(part_margin, part_margin, part_margin, part_margin);
	trashcan = new GUISingleLayout();

	def_panel.divide_v(0.4);
	// Top is the part list
	def_panel.child_0->layout = part_list;
	def_panel.child_1->divide_v(0.2f);
	def_panel.child_1->child_0->layout = trashcan;

	for(auto& p : osp->game_database->plumbing_machines)
	{
		GUIImageButton* btn = new GUIImageButton();
		// Load the machine icon
		AssetHandle<Config> config = AssetHandle<Config>(p);
		std::string icon = config->root->get_qualified_as<std::string>("icon").value_or("core:notex.png");
		btn->default_size = glm::ivec2(64, 64);
		btn->name = config->root->get_qualified_as<std::string>("name").value_or("");
		btn->name = osp->game_database->get_string(btn->name);
		AssetHandle<Image> img = AssetHandle<Image>(icon);

		btn->set_image(vg,img.duplicate());
		part_list->add_widget(btn);

		btn->on_clicked.add_handler([p, this](int btn)
		{
			if(btn == GUI_LEFT_BUTTON)
			{
				create_machine(p);
			}
		});
	}

	trashcan_button = new GUITextButton("", "medium");
	trashcan_button->on_clicked.add_handler([this](int btn)
	{
		if(btn == 0)
		{
			auto selected = edveh_int->plumbing_interface.pb_editor.get_selected();
			// We are certain they are all plumbing machines
			for(PlumbingMachine* m : selected)
			{
				auto& mvec = m->in_machine->in_part->attached_machines;
				auto pos = std::find(mvec.begin(), mvec.end(), m->in_machine);
				logger->check(pos != mvec.end(), "Only plumbing machines can be deleted from plumbing");
				mvec.erase(pos);
			}
		}
	});
	trashcan->add_widget(trashcan_button);

	// Find a part to assign as target, will usually be root
	// TODO: Serialize this for user experience improvement
	if(sc->vehicle->veh->root->part)
	{
		target_part = sc->vehicle->veh->root->part;
	}
	else
	{
		sc->vehicle->veh->parts[0];
	}
}

void PlumbingPanel::add_gui(int width, int panel_width, int height, GUIScreen* screen)
{
	auto selected = edveh_int->plumbing_interface.pb_editor.get_selected();
	bool all_plumbing = !selected.empty();
	for(PlumbingMachine* m : selected)
	{
		if(!vector_contains(m->in_machine->in_part->attached_machines, m->in_machine))
		{
			all_plumbing = false;
			break;
		}
	}

	if(all_plumbing)
	{
		trashcan_button->text = "Delete selected machines";
		trashcan_button->disabled = false;
	}
	else
	{
		trashcan_button->text = "Select plumbing machines";
		trashcan_button->disabled = true;
	}

	screen->add_canvas(&def_panel, glm::ivec2(0, 0), glm::ivec2(panel_width, height));

}

void PlumbingPanel::create_machine(std::string db_id)
{
	// We create the machine and instantly select it in the plumbing editor
	AssetHandle<Config> config = AssetHandle<Config>(db_id);
	auto[pkg, name] = osp->assets->get_package_and_name(db_id, "core");
	auto machine_table = config->root->get_table_qualified("machine");
	Machine* pb_machine = new Machine(machine_table, pkg);

	// We attach it to currently assigned target part
	target_part->attached_machines.push_back(pb_machine);
	pb_machine->init(&osp->game_state->universe.lua_state, target_part);

	// Place it in a reasonable position
	pb_machine->plumbing.editor_rotation = 0;
	pb_machine->plumbing.editor_position =
			edveh_int->edveh->veh->plumbing.find_free_space(pb_machine->plumbing.get_size(true));

	// We take control of it in the plumbing editor
	edveh_int->plumbing_interface.pb_editor.selected.clear();
	edveh_int->plumbing_interface.pb_editor.selected.push_back(&pb_machine->plumbing);
}
