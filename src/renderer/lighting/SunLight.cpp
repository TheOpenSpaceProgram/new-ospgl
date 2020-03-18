#include "SunLight.h"
#include "../../assets/AssetManager.h"
#include "../../util/render/TextureDrawer.h"

SunLight::SunLight()
{
	shader = assets->get<Shader>("core", "shaders/light/sunlight.vs");
	color = glm::vec3(1.0f, 0.9f, 0.9f);
	spec_color = glm::vec3(1.0f, 0.8f, 0.8f);
	ambient_color = glm::vec3(0.0f, 0.0f, 0.0f);
}


SunLight::~SunLight()
{
}

void SunLight::do_pass(CameraUniforms& cu, GBuffer* gbuf)
{
	prepare_shader(shader, gbuf);

	// Sun pso relative to the camera
	glm::dvec3 sun_pos = glm::dvec3(0, 0, 0) - cu.cam_pos;
	shader->setVec3("sun_pos", sun_pos);
	shader->setVec3("color", color);
	shader->setVec3("spec_color", spec_color);
	shader->setVec3("ambient_color", ambient_color);

	texture_drawer->issue_fullscreen_rectangle();
}

