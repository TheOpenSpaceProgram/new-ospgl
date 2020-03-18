#pragma once
#include "Light.h"

// Similar to sunlight, but it has a limited range and 
// is located somewhere on the world
class PointLight : public Light
{
private:

	Shader* fullscreen;
	Shader* sphere;

	// Parameters for the light
	// Radius is approximated with min_brightness when setting parameters
	float constant, linear, quadratic, radius;

	float calculate_radius(float min_brightness);

public:
	glm::dvec3 pos;
	glm::vec3 color;
	glm::vec3 spec_color;

	void set_parameters(float constant, float linear, float quadratic, float min_brightness = 0.5f / 256.0f);


	float get_constant();
	float get_linear();
	float get_quadratic();
	float get_radius();

	virtual void do_pass(CameraUniforms& cu, GBuffer * gbuf) override;

	void set_shader_uniforms(Shader* sh, GBuffer* gbuffer, glm::dvec3 rel_pos);

	PointLight();
	~PointLight();
	
};

