#pragma once
#include "EditorPanel.h"
#include "../interfaces/ModifyInterface.h"
#include "universe/vehicle/SymmetryMode.h"
#include "gui/widgets/GUITextButton.h"


class EditorGUI;
class EditorScene;
// Shows a list of pieces / parts on the vehicle, alongside the utils toolbox and symmetry toolbox
// By default, clicking parts shows context menus, but all tools have certain interactions
class ModifyPanel : public EditorPanel
{
private:
	EditorScene* sc;
	EditorGUI* edgui;
	ModifyInterface* mod_int;
	EditorVehicleInterface* edveh_int;

	GUIInput* gui_input;
	NVGcontext* vg;

	GUICanvas panel;
	// Contains the part list and symmetry mode, to quickly replace the later
	std::shared_ptr<GUICanvas> child_0;

	void make_symmetry_canvas_default();
	void make_symmetry_canvas_info();
	void make_symmetry_canvas_creating();

	std::shared_ptr<GUICanvas> symmetry_canvas_default;
	std::shared_ptr<GUICanvas> symmetry_canvas_info_select;
	std::shared_ptr<GUICanvas> symmetry_canvas_info_create;
	// child_0 is set by the lua script, child_1 contains cancel/finish button
	std::shared_ptr<GUICanvas> symmetry_canvas_creating;

	std::shared_ptr<GUITextButton> finish_button;
	std::shared_ptr<GUITextButton> cancel_button;

	std::string creating_symmetry_name;

public:

	// nullptr if no symmetry is being modified
	SymmetryMode* modifying_symmetry;

	void set_symmetry_canvas(std::shared_ptr<GUICanvas> canvas, bool can_finish, bool can_go_back);

	void change_state(ModifyInterface::State state, Piece* selected_piece);
	void init(EditorScene* sc, NVGcontext* vg) override;
	void add_gui(int width, int panel_width, int height, GUIScreen* screen) override;


	ModifyPanel();
	~ModifyPanel();

};
