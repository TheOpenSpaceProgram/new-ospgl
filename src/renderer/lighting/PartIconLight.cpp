#include "PartIconLight.h"
#include <assets/AssetManager.h>
#include <renderer/util/TextureDrawer.h>

PartIconLight::PartIconLight()
{
	color = glm::vec3(1.0, 1.0, 1.0);
	spec_color = glm::vec3(1.0, 1.0, 1.0);
	ambient_color = glm::vec3(0.3, 0.3, 0.3);
	sun_dir = -glm::normalize(glm::vec3(1.0, 1.0, 0.0));

	shader = assets->get<Shader>("core", "shaders/light/part_icon.vs");
}


void PartIconLight::do_pass(CameraUniforms& cu, GBuffer* gbuf)
{

	prepare_shader(shader, gbuf);

	// Sun pso relative to the camera
	glm::dvec3 sun_pos = glm::dvec3(0, 0, 0) - cu.cam_pos;
	shader->setVec3("sun_dir", sun_dir);
	shader->setVec3("color", color);
	shader->setVec3("spec_color", spec_color);
	shader->setVec3("ambient_color", ambient_color);

	texture_drawer->issue_fullscreen_rectangle();

}

