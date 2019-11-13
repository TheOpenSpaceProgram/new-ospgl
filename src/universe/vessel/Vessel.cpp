#include "Vessel.h"
#include "../../util/DebugDrawer.h"
#include "../../util/InputUtil.h"
#include <GLFW/glfw3.h>

void Vessel::simulate(ElementVector elements, PosVector positions, double star_radius, size_t closest, double dt)
{
	glm::dvec3 up = glm::dvec3(1.0f, 0.0f, 0.0f);
	glm::dvec3 fw = glm::dvec3(0.0f, 1.0f, 0.0f);
	glm::dvec3 rt = glm::dvec3(0.0f, 0.0f, 1.0f);

	if (glfwGetKey(input->window, GLFW_KEY_A))
	{
		apply_torque(-up * dt * 0.05);
	}

	if (glfwGetKey(input->window, GLFW_KEY_D))
	{
		apply_torque(up * dt * 0.05);
	}

	if (glfwGetKey(input->window, GLFW_KEY_Q))
	{
		apply_torque(-fw * dt * 0.05);
	}

	if (glfwGetKey(input->window, GLFW_KEY_E))
	{
		apply_torque(fw * dt * 0.05);
	}

	if (glfwGetKey(input->window, GLFW_KEY_W))
	{
		apply_torque(-rt * dt * 0.05);
	}

	if (glfwGetKey(input->window, GLFW_KEY_S))
	{
		apply_torque(rt * dt * 0.05);
	}

	if (glfwGetKey(input->window, GLFW_KEY_SPACE))
	{
		state.vel += get_forward() * dt * 100.0;
	}

	double l = glm::length(angular_momentum);
	if (angular_momentum != glm::dvec3(0.0, 0.0, 0.0))
	{
		rotation *= glm::angleAxis(l * dt, angular_momentum / l);
	}
} 

glm::dvec3 Vessel::get_forward()
{
	return rotation * glm::dvec4(0.0, 1.0, 0.0, 1.0);
}

glm::dvec3 Vessel::get_right()
{
	return rotation * glm::dvec4(1.0, 0.0, 0.0, 1.0);
}

glm::dvec3 Vessel::get_up()
{
	return rotation * glm::dvec4(1.0, 0.0, 0.0, 1.0);
}

void Vessel::apply_torque(glm::dvec3 torque)
{
	angular_momentum += torque;

	if (debug_drawer->debug_enabled)
	{
		debug_drawer->add_arrow(state.pos, state.pos + glm::normalize(torque) * 5.0, glm::dvec3(0.8, 0.8, 1.0));
	}
}

void Vessel::draw_debug()
{
	if (!debug_drawer->debug_enabled)
	{
		return;
	}

	double scale = 10.0;

	debug_drawer->add_point(state.pos, glm::vec3(1.0, 0.0, 1.0));
	debug_drawer->add_transform(state.pos, glm::toMat4(rotation), scale);
	debug_drawer->add_arrow(state.pos, state.pos + angular_momentum * scale, glm::vec3(0.3, 0.3, 1.0));
}

Vessel::Vessel()
{
	angular_momentum = glm::dvec3(0.0, 0.0, 0.0);
}


Vessel::~Vessel()
{
}
