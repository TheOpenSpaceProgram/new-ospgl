#include "PlanetTilePath.h"

double sizeAtPathDepth(size_t depth)
{
	return 1.0 / pow(2, depth);
}


size_t PlanetTilePath::get_depth() const
{
	return path.size();
}

glm::dvec2 PlanetTilePath::get_min() const
{
	glm::dvec2 out = glm::dvec2(0.0, 0.0);

	for (size_t i = 0; i < path.size(); i++)
	{
		double size = sizeAtPathDepth(i + 1);

		if (path[i] == NORTH_WEST)
		{
			// Nothing
		}
		else if (path[i] == NORTH_EAST)
		{
			out.x += size;
		}
		else if (path[i] == SOUTH_WEST)
		{
			out.y += size;
		}
		else
		{
			out.x += size;
			out.y += size;
		}
	}

	return out;
}



double PlanetTilePath::get_size() const
{
	return sizeAtPathDepth(path.size());
}

bool arePathsEqual(const std::vector<QuadTreeQuadrant>& a, const std::vector<QuadTreeQuadrant>& b)
{
	if (a.size() == b.size())
	{
		for (size_t i = 0; i < a.size(); i++)
		{
			if (a[i] != b[i])
			{
				return false;
			}
		}

		return true;
	}
	else
	{
		return false;
	}
}

bool operator==(const PlanetTilePath& a, const PlanetTilePath& b)
{
	return a.side == b.side && arePathsEqual(a.path, b.path);
}


glm::vec3 PlanetTilePath::get_tile_rotation() const
{
	// Tiles look by default into the positive Z so...
	float rot = glm::radians(90.0f);

	if (side == PX)
	{
		return glm::vec3(0.0f, rot, 0.0f);
	}
	else if (side == NX)
	{
		return glm::vec3(0.0f, -rot, 0.0f);
	}
	else if (side == PY)
	{
		return glm::vec3(rot, 0.0f, 0.0f);
	}
	else if (side == NY)
	{
		return glm::vec3(-rot, 0.0f, 0.0f);
	}
	else if (side == PZ)
	{
		return glm::vec3(0.0f, 0.0f, 0.0f);
	}
	else if (side == NZ)
	{
		return glm::vec3(0.0f, rot * 2.0f, 0.0f);
	}

	return glm::vec3(0.0f, 0.0f, 0.0f);
}

glm::vec3 PlanetTilePath::get_tile_postrotation() const
{
	float r_90 = glm::radians(90.0f);

	if (side == PX)
	{
		return glm::vec3(0.0f, 0.0f, 0.0f);
	}
	else if (side == NX)
	{
		return glm::vec3(0.0f, r_90 * 2.0f, 0.0f);
	}
	else if (side == PY)
	{
		return glm::vec3(r_90 * 2.0f, -r_90, 0.0f);
	}
	else if (side == NY)
	{
		return glm::vec3(r_90 * 2.0f, -r_90, 0.0f);
	}
	else if (side == PZ)
	{
		return glm::vec3(0.0f, 0.0f, 0.0f);
	}
	else if (side == NZ)
	{
		return glm::vec3(0.0f, 0.0f, 0.0f);
	}

	return glm::vec3(0.0f, 0.0f, 0.0f);
}

glm::vec3 PlanetTilePath::get_tile_origin() const
{
	return glm::vec3(0.0f, 0.0f, 0.0f);
}

glm::vec3 PlanetTilePath::get_tile_translation(bool get_spheric) const
{
	glm::vec2 deviation = glm::vec2((get_min().x - 0.5f) * 2.0f, (get_min().y - 0.5f) * 2.0f);
	//deviation += path.getSize() / 2.0f;

	glm::vec3 cubic;

	if (side == PX)
	{
		cubic = glm::vec3(1.0f, -deviation.y, -deviation.x);
	}
	else if (side == NX)
	{
		cubic = glm::vec3(1.0f, deviation.y, deviation.x);
	}
	else if (side == PY)
	{
		cubic = glm::vec3(deviation.x, 1.0f, deviation.y);
	}
	else if (side == NY)
	{
		cubic = glm::vec3(deviation.x, 1.0f, deviation.y);
	}
	else if (side == PZ)
	{
		cubic = glm::vec3(deviation.x, -deviation.y, 1.0f);
	}
	else if (side == NZ)
	{
		cubic = glm::vec3(-deviation.x, -deviation.y, -1.0f);
	}

	glm::vec3 spheric = MathUtil::cube_to_sphere(cubic);

	if (get_spheric)
	{
		return spheric;
	}

	return cubic;
}

glm::vec3 PlanetTilePath::get_tile_scale() const
{
	float scale = (float)get_size() * 2.0f;

	if (side == PX)
	{
		return glm::vec3(scale, -scale, scale);
	}
	else if (side == NX)
	{
		return glm::vec3(scale, scale, scale);
	}
	else if (side == PY)
	{
		return glm::vec3(scale, scale, scale);
	}
	else if (side == NY)
	{
		return glm::vec3(scale, -scale, scale);
	}
	else if (side == PZ)
	{
		return glm::vec3(scale, -scale, scale);
	}
	else if (side == NZ)
	{
		return glm::vec3(scale, -scale, scale);
	}
	
	return glm::vec3(scale, scale, scale);
}

glm::vec3 PlanetTilePath::get_tile_postscale() const
{
	if (side == PY)
	{
		return glm::vec3(1.0f, -1.0f, 1.0f);
	}
	else if (side == NY)
	{
		return glm::vec3(1.0f, 1.0f, -1.0f);
	}
	else if (side == NX)
	{
		return glm::vec3(1.0f, 1.0f, -1.0f);
	}

	return glm::vec3(1.0f, 1.0f, 1.0f);
}

glm::mat4 PlanetTilePath::get_model_matrix() const
{
	glm::mat4 translation_mat = glm::translate(glm::mat4(), get_tile_translation(false));
	glm::mat4 scale_mat = glm::scale(glm::mat4(), get_tile_scale());
	glm::mat4 origin_mat = glm::translate(glm::mat4(), get_tile_origin());
	glm::mat4 rotation_mat = glm::toMat4(glm::quat(get_tile_rotation()));
	glm::mat4 postscale_mat = glm::scale(glm::mat4(), get_tile_postscale());
	glm::mat4 postrotate_mat = glm::toMat4(glm::quat(get_tile_postrotation()));

	glm::mat4 model = postrotate_mat * postscale_mat * translation_mat * rotation_mat * scale_mat * origin_mat;

	return model;
}

glm::mat4 PlanetTilePath::get_model_spheric_matrix() const
{
	glm::mat4 translation_mat_sph = glm::translate(glm::mat4(), get_tile_translation(true));
	glm::mat4 scale_mat = glm::scale(glm::mat4(), get_tile_scale());
	glm::mat4 origin_mat = glm::translate(glm::mat4(), get_tile_origin());
	glm::mat4 rotation_mat = glm::toMat4(glm::quat(get_tile_rotation()));
	glm::mat4 postscale_mat = glm::scale(glm::mat4(), get_tile_postscale());
	glm::mat4 postrotate_mat = glm::toMat4(glm::quat(get_tile_postrotation()));

	glm::mat4 model_spheric = postrotate_mat * postscale_mat * translation_mat_sph * rotation_mat * scale_mat * origin_mat;

	return model_spheric;
}