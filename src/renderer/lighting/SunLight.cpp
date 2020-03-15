#include "SunLight.h"
#include "../../assets/AssetManager.h"
#include "../../util/render/TextureDrawer.h"

SunLight::SunLight()
{
	shader = assets->get<Shader>("core", "shaders/light/sunlight.vs");
}


SunLight::~SunLight()
{
}

void SunLight::do_pass(glm::dvec3 cam_pos, GBuffer* gbuf)
{
	prepare_shader(shader, gbuf);

	// Sun pso relative to the camera
	glm::dvec3 sun_pos = glm::dvec3(0, 0, 0) - cam_pos;
	shader->setVec3("sun_pos", sun_pos);

	texture_drawer->issue_fullscreen_rectangle();
}

