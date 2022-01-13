#include "PlumbingPanel.h"
#include "../EditorScene.h"
#include "../EditorVehicleInterface.h"
#include <game/database/GameDatabase.h>
#include <OSP.h>
#include <universe/Universe.h>
#include <game/GameState.h>

void PlumbingPanel::init(EditorScene *sc, NVGcontext *vg, GUISkin *skin)
{
	this->gui_skin = skin;
	this->vg = vg;
	this->edveh_int = sc->gui.edveh_int;
	this->scene = sc;

	part_list = new GUIListLayout(part_margin, part_margin);
	part_list->margins = glm::ivec4(part_margin, part_margin, part_margin, part_margin);
	def_panel.divide_v(0.4);
	// Top is the part list
	def_panel.child_0->layout = part_list;

	for(auto& p : osp->game_database->plumbing_machines)
	{
		GUIImageButton* btn = new GUIImageButton();
		// Load the machine icon
		AssetHandle<Config> config = AssetHandle<Config>(p);
		std::string icon = config->root->get_qualified_as<std::string>("icon").value_or("core:notex.png");
		btn->force_image_size = glm::ivec2(64, 64);
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

	// Find a part to assign as target, will usually be root
	// TODO: Serialize this for user experience improvement
	if(sc->vehicle.veh->root->part)
	{
		target_part = sc->vehicle.veh->root->part;
	}
	else
	{
		sc->vehicle.veh->parts[0];
	}
}

void PlumbingPanel::prepare_gui(int width, int panel_width, int height, GUIInput *gui_ipt)
{
	this->gui_input = gui_ipt;
	def_panel.prepare(glm::ivec2(0, 0), glm::ivec2(panel_width, height), gui_input);
}

void PlumbingPanel::do_gui(int width, int panel_width, int height)
{
	def_panel.draw(vg, gui_skin, glm::ivec4(0, 0, width, height));
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
