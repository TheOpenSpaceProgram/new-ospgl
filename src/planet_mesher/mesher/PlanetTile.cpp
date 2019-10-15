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

// Warning: Some template magic ahead, 
// T must have a pos and nrm member!

template<typename T>
void generate_normals(T* verts, size_t verts_size, glm::dmat4 model_spheric, bool clockwise)
{
	auto nrm_indices = get_nrm_indices();

	for (size_t i = 0; i < nrm_indices.size(); i += 3)
	{
		T* v0 = &verts[nrm_indices[i + 0]];
		T* v1 = &verts[nrm_indices[i + 1]];
		T* v2 = &verts[nrm_indices[i + 2]];

		glm::dvec3 p0 = model_spheric * glm::dvec4(v0->pos, 1.0);
		glm::dvec3 p1 = model_spheric * glm::dvec4(v1->pos, 1.0);
		glm::dvec3 p2 = model_spheric * glm::dvec4(v2->pos, 1.0);


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


	for (size_t i = 0; i < verts_size; i++)
	{
		T* v = &verts[i];
		v->nrm = glm::normalize(v->nrm);
	}
}

template<typename T, bool water>
void generate_vertices(T* verts, glm::dmat4 model, glm::dmat4 inverse_model_spheric, double* heights)
{
	for (int y = -1; y < PlanetTile::TILE_SIZE + 1; y++)
	{
		for (int x = -1; x < PlanetTile::TILE_SIZE + 1; x++)
		{
			size_t r_index = (y + 1) * (PlanetTile::TILE_SIZE + 2) + (x + 1);

			double tx = (double)x / ((double)PlanetTile::TILE_SIZE - 1.0);
			double ty = (double)y / ((double)PlanetTile::TILE_SIZE - 1.0);

			T vert;
			glm::dvec3 in_tile = glm::dvec3(tx, ty, 0.0);

			glm::dvec3 world_pos_cubic = model * glm::vec4(in_tile, 1.0);
			glm::dvec3 world_pos_spheric = MathUtil::cube_to_sphere(world_pos_cubic);

			double height = heights[r_index];

			if constexpr (water)
			{
				vert.col.x = -(float)height;
			}
			else
			{
				world_pos_spheric += glm::normalize(world_pos_spheric) * height;
			}

			vert.pos = (glm::vec3)(inverse_model_spheric * glm::dvec4(world_pos_spheric, 1.0));
			vert.nrm = glm::vec3(0.0f, 0.0f, 0.0f);

			verts[r_index] = vert;
		}
	}
}

template<typename T, typename Q>
void copy_vertices(T* origin, Q* destination)
{
	for (int y = 0; y < PlanetTile::TILE_SIZE; y++)
	{
		for (int x = 0; x < PlanetTile::TILE_SIZE; x++)
		{
			size_t o_index = (y + 1) * (PlanetTile::TILE_SIZE + 2) + (x + 1);
			size_t f_index = y * PlanetTile::TILE_SIZE + x;

			destination[f_index].pos = origin[o_index].pos;
			destination[f_index].nrm = origin[o_index].nrm;
			if constexpr (std::is_same<Q, PlanetTileWaterVertex>::value)
			{
				destination[f_index].depth = origin[o_index].col.x;
			}
		}
	}
}


bool PlanetTile::generate(PlanetTilePath path, double planet_radius, sol::state& lua_state, bool has_water,
	VertexArray<PlanetTileVertex>* work_array)
{
	
	bool errors = false;

	clockwise = false;

	if (path.side == PY ||
		path.side == NY ||
		path.side == NX)
	{
		clockwise = true;
	}

	glm::dmat4 model = path.get_model_matrix();
	glm::dmat4 model_spheric = path.get_model_spheric_matrix();
	glm::dmat4 inverse_model = glm::inverse(model);
	glm::dmat4 inverse_model_spheric = glm::inverse(model_spheric);

	std::array<double, (TILE_SIZE + 2) * (TILE_SIZE + 2)> heights;

	size_t depth = path.get_depth();

	lua_state["depth"] = depth;
	lua_state["radius"] = planet_radius;

	// We only need water if there is a tile over the water level (height = 0)
	bool needs_water = false;

	for (int x = -1; x < TILE_SIZE + 1; x++)
	{
		for (int y = -1; y < TILE_SIZE + 1; y++)
		{

			double tx = (double)x / ((double)TILE_SIZE - 1.0);
			double ty = (double)y / ((double)TILE_SIZE - 1.0);

			glm::dvec3 in_tile = glm::dvec3(tx, ty, 0.0);

			glm::dvec3 world_pos_cubic = model * glm::dvec4(in_tile, 1.0);
			glm::dvec3 world_pos_spheric = MathUtil::cube_to_sphere(world_pos_cubic);

			glm::dvec3 sphere = world_pos_spheric;
			glm::dvec2 projected = MathUtil::euclidean_to_spherical_r1(sphere);

			lua_state["coord_3d"]["x"] = sphere.x;
			lua_state["coord_3d"]["y"] = sphere.y;
			lua_state["coord_3d"]["z"] = sphere.z;
			lua_state["coord_2d"]["x"] = projected.x;
			lua_state["coord_2d"]["y"] = projected.y;

			size_t i = (y + 1) * (TILE_SIZE + 2) + (x + 1);
			
			sol::protected_function func = lua_state["generate"];
			 
			// TODO: Texture generation
			// If an error happens, generating will lag 
			// (exceptions are really slow)
			// So we instead flatten the whole world
			if (errors)
			{
				heights[i] = 0.0f;
			}
			else
			{
				auto result = func();
				if (!result.valid())
				{
					sol::error err = result;
					logger->error("Lua Runtime Error:\n{}", err.what());
					// We only write one error per tile so we don't overload the log
					errors = true;
					heights[i] = 0.0;
				}
				else
				{
					heights[i] = result.get<double>() / planet_radius;
					if (!needs_water)
					{
						if (heights[i] > 0.0)
						{
							needs_water = true;
						}
					}
				}
			}
		}
	}

	lua_state.collect_garbage();


	generate_vertices<PlanetTileVertex, false>(work_array->data(), model, inverse_model_spheric, &heights[0]);
	generate_normals(work_array->data(), work_array->size(), model_spheric, clockwise);
	copy_vertices(work_array->data(), vertices.data());

	if (has_water && needs_water || true)
	{
		generate_vertices<PlanetTileVertex, true>(work_array->data(), model, inverse_model_spheric, &heights[0]);
		generate_normals(work_array->data(), work_array->size(), model_spheric, clockwise);
		water_vertices = new std::array<PlanetTileWaterVertex, VERTEX_COUNT>();
		copy_vertices(work_array->data(), water_vertices->data());
	}

	// Generate skirt vertices (TODO)
	std::array<PlanetTileVertex, TILE_SIZE * 4> skirts;


	// Copy skirts
	for (size_t i = 0; i < skirts.size(); i++)
	{
		vertices[i + TILE_SIZE * TILE_SIZE] = skirts[i];
	}

	
	return errors;

}

void PlanetTile::upload()
{
	logger->check(!is_uploaded(), "Tried to upload an already uploaded tile");

	glGenBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(PlanetTileVertex) * vertices.size(), vertices.data(), GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	if (water_vertices != nullptr)
	{
		glGenBuffers(1, &water_vbo);
		glBindBuffer(GL_ARRAY_BUFFER, water_vbo);
		glBufferData(GL_ARRAY_BUFFER, sizeof(PlanetTileWaterVertex) * (*water_vertices).size(), (*water_vertices).data(), GL_STATIC_DRAW);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
	}
}


PlanetTile::PlanetTile()
{
	vbo = 0;
	water_vbo = 0;
	water_vertices = nullptr;

}

PlanetTile::~PlanetTile()
{

	if (water_vertices != nullptr)
	{
		delete water_vertices;
	}

	if (vbo != 0)
	{
		glDeleteBuffers(1, &vbo);
		vbo = 0;
	}

	if (water_vbo != 0)
	{
		glDeleteBuffers(1, &water_vbo);
		water_vbo = 0;
	}
}
