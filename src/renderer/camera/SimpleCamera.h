#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <tuple>
#include "CameraUniforms.h"
#include "Camera.h"

class SimpleCamera : public Camera
{
private:
	glm::dmat4 get_proj_view(int w, int h);
	glm::dmat4 get_cmodel();

public:

	constexpr static double NEAR_PLANE = 1e-6;

	bool keyboard_blocked;
	bool mouse_blocked;

	// In degrees
	double fov;

	double speed;


	glm::dvec3 pos;
	glm::dvec3 up;
	glm::dvec3 fw;

	glm::dvec3 center;

	void update(double dt);

	// Relative to system, not to center planet
	std::pair<glm::dvec3, glm::dvec3> get_camera_pos_dir();

	CameraUniforms get_camera_uniforms(int w, int h) override;

	void forwards(double dt)
	{
		pos += speed * glm::normalize(fw) * (double)dt;
	}

	void backwards(double dt)
	{
		pos -= speed * glm::normalize(fw) * (double)dt;
	}

	void leftwards(double dt)
	{
		pos -= speed * glm::normalize(glm::cross(fw, up)) * (double)dt;
	}

	void rightwards(double dt)
	{
		pos += speed * glm::normalize(glm::cross(fw, up)) * (double)dt;
	}

	void tilt(double dt, double dir)
	{
		glm::mat4 f = glm::rotate(glm::dmat4(1.0), (double)dir * (double)dt, fw);
		up = f * glm::dvec4(up, 1.0);
	}

	void upwards(double dt)
	{
		pos += speed * glm::normalize(up) * (double)dt;
	}

	void downwards(double dt)
	{
		pos -= speed * glm::normalize(up) * (double)dt;
	}

	void mouse(glm::dvec2 deltas, double dt)
	{
		glm::dvec3 or_forward = fw;
		glm::dvec3 or_up = up;

		// Rotate forward original up
		glm::mat4 hor = glm::rotate(glm::dmat4(1.0), -deltas.x * (double)dt * 0.45, or_up);

		glm::dvec3 right = glm::cross(or_forward, or_up);

		glm::mat4 vert = glm::rotate(glm::dmat4(1.0), -deltas.y * (double)dt * 0.45, right);
		up = vert * glm::dvec4(or_up, 1.0);
		fw = vert * hor * glm::dvec4(fw, 1.0);

	}

	SimpleCamera();

};
