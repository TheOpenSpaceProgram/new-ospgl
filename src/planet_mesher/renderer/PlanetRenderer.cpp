#include "PlanetRenderer.h"
#include "../../util/Logger.h"
#include "../../assets/AssetManager.h"

void PlanetRenderer::render(PlanetTileServer& server, QuadTreePlanet& planet, glm::dmat4 proj_view, 
	glm::dmat4 wmodel, glm::dmat4 normal_matrix, glm::dmat4 rot_tform,
	float far_plane, glm::dvec3 camera_pos,
	PlanetConfig& config, double time, glm::vec3 light_dir)
{
	auto render_tiles = planet.get_all_render_leaf_paths();

	// Renderer really needs the tiles so some tiny
	// lags could be noticed by the user if there is
	// a thread that happens to be pushing a new tile
	//
	// TODO: Test another configuration with the mutex lock
	// inside the loop so we expend less time locked
	// at the cost of having to lock every iteration
	{

		auto tiles_w = server.tiles.get();

		shader->use();
		shader->setFloat("f_coef", 2.0f / glm::log2(far_plane + 1.0f));
		shader->setVec3("camera_pos", (glm::vec3)(camera_pos / config.radius));
		shader->setFloat("atmo_radius", (float)(config.atmo.radius / config.radius));
		shader->setVec3("atmo_main_color", config.atmo.main_color);
		shader->setVec3("atmo_sunset_color", config.atmo.sunset_color);
		shader->setFloat("atmo_exponent", (float)config.atmo.exponent);
		shader->setFloat("sunset_exponent", (float)config.atmo.sunset_exponent);
		shader->setVec3("light_dir", light_dir);
		shader->setMat4("normal_tform", normal_matrix);

		bool cw_mode = false;
		glFrontFace(GL_CCW);
		for (size_t i = 0; i < render_tiles.size(); i++)
		{
			auto it = tiles_w->find(render_tiles[i]);
			if (it == tiles_w->end())
			{
				// Really should not happen on normal
				// gameplay, but it can happen when a 
				// very sharp LOD change happens
				// For example, teleporting to a surface
				// Visually, it probably is a small flicker
				continue;
			}
			auto tile = it->second;
			auto path = it->first;

			glm::dmat4 model = path.get_model_spheric_matrix();



			if (!tile->is_uploaded())
			{
				continue;
			}

			if (tile->clockwise && !cw_mode)
			{
				glFrontFace(GL_CW);
				cw_mode = true;
			}

			if (!tile->clockwise && cw_mode)
			{
				glFrontFace(GL_CCW);
				cw_mode = false;
			}



			shader->setMat4("tform", (glm::mat4)(proj_view * wmodel * model));
			shader->setMat4("m_tform", (glm::mat4)(model));
			shader->setMat4("rotm_tform", (glm::mat4)(rot_tform * model));

			glm::vec3 tile_i = glm::vec3(path.get_min(), (float)path.get_depth());

			shader->setVec3("tile", tile_i);

			glBindVertexArray(vao);
			glBindVertexBuffer(0, tile->vbo, 0, sizeof(PlanetTileVertex));
			glBindVertexBuffer(1, uv_bo, 0, sizeof(glm::vec2));
			glDrawElements(GL_TRIANGLES, (GLsizei)indices.size(), GL_UNSIGNED_SHORT, (void*)0);
			glBindVertexArray(0);
			glBindBuffer(GL_VERTEX_ARRAY, 0);



		}

		if (config.surface.has_water)
		{
			// Draw water, another pass to only switch shaders once
			water_shader->use();
			water_shader->setFloat("f_coef", 2.0f / glm::log2(far_plane + 1.0f));
			water_shader->setVec3("camera_pos", (glm::vec3)(camera_pos / config.radius));
			water_shader->setFloat("time", (float)time);
			water_shader->setFloat("atmo_radius", (float)(config.atmo.radius / config.radius));
			water_shader->setVec3("atmo_main_color", config.atmo.main_color);
			water_shader->setVec3("atmo_sunset_color", config.atmo.sunset_color);
			water_shader->setFloat("atmo_exponent", (float)config.atmo.exponent);
			water_shader->setFloat("sunset_exponent", (float)config.atmo.sunset_exponent);
			water_shader->setVec3("light_dir", light_dir);
			water_shader->setMat4("normal_tform", normal_matrix);

			cw_mode = false;
			glFrontFace(GL_CCW);
			for (size_t i = 0; i < render_tiles.size(); i++)
			{
				auto it = tiles_w->find(render_tiles[i]);
				if (it == tiles_w->end())
				{
					continue;
				}
				auto tile = it->second;
				auto path = it->first;

				glm::dmat4 model = path.get_model_spheric_matrix();

				if (tile->water_vbo == 0)
				{
					continue;
				}

				if (tile->clockwise && !cw_mode)
				{
					glFrontFace(GL_CW);
					cw_mode = true;
				}

				if (!tile->clockwise && cw_mode)
				{
					glFrontFace(GL_CCW);
					cw_mode = false;
				}

				// Can be used for tides, or simple waves as we do here
				double sfactor = 1.0 + sin(time * 0.3) * 0.00000025;

				glm::dmat4 t_model = glm::dmat4(1.0f);
				t_model = glm::scale(t_model, glm::dvec3(sfactor, sfactor, sfactor));

				water_shader->setMat4("tform", (glm::mat4)(proj_view * wmodel * t_model * model));
				water_shader->setMat4("tform_scaled", (glm::mat4)(model));
				water_shader->setMat4("rotm_tform", (glm::mat4)(rot_tform * model));
				water_shader->setInt("clockwise", tile->clockwise ? 1 : 0);
			
				glm::vec3 tile_i = glm::vec3(path.get_min(), (float)path.get_depth());

				water_shader->setVec3("tile", tile_i);

				glBindVertexArray(water_vao);
				glBindVertexBuffer(0, tile->water_vbo, 0, sizeof(PlanetTileWaterVertex));
				glBindVertexBuffer(1, uv_bo, 0, sizeof(glm::vec2));
				glDrawElements(GL_TRIANGLES, (GLsizei)bulk_index_count, GL_UNSIGNED_SHORT, (void*)0);
				glBindVertexArray(0);
				glBindBuffer(GL_VERTEX_ARRAY, 0);



			}
		}
	}

	glFrontFace(GL_CCW);
}


void PlanetRenderer::generate_and_upload_index_buffer()
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

	std::array<glm::vec2, PlanetTile::TILE_SIZE * PlanetTile::TILE_SIZE> uvs;

	// Bulk UVs
	for (size_t y = 0; y < PlanetTile::TILE_SIZE; y++)
	{
		for (size_t x = 0; x < PlanetTile::TILE_SIZE; x++)
		{
			uint16_t vi = (uint16_t)(y * PlanetTile::TILE_SIZE + x);

			uvs[vi] = glm::vec2(
				((double)x / (double)PlanetTile::TILE_SIZE) * 1000.0,
				((double)y / (double)PlanetTile::TILE_SIZE) * 1000.0);

		}
	}

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
	
	glGenVertexArrays(1, &vao);
	glGenVertexArrays(1, &water_vao);
	glGenBuffers(1, &uv_bo);
	glGenBuffers(1, &ebo);

	glBindVertexArray(vao);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices[0]) * indices.size(), indices.data(), GL_STATIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, uv_bo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(uvs[0]) * uvs.size(), uvs.data(), GL_STATIC_DRAW);


	// position
	glEnableVertexAttribArray(0);
	glVertexAttribFormat(0, 3, GL_FLOAT, GL_FALSE, offsetof(PlanetTileVertex, pos));
	glVertexAttribBinding(0, 0);
	// normal
	glEnableVertexAttribArray(1);
	glVertexAttribFormat(1, 3, GL_FLOAT, GL_FALSE, offsetof(PlanetTileVertex, nrm));
	glVertexAttribBinding(1, 0);
	// color
	glEnableVertexAttribArray(2);
	glVertexAttribFormat(2, 3, GL_FLOAT, GL_FALSE, offsetof(PlanetTileVertex, col));
	glVertexAttribBinding(2, 0);

	// UV sourced from buffer 1
	glEnableVertexAttribArray(3);
	glVertexAttribFormat(3, 2, GL_FLOAT, GL_FALSE, 0);
	glVertexAttribBinding(3, 1);

	glBindVertexArray(0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	// Water
	glBindVertexArray(water_vao);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);

	// position
	glEnableVertexAttribArray(0);
	glVertexAttribFormat(0, 3, GL_FLOAT, GL_FALSE, offsetof(PlanetTileWaterVertex, pos));
	glVertexAttribBinding(0, 0);
	// normal
	glEnableVertexAttribArray(1);
	glVertexAttribFormat(1, 3, GL_FLOAT, GL_FALSE, offsetof(PlanetTileWaterVertex, nrm));
	glVertexAttribBinding(1, 0);
	// depth
	glEnableVertexAttribArray(2);
	glVertexAttribFormat(2, 1, GL_FLOAT, GL_FALSE, offsetof(PlanetTileWaterVertex, depth));
	glVertexAttribBinding(2, 0);

	// UV sourced from buffer 1
	glEnableVertexAttribArray(3);
	glVertexAttribFormat(3, 2, GL_FLOAT, GL_FALSE, 0);
	glVertexAttribBinding(3, 1);

	glBindVertexArray(0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}


PlanetRenderer::PlanetRenderer()
{
	generate_and_upload_index_buffer();
	shader = assets->get<Shader>("core", "shaders/planet/tile.vs");
	water_shader = assets->get<Shader>("core", "shaders/planet/water.vs");
}


PlanetRenderer::~PlanetRenderer()
{
	glDeleteBuffers(1, &ebo);
	glDeleteVertexArrays(1, &vao);
}
