#include "GroupManager.h"
#include "../EditorScene.h"

void GroupManager::create_right_panel()
{
	Vehicle* veh = scene->vehicle_int.edveh->veh;

	GUIVerticalLayout* opts_layout = new GUIVerticalLayout();
	GUITextField* tfield = new GUITextField();
	GUITextButton* move_up = new GUITextButton(osp->game_database->get_string("core:gman_move_up"));
	GUITextButton* move_down = new GUITextButton(osp->game_database->get_string("core:gman_move_down"));
	GUIDropDown* move_to = new GUIDropDown();
	move_to->not_chosen_string = osp->game_database->get_string("core:gman_move_all");
	GUITextButton* remove_btn = new GUITextButton(osp->game_database->get_string("core:gman_delete"));
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
	win->canvas.child_1->layout = opts_layout;

}

void GroupManager::try_show()
{
	if(win != nullptr)
	{
		return;
	}

	Vehicle* veh = scene->vehicle_int.edveh->veh;
	win = scene->gui_screen.win_manager.create_window(&win,
																glm::ivec2(-1, -1),
																glm::ivec2(400, 200));
	win->title = osp->game_database->get_string("core:group_manager");

	win->canvas.divide_h(0.25f);

	// Left child: Group list and at the bottom new group button
	win->canvas.child_0->divide_v(0.8f);
	GUIVerticalLayout* list_layout = new GUIVerticalLayout();
	GUITextButton* btn = new GUITextButton(osp->game_database->get_string("core:default_group"));
	btn->toggled = selected_group < 0;
	list_layout->add_widget(btn);
	for(size_t id = 0; id < veh->group_names.size(); id++)
	{
		btn = new GUITextButton(veh->group_names[id]);
		btn->toggled = selected_group == id;
		list_layout->add_widget(btn);
	}

	GUISingleLayout* new_btn_layout = new GUISingleLayout();
	GUITextButton* new_btn = new GUITextButton("Create new");
	new_btn_layout->add_widget(new_btn);
	win->canvas.child_0->child_0->layout = list_layout;
	win->canvas.child_0->child_1->layout = new_btn_layout;

	create_right_panel();
}

void GroupManager::init(EditorScene* nsc, EditorPartList* npl)
{
	this->scene = nsc;
	this->pl = npl;
	win = nullptr;
	selected_group = -1;
}
