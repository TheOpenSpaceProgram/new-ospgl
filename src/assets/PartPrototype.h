#pragma once

#include "Model.h"
#include <BulletCollision/CollisionShapes/btCollisionShape.h>

#define PIECE_DEFAULT_FRICTION 0.7
#define PIECE_DEFAULT_RESTITUTION 0.1

struct Marker
{
	glm::dmat4 transform;
	glm::dvec3 origin;
	glm::dquat rotation;
	glm::dvec3 forward;
};

struct PieceAttachment
{
	std::string marker, name;
	bool radial, stack;
	double size;	

	void load(const cpptoml::table& toml);
};

// NOTE: PartPrototype handles the lifetime of collider
// NOTE: Colliders are always Z axis is up (same as blender!)
struct PiecePrototype
{
	GPUModelNodePointer model_node;

	btCollisionShape* collider;
	glm::dmat4 render_offset; //< Offset inside of the part, for rendering
	btTransform collider_offset;
	std::unordered_map<std::string, Marker> markers;
	std::vector<PieceAttachment> attachments;
	double mass;
	double friction;
	double restitution;
	// Can stuff attach radially to this part?
	bool allows_radial;
	
	void copy(const PiecePrototype& b)
	{
		this->collider = b.collider;
		this->collider_offset = b.collider_offset;
		this->render_offset = b.render_offset;
		this->mass = b.mass;
		this->restitution = b.restitution;
		this->friction = b.friction;
		this->markers = b.markers;
		this->attachments = b.attachments;
		this->allows_radial = b.allows_radial;
	}

	PiecePrototype(GPUModelNodePointer&& n) : model_node(std::move(n))
	{
		this->collider = nullptr;
		this->render_offset = glm::dmat4(1.0);
		this->collider_offset = btTransform::getIdentity();
		this->mass = 1.0;
		this->restitution = PIECE_DEFAULT_RESTITUTION;
		this->friction = PIECE_DEFAULT_FRICTION;
		this->allows_radial = true;
	}

	// We have to declare copy constructor
	PiecePrototype(const PiecePrototype& b) : model_node(std::move(b.model_node.duplicate()))
	{
		copy(b);
	}

	// Copy, used for std containers
	PiecePrototype& operator=(const PiecePrototype& b)
	{
		this->model_node = b.model_node.duplicate();
		copy(b);
		return *this;
	}

	PiecePrototype() : model_node()
	{
		collider = nullptr;
	}
};


// Part prototypes store everything that's shared across any spawned 
// part of this type such as the config or the model.
// It's an asset and can be loaded as one 
// Part toml files are prefixed with 'part_' by default, but that's
// just a convention. The package file must define all parts anyway.
//
// Model Conventions:
// - All pieces must be children of a node named 'part', stuff outside is ignored
// - Pieces must be prefixed with p_. Pieces cannot be children of other pieces (if they are, they are ignored)
// - Root piece is always named 'p_root'
// - Colliders must be prefixed with col_, all their children will be part of the collider if it's 'compound'
// - Everything else is loaded normally, with rendering enabled
// - Only one part per model
// - If there is only one piece in a part, then that piece is root, but you are recommended to name it 'p_root'
// - If there are no pieces named 'p_root', then the root piece may be any of them. Not recommended!
// - The model format MUST support attributes. You can enable it in blender export, .fbx works pretty well!
// - Usage of concave collision meshes should be avoided, they are quite slow!
//
// NOTE: We keep the model loaded!
class PartPrototype
{
private:

	

	void load_piece(const cpptoml::table& toml, GPUModelNodePointer&& n);

public:


	static constexpr const char* PIECE_PREFIX = "p_";
	static constexpr const char* ROOT_NAME = "p_root";


	// First one is always root
	std::unordered_map<std::string, PiecePrototype> pieces;
	std::vector<std::shared_ptr<cpptoml::table>> machines;

	GPUModelPointer model;
	std::string name;
	std::string country;
	std::string manufacturer;
	std::string description;
	std::vector<std::string> categories;

	std::string in_package;

	void load(const cpptoml::table& from);

	PartPrototype();
	~PartPrototype();
};

PartPrototype* load_part_prototype(const std::string& path, const std::string& name, const std::string& pkg, const cpptoml::table& cfg);


// We do a bare minimum here, PartPrototype actually does the heavy work
template<>
class GenericSerializer<PartPrototype>
{
public:

	static void serialize(const PartPrototype& what, cpptoml::table& target)
	{
		logger->check(false, "Not implemented");
	}

	static void deserialize(PartPrototype& to, const cpptoml::table& from)
	{
		SAFE_TOML_GET(to.name, "name", std::string);
		SAFE_TOML_GET_OR(to.country, "country", std::string, "Unknown");
		SAFE_TOML_GET_OR(to.manufacturer, "manufacturer", std::string, "Unknown");
		SAFE_TOML_GET(to.description, "description", std::string);

		to.categories = *from.get_array_of<std::string>("categories");
		to.categories.push_back("all");

		std::string model_path;
		SAFE_TOML_GET(model_path, "model", std::string);

		AssetHandle<Model> model = AssetHandle<Model>(model_path);

		// Load the model
		to.model = GPUModelPointer(std::move(model));

		to.load(from);
	}
};
