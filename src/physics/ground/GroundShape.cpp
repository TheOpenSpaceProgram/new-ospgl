#include "GroundShape.h"



void GroundShape::getAabb(const btTransform& t, btVector3& aabbMin, btVector3& aabbMax) const
{
	btVector3 trans = t.getOrigin();
	double s = body->config.radius + body->config.surface.max_height * 1.1;
	btVector3 size = btVector3(s, s, s);

	aabbMin = trans - size;
	aabbMax = trans + size;
}

#include <imgui/imgui.h>

void GroundShape::processAllTriangles(btTriangleCallback* callback, const btVector3& aabb_b₀, const btVector3& aabb_b₁) const
{
	btVector3 debug_b₀(btScalar(-BT_LARGE_FLOAT), btScalar(-BT_LARGE_FLOAT), btScalar(-BT_LARGE_FLOAT));
	btVector3 debug_b₁(btScalar(BT_LARGE_FLOAT), btScalar(BT_LARGE_FLOAT), btScalar(BT_LARGE_FLOAT));


	if (aabb_b₀ == debug_b₀ && aabb_b₁ == debug_b₁)
	{
		// We draw all loaded tiles
		for (auto it = server->cache.begin(); it != server->cache.end(); it++)
		{
			btVector3* verts = it->second->verts;

			// Return the triangles
			for (size_t i = 0; i < PlanetTile::PHYSICS_INDEX_COUNT; i += 3)
			{
				callback->processTriangle(&verts[i], (int)0, (int)(i / 3));
			}
		}
	}
	else
	{
		if (body->renderer.rocky == nullptr)
		{
			return;
		}

		glm::dvec3 aabb₀ = to_dvec3(aabb_b₀);
		glm::dvec3 aabb₁ = to_dvec3(aabb_b₁);

		glm::dvec3 Δaabb = aabb₁ - aabb₀;

		// We need to "project" the aabb into the sphere, to do so we build the box
		// with all its vertices
		glm::dvec3 aabb_box[8];
		aabb_box[0] = aabb₀;
		aabb_box[1] = aabb₀ + glm::dvec3(Δaabb.x, 0.0, 0.0);
		aabb_box[2] = aabb₀ + glm::dvec3(Δaabb.x, 0.0, Δaabb.z);
		aabb_box[3] = aabb₀ + glm::dvec3(0.0, 0.0, Δaabb.z);
		aabb_box[4] = aabb₀ + glm::dvec3(0.0, Δaabb.y, 0.0);
		aabb_box[5] = aabb₀ + glm::dvec3(Δaabb.x, Δaabb.y, 0.0);
		aabb_box[6] = aabb₀ + glm::dvec3(0.0, Δaabb.y, Δaabb.z);
		aabb_box[7] = aabb₁;

		// We create a new quad tree every time, perfomance is not that bad
		QuadTreePlanet quad_tree = QuadTreePlanet();


		glm::dvec3 normalized[8];

		size_t wanted_depth = body->config.surface.max_depth + PlanetTile::PHYSICS_GRAPHICS_RELATION;

		glm::dvec3 rel = glm::normalize(aabb_box[0]);

		// We now subdivide the quadtree down to a given depth using the
		// 3D points, and generate the edge triangles of every leaf node
		// of the wanted depth
		// Maybe we should do a line check or something like that
		// or a volume check to make sure the whole AABB gets high detail
		// BUT unless vehicles are totally massive this should
		// not really matter much
		// TODO: If we implement massive vehicles, write that code :P


		for (size_t i = 0; i < 8; i++)
		{
			normalized[i] = glm::normalize(aabb_box[i]);
			
			PlanetSide side = quad_tree.get_planet_side(normalized[i]);
			glm::dvec2 off = quad_tree.get_planet_side_offset(normalized[i], side);

			quad_tree.subdivide_to(off, side, wanted_depth);
		}

		/*ImGui::Begin("Lel", nullptr, ImGuiWindowFlags_AlwaysAutoResize);
		quad_tree.do_imgui(nullptr);
		ImGui::End();*/


		auto leafs = quad_tree.get_all_leafs();
		for (QuadTreeNode* leaf : leafs)
		{
			// We only want full depth leafs, the other ones dont
			// matter for physics
			if (leaf->depth == wanted_depth)
			{
				btVector3* verts = server->query(leaf, 1.0);

				// Return the triangles
				for (size_t i = 0; i < PlanetTile::PHYSICS_INDEX_COUNT; i+=3)
				{
					callback->processTriangle(&verts[i], (int)0, (int)(i / 3));
				}
			}
		}
		
	}
}

GroundShape::GroundShape(PlanetaryBody* body)
{
	this->body = body;
	// We have to use this instead of CUSTOM_CONCAVE_SHAPE_TYPE as 
	// otherwise bullet decides to simply not call us for triangles,
	// and crashes with some weird errors, this works flawlessly
	m_shapeType = TRIANGLE_MESH_SHAPE_PROXYTYPE;

	server = new GroundShapeServer(body);
}


GroundShape::~GroundShape()
{
	delete server;
}
