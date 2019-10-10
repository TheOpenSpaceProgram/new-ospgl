#include "PlanetRenderer.h"
#include "../../util/Logger.h"
#include "../../assets/AssetManager.h"

void PlanetRenderer::render(PlanetTileServer& server, QuadTreePlanet& planet, glm::mat4 projView)
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

			glm::mat4 model = path.get_model_spheric_matrix();

			bool cw_mode = false;
			glFrontFace(GL_CCW);

			if (tile->is_uploaded())
			{
				if (tile->clockwise && !cw_mode)
				{
					glFrontFace(GL_CW);
					cw_mode = true;
				}
				
				if(!tile->clockwise && cw_mode)
				{
					glFrontFace(GL_CCW);
					cw_mode = false;
				}

				shader->setMat4("tform", projView * model);


				glBindVertexArray(vao);
				glBindVertexBuffer(0, tile->vbo, 0, sizeof(PlanetTileVertex));
				glDrawElements(GL_TRIANGLES, (GLsizei)indices.size(), GL_UNSIGNED_SHORT, (void*)0);
				glBindVertexArray(0);
				glBindBuffer(GL_VERTEX_ARRAY, 0);
			}
			else
			{
				//logger->warn("Tile was not uploaded, there is a hole!");
			}


		}
	}

	glFrontFace(GL_CCW);
}


void PlanetRenderer::generate_and_upload_index_buffer()
{
	// Bulk indices
	for (size_t y = 0; y < PlanetTile::TILE_SIZE - 1; y++)
	{
		for (size_t x = 0; x < PlanetTile::TILE_SIZE - 1; x++)
		{
			uint16_t vi = (uint16_t)(y * PlanetTile::TILE_SIZE + x);
			size_t i = (y * PlanetTile::TILE_SIZE + x) * 6;

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

	// Now skirt indices
	// Y skirts
	/*for (size_t x = 0; x < PlanetTile::TILE_SIZE; x++)
	{
		int16_t vi_0 = (int16_t)x;
		int16_t vi_1 = (int16_t)((PlanetTile::TILE_SIZE - 1) * PlanetTile::TILE_SIZE + x);
		int16_t skirt_0_vi = (int16_t)PlanetTile::VERTEX_COUNT;
		int16_t skirt_1_vi = (int16_t)(PlanetTile::VERTEX_COUNT + PlanetTile::TILE_SIZE);
		// 0 Skirt

		// Left
		indices[vi_0 * 3 + 0] = vi_0;
		// Right
		indices[vi_0 * 3 + 1] = vi_0 + 1;
		// Skirt Left
		indices[vi_0 * 3 + 1] = skirt_0_vi + vi_0;


		// Right
		indices[vi_0 * 3 + 0] = vi_0 + 1;
		// Skirt Right
		indices[vi_0 * 3 + 1] = skirt_0_vi + vi_0 + 1;
		// Skirt Left
		indices[vi_0 * 3 + 1] = skirt_0_vi + vi_0;

		// +Y Skirt

		// Left
		indices[vi_0 * 3 + 0] = vi_1;
		// Skirt Left
		indices[vi_0 * 3 + 1] = skirt_1_vi + vi_0;
		// Right
		indices[vi_0 * 3 + 1] = vi_1 + 1;

		// Right
		indices[vi_0 * 3 + 0] = vi_1 + 1;
		// Skirt Right
		indices[vi_0 * 3 + 1] = skirt_1_vi + vi_0 + 1;
		// Skirt Left
		indices[vi_0 * 3 + 1] = skirt_1_vi + vi_0;
	}

	// X skirts
	for (size_t y = 0; y < PlanetTile::TILE_SIZE; y++)
	{
		int16_t vi_0 = (int16_t)(y * PlanetTile::TILE_SIZE);
		int16_t vi_1 = (int16_t)(y * PlanetTile::TILE_SIZE + PlanetTile::TILE_SIZE - 1);
		int16_t skirt_0_vi = (int16_t)(PlanetTile::VERTEX_COUNT + PlanetTile::TILE_SIZE * 2);
		int16_t skirt_1_vi = (int16_t)(PlanetTile::VERTEX_COUNT + PlanetTile::TILE_SIZE * 3);
		// 0 Skirt

		// Top
		indices[vi_0 * 3 + 0] = vi_0;
		// Skirt Top
		indices[vi_0 * 3 + 1] = skirt_0_vi + vi_0;
		// Bottom
		indices[vi_0 * 3 + 1] = vi_0 + PlanetTile::TILE_SIZE;


		// Bottom
		indices[vi_0 * 3 + 0] = vi_0 + PlanetTile::TILE_SIZE;
		// Skirt Top
		indices[vi_0 * 3 + 1] = skirt_0_vi + vi_0;
		// Skirt Bottom
		indices[vi_0 * 3 + 1] = skirt_0_vi + vi_0 + 1;

		// +X Skirt

		// Top
		indices[vi_0 * 3 + 0] = vi_1 + 1;
		// Skirt Top
		indices[vi_0 * 3 + 1] = skirt_1_vi + vi_0;
		// Bottom
		indices[vi_0 * 3 + 1] = vi_1 + PlanetTile::TILE_SIZE;

		// Bottom
		indices[vi_0 * 3 + 0] = vi_1 + PlanetTile::TILE_SIZE;
		// Skirt Top
		indices[vi_0 * 3 + 1] = skirt_1_vi + vi_0;
		// Skirt Bottom
		indices[vi_0 * 3 + 1] = skirt_1_vi + vi_0 + 1;
	}
	*/

	glGenVertexArrays(1, &vao);
	glGenBuffers(1, &ebo);

	glBindVertexArray(vao);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices[0]) * indices.size(), indices.data(), GL_STATIC_DRAW);

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
	// tex
	glEnableVertexAttribArray(3);
	glVertexAttribFormat(3, 2, GL_FLOAT, GL_FALSE, offsetof(PlanetTileVertex, uv));
	glVertexAttribBinding(3, 0);

	glBindVertexArray(0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

}


PlanetRenderer::PlanetRenderer()
{
	generate_and_upload_index_buffer();
	shader = assets->get<Shader>("planet/tile");

}


PlanetRenderer::~PlanetRenderer()
{
	glDeleteBuffers(1, &ebo);
	glDeleteVertexArrays(1, &vao);
}
