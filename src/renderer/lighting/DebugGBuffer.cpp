#include "DebugGBuffer.h"
#include "../../assets/AssetManager.h"
#include <renderer/util/TextureDrawer.h>

DebugGBuffer::DebugGBuffer()
{
	shader = nullptr;
	mode = NONE;

}

void DebugGBuffer::do_pass(CameraUniforms &cu, GBuffer *gbuf)
{
	if(shader == nullptr)
	{
		shader = osp->assets->get<Shader>("core", "shaders/light/debug_gbuffer.vs");
	}

	prepare_shader(shader, gbuf, cu.irradiance, cu.specular, cu.brdf);
	shader->setInt("mode", mode);
	texture_drawer->issue_fullscreen_rectangle();

}

const char *DebugGBuffer::get_mode_str(DebugGBuffer::View mode)
{
	if(mode == NONE)
		return "None";
	else if(mode == POS)
		return "Position";
	else if(mode == EMIT)
		return "Emission (intensity)";
	else if(mode == EMIT_ALBEDO)
		return "Emission (with color)";
	else if(mode == NORMAL)
		return "Normals";
	else if(mode == ALBEDO)
		return "Albedo";
	else if(mode == OCCLUSSION)
		return "Occlussion";
	else if(mode == METALLIC)
		return "Metallic";
	else if(mode == ROUGHNESS)
		return "Roughness";

	return "INVALID";
}

