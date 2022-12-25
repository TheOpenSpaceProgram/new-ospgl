#include "EditorTrashcan.h"
#include <game/scenes/editor/EditorScene.h>
#include "../EditorScene.h"
#include "EditorGUI.h"
#include <algorithm>

void EditorTrashcan::add_gui(int width, int panel_width, int height, GUIScreen *screen)
{
	screen->add_canvas(&panel, glm::ivec2(0, 0), glm::ivec2(panel_width, height));
}

void EditorTrashcan::init(EditorScene* sc, NVGcontext* vg)
{
	trash_image = AssetHandle<Image>("core:categories/trash.png");
	this->vg = vg;
	this->scene = sc;
	this->edgui = &sc->gui;

	trash_area_layout = new GUISingleLayout();
	panel.divide_v(0.5f).first->layout = trash_area_layout;
	//panel.layout = trash_area_layout;

	// Add a single button
	trash_button = GUIImageButton();
	trash_button.blocks_mouse = false;
	trash_button.set_image(vg, trash_image.duplicate());
	trash_button.img_mode = GUIImageButton::CENTER;
	trash_button.on_clicked.add_handler([this](int btn)
	{
		on_trash(btn);
	});
	trash_area_layout->add_widget(&trash_button);

	// It looks better this way
	trash_area_layout->block_mouse = false;

}

void EditorTrashcan::on_trash(int button)
{
	if(button == GUI_LEFT_BUTTON)
	{
		if(edgui->editor_mode == EditorGUI::ATTACHING)
		{
			// The editor vehicle is blocked, so we are responsible for "dropping" the piece
			Piece* parent = edgui->edveh_int->attach_interface.selected;
			edgui->edveh_int->attach_interface.selected = nullptr;

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
}

