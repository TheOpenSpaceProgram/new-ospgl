#include "EditorTrashcan.h"
#include "../EditorScene.h"
#include "EditorGUI.h"
#include <algorithm>


void EditorTrashcan::do_gui(int width, int panel_width, int height, GUIInput* gui_input)
{
	panel.prepare(glm::ivec2(0, 0), glm::ivec2(panel_width, height), gui_input);	
	panel.draw(vg, gui_skin, glm::ivec4(0, 0, width, height));
}

void EditorTrashcan::init(EditorScene* sc, NVGcontext* vg, GUISkin* skin)
{
	trash_image = AssetHandle<Image>("core:categories/trash.png");
	this->gui_skin = skin;
	this->vg = vg;
	this->edgui = &sc->gui;

	trash_area_layout = new GUISingleLayout();
	panel.divide_v(0.5f).first->layout = trash_area_layout;
	//panel.layout = trash_area_layout;

	// Add a single button
	trash_button = GUIImageButton();
	trash_button.set_image(vg, trash_image.data);
	trash_button.img_mode = GUIImageButton::CENTER;
	trash_button.on_clicked.add_handler([this](int btn)
	{
		on_trash(btn);
	});
	trash_area_layout->add_widget(&trash_button);

}

void EditorTrashcan::on_trash(int button)
{
	if(button == GUI_LEFT_BUTTON)
	{
		// The editor vehicle is blocked, so we are responsible for "dropping" the piece
		Piece* parent = edgui->edveh_int->selected;
		edgui->edveh_int->selected = nullptr;	

		EditorVehicle* edveh = edgui->edveh_int->edveh;

		// Find all pieces attached to p (children) and remove them, and also
		// remove them from the editor vehicle
		auto all_pieces = edveh->veh->get_children_of(parent);
		all_pieces.insert(all_pieces.begin(), parent);

		for(Piece* p : all_pieces)
		{
			// Remove from the vehicle
			edveh->veh->remove_piece(p);
			// Remove from the editor vehicle
			edveh->remove_collider(p);
			edveh->piece_meta.erase(p);
			// And then delete the piece 
			// TODO: Store it for later restoration
			delete p;
		}

	}
}