#include "Model.h"

#include <glm/gtx/quaternion.hpp>

#pragma warning(push, 0)
#include "btBulletCollisionCommon.h"
#pragma warning(pop)

void replace_all(std::string& data, const std::string& search, const std::string& replace)
{
	// Get the first occurrence
	size_t pos = data.find(search);

	// Repeat till end is reached
	while (pos != std::string::npos)
	{
		// Replace this occurrence of Sub String
		data.replace(pos, search.size(), replace);
		// Get the next occurrence from the current position
		pos = data.find(search, pos + replace.size());
	}
}


std::vector<Mesh*> Node::get_all_meshes_recursive(bool include_ours)
{
	std::vector<Mesh*> out;

	if(include_ours)
	{
		for(int i = 0; i < meshes.size(); i++)
		{	
			out.push_back(&meshes[i]);
		}
	}

	for(int i = 0; i < children.size(); i++)
	{
		std::vector<Mesh*> from_child = children[i]->get_all_meshes_recursive(true);
		out.insert(out.end(), from_child.begin(), from_child.end());
	}

	return out;
}

bool Mesh::is_drawable() const
{
	return drawable;
}

void Mesh::upload()
{
	if (drawable)
	{
		glGenBuffers(1, &vbo);
		glGenBuffers(1, &ebo);
		glGenVertexArrays(1, &vao);

		glBindVertexArray(vao);

		auto& indices = attributes["INDICES"];
		// Obtain the buffer data
		glBindBuffer(GL_ARRAY_BUFFER, buffer);
		glBufferData(GL_ARRAY_BUFFER, data_size, data, GL_STATIC_DRAW);

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, index_count * indices.get_unit_size(), indices.data(), GL_STATIC_DRAW);

		GLuint ptr_num = 0;
		GLsizei stride = (GLsizei)material->cfg.get_vertex_floats();
		size_t offset = 0;

		if (material->cfg.has_pos)
		{
			// Position
			glEnableVertexAttribArray(ptr_num);
			glVertexAttribPointer(ptr_num, 3, GL_FLOAT, GL_FALSE, stride * sizeof(float), (void*)offset);

			ptr_num++;
			offset += 3 * sizeof(float);
		}

		if (material->cfg.has_nrm)
		{
			// Normal
			glEnableVertexAttribArray(ptr_num);
			glVertexAttribPointer(ptr_num, 3, GL_FLOAT, GL_FALSE, stride * sizeof(float), (void*)offset);

			ptr_num++;
			offset += 3 * sizeof(float);
		}

		if (material->cfg.has_uv0)
		{
			// Texture 
			glEnableVertexAttribArray(ptr_num);
			glVertexAttribPointer(ptr_num, 2, GL_FLOAT, GL_FALSE, stride * sizeof(float), (void*)offset);

			ptr_num++;
			offset += 2 * sizeof(float);
		}

		if (material->cfg.has_tgt)
		{
			// Tangent
			glEnableVertexAttribArray(ptr_num);
			glVertexAttribPointer(ptr_num, 3, GL_FLOAT, GL_FALSE, stride * sizeof(float), (void*)offset);
			ptr_num++;
			offset += 3 * sizeof(float);

			// Bitangent
			glEnableVertexAttribArray(ptr_num);
			glVertexAttribPointer(ptr_num, 3, GL_FLOAT, GL_FALSE, stride * sizeof(float), (void*)offset);
			ptr_num++;
			offset += 3;
		}

		if (material->cfg.has_cl3)
		{
			// Color (RGB)
			glEnableVertexAttribArray(ptr_num);
			glVertexAttribPointer(ptr_num, 3, GL_FLOAT, GL_FALSE, stride * sizeof(float), (void*)offset);

			ptr_num++;
			offset += 3 * sizeof(float);
		}

		if (material->cfg.has_cl4)
		{
			// Color (RGBA)
			glEnableVertexAttribArray(ptr_num);
			glVertexAttribPointer(ptr_num, 4, GL_FLOAT, GL_FALSE, stride * sizeof(float), (void*)offset);

			ptr_num++;
			offset += 4 * sizeof(float);
		}

		glBindVertexArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

		material.get();
	}
}

void Mesh::unload()
{
	if (drawable)
	{
		glDeleteBuffers(1, &ebo);
		glDeleteVertexArrays(1, &vao);

		vao = 0;
		ebo = 0;

		material.unload();
	}
}

void Mesh::bind_uniforms(const CameraUniforms& uniforms, glm::dmat4 model, GLint did)
{
	logger->check(drawable, "Cannot draw a non-drawable mesh!");

	material->shader->use();

	material->set(textures, mat_override);
	material->set_core(uniforms, model, did);
}

void Mesh::draw_command() const
{
	logger->check(drawable, "Cannot draw a non-drawable mesh!");
	logger->check(vao != 0, "Tried to render a non-loaded mesh");

	glBindVertexArray(vao);
	glDrawElements(GL_TRIANGLES, (GLsizei)index_count, GL_UNSIGNED_INT, nullptr);
	glBindVertexArray(0);

}

std::vector<glm::vec3> Mesh::get_verts()
{
	std::vector<glm::vec3> out;
	auto indices = attributes["INDICES"];
	auto positions = attributes["POSITIONS"];

	for(int i = 0; i < index_count; i++)
	{
		int index = indices.get_index(i);
		out.push_back(positions.get_vec3(index));
	}

	return out;
}


void Model::upload()
{
	for (auto it = node_by_name.begin(); it != node_by_name.end(); it++)
	{
		for (size_t i = 0; i < it->second->meshes.size(); i++)
		{
			it->second->meshes[i].upload();
		}
	}
	
	uploaded = true;
}

void Model::unload()
{
	for (auto it = node_by_name.begin(); it != node_by_name.end(); it++)
	{
		for (size_t i = 0; i < it->second->meshes.size(); i++)
		{
			it->second->meshes[i].unload();
		}
	}
	uploaded = false;
}

void Model::get_gpu()
{
	gpu_users++;

	if (!uploaded)
	{
		upload();
	}
}

void Model::free_gpu()
{
	gpu_users--;
	if (gpu_users <= 0)
	{
		unload();
	}
}



Model::Model(const tinygltf::Model& model)
{
	gpu_users = 0;
	uploaded = false;

	// Copy all the buffers first
	// TODO: Handling of multiple buffers?
	buffer = model.buffers[0].data;

	const tinygltf::Scene scene = model.scenes[model.defaultScene];
	// Load recursively all the nodes
	Node* scene_node = new Node();
	scene_node->name = scene.name;

	for(int node : scene.nodes)
	{
		load_node(model, node, scene_node, this, true);
	}

	root = scene_node;
}

void Model::load_node(const tinygltf::Model &model, int node_idx, Node *parent, Model* rmodel, bool parent_draw)
{
	const tinygltf::Node node = model.nodes[node_idx];

	Node* n_node = new Node();
	n_node->name = node.name;

	if(node.extras.IsObject())
	{
		for(const std::string& key : node.extras.Keys())
		{
			tinygltf::Value sub = node.extras.Get(key);
			if(!sub.IsString())
			{
				logger->warn("Cannot parse property '{}', unsupported type (we only support string)", key);
			}
			else
			{
				n_node->properties[key] = sub.Get<std::string>();
			}
		}
	}

	if(node.matrix.size() != 0)
	{
		for(int i = 0; i < 16; i++)
		{
			n_node->sub_transform[i % 4][i / 4] = node.matrix[i];
		}
	}
	else
	{
		glm::dvec3 trans = glm::dvec3(0.0, 0.0, 0.0), scale = glm::dvec3(1.0, 1.0, 1.0);
		glm::dquat rot = glm::dquat(1.0, 0.0, 0.0, 0.0);

		for(int i = 0; i < 3; i++)
		{
			if(!node.translation.empty())
			{
				trans[i] = node.translation[i];
			}
			if(!node.scale.empty())
			{
				scale[i] = node.scale[i];
			}
		}

		if(!node.rotation.empty())
		{
			rot[0] = node.rotation[3];
			rot[1] = node.rotation[0];
			rot[2] = node.rotation[1];
			rot[3] = node.rotation[2];
		}

		n_node->sub_transform = glm::scale(glm::dmat4(1.0), scale);
		n_node->sub_transform = glm::toMat4(rot) * n_node->sub_transform;
		n_node->sub_transform = glm::translate(n_node->sub_transform, trans);
	}


	node_by_name[n_node->name] = n_node;


	bool drawable = parent_draw;
	if (n_node->name.rfind(Model::COLLIDER_PREFIX, 0) == 0
		|| n_node->name.rfind(Model::MARK_PREFIX, 0) == 0)
	{
		drawable = false;
	}

	if(node.mesh >= 0)
	{
		for (const auto& primitive : model.meshes[node.mesh].primitives)
		{
			load_mesh(model, primitive, rmodel, n_node, drawable);
		}
	}



	if(parent != nullptr)
	{
		parent->children.push_back(n_node);
	}

	for(int child : node.children)
	{
		load_node(model, child, n_node, rmodel, drawable);
	}
}

Model::~Model()
{
}

void Model::load_mesh(const tinygltf::Model& model, const tinygltf::Primitive &primitive, Model* rmodel, Node *node, bool drawable)
{
	AssetPointer mat_ptr;

	tinygltf::Material gltf_mat = tinygltf::Material();
	// We use PBR by default
	gltf_mat.name = "core:mat_pbr.toml";
	if(primitive.material >= 0)
	{
		gltf_mat = model.materials[primitive.material];
	}

	std::string mat_name = gltf_mat.name;
	size_t pos = mat_name.find_last_of('.');

	if (pos < mat_name.size() && mat_name.substr(pos) == ".toml")
	{
		mat_ptr = AssetPointer(mat_name);
	}
	else
	{
		// PBR material
		mat_ptr = AssetPointer("core:mat_pbr.toml");
	}


	AssetHandle<Material> mat = AssetHandle<Material>(mat_ptr);

	node->meshes.emplace_back(std::move(mat));
	Mesh* m = &node->meshes[node->meshes.size() - 1];
	m->drawable = drawable;

	// Load model textures (they must be embedded) and PBR stuff


	// Load the mesh itself, depending on the mesh configuration and the material
	// TODO: Support non-indexed meshes
	logger->check(primitive.indices >= 0, "Cannot load a non indexed mesh. Make sure you enable it in the 3d export");

	// Load the indices
	tinygltf::Accessor index_accessor = model.accessors[primitive.indices];
	tinygltf::BufferView index_bview = model.bufferViews[index_accessor.bufferView];


	if(drawable)
	{
	}

	std::vector<std::string> needed;
	if(m->material->cfg.has_pos){ needed.emplace_back("POSITION"); }
	if(m->material->cfg.has_nrm){ needed.emplace_back("NORMAL"); }
	if(m->material->cfg.has_tgt){ needed.emplace_back("TANGENT"); }
	if(m->material->cfg.has_uv0){ needed.emplace_back("TEXCOORD_0"); }
	if(m->material->cfg.has_uv1){ needed.emplace_back("TEXCOORD_1"); }
	if(m->material->cfg.has_cl3){ needed.emplace_back("COLOR_0"); }
	if(m->material->cfg.has_cl4){ needed.emplace_back("COLOR_0"); }
	// TODO: Implement joints and weights

	for(const std::string& need : needed)
	{
		auto it = primitive.attributes.find(need);
		if(it == primitive.attributes.end())
		{
			logger->warn("Mesh doesn't have required attribute '{}', it will be filled with zeros", need);

		}
		else
		{

		}



	}

}

Model* load_model(const std::string& path, const std::string& name, const std::string& pkg, const cpptoml::table& cfg)
{
	std::string extension = name.substr(name.find_last_of('.'));
	tinygltf::Model model;
	tinygltf::TinyGLTF loader;
	std::string err, wrn;

	if(extension == ".glb")
	{
		loader.LoadBinaryFromFile(&model, &err, &wrn, path);
	}
	else if(extension == ".gltf")
	{
		loader.LoadASCIIFromFile(&model, &err, &wrn, path);
	}
	else
	{
		logger->error("Model format {} not supported: {}:{}", extension, pkg, name);
	}

	if(!err.empty())
	{
		logger->error("Error loading model {}:{}\n{}", pkg, name, err);
		return nullptr;
	}
	else
	{
		if(!wrn.empty())
		{
			logger->warn("Warning loading model {}:{}\n{}", pkg, name, wrn);
		}


		return new Model(model);
	}
}


Node* GPUModelPointer::get_node(const std::string& name)
{
	auto it = model->node_by_name.find(name);

	logger->check(it != model->node_by_name.end(), "Tried to get a node which does not exist");

	return it->second;
}

Node* GPUModelPointer::get_root_node()
{
	return model->root;
}

GPUModelPointer::GPUModelPointer(AssetHandle<Model>&& m) : model(std::move(m))
{
	if (!is_null())
	{
		model->get_gpu();
	}
}

GPUModelPointer::~GPUModelPointer()
{
	if (!is_null())
	{
		model->free_gpu();
	}
}

void Node::draw_all_meshes(const CameraUniforms& uniforms, GLint did, glm::dmat4 model)
{
	for (Mesh& mesh : meshes)
	{
		if (mesh.is_drawable())
		{
			mesh.bind_uniforms(uniforms, model, did);
			mesh.draw_command();
		}
	}

}

void Node::draw_all_meshes_shadow(const ShadowCamera& sh_cam, glm::dmat4 model)
{
	for(Mesh& mesh : meshes)
	{
		if(mesh.is_drawable())
		{
			glm::dmat4 tform = sh_cam.tform * model;

			Shader* sh = mesh.material->shadow_shader;
			sh->use();
			sh->setMat4("tform", tform);

			mesh.draw_command();
		}
	}
}

void Node::draw_all_meshes_override(const CameraUniforms& uniforms, Material* mat, MaterialOverride* mat_over, 
		GLint did, glm::dmat4 model)
{
	for (Mesh& mesh : meshes)
	{
		if (mesh.is_drawable())
		{
			Material* def = mat;
			if(mat == nullptr)
			{
				mat = mesh.material.data;
			}

			mat->shader->use();
			if(mat_over)
			{
				mat->set(mesh.textures, *mat_over);
			}
			else
			{
				mat->set(mesh.textures, mesh.mat_override);
			}
			mat->set_core(uniforms, model, did);
			mesh.draw_command();

			mat = def;
		}
	}
}


void Node::draw(const CameraUniforms& uniforms, glm::dmat4 model, GLint did, bool ignore_our_subtform, bool increase_did)
{
	glm::dmat4 n_model;
	if (ignore_our_subtform)
	{
		n_model = model;
	}
	else
	{
		n_model = model * sub_transform; //< Transformations apply in reverse
	}

	draw_all_meshes(uniforms, did, n_model);

	for (Node* node : children)
	{
		if(increase_did)
		{
			// We increase the drawable id
			did++;
		}
		node->draw(uniforms, n_model, did, false, increase_did);
	}
}

void Node::draw_override(const CameraUniforms& uniforms, Material* mat, glm::dmat4 model, GLint did, 
			MaterialOverride* mat_override, bool ignore_our_subtform, bool increase_did)
{
	glm::dmat4 n_model;
	if (ignore_our_subtform)
	{
		n_model = model;
	}
	else
	{
		n_model = model * sub_transform; //< Transformations apply in reverse
	}

	draw_all_meshes_override(uniforms, mat, mat_override, did, n_model);

	for (Node* node : children)
	{
		if(increase_did)
		{
			// We increase the drawable id
			did++;
		}

		node->draw_override(uniforms, mat, n_model, did, mat_override, false, increase_did);
	}

}


void Node::draw_shadow(const ShadowCamera& sh_cam, glm::dmat4 model, bool ignore_our_subtform)
{
	glm::dmat4 n_model;
	if(ignore_our_subtform)
	{
		n_model = model;
	}
	else
	{
		n_model = model * sub_transform;
	}

	draw_all_meshes_shadow(sh_cam, n_model);

	for(Node* node : children)
	{
		node->draw_shadow(sh_cam, model, false);
	}
}


void ModelColliderExtractor::load_collider(btCollisionShape** target, Node* n)
{

	auto collider_prop = n->properties.find("collider");
	logger->check(collider_prop != n->properties.end(), 
			"No 'collider' property found in collider ({})! Make sure you add it and you enabled user properties",
			n->name);

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

	double margin = 0.02;

	auto margin_prop = n->properties.find("margin");

	if (margin_prop != n->properties.end())
	{

	}

	(*target)->setMargin(margin);

}


void ModelColliderExtractor::single_collider_common(Node* n)
{
	logger->check(n->meshes.size() == 1, 
			"Number of meshes in simple collider ({}) is bigger than 1 ({})", 
			n->name, n->meshes.size());

	// This kind of enforces modders to optimize their collision shape, so it's cool ;)
	logger->check(!n->meshes[0].is_drawable(), "Mesh must not be drawable to be a collider ({})!",
			n->name);
}


std::pair<glm::vec3, glm::vec3> ModelColliderExtractor::obtain_bounds(Mesh* m)
{
	return std::make_pair(m->min_bound, m->max_bound);
}

void ModelColliderExtractor::load_collider_compound(btCollisionShape** target, Node* n)
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

		tform.setOrigin(to_btVector3(translate));
		tform.setRotation(to_btQuaternion(orient));
		n_shape->setLocalScaling(to_btVector3(scale));

		target_c->addChildShape(tform, n_shape);
	}
}

void ModelColliderExtractor::load_collider_box(btCollisionShape** target, Node* n)
{
	single_collider_common(n);
	auto[min, max] = obtain_bounds(&n->meshes[0]);

	glm::vec3 half_extents = (max - min) / 2.0f;

	*target = new btBoxShape(to_btVector3(half_extents));
}

void ModelColliderExtractor::load_collider_sphere(btCollisionShape** target, Node* n)
{
	single_collider_common(n);
	auto[min, max] = obtain_bounds(&n->meshes[0]);

	glm::vec3 half_extents = (max - min) / 2.0f;

	*target = new btSphereShape(glm::max(half_extents.x, glm::max(half_extents.y, half_extents.z)));
}

void ModelColliderExtractor::load_collider_cylinder(btCollisionShape** target, Node* n)
{
	// Cylinder is ALWAYS z aligned
	single_collider_common(n);
	auto[min, max] = obtain_bounds(&n->meshes[0]);

	glm::vec3 half_extents = (max - min) / 2.0f;

	*target = new btCylinderShapeZ(to_btVector3(half_extents));
}

void ModelColliderExtractor::load_collider_cone(btCollisionShape** target, Node* n)
{
	// Cone is ALWAYS z aligned
	single_collider_common(n);
	auto[min, max] = obtain_bounds(&n->meshes[0]);

	glm::vec3 half_extents = (max - min) / 2.0f;

	*target = new btConeShapeZ(glm::max(half_extents.x, half_extents.y), half_extents.z * 2.0f);
}

void ModelColliderExtractor::load_collider_capsule(btCollisionShape** target, Node* n)
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

void ModelColliderExtractor::load_collider_concave(btCollisionShape** target, Node* n)
{
	logger->fatal("Not yet implemented");
}

void ModelColliderExtractor::load_collider_convex(btCollisionShape** target, Node* n)
{
	// TODO: Is this the best method?
	// Bullet's little documentation only cites this method, but there are 
	// other convex mesh classes

	*target = new btConvexHullShape();
	btConvexHullShape* target_c = (btConvexHullShape*)(*target);

	std::vector<glm::vec3> verts = n->meshes[0].get_verts();
	for (auto& vert : verts)
	{
		target_c->addPoint(to_btVector3(vert), false);
	}

	target_c->recalcLocalAabb();
	target_c->optimizeConvexHull();
}

uint8_t* Mesh::Attribute::get_ptr(int idx) const
{
	uint8_t* base = in_model->buffer.data() + byte_offset;
	return base + get_unit_size() * idx;
}

size_t Mesh::Attribute::get_size() const
{
	return get_unit_size() * count;
}

size_t Mesh::Attribute::get_component_size() const
{
	switch (component_type)
	{
		case(5120):
		case(5121):
			return 1;
		case(5122):
		case(5123):
			return 2;
		case(5125):
		case(5126):
			return 4;
		default:
			logger->fatal("Unknown component type: {}", component_type);
	}
}

glm::vec3 Mesh::Attribute::get_vec3(int idx) const
{
	void* ptr = (void*)get_ptr(idx);
	switch (component_type)
	{
		case(5120):
			return glm::vec3(*((int8_t*)ptr + 0), *((int8_t*)ptr + 1), *((int8_t*)ptr + 2));
		case(5121):
			return glm::vec3(*((uint8_t*)ptr + 0), *((uint8_t*)ptr + 1), *((uint8_t*)ptr + 2));
		case(5122):
			return glm::vec3(*((int16_t*)ptr + 0), *((int16_t*)ptr + 1), *((int16_t*)ptr + 2));
		case(5123):
			return glm::vec3(*((uint16_t*)ptr + 0), *((uint16_t*)ptr + 1), *((uint16_t*)ptr + 2));
		case(5125):
			return glm::vec3(*((int32_t*)ptr + 0), *((int32_t*)ptr + 1), *((int32_t*)ptr + 2));
		case(5126):
			return glm::vec3(*((float*)ptr + 0), *((float*)ptr + 1), *((float*)ptr + 2));
		default:
			logger->fatal("Unknown component type: {}", component_type);
	}
}

int Mesh::Attribute::get_index(int idx) const
{
	void* ptr = (void*)get_ptr(idx);
	switch (component_type)
	{
		case(5120):
			return (int)(*(int8_t*)ptr);
		case(5121):
			return (int)(*(uint8_t*)ptr);
		case(5122):
			return (int)(*(int16_t*)ptr);
		case(5123):
			return (int)(*(uint16_t*)ptr);
		case(5125):
			return (int)(*(int32_t*)ptr);
		case(5126):
			return (int)(*(float*)ptr);
		default:
			logger->fatal("Unknown component type: {}", component_type);
	}
}

size_t Mesh::Attribute::get_type_components() const
{
	if(type == "SCALAR")
			return 1;
	else if(type == "VEC2")
			return 2;
	else if(type == "VEC3")
			return 3;
	else if(type == "VEC4" || type == "MAT2")
			return 4;
	else if(type == "MAT3")
			return 9;
	else if(type == "MAT4")
			return 16;
	else
			logger->fatal("Unknown type: {}", type);
}

size_t Mesh::Attribute::get_unit_size() const
{
	return get_component_size() * get_type_components();
}

