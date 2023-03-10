#include "ModifyInterface.h"
#include "../EditorVehicleInterface.h"
#include "../EditorScene.h"

void ModifyInterface::update(double dt)
{
	edveh->clear_meta();
}

bool
ModifyInterface::do_interface(const CameraUniforms &cu, glm::dvec3 ray_start, glm::dvec3 ray_end, glm::dvec4 viewport,
							  NVGcontext *vg, GUIInput *gui_input, GUISkin *gui_skin)
{
	EditorVehicleInterface::RaycastResult rresult = edveh_int->raycast(ray_start, ray_end, false);

	Piece* hovered = nullptr;
	if(rresult.has_hit)
	{
		hovered = rresult.p;
	}

	if(cur_state == IDLE)
	{
		return do_interface_idle(hovered, gui_input);
	}
	else if(cur_state == SELECTING_SYMMETRY)
	{
		return do_interface_select_symmetry(hovered, gui_input);
	}
	else if(cur_state == CREATING_SYMMETRY)
	{
		return do_interface_create_symmetry(hovered, gui_input);
	}

	return false;
}

void ModifyInterface::leave()
{
	edveh_int->emit_event("close_context_menus");

}

bool ModifyInterface::can_leave()
{
	return true;
}

ModifyInterface::ModifyInterface(EditorVehicleInterface* eint)
{
	this->edveh_int = eint;
	this->edveh = eint->edveh;
	cur_state = IDLE;
}

void ModifyInterface::change_state(ModifyInterface::State st)
{
	if(st == IDLE)
	{
	}
	else if(st == CREATING_SYMMETRY)
	{

	}
	else if(st == RE_ROOTING)
	{

	}
	else if(st == SELECTING_PIECE)
	{

	}

	edveh_int->scene->gui.modify_tools.change_state(st, nullptr);

	cur_state = st;
}

bool ModifyInterface::do_interface_idle(Piece* hovered, GUIInput* ipt)
{
	if(hovered)
	{
		edveh->piece_meta[hovered].highlight = glm::vec3(1.0f);
	}

	if(!ipt->mouse_blocked)
	{
		if(input->mouse_down(GLFW_MOUSE_BUTTON_LEFT))
		{
			int id = hovered ? hovered->id : -1;
			edveh_int->emit_event("on_piece_click", id);
		}
	}

	return false;

}

bool ModifyInterface::do_interface_select_symmetry(Piece *hovered, GUIInput *ipt)
{
	if(selected_piece == nullptr)
	{
		// Highlight all symmetry roots (and child pieces)
	}
	else
	{
		// Highlight selected symmetry group
	}

	return false;
}

bool ModifyInterface::do_interface_create_symmetry(Piece *hovered, GUIInput *ipt)
{
	if(selected_piece == nullptr)
	{
		if (hovered)
		{
			// Make sure the piece can be disconnected
			if (hovered->editor_dettachable)
			{
				highlight_symmetry(hovered);
				if (!ipt->mouse_blocked)
				{
					if (input->mouse_down(GLFW_MOUSE_BUTTON_LEFT))
					{
						selected_piece = hovered;
						// Tell the panel that piece was selected
						edveh_int->scene->gui.modify_tools.change_state(CREATING_SYMMETRY, selected_piece);
					}
				}
			}
			else
			{
				edveh->piece_meta[hovered].highlight = glm::vec3(1.0f, 0.0f, 0.0f);
			}
		}
	}
	else
	{
		auto children = highlight_symmetry(selected_piece);
		if(pick_another_piece && hovered)
		{
			bool is_in_symmetry = false;
			for(Piece* p : children)
			{
				if(p == hovered)
				{
					is_in_symmetry = true;
					break;
				}
			}
			if(!is_in_symmetry && hovered != selected_piece)
			{
				edveh->piece_meta[hovered].highlight = glm::vec3(1.0f);

				if (!ipt->mouse_blocked)
				{
					if (input->mouse_down(GLFW_MOUSE_BUTTON_LEFT))
					{
						edveh_int->emit_event("on_select_piece", hovered->id);
					}
				}
			}
		}
	}


	return false;
}

std::vector<Piece*> ModifyInterface::highlight_symmetry(Piece* root)
{
	edveh->piece_meta[root].highlight = glm::vec3(0.5f, 0.5f, 1.0f);
	auto children = edveh->veh->get_children_of(root);
	for (Piece *p: children)
	{
		edveh->piece_meta[p].highlight = glm::vec3(0.0f, 0.0f, 1.0f);
	}
	return children;
}
