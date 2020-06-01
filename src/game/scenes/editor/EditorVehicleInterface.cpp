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

bool EditorVehicleInterface::handle_input(const CameraUniforms& cu, glm::dvec4 viewport, glm::dvec2 screen_size)
{
	// We are only called if input is free
	
	glm::dvec2 subscreen_size = screen_size * glm::dvec2(viewport.z - viewport.x, viewport.w - viewport.y);
	glm::dvec2 subscreen_pos = screen_size * glm::dvec2(viewport.x, viewport.y);
	// This goes from -1 to 1
	glm::dvec2 in_subscreen = (((input->mouse_pos - subscreen_pos) / subscreen_size) - 0.5) * 2.0; 
	in_subscreen.y = -in_subscreen.y;

	auto[ray_start, ray_end] = MathUtil::screen_raycast(in_subscreen, glm::inverse(cu.tform), 1000.0);


	Piece* hovered = nullptr;

	edveh->draw_attachments = selected != nullptr;

	if(selected == nullptr)
	{
		btCollisionWorld::ClosestRayResultCallback callback(to_btVector3(ray_start), to_btVector3(ray_end));
		// We cast the ray using bullet
		scene->bt_world->rayTest(to_btVector3(ray_start), to_btVector3(ray_end), callback);

		if(callback.hasHit())
		{
			RigidBodyUserData* udata = (RigidBodyUserData*)callback.m_collisionObject->getUserPointer();
			logger->check(udata != nullptr, "A rigidbody did not have an user data attached");

			// We only care about PIECE colliders, so ignore everything else 
			if(udata->type == RigidBodyType::PIECE)
			{
				hovered = udata->as_piece;

				glm::dvec3 hit_point = to_dvec3(callback.m_hitPointWorld);
				glm::dvec3 hit_normal = to_dvec3(callback.m_hitNormalWorld);

				debug_drawer->add_line(hit_point, hit_point + hit_normal * 10.0, glm::vec3(1.0, 1.0, 0.0));
			}
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
	else
	{
		glm::dmat4 tform_0 = selected->get_graphics_matrix();

		glm::dmat4 tform_marker = glm::dmat4(1.0);

		if(selected_attachment != "none")
		{
			tform_marker = selected->get_marker_transform(selected_attachment);
			
			//tform_marker = glm::translate(glm::dmat4(1.0), selected->get_marker_position(selected_attachment));
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


		// Attempt attachment
		// First do stack (TODO)
		// Then do radial attachment
		if(allow_radial)
		{
			
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
						for(Piece* child : children)
						{
							if(child == udata->as_piece)
							{
								is_selected = true;
								break;
							}
						}

						if(dist <= closest_dist && !is_selected && 
							udata->as_piece->piece_prototype->allows_radial)
						{
							closest_index = int(i);
							closest_dist = dist;
						}
					}
				}

				if(closest_index >= 0)
				{
					// Position the piece for radial attachment, adjusting distance and rotation
					glm::dvec3 hit_point = to_dvec3(callback.m_hitPointWorld[closest_index]);
					glm::dvec3 hit_normal = to_dvec3(callback.m_hitNormalWorld[closest_index]);
					selected_distance = glm::length(hit_point - ray_start);

					selected_rotation = MathUtil::quat_look_at(glm::vec3(0, 0, 0), hit_normal);
				
					debug_drawer->add_line(hit_point, hit_point + hit_normal * 10.0, glm::vec3(1.0, 0.0, 1.0));

					
					if(input->mouse_down(GLFW_MOUSE_BUTTON_LEFT))
					{
						RigidBodyUserData* udata = 
							(RigidBodyUserData*)callback.m_collisionObjects[closest_index]->getUserPointer();

						selected->attached_to = udata->as_piece;
						selected = nullptr;
					}
				}
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
		// Width first search for a valid children (TODO)
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

