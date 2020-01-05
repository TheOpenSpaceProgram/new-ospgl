#include "PartPrototype.h"
#include "Model.h"

#pragma warning(push, 0)
#include "btBulletCollisionCommon.h"
#pragma warning(pop)
#include "../physics/glm/BulletGlmCompat.h"

// TODO: Find out why the hell are these / 2.0 required all over the place for correct alignment

void PartPrototype::load_collider(btCollisionShape** target, Node* n)
{
	
	auto collider_prop = n->properties.find("collider");
	logger->check(collider_prop != n->properties.end(), "No 'collider' property found in collider! Make sure you add it and you enabled user properties");

	std::string collider_prop_s = collider_prop->second;

	if (collider_prop_s == "compound")
	{
		load_collider_compound(target, n);
	}
	else if (collider_prop_s == "box")
	{
		load_collider_box(target, n);
	}
	else if (collider_prop_s == "sphere")
	{
		load_collider_sphere(target, n);
	}
	else if (collider_prop_s == "cylinder")
	{
		load_collider_cylinder(target, n);
	}
	else if (collider_prop_s == "cone")
	{
		load_collider_cone(target, n);
	}
	else if (collider_prop_s == "capsule")
	{
		load_collider_capsule(target, n);
	}
	else if (collider_prop_s == "concave")
	{
		load_collider_concave(target, n);
	}
	else if (collider_prop_s == "convex")
	{
		load_collider_convex(target, n);
	}


}

void PartPrototype::load_piece(const cpptoml::table& toml, GPUModelNodePointer&& n)
{
	PiecePrototype proto = PiecePrototype(std::move(n));
	
	bool loaded = false;
	// Extract collider, the hardest part
	for (Node* child : n->children)
	{
		if (child->name.rfind(Model::COLLIDER_PREFIX, 0) == 0)
		{
			if (!loaded)
			{
				load_collider(&proto.collider, child);

				btTransform tform;
				glm::dvec3 scale, translate, skew;
				glm::dquat orient;
				glm::dvec4 persp;

				glm::decompose(child->sub_transform * n->sub_transform, scale, orient, translate, skew, persp);


				tform.setOrigin(to_btVector3(translate / 2.0));
				tform.setRotation(to_btQuaternion(orient));
				proto.collider->setLocalScaling(to_btVector3(scale));

				proto.collider_offset = tform;

				loaded = true;
			}
			else
			{
				logger->warn("Multiple colliders found in piece '{}', this is likely a mistake", n->name);
			}
			
		}
	}

	if (proto.model_node->name == PartPrototype::ROOT_NAME)
	{
		pieces.insert(pieces.begin(), proto);
	}
	else
	{
		pieces.push_back(proto);
	}


}

void PartPrototype::load(const cpptoml::table& from)
{
	// All "easy" stuff has already been set, we now have to create the PiecePrototypes

	auto it = model->node_by_name.find("part");
	logger->check_important(it != model->node_by_name.end(), "Part model must have 'part' node");

	Node* part_root = it->second;


	// Find all pieces
	for (Node* child : part_root->children)
	{
		if (child->name.rfind(PIECE_PREFIX, 0) == 0)
		{
			load_piece(from, GPUModelNodePointer(model.duplicate(), child->name));
		}
	}

}

PartPrototype::PartPrototype() : model(std::move(AssetHandle<Model>()))
{
}


PartPrototype::~PartPrototype()
{
	for (size_t i = 0; i < pieces.size(); i++)
	{
		if (pieces[i].collider)
		{
			delete pieces[i].collider;
		}
	}
}

PartPrototype* loadPartPrototype(const std::string& path, const std::string& name, const std::string& pkg, const cpptoml::table& cfg)
{
	std::shared_ptr<cpptoml::table> toml = SerializeUtil::load_file(path);

	PartPrototype* proto = new PartPrototype();

	SerializeUtil::read_to(*toml, *proto);

	return proto;
}

void PartPrototype::single_collider_common(Node* n)
{
	logger->check(n->meshes.size() == 1, "Number of meshes in simple collider is not 1");

	// This kind of enforces modders to optimize their collision shape, so it's cool ;)
	logger->check(!n->meshes[0].is_drawable(), "Mesh must not be drawable to be a collider!");
}


std::pair<glm::vec3, glm::vec3> PartPrototype::obtain_bounds(Mesh* m)
{
	constexpr float BIG_NUMBER = 99999999999999.0f;
	glm::vec3 min = glm::vec3(BIG_NUMBER, BIG_NUMBER, BIG_NUMBER); 
	glm::vec3 max = glm::vec3(-BIG_NUMBER, -BIG_NUMBER, -BIG_NUMBER);

	for (size_t i = 0; i < m->verts.size(); i++) 
	{
		glm::vec3 v = m->verts[i];

		if (v.x < min.x){min.x = v.x;}
		if (v.y < min.y){min.y = v.y;}
		if (v.z < min.z){min.z = v.z;}

		if (v.x > max.x) { max.x = v.x; }
		if (v.y > max.y) { max.y = v.y; }
		if (v.z > max.z) { max.z = v.z; }
	}

	return std::make_pair(min / 2.0f, max / 2.0f);

}

void PartPrototype::load_collider_compound(btCollisionShape** target, Node* n)
{
	
	*target = new btCompoundShape();
	btCompoundShape* target_c = (btCompoundShape*)*target;

	// All children are loaded as colliders
	for (Node* child : n->children)
	{
		btCollisionShape* n_shape;
		load_collider(&n_shape, child);

		btTransform tform;
		glm::dvec3 scale, translate, skew;
		glm::dquat orient;
		glm::dvec4 persp;

		glm::decompose(child->sub_transform, scale, orient, translate, skew, persp);

		tform.setOrigin(to_btVector3(translate / 2.0));
		tform.setRotation(to_btQuaternion(orient));
		n_shape->setLocalScaling(to_btVector3(scale));

		target_c->addChildShape(tform, n_shape);
	}
}

void PartPrototype::load_collider_box(btCollisionShape** target, Node* n)
{
	single_collider_common(n);
	auto[min, max] = obtain_bounds(&n->meshes[0]);

	glm::vec3 half_extents = (max - min) / 2.0f;

	*target = new btBoxShape(to_btVector3(half_extents));
}

void PartPrototype::load_collider_sphere(btCollisionShape** target, Node* n)
{
	single_collider_common(n);
	auto[min, max] = obtain_bounds(&n->meshes[0]);

	glm::vec3 half_extents = (max - min) / 2.0f;

	*target = new btSphereShape(glm::max(half_extents.x, glm::max(half_extents.y, half_extents.z)));
}

void PartPrototype::load_collider_cylinder(btCollisionShape** target, Node* n)
{
	// Cylinder is ALWAYS z aligned
	single_collider_common(n);
	auto[min, max] = obtain_bounds(&n->meshes[0]);

	glm::vec3 half_extents = (max - min) / 2.0f;

	*target = new btCylinderShapeZ(to_btVector3(half_extents));
}

void PartPrototype::load_collider_cone(btCollisionShape** target, Node* n)
{
	// Cone is ALWAYS z aligned
	single_collider_common(n);
	auto[min, max] = obtain_bounds(&n->meshes[0]);

	glm::vec3 half_extents = (max - min) / 2.0f;

	*target = new btConeShapeZ(glm::max(half_extents.x, half_extents.y), half_extents.z * 2.0f);
}

void PartPrototype::load_collider_capsule(btCollisionShape** target, Node* n)
{
	single_collider_common(n);
	auto[min, max] = obtain_bounds(&n->meshes[0]);

	glm::vec3 half_extents = (max - min) / 2.0f;

	float radius = glm::max(half_extents.x, half_extents.y);
	float total_height = half_extents.z * 2.0f;
	float height = total_height - radius * 2.0f;

	if (height < 0.0f)
	{
		logger->warn("Capsule collider too small while processing node '{}'", n->name);
		// This means the capsule is better represented as a sphere, but it's probably a design error

		height = 0.0f;
	}

	*target = new btCapsuleShapeZ(radius, height);
}

void PartPrototype::load_collider_concave(btCollisionShape** target, Node* n)
{
}

void PartPrototype::load_collider_convex(btCollisionShape** target, Node* n)
{
}
