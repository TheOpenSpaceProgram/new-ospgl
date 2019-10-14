#include "WaterRenderer.h"
#include <array>


void WaterRenderer::render(glm::dmat4 proj_view, glm::dvec3 camera_pos, glm::dmat4 model, float far_plane)
{
	shader->use();
	shader->setFloat("f_coef", 2.0f / glm::log2(far_plane + 1.0f));
	shader->setVec3("camera_pos", camera_pos);
	// Draw six sides
	for (int i = 0; i < 6; i++)
	{

		glm::dmat4 submodel = glm::dmat4(1.0);

		if (i == PY)
		{
			submodel = glm::rotate(submodel, -glm::half_pi<double>(), glm::dvec3(1.0, 0.0, 0.0));
		}
		else if (i == PZ)
		{
			submodel = glm::rotate(submodel, glm::half_pi<double>(), glm::dvec3(0.0, 1.0, 0.0));
		}
		else if (i == NX)
		{
			submodel = glm::rotate(submodel, glm::pi<double>(), glm::dvec3(0.0, 1.0, 0.0));
		}
		else if (i == NY)
		{
			submodel = glm::rotate(submodel, glm::half_pi<double>(), glm::dvec3(1.0, 0.0, 0.0));
		}
		else if (i == NZ)
		{
			submodel = glm::rotate(submodel, -glm::half_pi<double>(), glm::dvec3(0.0, 1.0, 0.0));
		}

		glm::dmat4 model_final = model * submodel;
		glm::mat4 tform = glm::mat4(proj_view * model_final);

		shader->setMat4("tform", tform);
		shader->setMat4("model", submodel);

		glBindVertexArray(vao);
		glDrawElements(GL_TRIANGLES, (GLsizei)INDEX_COUNT, GL_UNSIGNED_SHORT, (void*)0);
	}
}

WaterRenderer::WaterRenderer()
{
	// Generate x-facing face
	std::array<WaterVertex, VERTEX_COUNT>* verts = new std::array<WaterVertex, VERTEX_COUNT>();
	std::array<uint16_t, INDEX_COUNT>* indices = new std::array<uint16_t, INDEX_COUNT>();


	// Generate indices
	for (size_t y = 0; y < VERTICES_PER_FACE - 1; y++)
	{
		for (size_t x = 0; x < VERTICES_PER_FACE - 1; x++)
		{
			uint16_t vi = (uint16_t)(y * VERTICES_PER_FACE + x);
			size_t i = (y * VERTICES_PER_FACE + x) * 6;

			// Right
			(*indices)[i + 0] = vi + 1;
			// Bottom
			(*indices)[i + 1] = vi + VERTICES_PER_FACE;
			// Center
			(*indices)[i + 2] = vi;



			// Bottom Right
			(*indices)[i + 0 + 3] = vi + 1 + VERTICES_PER_FACE;
			// Bottom
			(*indices)[i + 1 + 3] = vi + VERTICES_PER_FACE;
			// Right
			(*indices)[i + 2 + 3] = vi + 1;


		}
	}

	for (int y = 0; y < VERTICES_PER_FACE; y++)
	{
		for (int x = 0; x < VERTICES_PER_FACE; x++)
		{
			size_t i = y * VERTICES_PER_FACE + x;
			double tx = (double)x / ((double)VERTICES_PER_FACE - 1);
			double ty = (double)y / ((double)VERTICES_PER_FACE - 1);
			tx *= 2.0; ty *= 2.0;
			tx -= 1.0; ty -= 1.0;


			glm::dvec3 in_tile = glm::dvec3(tx, ty, 1.0);

			glm::dvec3 world_pos_spheric = MathUtil::cube_to_sphere(in_tile);


			(*verts)[i].pos = world_pos_spheric;
			(*verts)[i].nrm = glm::vec3(0.0f, 0.0f, 0.0f);
		}
	}

	// Generate normals
	for (size_t i = 0; i < indices->size(); i += 3)
	{
		WaterVertex* v0 = &(*verts)[(*indices)[i + 0]];
		WaterVertex* v1 = &(*verts)[(*indices)[i + 1]];
		WaterVertex* v2 = &(*verts)[(*indices)[i + 2]];

		glm::vec3 face_normal;

		face_normal = glm::triangleNormal(v1->pos, v2->pos, v0->pos);

		v0->nrm += face_normal;
		v1->nrm += face_normal;
		v2->nrm += face_normal;
	}

	for (size_t i = 0; i < verts->size(); i++)
	{
		WaterVertex* v = &(*verts)[i];
		v->nrm = glm::normalize(v->nrm);
	}

	// Upload to OpenGL
	glGenVertexArrays(1, &vao);
	glGenBuffers(1, &ebo);
	glGenBuffers(1, &vbo);

	glBindVertexArray(vao);


	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof((*indices)[0]) * indices->size(), indices->data(), GL_STATIC_DRAW);


	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof((*verts)[0]) * verts->size(), verts->data(), GL_STATIC_DRAW);


	// pos
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);

	// nrm
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));


	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	delete verts;

	shader = assets->get<Shader>("planet/water");
}


WaterRenderer::~WaterRenderer()
{
}
