#include "PartPrototype.h"
#include "Model.h"

#pragma warning(push, 0)
#include "btBulletCollisionCommon.h"
#pragma warning(pop)
#include "../physics/glm/BulletGlmCompat.h"


void PartPrototype::load_piece(const cpptoml::table& toml, GPUModelNodePointer&& n)
{
	PiecePrototype proto = PiecePrototype(std::move(n));
	
	auto node_toml = toml.get_table(n->name);

	bool loaded = false;
	// Extract collider, the hardest part
	for (Node* child : n->children)
	{
		if (child->name.rfind(Model::COLLIDER_PREFIX, 0) == 0)
		{
			if (!loaded)
			{
				if (node_toml == nullptr)
				{
					logger->fatal("Piece '{}' has collider and does not have a config entry", n->name);
				}

				proto.render_offset = child->sub_transform;
				ModelColliderExtractor::load_collider(&proto.collider, child);

				auto mass_toml = node_toml->get_as<double>("mass");

				if (!mass_toml)
				{
					logger->fatal("Piece '{}' has no mass entry", n->name);
				}

				proto.mass = *mass_toml;

				proto.friction = node_toml->get_as<double>("friction").value_or(PIECE_DEFAULT_FRICTION);
				proto.restitution = node_toml->get_as<double>("restitution").value_or(PIECE_DEFAULT_RESTITUTION);

				btTransform tform;
				glm::dvec3 scale, translate, skew;
				glm::dquat orient;
				glm::dvec4 persp;

				glm::decompose(child->sub_transform * n->sub_transform, scale, orient, translate, skew, persp);


				tform.setOrigin(to_btVector3(translate));
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


	pieces[proto.model_node->name] = proto;


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

	auto machines = from.get_table_array("machine");
	if(machines)
	{
		for(auto machine : *machines)
		{
			this->machines.push_back(machine);
		}
	}
}

PartPrototype::PartPrototype() : model(std::move(AssetHandle<Model>()))
{
}


PartPrototype::~PartPrototype()
{
	for (auto it = pieces.begin(); it != pieces.end(); it++)
	{
		if (it->second.collider)
		{
			delete it->second.collider;
		}
	}
}

PartPrototype* loadPartPrototype(const std::string& path, const std::string& name, const std::string& pkg, const cpptoml::table& cfg)
{
	std::shared_ptr<cpptoml::table> toml = SerializeUtil::load_file(path);

	PartPrototype* proto = new PartPrototype();
	proto->in_package = pkg;

	SerializeUtil::read_to(*toml, *proto);

	return proto;
}
