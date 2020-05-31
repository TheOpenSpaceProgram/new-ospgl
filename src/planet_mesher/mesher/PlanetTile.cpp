#include "PlanetTile.h"
#include "../../util/Logger.h"

template<int S>
constexpr std::array<uint16_t, (S + 2) * (S + 2) * 6> get_nrm_indices()
{
	std::array<uint16_t, (S + 2) * (S + 2) * 6> out = 
		std::array<uint16_t, (S + 2) * (S + 2) * 6>();

	for (int y = -1; y < S; y++)
	{
		for (int x = -1; x < S; x++)
		{
			uint16_t vi = (uint16_t)((y + 1) * (S + 2) + (x + 1));
			size_t i = ((y + 1) * (S + 2) + (x + 1)) * 6;

			// Right
			out[i + 0] = vi + 1;
			// Center
			out[i + 1] = vi;
			// Bottom
			out[i + 2] = vi + (S + 2);



			// Bottom Right
			out[i + 0 + 3] = vi + 1 + (S + 2);
			// Right
			out[i + 1 + 3] = vi + 1;
			// Bottom
			out[i + 2 + 3] = vi + (S + 2);

		}
	}

	return out;
}

// Warning: Some template magic ahead, 
// T must have a pos and nrm member!

template<int S, typename T >
void generate_normals(T* verts, size_t verts_size, glm::dmat4 model_spheric, bool clockwise)
{
	auto nrm_indices = get_nrm_indices<S>();

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

template<int S, typename T, bool water>
void generate_vertices(T* verts, glm::dmat4 model, glm::dmat4 inverse_model_spheric, double* heights, glm::vec3* colors)
{
	for (int y = -1; y < S + 1; y++)
	{
		for (int x = -1; x < S + 1; x++)
		{
			size_t r_index = (y + 1) * (S + 2) + (x + 1);

			double tx = (double)x / ((double)S - 1.0);
			double ty = (double)y / ((double)S - 1.0);

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
				vert.col = colors[r_index];
			}

			vert.pos = (glm::vec3)(inverse_model_spheric * glm::dvec4(world_pos_spheric, 1.0));
			vert.nrm = glm::vec3(0.0f, 0.0f, 0.0f);

			verts[r_index] = vert;
		}
	}
}


template<typename T>
void generate_vertices_simple(T* verts, glm::dmat4 model, glm::dmat4 inverse_model_spheric, double* heights)
{
	// We need some small tricks to keep the render and physics vertices aligned
	for (int y = 0; y < PlanetTile::PHYSICS_SIZE; y++)
	{
		for (int x = 0; x < PlanetTile::PHYSICS_SIZE; x++)
		{
			size_t r_index = y * PlanetTile::PHYSICS_SIZE + x;

			double tx = (double)x / ((double)PlanetTile::PHYSICS_SIZE - 1.0);
			double ty = (double)y / ((double)PlanetTile::PHYSICS_SIZE - 1.0);

			double height = heights[r_index];

			T vert;
			glm::dvec3 in_tile = glm::dvec3(tx, ty, 0.0);

			glm::dvec3 world_pos_cubic = model * glm::vec4(in_tile, 1.0);
			glm::dvec3 world_pos_spheric = MathUtil::cube_to_sphere(world_pos_cubic);

			world_pos_spheric += glm::normalize(world_pos_spheric) * height;

			vert.pos = (glm::vec3)(inverse_model_spheric * glm::dvec4(world_pos_spheric, 1.0));

			verts[r_index] = vert;
		}
	}
}

template<int S, typename T, typename Q>
void copy_vertices(T* origin, Q* destination)
{
	for (int y = 0; y < S; y++)
	{
		for (int x = 0; x < S; x++)
		{
			size_t o_index = (y + 1) * (S + 2) + (x + 1);
			size_t f_index = y * S + x;

			destination[f_index].pos = origin[o_index].pos;
			destination[f_index].nrm = origin[o_index].nrm;
			if constexpr (!std::is_same<Q, PlanetTileSimpleVertex>::value)
			{
				if constexpr (std::is_same<Q, PlanetTileWaterVertex>::value)
				{
					destination[f_index].depth = origin[o_index].col.x;
				}
				else
				{
					destination[f_index].col = origin[o_index].col;
				}
			}
		}
	}
}


void generate_skirt(PlanetTileVertex* target, glm::dmat4 model, glm::dmat4 inverse_model_spheric, PlanetTileVertex& copy_vert)
{
	PlanetTileVertex vert;

	double tx = 0.5;
	double ty = 0.5;

	glm::dvec3 in_tile = glm::dvec3(tx, ty, 0.0);

	glm::dvec3 world_pos_cubic = model * glm::vec4(in_tile, 1.0);
	glm::dvec3 world_pos_spheric = MathUtil::cube_to_sphere(world_pos_cubic);

	world_pos_spheric *= 0.95;

	vert = copy_vert;
	vert.pos = (glm::vec3)(inverse_model_spheric * glm::dvec4(world_pos_spheric, 1.0));

	*target = vert;
}

#include <util/Timer.h>

bool PlanetTile::generate(PlanetTilePath path, double planet_radius, sol::state& lua_state, bool has_water,
	VertexArray<PlanetTileVertex, PlanetTile::TILE_SIZE>* work_array)
{
	Timer benchmark;	

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
	std::array<glm::vec3, (TILE_SIZE + 2) * (TILE_SIZE + 2)> colors;

	size_t depth = path.get_depth();

	// We only need water if there is a tile over the water level (height = 0)
	bool needs_water = false;

	GeneratorInfo info;
	info.depth = (int)depth;
	info.radius = planet_radius;
	info.needs_color = true;

	sol::protected_function func = lua_state["generate"];


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

			size_t i = (y + 1) * (TILE_SIZE + 2) + (x + 1);
			
			info.coord_3d = sphere;
			info.coord_2d = projected;

			GeneratorOut out;
			out.height = 1.0;
			out.color = glm::dvec3(1.0, 0.0, 1.0);

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
				auto result = func(info, &out);
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
					heights[i] = (out.height) / planet_radius;
					if (!needs_water)
					{
						if (heights[i] < 0.0)
						{
							needs_water = true;
						}
					}

					colors[i] = (glm::vec3)out.color;

				}
			}
		}
	}

	lua_state.collect_garbage();

	generate_vertices<TILE_SIZE, PlanetTileVertex, false>(work_array->data(), model, inverse_model_spheric, &heights[0], &colors[0]);
	generate_normals<TILE_SIZE>(work_array->data(), work_array->size(), model_spheric, clockwise);
	copy_vertices<TILE_SIZE>(work_array->data(), vertices.data());

	water_vertices = nullptr;
	water_vbo = 0;
	if (has_water && needs_water)
	{
		generate_vertices<TILE_SIZE, PlanetTileVertex, true>(work_array->data(), model, inverse_model_spheric, &heights[0], nullptr);
		generate_normals<TILE_SIZE>(work_array->data(), work_array->size(), model_spheric, clockwise);
		water_vertices = new std::array<PlanetTileWaterVertex, VERTEX_COUNT>();
		copy_vertices<TILE_SIZE>(work_array->data(), water_vertices->data());
	}

	std::array<PlanetTileVertex, 4> skirts;
	// Up
	generate_skirt(&skirts[0], model, inverse_model_spheric, vertices[0 * TILE_SIZE + 0]);

	// Down
	generate_skirt(&skirts[1], model, inverse_model_spheric, vertices[(TILE_SIZE - 1) * TILE_SIZE + 0]);

	// Left
	generate_skirt(&skirts[2], model, inverse_model_spheric, vertices[0 * TILE_SIZE + 0]);

	// Right
	generate_skirt(&skirts[3], model, inverse_model_spheric, vertices[0 * TILE_SIZE + (TILE_SIZE - 1)]);

	// Copy skirts
	for (size_t i = 0; i < skirts.size(); i++)
	{
		vertices[i + TILE_SIZE * TILE_SIZE] = skirts[i];
	}

	double time = benchmark.get_elapsed_time();
	logger->info("Time took: {}s", time);
	
	return errors;

}


bool PlanetTile::generate_physics(PlanetTilePath path, double planet_radius, sol::state& lua_state,
	SimpleVertexArray<PlanetTile::PHYSICS_SIZE>* work_array)
{
	bool errors = false;

	glm::dmat4 model = path.get_model_matrix();
	glm::dmat4 model_spheric = path.get_model_spheric_matrix();
	glm::dmat4 inverse_model = glm::inverse(model);
	glm::dmat4 inverse_model_spheric = glm::inverse(model_spheric);

	std::array<double, PHYSICS_SIZE * PHYSICS_SIZE> heights;

	size_t depth = path.get_depth();

	GeneratorInfo info;
	info.depth = (int)depth;
	info.radius = planet_radius;
	info.needs_color = false;

	sol::protected_function func = lua_state["generate"];

	 
	// We need some small tricks to keep the render and physics vertices aligned
	for (int y = 0; y < PlanetTile::PHYSICS_SIZE; y++)
	{
		for (int x = 0; x < PlanetTile::PHYSICS_SIZE; x++)
		{

			size_t r_index = y * PlanetTile::PHYSICS_SIZE + x;

			double tx = (double)x / ((double)PlanetTile::PHYSICS_SIZE - 1.0);
			double ty = (double)y / ((double)PlanetTile::PHYSICS_SIZE - 1.0);

			glm::dvec3 in_tile = glm::dvec3(tx, ty, 0.0);

			glm::dvec3 world_pos_cubic = model * glm::dvec4(in_tile, 1.0);
			glm::dvec3 world_pos_spheric = MathUtil::cube_to_sphere(world_pos_cubic);

			glm::dvec3 sphere = world_pos_spheric;
			glm::dvec2 projected = MathUtil::euclidean_to_spherical_r1(sphere);


			info.coord_3d = sphere;
			info.coord_2d = projected;

			GeneratorOut out;
			out.height = 1.0;
			out.color = glm::dvec3(1.0, 0.0, 1.0);

			// If an error happens, generating will lag 
			// (exceptions are really slow)
			// So we instead flatten the whole world
			if (errors)
			{
				heights[r_index] = 0.0f;
			}
			else
			{
				auto result = func(info, &out);
				if (!result.valid())
				{
					sol::error err = result;
					logger->error("Lua Runtime Error:\n{}", err.what());
					// We only write one error per tile so we don't overload the log
					errors = true;
					heights[r_index] = 0.0;
				}
				else
				{
					heights[r_index] = (out.height) / planet_radius;
				}
			}
		}
	}

	lua_state.collect_garbage();

	generate_vertices_simple<PlanetTileSimpleVertex>(work_array->data(), model, inverse_model_spheric, heights.data());

	return errors;
}

void PlanetTile::prepare_lua(sol::state& lua_state)
{
	lua_core->load(lua_state, assets->get_current_package());


	// We must define the little utility struct GeneratorInfo
	lua_state.new_usertype<GeneratorInfo>("generator_info",
		"coord_3d", &GeneratorInfo::coord_3d,
		"coord_2d", &GeneratorInfo::coord_2d,
		"radius", &GeneratorInfo::radius,
		"depth", &GeneratorInfo::depth);

	lua_state.new_usertype<GeneratorOut>("generator_out",
		"height", &GeneratorOut::height,
		"color", &GeneratorOut::color);
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


void PlanetTile::generate_index_array_with_skirts(std::array<uint16_t, INDEX_COUNT>& indices, size_t& bulk_index_count)
{
	for (size_t i = 0; i < indices.size(); i++)
	{
		indices[i] = 65535;
	}

	// Bulk indices
	for (size_t y = 0; y < PlanetTile::TILE_SIZE - 1; y++)
	{
		for (size_t x = 0; x < PlanetTile::TILE_SIZE - 1; x++)
		{
			uint16_t vi = (uint16_t)(y * PlanetTile::TILE_SIZE + x);
			size_t i = (y * (PlanetTile::TILE_SIZE - 1) + x) * 6;

			// Right
			indices[i + 0] = vi + 1;
			// Center
			indices[i + 1] = vi;
			// Bottom
			indices[i + 2] = vi + PlanetTile::TILE_SIZE;


			// Bottom Right
			indices[i + 0 + 3] = vi + 1 + PlanetTile::TILE_SIZE;
			// Right
			indices[i + 1 + 3] = vi + 1;
			// Bottom
			indices[i + 2 + 3] = vi + PlanetTile::TILE_SIZE;

		}
	}

	bulk_index_count = (PlanetTile::TILE_SIZE - 1) * (PlanetTile::TILE_SIZE - 1) * 6;

	// Now skirt indices
	size_t skirt_ptr = (PlanetTile::TILE_SIZE - 1) * (PlanetTile::TILE_SIZE - 1);
	size_t skirt_off = PlanetTile::TILE_SIZE * PlanetTile::TILE_SIZE;
	size_t skirt_offset = (PlanetTile::TILE_SIZE - 1) * 3;
	// Up skirt
	for (size_t x = 0; x < PlanetTile::TILE_SIZE - 1; x++)
	{
		indices[skirt_ptr * 6 + 0 + x * 3 + skirt_offset * 0] = (uint16_t)(0 * PlanetTile::TILE_SIZE + x);
		indices[skirt_ptr * 6 + 1 + x * 3 + skirt_offset * 0] = (uint16_t)(0 * PlanetTile::TILE_SIZE + x + 1);
		indices[skirt_ptr * 6 + 2 + x * 3 + skirt_offset * 0] = (uint16_t)(skirt_off);
	}

	// Down skirt
	for (size_t x = 0; x < PlanetTile::TILE_SIZE - 1; x++)
	{
		indices[skirt_ptr * 6 + 0 + x * 3 + skirt_offset * 1] = (uint16_t)((PlanetTile::TILE_SIZE - 1) * PlanetTile::TILE_SIZE + x + 1);
		indices[skirt_ptr * 6 + 1 + x * 3 + skirt_offset * 1] = (uint16_t)((PlanetTile::TILE_SIZE - 1) * PlanetTile::TILE_SIZE + x);
		indices[skirt_ptr * 6 + 2 + x * 3 + skirt_offset * 1] = (uint16_t)(skirt_off + 1);
	}

	// Left skirt
	for (size_t y = 0; y < PlanetTile::TILE_SIZE - 1; y++)
	{
		indices[skirt_ptr * 6 + 0 + y * 3 + skirt_offset * 2] = (uint16_t)((y + 1) * PlanetTile::TILE_SIZE + 0);
		indices[skirt_ptr * 6 + 1 + y * 3 + skirt_offset * 2] = (uint16_t)(y * PlanetTile::TILE_SIZE + 0);
		indices[skirt_ptr * 6 + 2 + y * 3 + skirt_offset * 2] = (uint16_t)(skirt_off + 2);
	}

	// Right skirt
	for (size_t y = 0; y < PlanetTile::TILE_SIZE - 1; y++)
	{

		indices[skirt_ptr * 6 + 0 + y * 3 + skirt_offset * 3] = (uint16_t)(y * PlanetTile::TILE_SIZE + (PlanetTile::TILE_SIZE - 1));
		indices[skirt_ptr * 6 + 1 + y * 3 + skirt_offset * 3] = (uint16_t)((y + 1) * PlanetTile::TILE_SIZE + (PlanetTile::TILE_SIZE - 1));
		indices[skirt_ptr * 6 + 2 + y * 3 + skirt_offset * 3] = (uint16_t)(skirt_off + 3);
	}
}

void PlanetTile::generate_physics_index_array(std::array<uint16_t, PHYSICS_INDEX_COUNT>& indices)
{
	for (size_t i = 0; i < indices.size(); i++)
	{
		indices[i] = 65535;
	}

	// Only Bulk indices
	for (size_t y = 0; y < PlanetTile::PHYSICS_SIZE - 1; y++)
	{
		for (size_t x = 0; x < PlanetTile::PHYSICS_SIZE - 1; x++)
		{
			uint16_t vi = (uint16_t)(y * PlanetTile::PHYSICS_SIZE + x);
			size_t i = (y * (PlanetTile::PHYSICS_SIZE - 1) + x) * 6;

			// Right
			indices[i + 0] = vi + 1;
			// Center
			indices[i + 1] = vi;
			// Bottom
			indices[i + 2] = vi + PlanetTile::PHYSICS_SIZE;


			// Bottom Right
			indices[i + 0 + 3] = vi + 1 + PlanetTile::PHYSICS_SIZE;
			// Right
			indices[i + 1 + 3] = vi + 1;
			// Bottom
			indices[i + 2 + 3] = vi + PlanetTile::PHYSICS_SIZE;

		}
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
