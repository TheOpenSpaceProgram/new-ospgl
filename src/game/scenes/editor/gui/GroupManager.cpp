#include "GroupManager.h"
#include "../EditorScene.h"

void GroupManager::create_right_panel()
{
	Vehicle* veh = scene->vehicle_int.edveh->veh;

	auto opts_layout = std::make_shared<GUIVerticalLayout>();
	auto tfield = std::make_shared<GUITextField>();
	auto move_up = std::make_shared<GUITextButton>(osp->game_database->get_string("core:gman_move_up"));
	auto move_down = std::make_shared<GUITextButton>(osp->game_database->get_string("core:gman_move_down"));
	auto move_to = std::make_shared<GUIDropDown>();
	move_to->not_chosen_string = osp->game_database->get_string("core:gman_move_all");
	auto remove_btn = std::make_shared<GUITextButton>(osp->game_database->get_string("core:gman_delete"));
	if(selected_group < 0)
	{
		tfield->disabled = true;
		move_up->disabled = true;
		move_down->disabled = true;
		remove_btn->disabled = true;
	}
	else if(selected_group == 0)
	{
		move_up->disabled = true;
	}

	tfield->string = veh->get_group_name(selected_group);

	opts_layout->add_widget(tfield);
	opts_layout->add_widget(move_up);
	opts_layout->add_widget(move_down);
	opts_layout->add_widget(move_to);
	opts_layout->add_widget(remove_btn);
	auto w = win.lock();
	w->canvas->child_1->layout = opts_layout;

}

void GroupManager::try_show()
{
	if(!win.expired())
	{
		return;
	}

	Vehicle* veh = scene->vehicle_int.edveh->veh;
	win = scene->gui_screen.win_manager.create_window(glm::ivec2(-1, -1),
													  glm::ivec2(400, 200));
	auto w = win.lock();
	w->title = osp->game_database->get_string("core:group_manager");

	w->canvas->divide_h(0.25f);

	// Left child: Group list and at the bottom new group button
	w->canvas->child_0->divide_v(0.8f);
	auto list_layout = std::make_shared<GUIVerticalLayout>();
	auto btn = std::make_shared<GUITextButton>(osp->game_database->get_string("core:default_group"));
	btn->toggled = selected_group < 0;
	list_layout->add_widget(btn);
	for(size_t id = 0; id < veh->group_names.size(); id++)
	{
		btn = std::make_shared<GUITextButton>(veh->group_names[id]);
		btn->toggled = selected_group == id;
		list_layout->add_widget(btn);
	}

	auto new_btn_layout = std::make_shared<GUISingleLayout>();
	auto new_btn = std::make_shared<GUITextButton>("Create new");
	new_btn_layout->add_widget(new_btn);
	w->canvas->child_0->child_0->layout = list_layout;
	w->canvas->child_0->child_1->layout = new_btn_layout;
	w->canvas->child_0->pixels_for_child_1 = true;
	w->canvas->child_0->child_pixels = 30;

	create_right_panel();
}

void GroupManager::init(EditorScene* nsc, EditorPartList* npl)
{
	this->scene = nsc;
	this->pl = npl;
	selected_group = -1;
}
