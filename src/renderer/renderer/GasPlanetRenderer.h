#pragma once
#include <universe/element/config/ElementConfig.h>
#include <assets/Shader.h>

class GasPlanetRenderer
{
public:

	AssetHandle<Shader> shader;

	GLuint sph_vao, sph_vbo, sph_ebo;
	size_t sph_index_count;

	void render(glm::dmat4 proj_view, glm::dmat4 model, glm::dmat4 rotation_matrix,
			 glm::dmat4 normal_matrix, float far_plane, glm::dvec3 camera_pos,
			 const ElementConfig& config, double time);

	GasPlanetRenderer();

};


