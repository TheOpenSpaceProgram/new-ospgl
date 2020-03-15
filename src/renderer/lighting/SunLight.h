#pragma once
#include "Light.h"
// Simplest thing, always at (0, 0, 0). It's actually a point-light
// but renders a fullscreen quad. It also allows setting ambient light
class SunLight : public Light
{
private:

	Shader* shader;

public:

	glm::vec3 color;
	glm::vec3 spec_color;
	glm::vec3 ambient_color;


	SunLight();
	~SunLight();

	// Inherited via Light
	virtual void do_pass(glm::dvec3 cam_pos, GBuffer * gbuf) override;
};

