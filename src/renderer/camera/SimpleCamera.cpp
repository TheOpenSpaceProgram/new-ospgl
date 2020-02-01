#include "SimpleCamera.h"
#include "../../util/InputUtil.h"
#include <imgui/imgui.h>

void SimpleCamera::update(double dt)
{
	if (!ImGui::IsAnyItemFocused() && !ImGui::IsAnyItemActive() && !ImGui::IsAnyWindowFocused())
	{
		bool moved = false;

		// Motion
		if (glfwGetKey(input->window, GLFW_KEY_W) == GLFW_PRESS)
		{
			forwards(dt);
			moved = true;
		}

		if (glfwGetKey(input->window, GLFW_KEY_S) == GLFW_PRESS)
		{
			backwards(dt);
			moved = true;
		}

		if (glfwGetKey(input->window, GLFW_KEY_A) == GLFW_PRESS)
		{
			leftwards(dt);
			moved = true;
		}

		if (glfwGetKey(input->window, GLFW_KEY_D) == GLFW_PRESS)
		{
			rightwards(dt);
			moved = true;
		}

		if (glfwGetKey(input->window, GLFW_KEY_R) == GLFW_PRESS)
		{
			upwards(dt);
			moved = true;
		}

		if (glfwGetKey(input->window, GLFW_KEY_F) == GLFW_PRESS)
		{
			downwards(dt);
			moved = true;
		}

		if (glfwGetKey(input->window, GLFW_KEY_Q) == GLFW_PRESS)
		{
			tilt(dt, -1.0f);
		}

		if (glfwGetKey(input->window, GLFW_KEY_E) == GLFW_PRESS)
		{
			tilt(dt, 1.0f);
		}


		if (glfwGetMouseButton(input->window, GLFW_MOUSE_BUTTON_2))
		{
			glfwSetInputMode(input->window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
			if (input->mouse_delta != glm::dvec2(0.0, 0.0))
			{
				mouse(input->mouse_delta, dt);
			}

			if (input->mouse_scroll_delta != 0)
			{
				speed += speed * input->mouse_scroll_delta * 0.05;
			}
		}
		else
		{
			glfwSetInputMode(input->window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
		}

	}
	else
	{
		glfwSetInputMode(input->window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
	}
}

std::pair<glm::dvec3, glm::dvec3> SimpleCamera::get_camera_pos_dir()
{
	return std::make_pair(pos + center, fw);
}

CameraUniforms SimpleCamera::get_camera_uniforms(int w, int h)
{
	CameraUniforms out;

	auto[camera_pos, camera_dir] = get_camera_pos_dir();
	// ~1 light year
	float far_plane = 1e16f;


	glm::dmat4 proj = glm::perspective(glm::radians(fov), (double)w / (double)h, NEAR_PLANE, (double)far_plane);
	glm::dmat4 view = glm::lookAt(glm::dvec3(0.0, 0.0, 0.0), camera_dir, up);
	glm::dmat4 proj_view = proj * view;

	out.proj = proj;
	out.view = view;
	out.proj_view = proj_view;
	out.c_model = glm::translate(glm::dmat4(1.0), -camera_pos);
	out.tform = proj * view * out.c_model;
	out.far_plane = far_plane;
	out.cam_pos = camera_pos;

	return out;
}


glm::dmat4 SimpleCamera::get_proj_view(int width, int height)
{
	auto[camera_pos, camera_dir] = get_camera_pos_dir();
	// ~1 light year
	float far_plane = 1e16f;


	glm::dmat4 proj = glm::perspective(glm::radians(fov), (double)width / (double)height, NEAR_PLANE, (double)far_plane);
	glm::dmat4 view = glm::lookAt(glm::dvec3(0.0, 0.0, 0.0), camera_dir, up);
	glm::dmat4 proj_view = proj * view;

	return proj_view;
}

glm::dmat4 SimpleCamera::get_cmodel()
{
	return glm::translate(glm::dmat4(1.0), -get_camera_pos_dir().first);
}

SimpleCamera::SimpleCamera()
{
	fov = 60.0;
	pos = glm::dvec3(0.0, 0.0, 0.0);
	fw = glm::dvec3(1.0, 0.0, 0.0);
	speed = 1.0;
	up = glm::dvec3(0.0, 1.0, 0.0);
}

