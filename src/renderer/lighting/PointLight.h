#pragma once
#include "Light.h"

// Similar to sunlight, but it has a limited range and 
// is located somewhere on the world
class PointLight : public Light
{
private:

	Shader* shader;

public:
	glm::dvec3 pos;
	glm::vec3 color;
	glm::vec3 spec_color;

	// Parameters for the light
	// TODO: Provide some helper functions to set these
	float constant, linear, quadratic;

	virtual void do_pass(glm::dvec3 cam_pos, GBuffer * gbuf) override;

	PointLight();
	~PointLight();
	
};

