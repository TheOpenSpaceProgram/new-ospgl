#include "EnvMap.h"
#include "../../assets/AssetManager.h"
#include <renderer/util/TextureDrawer.h>

EnvMap::EnvMap()
{
	shader = osp->assets->get<Shader>("core", "shaders/light/env_map.vs");

}

void EnvMap::do_pass(CameraUniforms &cu, GBuffer *gbuf)
{
	prepare_shader(shader, gbuf, cu.irradiance, cu.specular, cu.brdf);

	glm::dmat4 pos_mat = glm::dmat4(1.0);
	texture_drawer->issue_fullscreen_rectangle();

}
