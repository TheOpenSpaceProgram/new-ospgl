#pragma once
#include "../assets/Shader.h"
#include "../assets/AssetManager.h"
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
	void do_pass(glm::dmat4 proj_view, glm::dmat4 model, float far_plane, 
		float planet_radius_relative, glm::vec3 cam_pos_relative, 
		glm::vec3 main_color, glm::vec3 sunset_color, float exponent, float sunset_exponent);

	AtmosphereRenderer();
	~AtmosphereRenderer();
};

