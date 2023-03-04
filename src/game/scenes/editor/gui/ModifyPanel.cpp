#include "ModifyPanel.h"
#include "gui/layouts/GUISingleLayout.h"
#include "gui/layouts/GUIVerticalLayout.h"
#include "gui/widgets/GUITextButton.h"
#include "gui/widgets/GUIIconTextButton.h"
#include "universe/vehicle/SymmetryMode.h"

ModifyPanel::ModifyPanel()
{

}

void ModifyPanel::init(EditorScene *nsc, NVGcontext *nvg)
{
	this->sc = nsc;
	this->vg = nvg;

	auto[nchild_0, utils_canvas] = panel.divide_v(0.5);
	child_0 = nchild_0;

	panel.child_pixels = 200;
	panel.pixels_for_child_1 = true;

	auto[part_list_canvas, _] = child_0->divide_v(0.7);
	make_symmetry_canvas_default();
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

void ModifyPanel::change_state(ModifyInterface::State state)
{

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
		btn->set_image(vg, smod.icon.duplicate());
		btn->default_size.y = btn->img_size.y + 10;
		list_layout->add_widget(btn);
	}

	list_c->set_layout(list_layout);

	// Below, buttons to modify or remove existing symmetry groups
	auto buttons_layout = std::make_shared<GUIVerticalLayout>();
	auto modify_btn = std::make_shared<GUITextButton>("Modify");
	auto remove_btn = std::make_shared<GUITextButton>("Remove");
	buttons_layout->add_widget(modify_btn);
	buttons_layout->mark_same_line();
	buttons_layout->add_widget(remove_btn);
	buttons_c->set_layout(buttons_layout);

}

