#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <vector>
#include <glad/glad.h>
#include "../assets/Shader.h"
#include "../assets/AssetManager.h"
#include "MathUtil.h"
#include "../universe/kepler/KeplerElements.h"

class DebugDrawer
{
private:

	struct DebugVertexf
	{
		glm::vec3 pos;
		glm::vec3 color;
	};

	struct DebugVertex
	{
		glm::dvec3 pos;
		glm::vec3 color;

		DebugVertex(glm::dvec3 p, glm::vec3 c) : pos(p), color(c) {};
		DebugVertexf transform(glm::dmat4 c_model)
		{
			DebugVertexf out;
			out.color = color;
			out.pos = glm::vec3(c_model * glm::dvec4(pos, 1.0));
			return out;
		}
	};

	struct DebugShape
	{
		std::vector<DebugVertex> verts;
	};

	std::vector<DebugShape> draw_list;

	GLuint points_vbo, points_vao;
	GLuint lines_vbo, lines_vao;

	Shader* shader;

public:

	float point_size;
	float line_size;

	void render(glm::dmat4 proj_view, glm::dmat4 c_model, float far_plane);

	void add_point(glm::dvec3 a, glm::vec3 color);
	void add_line(glm::dvec3 a, glm::dvec3 b, glm::vec3 color);
	void add_arrow(glm::dvec3 a, glm::dvec3 b, glm::vec3 color);

	void add_cone(glm::dvec3 base, glm::dvec3 tip, double radius, glm::vec3 color, int verts = 16);

	// Not the drawing method in the game, just for debug, really slow!
	void add_orbit(glm::dvec3 origin, KeplerOrbit orbit, glm::vec3 color, bool striped = false, int verts = 128);



	DebugDrawer();
	~DebugDrawer();
};

extern DebugDrawer* debug_drawer;

void create_global_debug_drawer();
void destroy_global_debug_drawer();
