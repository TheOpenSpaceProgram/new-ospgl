#include "PartPrototype.h"
#include "Model.h"

#pragma warning(push, 0)
#include "btBulletCollisionCommon.h"
#pragma warning(pop)
#include "../physics/glm/BulletGlmCompat.h"

void PieceAttachment::load(const cpptoml::table& from)
{
	SAFE_TOML_GET(marker, "marker", std::string);
	SAFE_TOML_GET(name, "name", std::string);
	SAFE_TOML_GET_OR(radial, "radial", bool, false);
	SAFE_TOML_GET_OR(stack, "stack", bool, true);
	SAFE_TOML_GET(size, "size", double);
}


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

				proto.collider->setMargin(0.005);

				auto mass_toml = node_toml->get_as<double>("mass");

				if (!mass_toml)
				{
					logger->fatal("Piece '{}' has no mass entry", n->name);
				}

				proto.mass = *mass_toml;

				proto.friction = node_toml->get_as<double>("friction").value_or(PIECE_DEFAULT_FRICTION);
				proto.restitution = node_toml->get_as<double>("restitution").value_or(PIECE_DEFAULT_RESTITUTION);

				proto.allows_radial = node_toml->get_as<bool>("allows_radial").value_or(true);

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
		else if (child->name.rfind(Model::MARK_PREFIX, 0) == 0)
		{
			// Load a marker
			Marker n_marker;

			// TODO: Apply parent's transform? Not neccesary but needs thinking,
			// what do markers represent?
			n_marker.transform = child->sub_transform;

			glm::dvec3 scale, translate, skew;
			glm::dquat orient;
			glm::dvec4 persp;
			glm::decompose(child->sub_transform, scale, orient, translate, skew, persp);


			n_marker.origin = translate;
			n_marker.forward = glm::dvec3(glm::dvec4(0.0, 0.0, 1.0, 0.0) * glm::toMat4(orient));

			// We need to change the quaternion so it's Z based (same as forward)
			// This was found by trial and error but probably makes mathematical sense
			// (Quaternions are hard)
			n_marker.rotation = glm::dquat(orient.w, -orient.x, -orient.y, -orient.z);
			
			//logger->info("Maker {} has quaternion  {} {} {} {} and forward {} {} {}",
			//		child->name, orient.w, orient.x, orient.y, orient.z, 
			//		n_marker.forward.x, n_marker.forward.y, n_marker.forward.z); 
			
			
			proto.markers[child->name] = n_marker;
		}
	}

	// Extract attachments if any is present
	auto attachments_toml = node_toml->get_table_array_qualified("attachment");
	if(attachments_toml)
	{
		for(auto attachment : *attachments_toml)
		{
			PieceAttachment attch = PieceAttachment();
			attch.load(*attachment);
			proto.attachments.push_back(attch);	
		}
	}

	pieces[proto.model_node->name] = proto;


}

void PartPrototype::load(const cpptoml::table& from)
{
	// All "easy" stuff has already been set, we now have to create the PiecePrototypes

	auto it = model->node_by_name.find("part");
	logger->check(it != model->node_by_name.end(), "Part model must have 'part' node");

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

PartPrototype* load_part_prototype(const std::string& path, const std::string& name, const std::string& pkg, const cpptoml::table& cfg)
{
	std::shared_ptr<cpptoml::table> toml = SerializeUtil::load_file(path);

	PartPrototype* proto = new PartPrototype();
	proto->in_package = pkg;

	SerializeUtil::read_to(*toml, *proto);

	return proto;
}
