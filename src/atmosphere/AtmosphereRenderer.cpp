#include "AtmosphereRenderer.h"



void AtmosphereRenderer::do_pass(glm::dmat4 proj_view, glm::dmat4 model, float far_plane, 
	float planet_radius_relative, glm::vec3 cam_pos_relative)
{
	
		float l = glm::length(cam_pos_relative);
		if (l <= 1.05f)
		{
			glCullFace(GL_FRONT);
		}
		else
		{
			glCullFace(GL_BACK);
		}

		glm::dmat4 extra_model = glm::dmat4(1.0);

		if (l > 1.0 && l <= 1.05f)
		{
			extra_model = glm::scale(extra_model, glm::dvec3(1.05, 1.05, 1.05));
		} 

		glBlendFunc(GL_SRC_ALPHA, GL_ONE);
		glDepthMask(GL_FALSE);

		atmo->use();

		atmo->setMat4("tform", proj_view * extra_model * model);
		atmo->setFloat("f_coef", 2.0f / glm::log2(far_plane + 1.0f));
		atmo->setVec3("camera_pos", cam_pos_relative);
		atmo->setFloat("planet_radius", planet_radius_relative);

		glBindVertexArray(atmo_vao);
		glDrawElements(GL_TRIANGLES, (GLsizei)index_count, GL_UNSIGNED_INT, 0);
		glBindVertexArray(0);

		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glDepthMask(GL_TRUE);
		glCullFace(GL_BACK);
	//}
	

	
}

AtmosphereRenderer::AtmosphereRenderer()
{
	float vertices[] = 
	{ 
		// positions   // texCoords
		-1.0f,  1.0f,  0.0f, 1.0f,
		-1.0f, -1.0f,  0.0f, 0.0f,
		 1.0f, -1.0f,  1.0f, 0.0f,

		-1.0f,  1.0f,  0.0f, 1.0f,
		 1.0f, -1.0f,  1.0f, 0.0f,
		 1.0f,  1.0f,  1.0f, 1.0f
	};

	glGenVertexArrays(1, &fullscreen_vao);
	glGenBuffers(1, &fullscreen_vbo);

	glBindVertexArray(fullscreen_vao);
	glBindBuffer(GL_ARRAY_BUFFER, fullscreen_vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));

	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	atmo = assets->get<Shader>("atmosphere/cheap");

	SphereGeometry::generate_and_upload(&atmo_vao, &atmo_vbo, &atmo_ebo, &index_count, 64);
}


AtmosphereRenderer::~AtmosphereRenderer()
{
}
