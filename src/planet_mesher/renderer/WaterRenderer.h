#pragma once
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtx/normal.hpp>
#include "../../util/MathUtil.h"
#include "../../assets/AssetManager.h"
#include "../../assets/Shader.h"
#include "../quadtree/QuadTreeDefines.h"

struct WaterVertex
{
	glm::vec3 pos;
	glm::vec3 nrm;
};

// TODO: This is a "brute-force" test
// We simply generate a high poly cubesphere
class WaterRenderer
{
private:

	GLuint vao, vbo, ebo;
	Shader* shader;

	const static int VERTICES_PER_FACE = 200;
	const static int VERTEX_COUNT = VERTICES_PER_FACE * VERTICES_PER_FACE;
	const static int INDEX_COUNT = VERTEX_COUNT * 6;

public:

	void render(glm::dmat4 proj_view, glm::dvec3 camera_pos, glm::dmat4 model, float far_plane);


	WaterRenderer();
	~WaterRenderer();
};

