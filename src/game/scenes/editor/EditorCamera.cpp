#include "EditorCamera.h"
#include <util/InputUtil.h>
#include <util/MathUtil.h>
#include <util/Logger.h>
#include <util/fmt/glm.h>

CameraUniforms EditorCamera::get_camera_uniforms(int w, int h)
{
	CameraUniforms out;

	// We don't need much more
	float far_plane = 1000.0f;

	glm::dvec3 camera_pos = center + direction * radius;
	glm::dvec3 camera_dir = -direction;

	glm::dmat4 proj = glm::perspective(glm::radians(fov), (double)w / (double)h, 0.001, (double)far_plane);
	glm::dmat4 view = glm::lookAt(glm::dvec3(0.0, 0.0, 0.0), camera_dir, up);
	glm::dmat4 proj_view = proj * view;

	out.proj = proj;
	out.view = view;
	out.proj_view = proj_view;
	out.c_model = glm::translate(glm::dmat4(1.0), -camera_pos);
	out.tform = proj * view * out.c_model;
	out.far_plane = far_plane;
	out.cam_pos = camera_pos;

	out.screen_size = glm::vec2((float)w, (float)h);
	out.iscreen_size = glm::ivec2(w, h);


	return out;
}



void EditorCamera::update(double dt, GUIInput* gui_input)
{

	if(gui_input->mouse_down(GUI_RIGHT_BUTTON) && !gui_input->mouse_blocked)
	{
		blocked = true;
	}
	
	if(gui_input->mouse_up(GUI_RIGHT_BUTTON))
	{
		blocked = false;
	}
	
	if(blocked)
	{
		bool pan;
		bool alt_pan;
		
		pan = glfwGetKey(input->window, pan_key) == GLFW_PRESS;
		alt_pan = glfwGetKey(input->window, altpan_key) == GLFW_PRESS;

		if(pan || alt_pan)
		{
			// Panning
			glm::dvec3 pan_axis_h, pan_axis_v;
			if(pan && !alt_pan)
			{
				glm::dvec3 side = glm::cross(up, direction);
				// Along camera plane (this works unless direction is near up)
				pan_axis_v = glm::normalize(glm::cross(side, direction));
				pan_axis_h = glm::normalize(glm::cross(pan_axis_v, direction));
			}

			if(alt_pan && !pan)
			{
				// Along O-Z-X plane
				pan_axis_h = glm::dvec3(-1.0, 0.0, 0.0);
				pan_axis_v = glm::dvec3(0.0, 0.0, -1.0);
			}

			if(alt_pan && pan)
			{
				// Along O-Z-Y plane
				pan_axis_h = glm::dvec3(0.0, -1.0, 0.0);
				pan_axis_v = glm::dvec3(0.0, 0.0, -1.0);
			}

			center += pan_axis_h * dt * input->mouse_delta.x * pan_vel;
			center -= pan_axis_v * dt * input->mouse_delta.y * pan_vel;
		}		
		else
		{
			// Rotating
			// Make y axis = z axis for correct calculation
			std::swap(direction.y, direction.z);
			glm::dvec2 circular = MathUtil::euclidean_to_spherical_r1(direction);
			circular.x -= input->mouse_delta.x * dt * rot_vel;
		  	circular.y -= input->mouse_delta.y * dt * rot_vel;

			double thresold = 0.02;
			double h = glm::half_pi<double>() * 2.0 * (1.0 - thresold);
			double l = thresold;

			if(circular.y >= h)
			{
				circular.y = h;
			}

			if(circular.y <= l)
			{
				circular.y = l;
			}

			direction = MathUtil::spherical_to_euclidean(circular.x, circular.y);	
			std::swap(direction.y, direction.z);
		}

	}

	bool changed_zoom = false;
	bool changed_updown = false;
	if(!gui_input->scroll_blocked || blocked)
	{
		// Scrolling
		if(input->mouse_scroll_delta != 0.0)
		{
			if(glfwGetKey(input->window, zoom_key))
			{
				radius_delta -= input->mouse_scroll_delta * zoom_vel;
				changed_zoom = true;
			}
			else
			{
				updown_delta += input->mouse_scroll_delta * updown_vel;	
				changed_updown = true;
			}

		}
	}

	radius += radius_delta * dt;
	if(!changed_zoom)
	{
		double sign = glm::sign(radius_delta);
		radius_delta += dt * zoom_attn * -sign * glm::max(glm::abs(radius_delta), 1.0);
		if(sign * radius_delta <= 0.0)
		{
			radius_delta = 0.0;
		}
	}

	center.z += updown_delta * dt;
	if(!changed_updown)
	{
		double sign = glm::sign(updown_delta);
		updown_delta += dt * updown_attn * -sign * glm::max(glm::abs(updown_delta), 1.0);
		if(sign * updown_delta <= 0.0)
		{
			updown_delta = 0.0;
		}

	}

}

EditorCamera::EditorCamera()
{
	center = glm::dvec3(0.0, 0.0, 0.0);
	direction = glm::dvec3(1.0, 0.0, 0.0);
	radius = 5.0;
	fov = 70.0;
	blocked = false;
	main_button = GLFW_MOUSE_BUTTON_RIGHT;
	pan_key = GLFW_KEY_LEFT_SHIFT;
	altpan_key = GLFW_KEY_LEFT_CONTROL;
	zoom_key = GLFW_KEY_LEFT_SHIFT;
}
