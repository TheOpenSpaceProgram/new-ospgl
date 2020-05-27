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


	Piece* hovered;

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
			}
		}

		if(hovered != nullptr)
		{
			if(glfwGetMouseButton(input->window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS)
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
			//tform_marker = selected->get_marker_transform(selected_attachment);
			tform_marker = glm::translate(glm::dmat4(1.0), selected->get_marker_position(selected_attachment));
		}

		glm::dvec3 npos = glm::normalize(ray_end - ray_start) * selected_distance + ray_start;
		glm::dmat4 tform_new = glm::translate(glm::dmat4(1.0), npos);
		tform_new = glm::inverse(tform_marker) * tform_new;

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

		// Coloring and visuals
		for(Piece* child : children)
		{
			edveh->piece_meta[child].draw_out_attachments = true;
			edveh->piece_meta[child].draw_in_attachments = false;
			edveh->piece_meta[child].highlight = glm::vec3(0.5f);
			if(child == selected)
			{
				edveh->piece_meta[child].highlight = glm::vec3(1.0f);

				edveh->piece_meta[child].attachment_color[selected_attachment] = 
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

void EditorVehicleInterface::find_free_attachment()
{
	// Find the first free attachment
	for(int i = 0; i < selected->attachments.size(); i++)
	{
		if(!selected->attachments[i].second)
		{
			logger->info("Found free: {}", selected->attachments[i].first.marker);
			selected_attachment = selected->attachments[i].first.marker;
			break;
		}
	}
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

	if(dir == -1)
	{
		// Go back in stack or otherwise go into the last piece of children
		if(selected_buffer.size() != 0)	
		{
			selected = selected_buffer.front();
		}			
		else 
		{
			if(children.size() != 0)
			{
				selected = children[children.size() - 1];
			}
			// Otherwise we can't move anywhere
		}
	}
	else if(dir == 1)
	{
		// Go into earliest children, for more complex selections a visual
		// interace is used (stuff with two or more children)
		if(children.size() != 0)
		{
			selected = children[0];	
		}	
	}

	if(old != selected)
	{
		reroot(old, selected);
		// We will need to find a new attachment most likely
		find_free_attachment();
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
	selected = nullptr;
}

