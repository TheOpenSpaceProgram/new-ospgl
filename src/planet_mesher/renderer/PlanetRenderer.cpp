#include "PlanetRenderer.h"
#include "../../util/Logger.h"
#include "../../assets/AssetManager.h"

#include <util/DebugDrawer.h>
#include <imgui/imgui.h>
#include <renderer/Renderer.h>

void PlanetRenderer::render(PlanetTileServer& server, QuadTreePlanet& planet, glm::dmat4 proj_view,
							glm::dmat4 wmodel, glm::dmat4 normal_matrix, glm::dmat4 rot_tform,
							float far_plane, glm::dvec3 camera_pos,
							ElementConfig& config, double time, glm::vec3 light_dir, glm::dmat4 dmodel,
							double rot)
{
	/*ImGui::Begin("Planet Surface", nullptr, ImGuiWindowFlags_AlwaysAutoResize);
	planet.do_imgui(nullptr);
	ImGui::End();*/

	auto render_tiles = planet.get_all_render_leaf_paths();

	// Renderer really needs the tiles so some tiny
	// lags could be noticed by the user if there is
	// a thread that happens to be pushing a new tile
	//
	// TODO: Test another configuration with the mutex lock
	// inside the loop so we expend less time locked
	// at the cost of having to lock every iteration
	{
		float detail_scale = 2000.0f;
		float detail_fade = 45000.0f;
		detail_fade = detail_fade * detail_fade;
		float triplanar_y_mult = 0.28f;
		float triplanar_power = 4.0f;

		auto tiles_w = server.tiles.get();

		shader->use();
		shader->setFloat("f_coef", 2.0f / glm::log2(far_plane + 1.0f));
		shader->setVec3("camera_pos", (glm::vec3)(camera_pos / config.radius));
		shader->setFloat("atmo_radius", (float)(config.atmo.radius / config.radius));
		shader->setFloat("planet_radius", 1.0f);
		shader->setVec3("kRlh", config.atmo.kRlh);
		shader->setFloat("kMie", config.atmo.kMie);
		shader->setFloat("atmo_exponent", (float)config.atmo.exponent);
		shader->setFloat("sunset_exponent", (float)config.atmo.sunset_exponent);
		shader->setVec3("light_dir", light_dir);
		shader->setMat4("normal_tform", normal_matrix);

		// TODO: Handle non-spherical planets with some kind of ellipsoid
		glm::dvec3 triplanar_up = camera_pos;
		if(osp->renderer->quality.use_planet_detail_map || osp->renderer->quality.use_planet_detail_normal)
		{
			shader->setFloat("detail_scale", detail_scale);
			shader->setFloat("detail_fade", detail_fade);
			shader->setFloat("triplanar_power", triplanar_power);
			shader->setFloat("triplanar_y_mult", triplanar_y_mult);

			// Triplanar textures
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, top_tex->id);
			glActiveTexture(GL_TEXTURE1);
			glBindTexture(GL_TEXTURE_2D, cliff_tex->id);
			glActiveTexture(GL_TEXTURE2);
			glBindTexture(GL_TEXTURE_2D, top_nrm->id);
			glActiveTexture(GL_TEXTURE3);
			glBindTexture(GL_TEXTURE_2D, cliff_nrm->id);

			shader->setInt("top_tex", 0);
			shader->setInt("cliff_tex", 1);
			shader->setInt("top_nrm", 2);
			shader->setInt("cliff_nrm", 3);
		}

		bool cw_mode = false;
		glFrontFace(GL_CCW);

		if(osp->renderer->quality.use_planet_detail_normal || osp->renderer->quality.use_planet_detail_map)
		{
			// Adjust triplanar up so it's tiled
			// TODO: This solution causes quite a bit of jumping when moving between
			// TODO: "detail tile boundaries". Need to think a better way but works for now
			// We rotate triplaner up with the planet to avoid offsets, remember the coordinate space
			glm::dvec3 nds = glm::dvec3(detail_scale, detail_scale, detail_scale) * 100.0;
			triplanar_up -= glm::mod(triplanar_up, nds);
			triplanar_up = glm::normalize(triplanar_up);
			// Now be careful, the planet is rotating, so need to keep that in mind and rotate
			// the whole thing around the old y axis by rot
			glm::dmat4 tri_matrix = MathUtil::rotate_from_to(triplanar_up, glm::dvec3(0, 1, 0));
			glm::dmat4 tri_normal_matrix = MathUtil::rotate_from_to(glm::normalize(camera_pos), glm::dvec3(0, 1, 0));
			// This is the cosine of the angle between the pole and our position
			glm::dmat4 rot_offset = glm::rotate(-rot, glm::dvec3(0, 1, 0));
			tri_matrix = tri_matrix * rot_offset;
			// tri_matrix now points into a weird direction but that's fixed with the planet
			// I figured out using my fingers as axes that this is the correction factor!
			// (Actually makes sense as the planet rotation will "weaken" the triplanar_up rotation
			double a = glm::abs(glm::dot(glm::normalize(triplanar_up), glm::dvec3(0, 1, 0)));
			glm::dmat4 correction = glm::rotate(glm::acos(a), glm::normalize(triplanar_up));
			tri_matrix = correction * tri_matrix;

			shader->setMat4("tri_matrix", tri_matrix);
			shader->setMat4("tri_nrm_matrix", tri_normal_matrix);
			shader->setMat4("inverse_tri_nrm_matrix", glm::inverse(tri_normal_matrix));
		}

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
			// We also apply the camera tform, used by the deferred renderer
			glm::dmat4 deferred_model = wmodel * model;

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
			shader->setMat4("m_tform", (glm::mat4)(deferred_model));

			shader->setMat4("m_tform_scaled", (glm::mat4)(wmodel * model));
			shader->setMat4("rotm_tform", (glm::mat4)(rot_tform * model));

			glm::dvec3 tile_or = wmodel * model * glm::dvec4(0.5, 0.5, 0.0, 1.0);
			// We use a reasonalbe distance to prevent gaps but also not show detail very far away
			// to reduce GPU load
			bool do_detail = glm::dot(tile_or, tile_or) < detail_fade * 20;
			shader->setInt("do_detail", do_detail);

			glm::vec3 tile_i = glm::vec3(path.get_min(), (float)path.get_depth());

			shader->setVec3("tile", tile_i);
			glBindVertexArray(vao);
			glBindVertexBuffer(0, tile->vbo, 0, sizeof(PlanetTileVertex));
			glBindVertexBuffer(1, uv_bo, 0, sizeof(glm::vec2));
			glDrawElements(GL_TRIANGLES, (GLsizei)indices.size(), GL_UNSIGNED_SHORT, (void*)0);
			glBindVertexArray(0);



		}

		if (config.surface.has_water)
		{
			// Draw water, another pass to only switch shaders once
			water_shader->use();
			water_shader->setFloat("f_coef", 2.0f / glm::log2(far_plane + 1.0f));
			water_shader->setVec3("camera_pos", (glm::vec3)(camera_pos / config.radius));
			water_shader->setFloat("time", (float)time);
			water_shader->setFloat("atmo_radius", (float)(config.atmo.radius / config.radius));
			water_shader->setFloat("planet_radius", 1.0f);
			water_shader->setVec3("kRlh", config.atmo.kRlh);
			water_shader->setFloat("kMie", config.atmo.kMie);
			water_shader->setFloat("atmo_exponent", (float)config.atmo.exponent);
			water_shader->setFloat("sunset_exponent", (float)config.atmo.sunset_exponent);
			water_shader->setVec3("light_dir", light_dir);

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
				glm::dmat4 deferred_model = wmodel * model;

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
				double sfactor = 1.0 + sin(time * 0.3) * 0.000000025;

				glm::dmat4 t_model = glm::dmat4(1.0f);
				t_model = glm::scale(t_model, glm::dvec3(sfactor, sfactor, sfactor));

				water_shader->setMat4("tform", (glm::mat4)(proj_view * wmodel * t_model * model));
				water_shader->setMat4("deferred_tform", (glm::mat4)(deferred_model));
				water_shader->setMat4("rotm_tform", (glm::mat4)(rot_tform * model));
				water_shader->setInt("clockwise", tile->clockwise ? 1 : 0);
			
				glm::vec3 tile_i = glm::vec3(path.get_min(), (float)path.get_depth());

				water_shader->setVec3("tile", tile_i);

				glBindVertexArray(water_vao);
				glBindVertexBuffer(0, tile->water_vbo, 0, sizeof(PlanetTileWaterVertex));
				glBindVertexBuffer(1, uv_bo, 0, sizeof(glm::vec2));
				glDrawElements(GL_TRIANGLES, (GLsizei)bulk_index_count, GL_UNSIGNED_SHORT, (void*)0);
				glBindVertexArray(0);



			}
		}
		
	}

	glFrontFace(GL_CCW);
}


void PlanetRenderer::generate_and_upload_index_buffer()
{
	PlanetTile::generate_index_array_with_skirts(indices, bulk_index_count);

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
	// Planet's global UV and texture chooser
	glEnableVertexAttribArray(3);
	glVertexAttribFormat(3, 3, GL_FLOAT, GL_FALSE, offsetof(PlanetTileVertex, planet_uv_tex));
	glVertexAttribBinding(3, 0);

	// PBR sourced from buffer 1 (PBR pipeline)
	glEnableVertexAttribArray(4);
	glVertexAttribFormat(4, 2, GL_FLOAT, GL_FALSE, 0);
	glVertexAttribBinding(4, 1);

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

	// PBR sourced from buffer 1
	glEnableVertexAttribArray(3);
	glVertexAttribFormat(3, 2, GL_FLOAT, GL_FALSE, 0);
	glVertexAttribBinding(3, 1);

	glBindVertexArray(0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}


PlanetRenderer::PlanetRenderer()
{
	generate_and_upload_index_buffer();
	shader = osp->assets->get<Shader>("core", "shaders/planet/tile.vs");
	water_shader = osp->assets->get<Shader>("core", "shaders/planet/water.vs");
	cliff_tex = AssetHandle<Image>("debug_system:planets/earth/textures/cliff.png");
	top_tex = AssetHandle<Image>("debug_system:planets/earth/textures/grass.png");
	cliff_nrm = AssetHandle<Image>("debug_system:planets/earth/textures/cliff_nrm.png");
	top_nrm= AssetHandle<Image>("debug_system:planets/earth/textures/grass_nrm.png");
}


PlanetRenderer::~PlanetRenderer()
{
	glDeleteBuffers(1, &ebo);
	glDeleteVertexArrays(1, &vao);
}
