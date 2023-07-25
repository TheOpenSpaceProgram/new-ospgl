#include "ModifyInterface.h"
#include "../EditorVehicleInterface.h"
#include "../EditorScene.h"

#include <glm/gtx/color_space.hpp>

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
	if(cur_state == IDLE)
	{
		return true;
	}
	else
	{
		return false;
	}
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
		selected_piece = nullptr;
		pick_another_piece = false;
	}
	else if(st == CREATING_SYMMETRY)
	{
		cur_attachment_point = -1;

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
		// We use a different color for each, which we obtain by rotating
		// around in the HSV cone
		float hue = 0.0f;
		for(SymmetryMode* mode : edveh->veh->meta.symmetry_groups)
		{
			std::vector<Piece*> pieces = mode->all_in_symmetry;
			bool sim_hovered = vector_contains(pieces, hovered);
			glm::vec3 color = glm::rgbColor(glm::vec3(hue, 1.0f, sim_hovered ? 1.0f : 0.35f));
			for(Piece* p : pieces)
			{
				edveh->piece_meta[p].highlight = color;
			}
			// Avoids perfect division of the circle so we get more total colors (find good value for this)
			hue += 47.5f;
		}
	}
	else
	{
		return do_interface_modify_symmetry(hovered, ipt);
	}

	return false;
}

bool ModifyInterface::do_interface_create_symmetry(Piece *hovered, GUIInput *ipt)
{
	if(selected_piece == nullptr)
	{
		if(hovered != prev_hover)
		{
			cur_attachment_point = -1;
		}

		if (hovered)
		{
			// Make sure the piece has free attachment points
			bool could_create = false;
			if (hovered->attachments.size() > 0)
			{

				if(cur_attachment_point == -1)
				{
					// Find a free attachment point
					for(int i = 0; i < hovered->attachments.size(); i++)
					{
						if(is_attachment_compatible(i, hovered))
						{
							cur_attachment_point = i;
							could_create = true;
							break;
						}
					}
				}
				else
				{
					could_create = true;
				}

				could_create &= hovered->attached_to == nullptr;

				if(could_create)
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
					if(!ipt->keyboard_blocked)
					{
						if(input->key_down(GLFW_KEY_PAGE_UP))
						{
							// Go to next free attachment
							for(int i = cur_attachment_point + 1; i < hovered->attachments.size(); i++)
								if(is_attachment_compatible(i, hovered))
								{
									cur_attachment_point = i;
									break;
								}
						}
						if(input->key_down(GLFW_KEY_PAGE_DOWN))
						{
							for(int i = cur_attachment_point - 1; i >= 0; i--)
								if(is_attachment_compatible(i, hovered))
								{
									cur_attachment_point = i;
									break;
								}
						}
					}
					edveh->draw_attachment_for.emplace_back(hovered, cur_attachment_point);
				}
			}

			if(!could_create)
			{
				edveh->piece_meta[hovered].highlight = glm::vec3(1.0f, 0.0f, 0.0f);
			}
		}
		else
		{
			cur_attachment_point = -1;
		}
	}
	else
	{
		return do_interface_modify_symmetry(hovered, ipt);
	}

	prev_hover = hovered;
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

void ModifyInterface::start_picking_piece(bool allow_only_radial, std::vector<Piece*> forbidden)
{
	forbidden_pieces = forbidden;
	only_radial_allowed = allow_only_radial;
	if(cur_state == CREATING_SYMMETRY)
	{
		pick_another_piece = true;
	}
	else
	{
		change_state(SELECTING_PIECE);
	}
}

bool ModifyInterface::do_interface_modify_symmetry(Piece *hovered, GUIInput *ipt)
{
	auto children = highlight_symmetry(selected_piece);
	if(pick_another_piece && hovered)
	{
		bool is_forbidden = vector_contains(forbidden_pieces, hovered);
		is_forbidden |= vector_contains(children, hovered);
		is_forbidden |= selected_piece == hovered;

		if((hovered->piece_prototype->allows_radial || !only_radial_allowed) && !is_forbidden && hovered != selected_piece)
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
		else
		{
			edveh->piece_meta[hovered].highlight = glm::vec3(1.0f, 0.0f, 0.0f);
		}
	}

	return false;
}

bool ModifyInterface::is_attachment_compatible(int i, Piece* p)
{
	SymmetryMode* sm = edveh_int->scene->gui.modify_tools.modifying_symmetry;
	if (!p->attachments[i].second)
	{
		auto attach = p->attachments[i].first;
		if ((attach.radial && sm->can_use_radial_attachments) ||
			(attach.stack && sm->can_use_stack_attachments))
		{
			return true;
		}
	}

	return false;
}