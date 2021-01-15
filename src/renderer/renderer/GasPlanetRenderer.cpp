#include "GasPlanetRenderer.h"
#include <renderer/geometry/SphereGeometry.h>

void GasPlanetRenderer::render(glm::dmat4 proj_view, glm::dmat4 model, glm::dmat4 rotation_matrix,
							   glm::dmat4 normal_matrix, float far_plane, glm::dvec3 camera_pos,
							   const ElementConfig &config, double time)
{
	// TODO: Draw stuff, we simply draw a sphere as this is used only for the sun as of now

	shader->use();
	shader->setMat4("tform", proj_view * model);
	shader->setFloat("f_coef", 2.0f / glm::log2(far_plane + 1.0f));


	glBindVertexArray(sph_vao);
	glDrawElements(GL_TRIANGLES, (GLsizei)sph_index_count, GL_UNSIGNED_INT, nullptr);
	glBindVertexArray(0);
}

GasPlanetRenderer::GasPlanetRenderer()
{
	SphereGeometry::generate_and_upload(&sph_vao, &sph_vbo, &sph_ebo, &sph_index_count);
	shader = AssetHandle<Shader>("core:shaders/simple.vs");
}
