#include "PlanetTile.h"
#include "../../util/Logger.h"

constexpr std::array<uint16_t, (PlanetTile::TILE_SIZE + 2) * (PlanetTile::TILE_SIZE + 2) * 6> get_nrm_indices()
{
	std::array<uint16_t, (PlanetTile::TILE_SIZE + 2) * (PlanetTile::TILE_SIZE + 2) * 6> out = 
		std::array<uint16_t, (PlanetTile::TILE_SIZE + 2) * (PlanetTile::TILE_SIZE + 2) * 6>();

	for (int y = -1; y < PlanetTile::TILE_SIZE; y++)
	{
		for (int x = -1; x < PlanetTile::TILE_SIZE; x++)
		{
			uint16_t vi = (uint16_t)((y + 1) * (PlanetTile::TILE_SIZE + 2) + (x + 1));
			size_t i = ((y + 1) * (PlanetTile::TILE_SIZE + 2) + (x + 1)) * 6;

			// Right
			out[i + 0] = vi + 1;
			// Center
			out[i + 1] = vi;
			// Bottom
			out[i + 2] = vi + (PlanetTile::TILE_SIZE + 2);



			// Bottom Right
			out[i + 0 + 3] = vi + 1 + (PlanetTile::TILE_SIZE + 2);
			// Right
			out[i + 1 + 3] = vi + 1;
			// Bottom
			out[i + 2 + 3] = vi + (PlanetTile::TILE_SIZE + 2);

		}
	}

	return out;
}

void PlanetTile::generate(PlanetTilePath path)
{
	clockwise = false;

	if (path.side == PY ||
		path.side == NY ||
		path.side == NX)
	{
		clockwise = true;
	}


	// We actually generate some more vertices than we need, so that
	// normals can smoothly interpolate between tiles

	// Note: This array does not include the skirts, but does include
	// the extra rows and columns
	std::array<PlanetTileVertex, (TILE_SIZE + 2) * (TILE_SIZE + 2)> surface_verts;

	glm::mat4 model = path.get_model_matrix();
	glm::mat4 model_spheric = path.get_model_spheric_matrix();
	glm::mat4 inverse_model = glm::inverse(model);
	glm::mat4 inverse_model_spheric = glm::inverse(model_spheric);


	for (int y = -1; y < TILE_SIZE + 1; y++)
	{
		for (int x = -1; x < TILE_SIZE + 1; x++)
		{
			size_t r_index = (y + 1) * (TILE_SIZE + 2) + (x + 1);

			float tx = (float)x / ((float)TILE_SIZE - 1.0f);
			float ty = (float)y / ((float)TILE_SIZE - 1.0f);

			PlanetTileVertex vert;
			glm::vec3 in_tile = glm::vec3(tx, ty, 0.0f);;

			glm::vec3 world_pos_cubic = model * glm::vec4(in_tile, 1.0f);
			glm::vec3 world_pos_spheric = MathUtil::cube_to_sphere(world_pos_cubic);

			vert.pos = inverse_model_spheric * glm::vec4(world_pos_spheric, 1.0f);
			vert.nrm = glm::vec3(0.0f, 0.0f, 0.0f);

			surface_verts[r_index] = vert;
		}
	}
	
	// Generate normals (TODO)
	auto nrm_indices = get_nrm_indices();
	for (size_t i = 0; i < nrm_indices.size(); i += 3)
	{
		PlanetTileVertex* v0 = &surface_verts[nrm_indices[i]];
		PlanetTileVertex* v1 = &surface_verts[nrm_indices[i + 1]];
		PlanetTileVertex* v2 = &surface_verts[nrm_indices[i + 2]];

		glm::vec3 p0 = model_spheric * glm::vec4(v0->pos, 1.0f);
		glm::vec3 p1 = model_spheric * glm::vec4(v1->pos, 1.0f);
		glm::vec3 p2 = model_spheric * glm::vec4(v2->pos, 1.0f);

		
		glm::vec3 face_normal;

		if (clockwise)
		{
			face_normal = glm::triangleNormal(p1, p0, p2);
		}
		else
		{
			face_normal = glm::triangleNormal(p0, p1, p2);
		}

		v0->nrm += face_normal;
		v1->nrm += face_normal;
		v2->nrm += face_normal;
	}


	for (size_t i = 0; i < surface_verts.size(); i++)
	{
		PlanetTileVertex* v = &surface_verts[i];
		v->nrm = glm::normalize(v->nrm);
	}

	// Generate skirt vertices (TODO)
	std::array<PlanetTileVertex, TILE_SIZE * 4> skirts;

	// Copy actual vertices
	for (int y = 0; y < TILE_SIZE; y++)
	{
		for (int x = 0; x < TILE_SIZE; x++)
		{
			size_t o_index = (y + 1) * (TILE_SIZE + 2) + (x + 1);
			size_t f_index = y * TILE_SIZE + x;

			vertices[f_index] = surface_verts[o_index];
		}
	}

	// Copy skirts
	for (size_t i = 0; i < skirts.size(); i++)
	{
		vertices[i + TILE_SIZE * TILE_SIZE] = skirts[i];
	}

}

void PlanetTile::upload()
{
	logger->check(!is_uploaded(), "Tried to upload an already uploaded tile");

	glGenBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices[0]) * vertices.size(), vertices.data(), GL_STATIC_DRAW);

}

PlanetTile::PlanetTile()
{
	vbo = 0;
}

PlanetTile::~PlanetTile()
{
	if (is_uploaded())
	{
		glDeleteBuffers(1, &vbo);
		vbo = 0;
	}
}
