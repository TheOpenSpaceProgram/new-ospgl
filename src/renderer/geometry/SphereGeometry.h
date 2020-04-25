#pragma once
#include <cinttypes>
#include <glm/glm.hpp>
#include <util/MathUtil.h>
#include <glad/glad.h>
#include <vector>

// Generates vertices and normals for arbitrary spheres,
// you need to have a vertex class with a 
// vec3 pos
// It assumes index buffer usage
class SphereGeometry
{
private:

	static size_t get_face_size(size_t verts_per_side)
	{
		return verts_per_side * verts_per_side;
	}

	static size_t get_face_index_size(size_t verts_per_side)
	{
		return get_face_size(verts_per_side) * 6;
	}

	template <typename T>
	static void generate_face(T* target, size_t offset, size_t verts_per_side, glm::mat4 rotation)
	{
		size_t r_offset = offset * get_face_size(verts_per_side);

		for (size_t y = 0; y < verts_per_side; y++)
		{
			for (size_t x = 0; x < verts_per_side; x++)
			{
				float xf = (float)x / (float)(verts_per_side - 1);
				float yf = (float)y / (float)(verts_per_side - 1);
				xf *= 2.0f;	xf -= 1.0f;
				yf *= 2.0f; yf -= 1.0f;

				// We start with the PX face
				glm::vec3 pos = glm::vec3(1.0f, yf, xf);
				pos = MathUtil::cube_to_sphere(pos);

				// Now rotate with the rotation matrix
				pos = rotation * glm::vec4(pos, 1.0f);

				target[y * verts_per_side + x + r_offset].pos = pos;
			}
		}
	}

public:


	static size_t get_vertex_size(size_t verts_per_side)
	{
		return get_face_size(verts_per_side) * 6;
	}

	static size_t get_index_size(size_t verts_per_side)
	{
		return get_face_index_size(verts_per_side) * 6;
	}

	template <typename T>
	static void generate_cubesphere(T* target, uint32_t* indices, size_t verts_per_side)
	{
		using glm::vec3;

		generate_face<T>(target, 0, verts_per_side, glm::mat4(1.0f));
		generate_face<T>(target, 1, verts_per_side, MathUtil::rotate_from_to(vec3(1.0f, 0.0f, 0.0f), vec3(0.0f, 1.0f, 0.0f)));
		generate_face<T>(target, 2, verts_per_side, MathUtil::rotate_from_to(vec3(1.0f, 0.0f, 0.0f), vec3(0.0f, 0.0f, 1.0f)));
		generate_face<T>(target, 3, verts_per_side, MathUtil::rotate_from_to(vec3(1.0f, 0.0f, 0.0f), vec3(0.0f, 0.0f, -1.0f)));
		generate_face<T>(target, 4, verts_per_side, MathUtil::rotate_from_to(vec3(1.0f, 0.0f, 0.0f), vec3(0.0f, -1.0f, 0.0f)));
		generate_face<T>(target, 5, verts_per_side, glm::rotate(glm::mat4(1.0f), glm::pi<float>(), vec3(0.0f, 1.0f, 0.0f)));

		size_t subi = verts_per_side * verts_per_side * 6;
		uint32_t subv = (uint32_t)(verts_per_side * verts_per_side);

		// Generate indices
		for (size_t y = 0; y < verts_per_side - 1; y++)
		{
			for (size_t x = 0; x < verts_per_side - 1; x++)
			{

				for (size_t j = 0; j < 6; j++)
				{
					uint32_t vi = (uint32_t)(y * verts_per_side + x) + subv * (uint32_t)j;
					size_t i = (y * verts_per_side + x) * 6;

					// Right
					indices[i + 0 + subi * j] = (uint32_t)(vi + 1);
					// Center
					indices[i + 1 + subi * j] = (uint32_t)vi;
					// Bottom
					indices[i + 2 + subi * j] = (uint32_t)(vi + verts_per_side);


					// Bottom Right
					indices[i + 0 + 3 + subi * j] = (uint32_t)(vi + 1 + verts_per_side);
					// Right
					indices[i + 1 + 3 + subi * j] = (uint32_t)(vi + 1);
					// Bottom
					indices[i + 2 + 3 + subi * j] = (uint32_t)(vi + verts_per_side);
				}

			}
		}
	}

	static void generate_and_upload(GLuint* vao, GLuint* vbo, GLuint* ebo, size_t* index_count, size_t verts_per_side = 16)
	{
		struct SimpleVertex
		{
			glm::vec3 pos;
		};

		std::vector<SimpleVertex> verts;
		verts.resize(get_vertex_size(verts_per_side));

		std::vector<uint32_t> indices;
		indices.resize(get_index_size(verts_per_side));

		*index_count = indices.size();

		generate_cubesphere(verts.data(), indices.data(), verts_per_side);

		glGenBuffers(1, vbo);
		glGenBuffers(1, ebo);
		glGenVertexArrays(1, vao);

		glBindVertexArray(*vao);

		glBindBuffer(GL_ARRAY_BUFFER, *vbo);
		glBufferData(GL_ARRAY_BUFFER, sizeof(SimpleVertex) * verts.size(), verts.data(), GL_STATIC_DRAW);

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, *ebo);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(uint32_t) * indices.size(), indices.data(), GL_STATIC_DRAW);

		// pos
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);

		glBindVertexArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	}
};

