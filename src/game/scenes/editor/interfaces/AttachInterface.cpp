#include "AttachInterface.h"
#include "../EditorScene.h"
#include <util/InputUtil.h>

void AttachInterface::use_attachment_port(Piece* target, std::string port)
{
	bool found = false;
	for(auto& pair : target->attachments)
	{
		if(pair.first.marker == port)
		{
			logger->check(pair.second == false, "Attachment port is already in use");
			pair.second = true;
			found = true;
			break;
		}
	}

	logger->check(found, "Could not find attachment port used in selected piece");

}

void AttachInterface::attach(Piece* target, std::string port)
{
	selected->attached_to = target;
	selected->from_attachment = selected_attachment->marker;
	selected->to_attachment = port;

	// TODO: Links
	selected->welded = true;

	//use_attachment_port(selected, selected_attachment->marker);
	if(port != "")
	{
		logger->info("Using port: {}", port);
		//use_attachment_port(target, port);
	}

	selected = nullptr;
	selected_attachment = nullptr;

	edveh->veh->update_attachments();
}

void AttachInterface::handle_input_hovering(const CameraUniforms& cu, 
		glm::dvec3 ray_start, glm::dvec3 ray_end, GUIInput* gui_input)
{
	EditorVehicleInterface::RaycastResult rresult = edveh_int->raycast(ray_start, ray_end, false);

	Piece* hovered = nullptr;
	if(rresult.has_hit)
	{
		hovered = rresult.p;
	}

	if(hovered != nullptr && !gui_input->mouse_blocked)
	{
		if(input->mouse_down(GLFW_MOUSE_BUTTON_LEFT))
		{
			selected = hovered;
			hovered = nullptr;
			on_selection_change(cu);
		}
	}

	for(auto& mp : edveh->piece_meta)
	{
		if(mp.first == hovered)
		{
			mp.second.highlight = glm::vec3(1.0f);
		}
	}
}

glm::dmat4 AttachInterface::get_current_tform(glm::dvec3 mpos)
{
	glm::dmat4 tform_marker = glm::dmat4(1.0);

	if(selected_attachment)
	{
		tform_marker = selected->get_marker_transform(selected_attachment->marker);
	}

	glm::dvec3 npos = mpos + selected_offset;
	glm::dmat4 tform_new = glm::translate(glm::dmat4(1.0), npos);
	tform_new = tform_new * glm::toMat4(selected_rotation) * glm::inverse(tform_marker);

	return tform_new;
}

glm::dquat AttachInterface::get_fixed_rotation()
{
	if(selected_attachment == nullptr)
	{
		return glm::dquat(1.0, 0.0, 0.0, 0.0);
	}

	// We want to get a rotation that changes current forward to (0, 0, 1)
	glm::dmat4 mat = selected->get_marker_transform(selected_attachment->marker);
	mat = glm::inverse(mat);

	glm::dvec3 _; glm::dvec4 __;
	glm::dquat orient;
	glm::decompose(mat, _, orient, _, _, __);

	return orient;
}

void AttachInterface::on_attachment_change()
{
	selected_rotation = get_fixed_rotation();
}

Piece* AttachInterface::try_attach_radial(glm::dvec3 ray_start, glm::dvec3 ray_end, 
	const std::vector<Piece*>& children, std::string& attach_to_marker)
{
	if(!selected_attachment->radial)
	{
		return nullptr;
	}

	EditorVehicleInterface::RaycastResult rresult = edveh_int->raycast(ray_start, ray_end, true, children);

	if(rresult.has_hit)
	{
		// Position the piece for radial attachment, adjusting distance and rotation
		selected_distance = glm::length(rresult.world_pos - ray_start);
		// Note that here we use +Z as up, not +Y!
		selected_rotation = MathUtil::quat_look_at(glm::vec3(0, 0, 0), rresult.world_nrm,
				glm::dvec3(0, 0, 1), glm::dvec3(0, 1, 0));
	
		return rresult.p;
		attach_to_marker = "";
	}

	return nullptr;
}


void AttachInterface::on_selection_change(double dist) 
{
	selected->attached_to = nullptr;
	// Fix for instant re-attaching
	ignore_attachment = selected->to_attachment;
	selected->to_attachment = "";
	selected->from_attachment = "";

	edveh->veh->update_attachments();
	selected_distance = dist;

	selected_buffer.clear();

	selected_attachment = find_free_attachment();
	on_attachment_change();

	auto children = edveh->veh->get_children_of(selected);
	children.push_back(selected);
	edveh_int->ignore_center = children;
}

Piece* AttachInterface::try_attach_stack(glm::dvec3 r0, glm::dvec3 r1, glm::dvec3 selected_pos, 
		const std::vector<Piece*>& children, std::string& attach_to_marker)
{
	if(!selected_attachment->stack)
	{
		return nullptr;
	}

	// Raycast from the camera towards the attachment point and find the closest attachment points
	// using ray-point distance


	// We first find the ray collision point, if any, to avoid attaching to unreachable
	// points
	EditorVehicleInterface::RaycastResult rresult = edveh_int->raycast(r0, r1, false, children);
	double max_walk = 9999999999.9;
	if(rresult.has_hit)
	{
		max_walk = glm::distance(rresult.world_pos, r0);
	}

	Piece* closest = nullptr;
	std::string closest_attch = "";
	double dist = 99999999.9;
	glm::dvec3 closest_pos;
	glm::dvec3 closest_fw;

	glm::dvec3 dir = glm::normalize(r1 - r0);
	for(Piece* p : edveh->veh->all_pieces)
	{
		if(std::find(children.begin(), children.end(), p) == children.end())
		{
			// Go over all free stack attachment
			for(auto& pair : p->attachments)
			{
				if(pair.first.stack && !pair.second)
				{
					glm::dmat4 m_tform = p->get_marker_transform(pair.first.marker);
					glm::dmat4 p_tform = p->get_graphics_matrix();
					glm::dvec3 pos = glm::dvec3(p_tform * m_tform * glm::dvec4(0, 0, 0, 1));

					// We use the parametric form of r0 + t * dir to obtain the formula for distance
					// and then calculate the first derivative to find the minimum distance point:
					// (I used maxima to solve for t and then extracted the vectorial form)
					double t = glm::dot(pos - r0, dir) / glm::length2(dir);
					glm::dvec3 rpos = r0 + dir * t;
					double ndist = glm::distance(rpos, pos);
					if(t >= max_walk || t <= 0)
					{
						ndist = 999999999999.9;
					}

					if(ndist <= dist)
					{
						dist = ndist;
						closest_attch = pair.first.marker;
						closest = p;
						closest_pos = pos;
						// Note that in the editor +Z = forward
						closest_fw = glm::dvec3(p_tform * m_tform * glm::dvec4(0, 0, 1, 1)) - closest_pos;
					}
				}
			}
		}
	}

	if(dist < 0.6)
	{
		if(closest_attch != ignore_attachment)
		{
			selected_rotation = MathUtil::quat_look_at(glm::dvec3(0), closest_fw);
			selected_offset = closest_pos - selected_pos;
			attach_to_marker = closest_attch;
			return closest;
		}
		else
		{
			return nullptr;
		}
	}

	ignore_attachment = "";
	attach_to_marker = "";
	return nullptr;

}

void AttachInterface::handle_input_selected(const CameraUniforms& cu, 
		glm::dvec3 ray_start, glm::dvec3 ray_end, GUIInput* gui_input)
{
	
	std::vector<Piece*> children = edveh->veh->get_children_of(selected);
	children.push_back(selected);

	if(!gui_input->keyboard_blocked)
	{
		if(input->key_down(GLFW_KEY_PAGE_UP))
		{
			if(input->key_pressed(GLFW_KEY_LEFT_CONTROL))
			{
				cycle_pieces(-1);
			}
			else
			{
				cycle_attachments(-1);
			}
		}

		if(input->key_down(GLFW_KEY_PAGE_DOWN))
		{
			if(input->key_pressed(GLFW_KEY_LEFT_CONTROL))
			{
				cycle_pieces(1);
			}
			else
			{
				cycle_attachments(1);
			}
		}
	}


	Piece* attach_to = nullptr;
	std::string attach_to_marker = "";

	selected_offset = glm::vec3(0);

	glm::dvec3 selected_pos = ray_start + glm::normalize(ray_end - ray_start) * selected_distance;

	if(selected_attachment)
	{
		// Attempt attachment
		if(allow_stack)
		{
			attach_to = try_attach_stack(ray_start, ray_end, selected_pos, children, attach_to_marker);
		}
		else
		{
			ignore_attachment = "";
		}

		if(allow_radial && attach_to == nullptr)
		{
			attach_to = try_attach_radial(ray_start, ray_end, children, attach_to_marker);
		}
	}

	// Coloring and visuals
	for(Piece* child : children)
	{
		EditorVehiclePiece& piece_meta = edveh->piece_meta[child];
		piece_meta.draw_out_attachments = true;
		piece_meta.draw_in_attachments = false;
		piece_meta.highlight = glm::vec3(0.5f);

		edveh->update_collider(child);

		if(child == selected)
		{
			piece_meta.highlight = glm::vec3(1.0f);

			if(selected_attachment)
			{
				piece_meta.attachment_color[selected_attachment->marker] = 
					glm::vec4(1.0f, 1.0f, 0.5f, 1.0f);
			}
		}

	}
	
	glm::dvec3 ray_fw = glm::normalize(ray_end - ray_start);
	glm::dmat4 tform_new = get_current_tform(ray_start + ray_fw * selected_distance);
	glm::dmat4 tform_0 = selected->get_graphics_matrix();

	// Update all children
	for(Piece* child : children)
	{
		glm::dmat4 relative = glm::inverse(tform_0) * child->get_graphics_matrix() * child->collider_offset;
		glm::dmat4 final = tform_new * relative; 
		child->packed_tform = to_btTransform(final);
	}
	
	if(gui_input->mouse_down(GUI_LEFT_BUTTON) && !gui_input->mouse_blocked)
	{
		if(attach_to == nullptr)
		{
			selected = nullptr;
		}
		else
		{
			attach(attach_to, attach_to_marker);
		}
	}


}

void AttachInterface::on_selection_change(const CameraUniforms& cu)
{
	if(!selected->editor_dettachable)
	{
		// We flip the link around so that the piece is not dettached
		reroot(selected->attached_to, selected);
	}

	glm::dvec3 diff = cu.cam_pos - to_dvec3(selected->packed_tform.getOrigin()); 
	on_selection_change(glm::length(diff));
}

	

PieceAttachment* AttachInterface::find_free_attachment()
{
	// Find the first free attachment
	for(int i = 0; i < selected->attachments.size(); i++)
	{
		if(!selected->attachments[i].second && !selected->attachments[i].first.hidden)
		{
			//selected_attachment = selected->attachments[i].first.marker;
			return &selected->attachments[i].first;
		}
	}

	return nullptr;
}

void AttachInterface::cycle_attachments(int dir)
{
	int cur_index = -1;
	bool any_free = false;
	for(int i = 0; i < selected->attachments.size(); i++)
	{
		if(&selected->attachments[i].first == selected_attachment)
		{
			cur_index = i;
		}

		if(!selected->attachments[i].second && !selected->attachments[i].first.hidden)
		{
			any_free = true;
		}
	}

	if(!any_free)
	{
		return;
	}

	bool found = false;
	while(!found)
	{
		cur_index += dir;
		if(cur_index >= (int)selected->attachments.size())
		{
			cur_index = 0;
		}

		if(cur_index < 0)
		{
			cur_index = selected->attachments.size() - 1;	
		}
		
		if(!selected->attachments[cur_index].second && !selected->attachments[cur_index].first.hidden)
		{
			selected_attachment = &selected->attachments[cur_index].first;
			found = true;
			on_attachment_change();
		}

	}
}

void AttachInterface::cycle_pieces(int dir)
{
	std::vector<Piece*> children = edveh->veh->get_children_of(selected);

	Piece* old = selected;

	if(dir == 1)
	{
		// TODO: Improve this method?
		// Find eaerlist children that is not in revert buffer
		for(Piece* child : children)
		{
			bool found = false;

			for(Piece* buf : selected_buffer)
			{
				if(child == buf)
				{
					found = true;
					break;
				}
			}

			if(!found)
			{
				// Try attach
				selected = child;
				PieceAttachment* n_attachment = find_free_attachment();
				if(n_attachment != nullptr)
				{
					// Found it
					selected_attachment = n_attachment;
					on_attachment_change();
					break;
				}	
				else
				{
					selected = old;					
				}
			}
		}	
	}
	else
	{
		while(selected_buffer.size() != 0)
		{
			// Go up in the reverse buffer until it's empty
			if(selected_buffer.size() != 0)
			{
				selected = selected_buffer.front();
				selected_buffer.pop_back();
				PieceAttachment* n_attachment = find_free_attachment();
				if(n_attachment != nullptr)
				{
					// We found a valid one! We are done
					selected_attachment = n_attachment;
					on_attachment_change();
					break;
				}
				else
				{
					selected = old;
				}
			}
		}
	}

	if(old != selected)
	{
		reroot(old, selected);
	}
}

void AttachInterface::reroot(Piece* current, Piece* new_root)
{
	// We simply change the attached_to, but don't do tree sorting
	// as it's used only in the editor
		
	// Everything which is attached to current, except these before, needs to change
	std::vector<Piece*> current_children = edveh->veh->get_children_of(current);

	// We have to make the graph flow towards new_root instead of towards current
	// To do so, we only need to change the pieces on the tree branch that contains
	// the new root (the other branches are already correct as they have to go
	// through current to reach new_root)

	// We find the branch that contains new_root
	
	std::vector<Piece*> branch;
	// To do so, we walk up from new_root until we reach current root
		
	Piece* ptr = new_root;
	while(ptr != current)
	{
		branch.push_back(ptr);
		ptr = ptr->attached_to;
	}
	branch.push_back(current);

	// We now have all pieces which need to be re-arranged
	// in the opposite order to the wanted order
	// We simply have to invert the links traversing the array
	// in the opposite order
	
	for(auto it = branch.rbegin(); it != branch.rend(); it++)
	{
		auto next = it + 1;
		if(next == branch.rend())
		{
			// We are the new root, no attachment
			(*it)->attached_to = nullptr;		
		}
		else
		{
			// Copy link stuff from next and invert link
			(*it)->attached_to = (*next);
			(*it)->link = std::move((*next)->link);
			// Note the inversion here, very important
			(*it)->to_attachment = (*next)->from_attachment;
			(*it)->from_attachment = (*next)->to_attachment;
			std::swap((*it)->editor_dettachable, (*next)->editor_dettachable);


			(*it)->link_from = (*next)->link_to;
			(*it)->link_to = (*next)->link_from;
			// TODO: Invert this too
			(*it)->link_rot = (*next)->link_rot;
		}
	}	

	edveh->veh->update_attachments();

}

void AttachInterface::update(double dt)
{
	edveh->clear_meta();

	if(selected == nullptr)
	{
		edveh_int->ignore_center.clear();
	}
}

void AttachInterface::leave() 
{
}

bool AttachInterface::can_leave() 
{
	return selected == nullptr;
}

bool AttachInterface::do_interface(const CameraUniforms& cu, glm::dvec3 ray_start, glm::dvec3 ray_end,
		glm::dvec4 viewport, NVGcontext* vg, GUIInput* gui_input, GUISkin* gui_skin)
{
	edveh_int->middle_click_focus(ray_start, ray_end);

	edveh->draw_attachments = selected != nullptr;

	if(selected == nullptr)
	{
		handle_input_hovering(cu, ray_start, ray_end, gui_input);
	}
	else
	{
		handle_input_selected(cu, ray_start, ray_end, gui_input);
	}

	return false;
}


AttachInterface::AttachInterface(EditorVehicleInterface* edveh_int) 
{
	this->edveh_int = edveh_int;
	this->edveh = edveh_int->edveh;
	this->scene = edveh_int->scene;

	allow_radial = true;
	allow_stack = true;
	selected = nullptr;	
}
