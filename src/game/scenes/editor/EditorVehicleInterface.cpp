#include "EditorVehicleInterface.h"
#include "EditorScene.h"
#include <util/InputUtil.h>

void EditorVehicleInterface::update(double dt)
{
	for(auto& mp : edveh->piece_meta)
	{
		mp.second.highlight = glm::vec3(0.0f);
		mp.second.draw_out_attachments = false;
		mp.second.draw_in_attachments = true;
		mp.second.attachment_color.clear();
	}
}

void EditorVehicleInterface::use_attachment_port(Piece* target, std::string port)
{
	bool found = false;
	for(auto& pair : selected->attachments)
	{
		if(pair.first.marker == selected_attachment)
		{
			logger->check(pair.second == false, "Attachment port is already in use");
			pair.second = true;
			found = true;
			break;
		}
	}

	logger->check(found, "Could not find attachment port used in selected piece");

}

void EditorVehicleInterface::attach(Piece* target, std::string port)
{
	selected->attached_to = target;
	

	use_attachment_port(selected, selected_attachment);

	if(port != "")
	{
		use_attachment_port(target, port);
	}

	selected = nullptr;
	selected_attachment = "";

}

void EditorVehicleInterface::handle_input_hovering(const CameraUniforms& cu, 
		glm::dvec3 ray_start, glm::dvec3 ray_end)
{
	RaycastResult rresult = raycast(ray_start, ray_end, false);

	Piece* hovered = nullptr;
	if(rresult.has_hit)
	{
		hovered = rresult.p;
	}

	if(hovered != nullptr)
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

void EditorVehicleInterface::handle_input_selected(const CameraUniforms& cu, 
		glm::dvec3 ray_start, glm::dvec3 ray_end)
{
	glm::dmat4 tform_0 = selected->get_graphics_matrix();

	glm::dmat4 tform_marker = glm::dmat4(1.0);

	if(selected_attachment != "none")
	{
		tform_marker = selected->get_marker_transform(selected_attachment);
	}

	glm::dvec3 npos = glm::normalize(ray_end - ray_start) * selected_distance + ray_start;
	glm::dmat4 tform_new = glm::translate(glm::dmat4(1.0), npos);
	tform_new = tform_new * glm::toMat4(selected_rotation) * glm::inverse(tform_marker);

	// Update all children
	std::vector<Piece*> children = edveh->veh->get_children_of(selected);
	children.push_back(selected);

	for(Piece* child : children)
	{
		glm::dmat4 relative = glm::inverse(tform_0) * child->get_graphics_matrix() * child->collider_offset;
		glm::dmat4 final = tform_new * relative; 
		child->packed_tform = to_btTransform(final);
	}
	
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


	Piece* attach_to = nullptr;
	std::string attach_to_marker = "";

	// Attempt attachment
	// First do stack (TODO)
	// Then do radial attachment
	if(allow_radial && attach_to == nullptr)
	{
		
		RaycastResult rresult = raycast(ray_start, ray_end, true, children);

		if(rresult.has_hit)
		{
			// Position the piece for radial attachment, adjusting distance and rotation
			selected_distance = glm::length(rresult.world_pos - ray_start);
			// Note that here we use +Z as up, not +Y!
			selected_rotation = MathUtil::quat_look_at(glm::vec3(0, 0, 0), rresult.world_nrm,
					glm::dvec3(0, 0, 1), glm::dvec3(0, 1, 0));
		
			attach_to = rresult.p;
			attach_to_marker = "";
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

			piece_meta.attachment_color[selected_attachment] = 
				glm::vec4(1.0f, 1.0f, 0.5f, 1.0f);
		}

	}
	
	if(input->mouse_down(GLFW_MOUSE_BUTTON_LEFT))
	{
		attach(attach_to, attach_to_marker);
	}

}


bool EditorVehicleInterface::handle_input(const CameraUniforms& cu, glm::dvec4 viewport, glm::dvec2 screen_size)
{
	// We are only called if input is free
	glm::dvec2 subscreen_size = screen_size * glm::dvec2(viewport.z - viewport.x, viewport.w - viewport.y);
	glm::dvec2 subscreen_pos = screen_size * glm::dvec2(viewport.x, viewport.y);
	// This goes from -1 to 1
	glm::dvec2 in_subscreen = (((input->mouse_pos - subscreen_pos) / subscreen_size) - 0.5) * 2.0; 
	in_subscreen.y = -in_subscreen.y;
	auto[ray_start, ray_end] = MathUtil::screen_raycast(in_subscreen, glm::inverse(cu.tform), 1000.0);

	edveh->draw_attachments = selected != nullptr;

	if(selected == nullptr)
	{
		handle_input_hovering(cu, ray_start, ray_end);
	}
	else
	{
		handle_input_selected(cu, ray_start, ray_end);
	}

	return false;
}

void EditorVehicleInterface::on_selection_change(const CameraUniforms& cu)
{
	selected->attached_to = nullptr;
	edveh->veh->update_attachments();

	glm::dvec3 diff = cu.cam_pos - to_dvec3(selected->packed_tform.getOrigin()); 
	selected_distance = glm::length(diff);

	selected_buffer.clear();

	selected_attachment = "none";

	find_free_attachment();
}

std::string EditorVehicleInterface::find_free_attachment()
{
	// Find the first free attachment
	for(int i = 0; i < selected->attachments.size(); i++)
	{
		if(!selected->attachments[i].second)
		{
			logger->info("Found free: {}", selected->attachments[i].first.marker);
			selected_attachment = selected->attachments[i].first.marker;
			return selected->attachments[i].first.marker;
		}
	}

	return "";
}

void EditorVehicleInterface::cycle_attachments(int dir)
{
	int cur_index = -1;
	for(int i = 0; i < selected->attachments.size(); i++)
	{
		if(selected->attachments[i].first.marker == selected_attachment)
		{
			cur_index = i;
		}
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
		
		if(!selected->attachments[cur_index].second)
		{
			selected_attachment = selected->attachments[cur_index].first.marker;
			found = true;
		}

	}
}

void EditorVehicleInterface::cycle_pieces(int dir)
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
				std::string n_attachment = find_free_attachment();
				if(n_attachment != "")
				{
					// Found it
					selected_attachment = n_attachment;
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
				std::string n_attachment = find_free_attachment();
				if(n_attachment != "")
				{
					// We found a valid one! We are done
					selected_attachment = n_attachment;
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

EditorVehicleInterface::RaycastResult EditorVehicleInterface::raycast(
	glm::dvec3 ray_start, glm::dvec3 ray_end, bool ignore_non_radial,
	std::vector<Piece*> ignore)
{
	RaycastResult res;
	res.has_hit = false;
	res.p = nullptr;

	btCollisionWorld::AllHitsRayResultCallback callback(to_btVector3(ray_start), to_btVector3(ray_end));
	// We cast the ray using bullet
	scene->bt_world->rayTest(to_btVector3(ray_start), to_btVector3(ray_end), callback);

	if(callback.hasHit())
	{
		// We could choose any member array, we just need the length
		size_t hit_count = callback.m_hitPointWorld.size();

		double closest_dist = 99999999.0;
		int closest_index = -1;

		// Find the closest one that isn't a selected part
		for(size_t i = 0; i < hit_count; i++)
		{
			double dist = glm::length(to_dvec3(callback.m_hitPointWorld[i]) - ray_start);
			RigidBodyUserData* udata = (RigidBodyUserData*)callback.m_collisionObjects[i]->getUserPointer();

			logger->check(udata != nullptr, "A rigidbody did not have an user data attached");

			
			if(udata->type == RigidBodyType::PIECE)
			{
				bool is_selected = false;
				// We can't simply check against selected, we need to check
				// all children
				for(Piece* ign : ignore)
				{
					if(ign == udata->as_piece)
					{
						is_selected = true;
						break;
					}
				}

				if(dist <= closest_dist && !is_selected && 
					(udata->as_piece->piece_prototype->allows_radial || !ignore_non_radial))
				{
					closest_index = int(i);
					closest_dist = dist;

					res.has_hit = true;
					res.p = udata->as_piece;
					res.world_pos = to_dvec3(callback.m_hitPointWorld[i]);
					res.world_nrm = to_dvec3(callback.m_hitNormalWorld[i]);
				}
			}
		}

	}

	return res;
}

void EditorVehicleInterface::reroot(Piece* current, Piece* new_root)
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

			(*it)->link_from = (*next)->link_to;
			(*it)->link_to = (*next)->link_from;
			// TODO: Invert this too
			(*it)->link_rot = (*next)->link_rot;


		}
	}	

}

EditorVehicleInterface::EditorVehicleInterface()
{
	allow_radial = true;
	allow_stack = true;
	selected = nullptr;
}

