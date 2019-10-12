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


glm::dvec3 PlanetTilePath::get_tile_rotation() const
{
	// Tiles look by default into the positive Z so...
	double rot = glm::radians(90.0);

	if (side == PX)
	{
		return glm::dvec3(0.0, rot, 0.0);
	}
	else if (side == NX)
	{
		return glm::dvec3(0.0, -rot, 0.0);
	}
	else if (side == PY)
	{
		return glm::dvec3(rot, 0.0, 0.0);
	}
	else if (side == NY)
	{
		return glm::dvec3(-rot, 0.0, 0.0);
	}
	else if (side == PZ)
	{
		return glm::dvec3(0.0, 0.0, 0.0);
	}
	else if (side == NZ)
	{
		return glm::dvec3(0.0, rot * 2.0, 0.0);
	}

	return glm::dvec3(0.0, 0.0, 0.0);
}

glm::dvec3 PlanetTilePath::get_tile_postrotation() const
{
	double r_90 = glm::radians(90.0);

	if (side == PX)
	{
		return glm::dvec3(0.0, 0.0, 0.0);
	}
	else if (side == NX)
	{
		return glm::dvec3(0.0, r_90 * 2.0, 0.0);
	}
	else if (side == PY)
	{
		return glm::dvec3(r_90 * 2.0, -r_90, 0.0);
	}
	else if (side == NY)
	{
		return glm::dvec3(r_90 * 2.0, -r_90, 0.0);
	}
	else if (side == PZ)
	{
		return glm::dvec3(0.0, 0.0, 0.0);
	}
	else if (side == NZ)
	{
		return glm::dvec3(0.0, 0.0, 0.0);
	}

	return glm::dvec3(0.0, 0.0, 0.0);
}

glm::dvec3 PlanetTilePath::get_tile_origin() const
{
	return glm::dvec3(0.0, 0.0, 0.0);
}

glm::dvec3 PlanetTilePath::get_tile_translation(bool get_spheric) const
{
	glm::dvec2 deviation = glm::dvec2((get_min().x - 0.5f) * 2.0f, (get_min().y - 0.5f) * 2.0f);
	//deviation += path.getSize() / 2.0f;

	glm::dvec3 cubic;

	if (side == PX)
	{
		cubic = glm::dvec3(1.0f, -deviation.y, -deviation.x);
	}
	else if (side == NX)
	{
		cubic = glm::dvec3(1.0f, deviation.y, deviation.x);
	}
	else if (side == PY)
	{
		cubic = glm::dvec3(deviation.x, 1.0f, deviation.y);
	}
	else if (side == NY)
	{
		cubic = glm::dvec3(deviation.x, 1.0f, deviation.y);
	}
	else if (side == PZ)
	{
		cubic = glm::dvec3(deviation.x, -deviation.y, 1.0f);
	}
	else if (side == NZ)
	{
		cubic = glm::dvec3(-deviation.x, -deviation.y, -1.0f);
	}

	glm::dvec3 spheric = MathUtil::cube_to_sphere(cubic);

	if (get_spheric)
	{
		return spheric;
	}

	return cubic;
}

glm::dvec3 PlanetTilePath::get_tile_scale() const
{
	double scale = get_size() * 2.0;

	if (side == PX)
	{
		return glm::dvec3(scale, -scale, scale);
	}
	else if (side == NX)
	{
		return glm::dvec3(scale, scale, scale);
	}
	else if (side == PY)
	{
		return glm::dvec3(scale, scale, scale);
	}
	else if (side == NY)
	{
		return glm::dvec3(scale, -scale, scale);
	}
	else if (side == PZ)
	{
		return glm::dvec3(scale, -scale, scale);
	}
	else if (side == NZ)
	{
		return glm::dvec3(scale, -scale, scale);
	}
	
	return glm::dvec3(scale, scale, scale);
}

glm::dvec3 PlanetTilePath::get_tile_postscale() const
{
	if (side == PY)
	{
		return glm::dvec3(1.0f, -1.0f, 1.0f);
	}
	else if (side == NY)
	{
		return glm::dvec3(1.0f, 1.0f, -1.0f);
	}
	else if (side == NX)
	{
		return glm::dvec3(1.0f, 1.0f, -1.0f);
	}

	return glm::dvec3(1.0f, 1.0f, 1.0f);
}

glm::dmat4 PlanetTilePath::get_model_matrix() const
{
	glm::dmat4 translation_mat = glm::translate(glm::dmat4(), get_tile_translation(false));
	glm::dmat4 scale_mat = glm::scale(glm::dmat4(), get_tile_scale());
	glm::dmat4 origin_mat = glm::translate(glm::dmat4(), get_tile_origin());
	glm::dmat4 rotation_mat = glm::toMat4(glm::dquat(get_tile_rotation()));
	glm::dmat4 postscale_mat = glm::scale(glm::dmat4(), get_tile_postscale());
	glm::dmat4 postrotate_mat = glm::toMat4(glm::dquat(get_tile_postrotation()));

	glm::mat4 model = postrotate_mat * postscale_mat * translation_mat * rotation_mat * scale_mat * origin_mat;

	return model;
}

glm::dmat4 PlanetTilePath::get_model_spheric_matrix() const
{
	glm::dmat4 translation_mat_sph = glm::translate(glm::dmat4(), get_tile_translation(true));
	glm::dmat4 scale_mat = glm::scale(glm::dmat4(), get_tile_scale());
	glm::dmat4 origin_mat = glm::translate(glm::dmat4(), get_tile_origin());
	glm::dmat4 rotation_mat = glm::toMat4(glm::dquat(get_tile_rotation()));
	glm::dmat4 postscale_mat = glm::scale(glm::dmat4(), get_tile_postscale());
	glm::dmat4 postrotate_mat = glm::toMat4(glm::dquat(get_tile_postrotation()));

	glm::dmat4 model_spheric = postrotate_mat * postscale_mat * translation_mat_sph * rotation_mat * scale_mat * origin_mat;

	return model_spheric;
}