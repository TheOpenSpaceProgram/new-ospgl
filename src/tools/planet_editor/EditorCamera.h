#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform.hpp>

class EditorCamera 
{
public:

	glm::dvec3 pos;
	glm::dvec3 up;
	glm::dvec3 forward;

	double speed;

	EditorCamera()
	{
		reset(1.0);
		speed = 1.0;
	}

	void reset(double radius)
	{
		pos = glm::dvec3(radius * 2.0, 0.0, 0.0);
		up = glm::dvec3(0.0, 1.0, 0.0);
		forward = glm::dvec3(-1.0, 0.0, 0.0);

	}

	void forwards(float dt)
	{
		pos += speed * glm::normalize(forward) * (double)dt;
	}

	void backwards(float dt)
	{
		pos -= speed * glm::normalize(forward) * (double)dt;
	}

	void leftwards(float dt)
	{
		pos -= speed * glm::normalize(glm::cross(forward, up)) * (double)dt;
	}

	void rightwards(float dt)
	{
		pos += speed * glm::normalize(glm::cross(forward, up)) * (double)dt;
	}

	void tilt(float dt, float dir)
	{
		glm::mat4 f = glm::rotate(glm::dmat4(1.0), (double)dir * (double)dt, forward);
		up = f * glm::dvec4(up, 1.0);
	}

	void mouse(glm::dvec2 deltas, float dt)
	{
		glm::dvec3 or_forward = forward; 
		glm::dvec3 or_up = up;

		// Rotate forward original up
		glm::mat4 hor = glm::rotate(glm::dmat4(1.0), -deltas.x * (double)dt * 0.45, or_up);

		glm::dvec3 right = glm::cross(or_forward, or_up);

		glm::mat4 vert = glm::rotate(glm::dmat4(1.0), -deltas.y * (double)dt * 0.45, right);
		up = vert * glm::dvec4(or_up, 1.0);
		forward = vert * hor * glm::dvec4(forward, 1.0);
		
	}
};