#include "Skybox.h"
#include <renderer/geometry/CubeGeometry.h>

Skybox::Skybox(AssetHandle<Cubemap> &&ncubemap) : vao(0), vbo(0), intensity(1.0f)
{
	shader = AssetHandle<Shader>("core:shaders/skybox.vs");
	cubemap = std::move(ncubemap);
	CubeGeometry::generate_cubemap(&vao, &vbo);

	cubemap->generate_ibl_irradiance(32);

}

void Skybox::forward_pass(CameraUniforms &cu, bool is_env_map)
{
	shader->use();
	shader->setInt("skybox", 0);
	shader->setMat4("tform", cu.proj * cu.view);
	shader->setFloat("intensity", intensity);
	glDepthFunc(GL_LEQUAL);
	glBindVertexArray(vao);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_CUBE_MAP, cubemap->id);
	glDrawArrays(GL_TRIANGLES, 0, 36);
	glBindVertexArray(0);
	glDepthFunc(GL_LESS);
}

Skybox::~Skybox()
{
	glDeleteVertexArrays(1, &vao);
	glDeleteBuffers(1, &vbo);
}

