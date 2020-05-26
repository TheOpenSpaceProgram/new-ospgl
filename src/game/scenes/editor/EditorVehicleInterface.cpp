#include "EditorVehicleInterface.h"
#include "EditorScene.h"
#include <util/InputUtil.h>

void EditorVehicleInterface::update(double dt)
{
	for(auto& mp : edveh->piece_meta)
	{
		mp.second.highlight = false;
		mp.second.draw_all_attachments = false;
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
				mp.second.highlight = true;
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
			cycle_attachments(-1);
		}

		if(input->key_down(GLFW_KEY_PAGE_DOWN))
		{
			cycle_attachments(1);
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

	selected_attachment = "none";

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

EditorVehicleInterface::EditorVehicleInterface()
{
	selected = nullptr;
}

