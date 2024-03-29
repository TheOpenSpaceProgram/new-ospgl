#include "DebugDrawer.h"

DebugDrawer* debug_drawer;

void DebugDrawer::render(glm::dmat4 proj_view, glm::dmat4 c_model, float far_plane)
{

	if (draw_list.size() > 0)
	{
		if (points_vbo != 0)
		{
			glDeleteBuffers(1, &points_vbo);
			glDeleteBuffers(1, &lines_vbo);
			glDeleteVertexArrays(1, &points_vao);
			glDeleteVertexArrays(1, &lines_vao);
		}

		glGenBuffers(1, &points_vbo);
		glGenBuffers(1, &lines_vbo);
		glGenVertexArrays(1, &points_vao);
		glGenVertexArrays(1, &lines_vao);

		std::vector<DebugVertexf> points;
		std::vector<DebugVertexf> lines; //< Lines go in pairs

		for (size_t i = 0; i < draw_list.size(); i++)
		{
			if (draw_list[i].verts.size() == 1)
			{
				points.push_back(draw_list[i].verts[0].transform(c_model));
			}
			else
			{
				for (size_t j = 0; j < draw_list[i].verts.size(); j++)
				{
					lines.push_back(draw_list[i].verts[j].transform(c_model));
				}
			}
		}


		glBindVertexArray(points_vao);

		glBindBuffer(GL_ARRAY_BUFFER, points_vbo);
		glBufferData(GL_ARRAY_BUFFER, sizeof(DebugVertexf) * points.size(), points.data(), GL_STATIC_DRAW);

		// pos
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
		// color
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));

		glBindVertexArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, 0);

		glBindVertexArray(lines_vao);

		glBindBuffer(GL_ARRAY_BUFFER, lines_vbo);
		glBufferData(GL_ARRAY_BUFFER, sizeof(DebugVertexf) * lines.size(), lines.data(), GL_STATIC_DRAW);

		// pos
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
		// color
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));

		glBindVertexArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, 0);


		glPointSize(point_size);
		glLineWidth(line_size);

		shader->use();
		shader->setMat4("tform", proj_view);
		shader->setFloat("f_coef", 2.0f / glm::log2(far_plane + 1.0f));

		// Draw them
		glBindVertexArray(points_vao);
		glDrawArrays(GL_POINTS, 0, (GLsizei)points.size());
		glBindVertexArray(0);

		glBindVertexArray(lines_vao);
		glDrawArrays(GL_LINES, 0, (GLsizei)lines.size());
		glBindVertexArray(0);

		draw_list.clear();

	}

}

void DebugDrawer::add_cone(glm::dvec3 base, glm::dvec3 tip, double radius, glm::vec3 color, int verts)
{
	// We want lines from the tip to every single vertex in the 
	// base, and from every single vertex in the base to the next one
	DebugShape shape;

	glm::dvec3 up = glm::dvec3(0.0, 1.0, 0.0);
	double dot = glm::abs(glm::dot(glm::normalize(base - tip), glm::dvec3(0.0, 1.0, 0.0)));

	if (dot <= 1.1 && dot >= 0.9)
	{
		up = glm::dvec3(1.0, 0.0, 0.0);
	}

	glm::dmat4 rot = MathUtil::rotate_from_to(up, tip - base);

	glm::dvec3 prev;
	bool has_prev = false;
	for (int v = 0; v < verts + 1; v++)
	{
		double theta = ((double)v / (double)verts) * glm::two_pi<double>();

		glm::dvec3 flat;
		if (up == glm::dvec3(1.0, 0.0, 0.0))
		{
			flat = glm::dvec3(0.0, sin(theta) * radius, cos(theta) * radius);
		}
		else
		{
			flat = glm::dvec3(sin(theta) * radius, 0.0, cos(theta) * radius);
		}

		// Now rotate flat so that (0, 1, 0) becomes (tip - base)
		glm::dvec3 dim = glm::dvec3(rot * glm::dvec4(flat, 1.0)) + base;

		if (v < verts)
		{
			shape.verts.push_back(DebugVertex(tip, color));
			shape.verts.push_back(DebugVertex(dim, color));
		}

		if (has_prev)
		{
			// To previous
			shape.verts.push_back(DebugVertex(dim, color));
			shape.verts.push_back(DebugVertex(prev, color));
		}

		prev = dim;
		has_prev = true;
	}


	draw_list.push_back(shape);
}

void DebugDrawer::add_orbit(glm::dvec3 origin, KeplerOrbit orbit, glm::vec3 color, bool striped, int verts)
{
	DebugShape shape;

	KeplerElements elems = KeplerElements();
	elems.orbit = orbit;

	elems.eccentric_anomaly = 0.0;
	glm::dvec3 prev = elems.get_position();

	for (int v = 0; v < verts; v++)
	{
		double theta = (double)v / (double)(verts - 1);
		
		elems.eccentric_anomaly = theta * 360.0;

		glm::dvec3 pos = elems.get_position();

		if (v % 2 == 0 || !striped)
		{
			shape.verts.push_back(DebugVertex(prev + origin, color));
			shape.verts.push_back(DebugVertex(pos + origin, color));
		}


		prev = pos;
	}

	draw_list.push_back(shape);
}

void DebugDrawer::add_line(glm::dvec3 a, glm::dvec3 b, glm::vec3 color)
{
	DebugShape shape;
	shape.verts.push_back(DebugVertex(a, color));
	shape.verts.push_back(DebugVertex(b, color));
	draw_list.push_back(shape);
}

void DebugDrawer::add_line(glm::dvec3 a, glm::dvec3 b, glm::vec3 acolor, glm::vec3 bcolor)
{
	DebugShape shape;
	shape.verts.push_back(DebugVertex(a, acolor));
	shape.verts.push_back(DebugVertex(b, bcolor));
	draw_list.push_back(shape);
}

void DebugDrawer::add_arrow(glm::dvec3 a, glm::dvec3 b, glm::vec3 color)
{
	add_line(a, b, color);
	
	glm::dvec3 base = a + (b - a) * 0.85;
	
	add_cone(base, b, glm::distance(a, b) * 0.03, color, 8);
}

void DebugDrawer::add_point(glm::dvec3 a, glm::vec3 color)
{
	DebugShape shape;
	shape.verts.push_back(DebugVertex(a, color));
	draw_list.push_back(shape);
}

void DebugDrawer::add_transform(glm::dvec3 origin, glm::dmat4 tform, double length)
{
	glm::dvec3 x = tform * glm::dvec4(1.0, 0.0, 0.0, 1.0) * length;
	glm::dvec3 y = tform * glm::dvec4(0.0, 1.0, 0.0, 1.0) * length;
	glm::dvec3 z = tform * glm::dvec4(0.0, 0.0, 1.0, 1.0) * length;

	add_arrow(origin, origin + x, glm::vec3(1.0, 0.0, 0.0));
	add_arrow(origin, origin + y, glm::vec3(0.0, 1.0, 0.0));
	add_arrow(origin, origin + z, glm::vec3(0.0, 0.0, 1.0));
}

DebugDrawer::DebugDrawer()
{
	points_vbo = 0;
	points_vao = 0;
	lines_vbo = 0;
	lines_vao = 0;

	shader = osp->assets->get<Shader>("core", "shaders/debug.vs");
	point_size = 4.0f;
	line_size = 1.0f;

	debug_enabled = true;
}


DebugDrawer::~DebugDrawer()
{
}

void DebugDrawer::add_aabb(glm::dmat4 tranform, glm::dvec3 half_extents, glm::vec3 color)
{
	// Top square
	glm::dvec3 v0 = -half_extents;
	glm::dvec3 v1 = v0 + 2.0 * glm::dvec3(half_extents.x, 0, 0);
	glm::dvec3 v2 = v1 + 2.0 * glm::dvec3(0, half_extents.y, 0);
	glm::dvec3 v3 = v2 - 2.0 * glm::dvec3(half_extents.x, 0, 0);
	// Bottom square
	glm::dvec3 b0 = v0 + 2.0 * glm::dvec3(0, 0, half_extents.z);
	glm::dvec3 b1 = v1 + 2.0 * glm::dvec3(0, 0, half_extents.z);
	glm::dvec3 b2 = v2 + 2.0 * glm::dvec3(0, 0, half_extents.z);
	glm::dvec3 b3 = v3 + 2.0 * glm::dvec3(0, 0, half_extents.z);

	// Transform the points
	glm::dvec3 tv0 = glm::dvec3(tranform * glm::dvec4(v0, 1.0));
	glm::dvec3 tv1 = glm::dvec3(tranform * glm::dvec4(v1, 1.0));
	glm::dvec3 tv2 = glm::dvec3(tranform * glm::dvec4(v2, 1.0));
	glm::dvec3 tv3 = glm::dvec3(tranform * glm::dvec4(v3, 1.0));

	glm::dvec3 tb0 = glm::dvec3(tranform * glm::dvec4(b0, 1.0));
	glm::dvec3 tb1 = glm::dvec3(tranform * glm::dvec4(b1, 1.0));
	glm::dvec3 tb2 = glm::dvec3(tranform * glm::dvec4(b2, 1.0));
	glm::dvec3 tb3 = glm::dvec3(tranform * glm::dvec4(b3, 1.0));

	// Draw the top square
	add_line(tv0, tv1, color);
	add_line(tv1, tv2, color);
	add_line(tv2, tv3, color);
	add_line(tv3, tv0, color);

	// Draw the bototm square
	add_line(tb0, tb1, color);
	add_line(tb1, tb2, color);
	add_line(tb2, tb3, color);
	add_line(tb3, tb0, color);

	// Join both
	add_line(tv0, tb0, color);
	add_line(tv1, tb1, color);
	add_line(tv2, tb2, color);
	add_line(tv3, tb3, color);


}

void DebugDrawer::add_box(glm::dmat4 transform, glm::dvec3 corner_1, glm::dvec3 corner_2, glm::vec3 color)
{
	glm::dvec3 size = corner_2 - corner_1;
	glm::dvec3 center = (corner_1 + corner_2) * 0.5;
	// Move the transform to the new center
	glm::dmat4 ftransform = transform * glm::translate(glm::dmat4(), -center);
	add_aabb(ftransform, size * 0.5, color);
}

void create_global_debug_drawer()
{
	debug_drawer = new DebugDrawer();
}

void destroy_global_debug_drawer()
{
	delete debug_drawer;
}
