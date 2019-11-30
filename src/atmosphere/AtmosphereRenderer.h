#pragma once
#include "../assets/Shader.h"
#include "../assets/AssetManager.h"
#include "../universe/element/body/config/PlanetConfig.h"
#include "../util/geometry/SphereGeometry.h"

class AtmosphereRenderer
{
private:

	GLuint fullscreen_vao, fullscreen_vbo;
	GLuint atmo_vao, atmo_vbo, atmo_ebo;
	size_t index_count;

public:

	Shader* atmo;

	// Atmospheres are rendered in z-sorted passes
	void do_pass(glm::dmat4 proj_view, glm::dmat4 model, float far_plane, glm::vec3 cam_pos_relative, 
		PlanetConfig& config,	glm::vec3 light_dir);

	AtmosphereRenderer();
	~AtmosphereRenderer();
};

