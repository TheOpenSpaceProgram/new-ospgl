#include "LuaDebugDrawer.h"
#include "../../util/DebugDrawer.h"

void LuaDebugDrawer::load_to(sol::table& table)
{
	table.set_function("add_point", [](glm::dvec3 p, glm::dvec3 color)
	{
		debug_drawer->add_point(p, (glm::vec3)color);
	});

	table.set_function("add_line", sol::overload(
		[](glm::dvec3 p0, glm::dvec3 p1, glm::dvec3 color)
		{
			debug_drawer->add_line(p0, p1, (glm::vec3)color);
		},
		[](glm::dvec3 p0, glm::dvec3 p1, glm::dvec3 acolor, glm::dvec3 bcolor)
		{
			debug_drawer->add_line(p0, p1, (glm::vec3)acolor, (glm::vec3)bcolor);
		})
	);

	table.set_function("add_arrow", [](glm::dvec3 p0, glm::dvec3 p1, glm::dvec3 color)
	{
		debug_drawer->add_arrow(p0, p1, (glm::vec3)color);
	});

	table.set_function("add_cone", [](glm::dvec3 base, glm::dvec3 tip, double radius, glm::dvec3 color)
	{
		debug_drawer->add_cone(base, tip, radius, (glm::vec3)color);
	});

	table.set_function("add_aabb", [](glm::dmat4 tform, glm::dvec3 half_extents, glm::dvec3 color)
	{
		debug_drawer->add_aabb(tform, half_extents, (glm::vec3)color);
	});

	table.set_function("add_box", [](glm::dmat4 tform, glm::dvec3 corner_1, glm::dvec3 corner_2, glm::dvec3 color)
	{
		debug_drawer->add_box(tform, corner_1, corner_2, (glm::vec3)color);
	});

}