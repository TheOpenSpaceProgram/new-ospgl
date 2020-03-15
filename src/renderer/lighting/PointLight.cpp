#include "PointLight.h"
#include "../../assets/AssetManager.h"
#include "../../util/render/TextureDrawer.h"


PointLight::PointLight()
{
	shader = assets->get<Shader>("core", "shaders/light/point.vs");
	pos = glm::dvec3(0, 0, 0);
	color = glm::vec3(1.0f, 0.9f, 0.9f);
	spec_color = glm::vec3(1.0f, 0.8f, 0.8f);

	// Setup a quick default light with a range of
	// ~100 units
	constant = 1.0f;
	linear = 0.045f;
	quadratic = 0.0075f;
}


PointLight::~PointLight()
{
}

void PointLight::do_pass(glm::dvec3 cam_pos, GBuffer* gbuf)
{
	prepare_shader(shader, gbuf);

	// Sun poo relative to the camera
	glm::dvec3 rel_pos = pos - cam_pos;
	shader->setVec3("light_pos", rel_pos);
	shader->setVec3("color", color);
	shader->setVec3("spec_color", spec_color);
	shader->setFloat("p_constant", constant);
	shader->setFloat("p_linear", linear);
	shader->setFloat("p_quadratic", quadratic);

	// TODO: Use a sphere
	texture_drawer->issue_fullscreen_rectangle();
}
