#include "ModifyPanel.h"
#include "gui/layouts/GUISingleLayout.h"
#include "gui/layouts/GUIVerticalLayout.h"
#include "gui/widgets/GUITextButton.h"
#include "gui/widgets/GUIIconTextButton.h"
#include "gui/widgets/GUILabel.h"
#include "../EditorScene.h"
#include "../interfaces/ModifyInterface.h"

ModifyPanel::ModifyPanel()
{

}

void ModifyPanel::init(EditorScene *nsc, NVGcontext *nvg)
{
	this->sc = nsc;
	this->vg = nvg;
	mod_int = &nsc->vehicle_int.modify_interface;
	edveh_int = &nsc->vehicle_int;

	auto[nchild_0, utils_canvas] = panel.divide_v(0.5);
	child_0 = nchild_0;

	panel.child_pixels = 200;
	panel.pixels_for_child_1 = true;

	auto[part_list_canvas, _] = child_0->divide_v(0.7);
	make_symmetry_canvas_default();
	make_symmetry_canvas_info();
	make_symmetry_canvas_creating();
	child_0->child_1 = symmetry_canvas_default;

	auto part_lists_layout = std::make_shared<GUISingleLayout>();
	// Create the part tree widget
	auto widget = std::make_shared<GUITextButton>("Part list here");
	part_lists_layout->add_widget(widget);
	part_list_canvas->set_layout(part_lists_layout);



}

void ModifyPanel::add_gui(int width, int panel_width, int height, GUIScreen *screen)
{
	screen->add_canvas(&panel, glm::ivec2(0, 0), glm::ivec2(panel_width, height));
}

void ModifyPanel::change_state(ModifyInterface::State st, Piece* selected_piece)
{
	if(st == ModifyInterface::IDLE)
	{
		child_0->child_1 = symmetry_canvas_default;
	}
	else if(st == ModifyInterface::SELECTING_SYMMETRY)
	{
		if(selected_piece == nullptr)
		{
			child_0->child_1 = symmetry_canvas_info_select;
		}
		else
		{
			// Let lua create the GUI
		}
	}
	else if(st == ModifyInterface::CREATING_SYMMETRY)
	{
		if(selected_piece == nullptr)
		{
			child_0->child_1 = symmetry_canvas_info_create;

			// Create the symmetry mode proper in the vehicle
			modifying_symmetry = new SymmetryMode();
			sc->vehicle->veh->meta.symmetry_groups.push_back(modifying_symmetry);

			auto config = AssetHandle<Config>(creating_symmetry_name);
			config->read_to(*modifying_symmetry);
			// We initialize the symmetry mode without any config, as it's being created
			modifying_symmetry->init(&sc->lua_state, edveh_int->edveh, config.pkg);

			// empty canvas by default
			symmetry_canvas_creating->child_0 = std::make_shared<GUICanvas>();
			child_0->child_1 = symmetry_canvas_creating;
		}
		else
		{
			// Finally, a piece is selected and the symmetry takes control, pass it to the
			// symmetry
			modifying_symmetry->root = selected_piece;
			modifying_symmetry->original_root_pos = to_dvec3(selected_piece->packed_tform.getOrigin());
			modifying_symmetry->original_root_rot = to_dquat(selected_piece->packed_tform.getRotation());

			std::string mname = selected_piece->attachments[mod_int->cur_attachment_point].first.marker;
			modifying_symmetry->attachment_used = mname;
			//modifying_symmetry->root_attachment = mod_int->cur_attachment_point;
			modifying_symmetry->take_gui_control(this, mod_int, edveh_int);
		}
	}

}

void ModifyPanel::make_symmetry_canvas_default()
{
	symmetry_canvas_default = std::make_shared<GUICanvas>();
	symmetry_canvas_default->clear();

	// On the first half, a list of all symmetry modes
	auto[list_c, buttons_c] = symmetry_canvas_default->divide_v(0.5);
	symmetry_canvas_default->child_pixels = 40;
	symmetry_canvas_default->pixels_for_child_1 = true;

	auto list_layout = std::make_shared<GUIVerticalLayout>();
	for(const std::string& sym_mode : osp->game_database->symmetry_modes)
	{
		// We load the symmetry mode to obtain the data, no real need to cache (this is very light)
		// so the asset is freed immediatly
		SymmetryMode smod;
		auto config = AssetHandle<Config>(sym_mode);
		config->read_to(smod);

		auto btn = std::make_shared<GUIIconTextButton>(smod.ui_name);
		btn->sign_up_for_event("on_clicked", EventHandler([this, sym_mode](EventArguments& args)
		{
			creating_symmetry_name = sym_mode;
			mod_int->change_state(ModifyInterface::CREATING_SYMMETRY);
		}));
		btn->set_image(vg, smod.icon.duplicate());
		btn->default_size.y = btn->img_size.y + 10;
		list_layout->add_widget(btn);
	}

	list_c->set_layout(list_layout);

	// Below, buttons to modify or remove existing symmetry groups
	auto buttons_layout = std::make_shared<GUIVerticalLayout>();
	auto modify_btn = std::make_shared<GUITextButton>("Modify");
	modify_btn->sign_up_for_event("on_clicked", EventHandler([this](EventArguments& args)
  	{
		mod_int->change_state(ModifyInterface::SELECTING_SYMMETRY);
  	}));

	auto remove_btn = std::make_shared<GUITextButton>("Remove");
	remove_btn->sign_up_for_event("on_clicked", EventHandler([this](EventArguments& args)
	{
		mod_int->change_state(ModifyInterface::ERASING_SYMMETRY);
	}));

	buttons_layout->add_widget(modify_btn);
	buttons_layout->mark_same_line();
	buttons_layout->add_widget(remove_btn);
	buttons_c->set_layout(buttons_layout);

}

void ModifyPanel::make_symmetry_canvas_info()
{
	symmetry_canvas_info_select = std::make_shared<GUICanvas>();
	auto layout = std::make_shared<GUIVerticalLayout>();
	auto str = osp->game_database->get_string("core:select_symmetry");
	auto str2 = osp->game_database->get_string("core:cancel");
	auto text1 = std::make_shared<GUILabel>(str);
	text1->style = GUILabel::SEPARATOR;
	auto btn2 = std::make_shared<GUITextButton>(str2);
	layout->add_widget(text1);
	layout->add_widget(btn2);
	symmetry_canvas_info_select->set_layout(layout);

	symmetry_canvas_info_create = std::make_shared<GUICanvas>();
	auto layout2 = std::make_shared<GUIVerticalLayout>();
	auto str3 = osp->game_database->get_string("core:create_symmetry");
	auto text2 = std::make_shared<GUILabel>(str3);
	text2->style = GUILabel::SEPARATOR;
	layout2->add_widget(text2);
	layout2->add_widget(btn2);
	symmetry_canvas_info_create->set_layout(layout2);
}

void ModifyPanel::make_symmetry_canvas_creating()
{
	symmetry_canvas_creating = std::make_shared<GUICanvas>();
	auto[_, bottom] = symmetry_canvas_creating->divide_v(0.5);
	symmetry_canvas_creating->child_pixels = 40;
	symmetry_canvas_creating->pixels_for_child_1 = true;
	auto layout = std::make_shared<GUIVerticalLayout>();
	auto str = osp->game_database->get_string("core:cancel");
	cancel_button = std::make_shared<GUITextButton>(str);
	cancel_button->sign_up_for_event("on_clicked", EventHandler([this](EventArguments& args)
	{
		if(cancel_goes_back)
		{
			modifying_symmetry->gui_go_back();
		}
		else
		{
			// Remove the symmetry mode which has not been created
			std::vector<SymmetryMode*>& array = sc->vehicle->veh->meta.symmetry_groups;
			int i = -1;
			for(i = 0; i < array.size(); i++)
			{
				if(array[i] == modifying_symmetry)
				{
					break;
				}
			}
			logger->check(i >= 0);
			array.erase(array.begin() + i);
			modifying_symmetry->leave_gui_control();
			modifying_symmetry->remove(edveh_int->edveh);
			delete modifying_symmetry;
			modifying_symmetry = nullptr;
			mod_int->change_state(ModifyInterface::IDLE);
		}
	}));
	layout->add_widget(cancel_button);
	auto str2 = osp->game_database->get_string("core:finish");
	finish_button  = std::make_shared<GUITextButton>(str2);
	finish_button->sign_up_for_event("on_clicked", EventHandler([this](EventArguments& args)
	{
		// Exit, the symmetry mode is saved in the vehicle
		mod_int->change_state(ModifyInterface::IDLE);
		modifying_symmetry->leave_gui_control();
		modifying_symmetry = nullptr;
		edveh_int->edveh->veh->meta.cleanup_symmetry_groups();
	}));
	layout->mark_same_line();
	layout->add_widget(finish_button);
	bottom->set_layout(layout);
}

void ModifyPanel::set_symmetry_canvas(std::shared_ptr<GUICanvas> canvas, bool can_finish, bool can_go_back)
{
	child_0->child_1->child_0 = canvas;
	finish_button->disabled = !can_finish;

	std::string back;
	if(can_go_back)
	{
		back = osp->game_database->get_string("core:back");
		cancel_goes_back = true;
	}
	else
	{
		cancel_goes_back = false;

		if(mod_int->cur_state == ModifyInterface::SELECTING_SYMMETRY)
		{
			back = osp->game_database->get_string("core:undo");
		}
		else
		{
			back = osp->game_database->get_string("core:cancel");
		}
	}

	cancel_button->text = back;

}

ModifyPanel::~ModifyPanel()
{

}

