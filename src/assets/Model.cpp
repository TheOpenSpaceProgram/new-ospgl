#include "Model.h"

#include <glm/glm.hpp>
#include <glm/gtx/quaternion.hpp>

#pragma warning(push, 0)
#include "btBulletCollisionCommon.h"
#pragma warning(pop)

static void* get_buffer_ptr(const tinygltf::Model& model, const tinygltf::Accessor& acc)
{
	const tinygltf::BufferView& bv = model.bufferViews[acc.bufferView];
	const tinygltf::Buffer& b = model.buffers[bv.buffer];

	void* ptr = (void*)((uint8_t*)b.data.data() + bv.byteOffset + acc.byteOffset);
	return ptr;
}

static size_t get_accessor_unit_size(const tinygltf::Model& model, const tinygltf::Accessor& acc)
{
	return tinygltf::GetComponentSizeInBytes(acc.componentType) * tinygltf::GetNumComponentsInType(acc.type);
}

static size_t get_accessor_size(const tinygltf::Model& model, const tinygltf::Accessor& acc)
{
	return acc.count * get_accessor_unit_size(model, acc);
}

static const uint8_t* get_ptr_from_accessor(const tinygltf::Model& model, const tinygltf::Accessor& acc, int idx)
{
	size_t byte_offset = acc.byteOffset + model.bufferViews[acc.bufferView].byteOffset;
	const uint8_t* base = model.buffers[model.bufferViews[acc.bufferView].buffer].data.data() + byte_offset;
	return base + get_accessor_unit_size(model, acc) * idx;
}

// This is used to extract single floats, not whole units
static const uint8_t* get_subptr_from_accessor(const tinygltf::Model& model, const tinygltf::Accessor& acc, int idx)
{
	size_t byte_offset = acc.byteOffset + model.bufferViews[acc.bufferView].byteOffset;
	const uint8_t* base = model.buffers[model.bufferViews[acc.bufferView].buffer].data.data() + byte_offset;
	return base + tinygltf::GetComponentSizeInBytes(acc.componentType) * idx;
}

static glm::vec2 get_vec2_from_accessor(const tinygltf::Model& model, const tinygltf::Accessor& acc, int idx)
{
	void* ptr = (void*)get_ptr_from_accessor(model, acc, idx);
	switch (acc.componentType)
	{
		case(TINYGLTF_COMPONENT_TYPE_BYTE):
			return glm::vec2(*((int8_t*)ptr + 0), *((int8_t*)ptr + 1));
		case(TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE):
			return glm::vec2(*((uint8_t*)ptr + 0), *((uint8_t*)ptr + 1));
		case(TINYGLTF_COMPONENT_TYPE_SHORT):
			return glm::vec2(*((int16_t*)ptr + 0), *((int16_t*)ptr + 1));
		case(TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT):
			return glm::vec2(*((uint16_t*)ptr + 0), *((uint16_t*)ptr + 1));
		case(TINYGLTF_COMPONENT_TYPE_INT):
			return glm::vec2(*((int32_t*)ptr + 0), *((int32_t*)ptr + 1));
		case(TINYGLTF_COMPONENT_TYPE_FLOAT):
			return glm::vec2(*((float*)ptr + 0), *((float*)ptr + 1));
		default:
			logger->fatal("Unknown component type: {}", acc.componentType);
			return glm::vec2(0.0);
	}
}

static glm::vec3 get_vec3_from_accessor(const tinygltf::Model& model, const tinygltf::Accessor& acc, int idx)
{
	void* ptr = (void*)get_ptr_from_accessor(model, acc, idx);
	switch (acc.componentType)
	{
		case(TINYGLTF_COMPONENT_TYPE_BYTE):
			return glm::vec3(*((int8_t*)ptr + 0), *((int8_t*)ptr + 1), *((int8_t*)ptr + 2));
		case(TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE):
			return glm::vec3(*((uint8_t*)ptr + 0), *((uint8_t*)ptr + 1), *((uint8_t*)ptr + 2));
		case(TINYGLTF_COMPONENT_TYPE_SHORT):
			return glm::vec3(*((int16_t*)ptr + 0), *((int16_t*)ptr + 1), *((int16_t*)ptr + 2));
		case(TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT):
			return glm::vec3(*((uint16_t*)ptr + 0), *((uint16_t*)ptr + 1), *((uint16_t*)ptr + 2));
		case(TINYGLTF_COMPONENT_TYPE_INT):
			return glm::vec3(*((int32_t*)ptr + 0), *((int32_t*)ptr + 1), *((int32_t*)ptr + 2));
		case(TINYGLTF_COMPONENT_TYPE_FLOAT):
			return glm::vec3(*((float*)ptr + 0), *((float*)ptr + 1), *((float*)ptr + 2));
		default:
			logger->fatal("Unknown component type: {}", acc.componentType);
			return glm::vec3(0.0);
	}
}

static int get_index_from_accessor(const tinygltf::Model& model, const tinygltf::Accessor& acc, int idx)
{
	void* ptr = (void*)get_ptr_from_accessor(model, acc, idx);
	switch (acc.componentType)
	{
		case(TINYGLTF_COMPONENT_TYPE_BYTE):
			return (int)(*(int8_t*)ptr);
		case(TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE):
			return (int)(*(uint8_t*)ptr);
		case(TINYGLTF_COMPONENT_TYPE_SHORT):
			return (int)(*(int16_t*)ptr);
		case(TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT):
			return (int)(*(uint16_t*)ptr);
		case(TINYGLTF_COMPONENT_TYPE_INT):
			return (int)(*(int32_t*)ptr);
		case(TINYGLTF_COMPONENT_TYPE_FLOAT):
			return (int)(*(float*)ptr);
		default:
			logger->fatal("Unknown component type: {}", acc.componentType);
			return 0;
	}
}

static float get_subfloat_from_accessor(const tinygltf::Model& model, const tinygltf::Accessor& acc, int idx)
{
	void* ptr = (void*)get_subptr_from_accessor(model, acc, idx);
	switch (acc.componentType)
	{
		case(TINYGLTF_COMPONENT_TYPE_BYTE):
			return (float)(*(int8_t*)ptr);
		case(TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE):
			return (float)(*(uint8_t*)ptr);
		case(TINYGLTF_COMPONENT_TYPE_SHORT):
			return (float)(*(int16_t*)ptr);
		case(TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT):
			return (float)(*(uint16_t*)ptr);
		case(TINYGLTF_COMPONENT_TYPE_INT):
			return (float)(*(int32_t*)ptr);
		case(TINYGLTF_COMPONENT_TYPE_FLOAT):
			return (float)(*(float*)ptr);
		default:
			logger->fatal("Unknown component type: {}", acc.componentType);
			return 0;
	}
}

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


std::vector<const Mesh*> Node::get_all_meshes_recursive(bool include_ours) const
{
	std::vector<const Mesh*> out;

	if(include_ours)
	{
		for(const auto& mesh : meshes)
		{	
			out.push_back(&mesh);
		}
	}

	for(auto i : children)
	{
		std::vector<const Mesh*> from_child = i->get_all_meshes_recursive(true);
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
		glGenVertexArrays(1, &vao);
		glGenBuffers(1, &ebo);


		tinygltf::Primitive prim = in_model->gltf.meshes[mesh_idx].primitives[prim_idx];
		logger->check(prim.indices >= 0, "We don't support non-indexed drawing");

		tinygltf::Accessor index_acc = in_model->gltf.accessors[prim.indices];


		glBindVertexArray(vao);

		// Index data
		auto size = get_accessor_size(in_model->gltf, index_acc);
		auto ptr = get_buffer_ptr(in_model->gltf, index_acc);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER,
			   (GLsizeiptr)get_accessor_size(in_model->gltf, index_acc),
			   get_buffer_ptr(in_model->gltf, index_acc), GL_STATIC_DRAW);


		// All other buffers
		// (name, gltf_has_attribute, count)
		std::vector<std::tuple<std::string, tinygltf::Accessor*, size_t>> order;

		auto get_accessor_fnc = [&prim, this](const std::string& name) -> tinygltf::Accessor*
		{
			auto it = prim.attributes.find(name);
			if(it == prim.attributes.end())
			{
				return nullptr;
			}
			else
			{
				return &this->in_model->gltf.accessors[it->second];
			}

		};

		size_t stride = material->cfg.get_vertex_floats();
		tinygltf::Accessor *nrm_acc;

		if(material->cfg.has_pos)
			order.emplace_back("POSITION", get_accessor_fnc("POSITION"), 3);
		if(drawable)
		{
			if (material->cfg.has_nrm)
				order.emplace_back("NORMAL", get_accessor_fnc("NORMAL"), 3);
			if (material->cfg.has_uv0)
				order.emplace_back("TEXCOORD_0", get_accessor_fnc("TEXCOORD_0"), 2);
			if (material->cfg.has_uv1)
				order.emplace_back("TEXCOORD_1", get_accessor_fnc("TEXCOORD_1"), 2);
			if (material->cfg.has_tgt)
				order.emplace_back("TANGENT", get_accessor_fnc("TANGENT"), 6);
			if (material->cfg.has_cl3)
				order.emplace_back("COLOR_0", get_accessor_fnc("COLOR_0"), 3);
			if (material->cfg.has_cl4)
				order.emplace_back("COLOR_0", get_accessor_fnc("COLOR_0"), 4);

			if (material->cfg.has_tgt)
			{
				logger->check(material->cfg.has_nrm, "Cannot have a mesh with tangents and no normal");
				nrm_acc = get_accessor_fnc("NORMAL");
				logger->check(nrm_acc, "Cannot have null normals in a material with tangents");

				if (prim.attributes.find("TANGENT") == prim.attributes.end())
				{
					logger->warn("Could not find tangents on the model. Note for blender users:");
					logger->warn(" - Try triangulating your mesh, there's a bug in the gltf exporter which");
					logger->warn("   prevents tangents from being generated on non-triangulated meshes.");

				}
			}

		}
		else
		{
			stride = 3;
		}

		size_t vert_count = in_model->gltf.accessors[prim.attributes["POSITION"]].count;

		std::vector<float> tmp_buffer = std::vector<float>();
		tmp_buffer.resize(stride * vert_count);

		// We manually interleave the data and send it to a single VBO for perfomance reasons,
		// convert everything to the float type and calculate the bitangents
		size_t buff_ptr = 0;
		for(size_t i = 0; i < vert_count; i++)
		{
			for(const auto& tuple : order)
			{
				tinygltf::Accessor* acc = std::get<1>(tuple);
				for(size_t j = 0; j < std::get<2>(tuple); j++)
				{
					if (acc)
					{
						// Obtain data from gltf
						size_t float_count = 1;
						if(acc->type != TINYGLTF_TYPE_SCALAR) { float_count = acc->type; }

						float val = 0.0f;
						if(j < float_count)
						{
							val = get_subfloat_from_accessor(in_model->gltf, *acc, (i * float_count + j));
						}

						tmp_buffer[buff_ptr] = val;

						if(std::get<0>(tuple) == "TEXCOORD_0" && j == 1 && material->cfg.flip_uv)
						{
							tmp_buffer[buff_ptr] = -tmp_buffer[buff_ptr];
						}

						if(std::get<0>(tuple) == "TANGENT" && j == 3)
						{
							// Calculate the bitangent for indices 3, 4, 5 and skip to the end
							glm::vec3 normal;
							glm::vec4 tangent;

							for(size_t ti = 0; ti < 4; ti++)
								tangent[ti] = get_subfloat_from_accessor(in_model->gltf, *acc, (i * float_count + ti));
							for(size_t tn = 0; tn < 3; tn++)
								normal[tn] = get_subfloat_from_accessor(in_model->gltf, *nrm_acc, (i * 3 + tn));

							glm::vec3 bitangent = glm::cross(normal, glm::vec3(tangent)) * tangent.w;
							tmp_buffer[buff_ptr + 0] = bitangent.x;
							tmp_buffer[buff_ptr + 1] = bitangent.y;
							tmp_buffer[buff_ptr + 2] = bitangent.z;

							buff_ptr += 3;
							break;
						}
					}
					else
					{
						// Default data (zeroes)
						tmp_buffer[buff_ptr] = 0.0f;
					}

					buff_ptr++;
				}
			}
		}

		// Upload the buffer to the GPU
		GLuint vbo;
		glGenBuffers(1, &vbo);
		vbos.push_back(vbo);

		glBindBuffer(GL_ARRAY_BUFFER, vbo);
		glBufferData(GL_ARRAY_BUFFER, sizeof(float) * stride * vert_count, tmp_buffer.data(), GL_STATIC_DRAW);

		// And bind the vertex attributes in the order they appear
		int idx = 0;
		size_t off = 0;
		for(const auto& tuple : order)
		{
			if(std::get<0>(tuple) == "TANGENT")
			{
				// We bind two attributes
				glVertexAttribPointer(idx, 3, GL_FLOAT, GL_FALSE, stride * sizeof(float),
									  (void *) ((float *) nullptr + off));
				glEnableVertexAttribArray(idx);
				off += 3;
				glVertexAttribPointer(idx + 1, 3, GL_FLOAT, GL_FALSE, stride * sizeof(float),
									  (void *) ((float *) nullptr + off));
				glEnableVertexAttribArray(idx + 1);
				off += 3;
				idx++;
			}
			else
			{
				glVertexAttribPointer(idx, std::get<2>(tuple), GL_FLOAT, GL_FALSE, stride * sizeof(float),
									  (void *) ((float *) nullptr + off));
				glEnableVertexAttribArray(idx);
				idx++;
				off += std::get<2>(tuple);
			}
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

void Mesh::bind_uniforms(const CameraUniforms& uniforms, glm::dmat4 model, GLint did) const
{
	logger->check(drawable, "Cannot draw a non-drawable mesh!");

	material.data->shader->use();

	int gl_tex = material.data->set(textures, mat_override);
	material.data->set_core(&gl_tex, uniforms, model, did);
}

void Mesh::draw_command() const
{
	logger->check(drawable, "Cannot draw a non-drawable mesh!");
	logger->check(vao != 0, "Tried to render a non-loaded mesh");

	glBindVertexArray(vao);

	tinygltf::Primitive prim = in_model->gltf.meshes[mesh_idx].primitives[prim_idx];
	tinygltf::Accessor index_acc = in_model->gltf.accessors[prim.indices];
	glDrawElements(GL_TRIANGLES, (GLsizei)index_acc.count, index_acc.componentType, nullptr);

	glBindVertexArray(0);

}

std::vector<glm::vec3> Mesh::get_verts()
{
	std::vector<glm::vec3> out;

	tinygltf::Primitive prim = in_model->gltf.meshes[mesh_idx].primitives[prim_idx];
	logger->check(prim.indices >= 0, "We don't support non-indexed drawing");
	tinygltf::Accessor index_acc = in_model->gltf.accessors[prim.indices];
	tinygltf::Accessor pos_acc = in_model->gltf.accessors[prim.attributes["POSITION"]];

	for(int i = 0; i < index_acc.count; i++)
	{
		int index = get_index_from_accessor(in_model->gltf, index_acc, i);
		out.push_back(get_vec3_from_accessor(in_model->gltf, pos_acc, index));
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



Model::Model(tinygltf::Model&& model, ASSET_INFO) : Asset(ASSET_INFO_P)
{
	this->gltf = std::move(model);
	gpu_users = 0;
	uploaded = false;

	const tinygltf::Scene scene = gltf.scenes[gltf.defaultScene];
	// Load recursively all the nodes
	Node* scene_node = new Node();
	scene_node->name = scene.name;
	scene_node->sub_transform = glm::dmat4(1.0);

	for(int node : scene.nodes)
	{
		load_node(gltf, node, scene_node, this, true);
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

		for(int i = 0; i < 4; i++)
		{
			if (!node.rotation.empty())
			{
				rot[i] = node.rotation[i];
			}
		}

		n_node->sub_transform =
				glm::translate(glm::dmat4(1.0), trans) *
				glm::toMat4(rot) *
				glm::scale(glm::dmat4(1.0), scale);

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
		for(size_t i = 0; i < model.meshes[node.mesh].primitives.size(); i++)
		{
			const auto& primitive = model.meshes[node.mesh].primitives[i];
			load_mesh(model, primitive, rmodel, n_node, drawable, node.mesh, i);
		}
	}

	// Generate bounds
	n_node->min_bound = glm::vec3(HUGE_VALF);
	n_node->max_bound = glm::vec3(-HUGE_VALF);
	for(const Mesh& m : n_node->meshes)
	{
		// As both are in same coordiante space, two checks are enough
		n_node->min_bound = glm::min(n_node->min_bound, m.min_bound);
		n_node->max_bound = glm::max(n_node->max_bound, m.max_bound);
	}



	if(parent != nullptr)
	{
		parent->children.push_back(n_node);
	}

	for(int child : node.children)
	{
		load_node(model, child, n_node, rmodel, drawable);
	}

	auto all_children = n_node->get_children_recursive();
	for(Node* n : all_children)
	{
		n_node->child_names[n->name] = n;
	}
}

Model::~Model()
{
}

void Model::load_mesh(const tinygltf::Model& model, const tinygltf::Primitive &primitive, Model* rmodel, Node *node,
					  bool drawable, int mesh_idx, int prim_idx)
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

	node->meshes.emplace_back(std::move(mat), rmodel);
	Mesh* m = &node->meshes[node->meshes.size() - 1];
	m->drawable = drawable;

	m->prim_idx = prim_idx;
	m->mesh_idx = mesh_idx;


	auto load_texture = [&model, &m](ModelTexture::TextureType type, int index) -> void
	{
		if(index >= 0)
		{
			const tinygltf::Texture& tex = model.textures[index];
			const tinygltf::Image& img = model.images[tex.source];
			ModelTexture mtex;
			mtex.first = type;

			if(img.uri.empty())
			{
				mtex.is_asset = false;
				GLuint wrapS = GL_REPEAT, wrapT = GL_REPEAT, min_filter = GL_LINEAR, mag_filter = GL_LINEAR;
				if(tex.sampler >= 0)
				{
					const tinygltf::Sampler &samp = model.samplers[tex.sampler];
					wrapS = samp.wrapS;
					wrapT = samp.wrapT;
					min_filter = samp.minFilter;
					mag_filter = samp.magFilter;
				}

				mtex.second_ptr = std::make_shared<Image>(img.image.data(), img.width, img.height, img.bits, img.component,
											mag_filter, min_filter, wrapS, wrapT,
											type == ModelTexture::BASE_COLOR, GENERATED_ASSET_INFO);
			}
			else
			{
				mtex.is_asset = true;
				// TODO: Solve the path
				mtex.second = AssetHandle<Image>(img.uri);
			}

			m->textures.emplace_back(std::move(mtex));
		}
	};
	// Load model textures (they must be embedded) and PBR stuff
	load_texture(ModelTexture::BASE_COLOR, gltf_mat.pbrMetallicRoughness.baseColorTexture.index);
	load_texture(ModelTexture::METALLIC_ROUGHNESS, gltf_mat.pbrMetallicRoughness.metallicRoughnessTexture.index);
	load_texture(ModelTexture::EMISSIVE, gltf_mat.emissiveTexture.index);
	load_texture(ModelTexture::NORMAL_MAP, gltf_mat.normalTexture.index);
	load_texture(ModelTexture::AMBIENT_OCCLUSION, gltf_mat.occlusionTexture.index);



	// Load the mesh itself, depending on the mesh configuration and the material
	// TODO: Support non-indexed meshes
	logger->check(primitive.indices >= 0, "Cannot load a non indexed mesh. Make sure you enable it in the 3d export");

	const tinygltf::Accessor& index_accessor = model.accessors[primitive.indices];
	const tinygltf::BufferView& index_bview = model.bufferViews[index_accessor.bufferView];


	if(drawable)
	{
	}

	std::vector<std::string> needed;
	if(m->material->cfg.has_pos){ needed.emplace_back("POSITION"); }
	if( m->is_drawable())
	{
		if (m->material->cfg.has_nrm)
			needed.emplace_back("NORMAL");
		if (m->material->cfg.has_tgt)
			needed.emplace_back("TANGENT");
		if (m->material->cfg.has_uv0)
			needed.emplace_back("TEXCOORD_0");
		if (m->material->cfg.has_uv1)
			needed.emplace_back("TEXCOORD_1");
		if (m->material->cfg.has_cl3)
			needed.emplace_back("COLOR_0");
		if (m->material->cfg.has_cl4)
			needed.emplace_back("COLOR_0");
	}
	// TODO: Implement joints and weights

	for(const std::string& need : needed)
	{
		auto it = primitive.attributes.find(need);
		if(it == primitive.attributes.end())
		{
			logger->warn("Mesh in '{}' doesn't have required attribute '{}'", node->name, need);
		}
		else
		{
			if(it->first == "POSITION")
			{
				if(model.accessors[it->second].minValues.empty())
				{
					logger->warn("Mesh in '{}' doesn't have max/min values, may cause issues", node->name);
				}
				else
				{
					for (int i = 0; i < 3; i++)
					{
						m->min_bound[i] = model.accessors[it->second].minValues[i];
						m->max_bound[i] = model.accessors[it->second].maxValues[i];
					}
				}
			}
		}
	}

}

glm::dmat4 Node::get_tform(const Node* n) const
{
	auto path = get_path(n, this);
	glm::dmat4 tform = glm::dmat4();
	// Iterate from end to top for correct matrix multiplication order
	// tform = third * second * first
	for(size_t i = path.size() - 1; i > 0; i--)
	{
		tform *= path[i]->sub_transform;
	}

	return tform;
}

std::vector<const Node*> Node::get_path(const Node* to, const Node* from) const
{
	std::vector<const Node*> out;
	out.push_back(from);
	bool found = false;
	for (Node *child: from->children)
	{
		if (child == to)
		{
			out.push_back(to);
			found = true;
			break;
		}
	}
	if(found)
		return out;

	for(Node* child : from->children)
	{
		auto sub_path = get_path(to, child);
		if(sub_path.size() > 1)
		{
			// We found the path
			out.insert(out.end(), sub_path.begin(), sub_path.end());
			break;
		}
	}
	return out;
}

Model* load_model(ASSET_INFO, const cpptoml::table& cfg)
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


		return new Model(std::move(model), ASSET_INFO_P);
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
		model.get_noconst()->get_gpu();
	}
}

GPUModelPointer::~GPUModelPointer()
{
	if (!is_null())
	{
		model.get_noconst()->free_gpu();
	}
}

void Node::draw_all_meshes(const CameraUniforms& uniforms, GLint did, glm::dmat4 model) const
{
	for (const Mesh& mesh : meshes)
	{
		if (mesh.is_drawable())
		{
			mesh.bind_uniforms(uniforms, model, did);
			mesh.draw_command();
		}
	}

}

void Node::draw_all_meshes_shadow(const ShadowCamera& sh_cam, glm::dmat4 model) const
{
	for(const Mesh& mesh : meshes)
	{
		if(mesh.is_drawable())
		{
			glm::dmat4 tform = sh_cam.tform * model;

			// We can safely use data here as materials are loaded as long as we are
			Shader* sh = mesh.material.data->shadow_shader;
			sh->use();
			sh->setMat4("tform", tform);

			mesh.draw_command();
		}
	}
}

void Node::draw_all_meshes_override(const CameraUniforms& uniforms, const Material* mat, const MaterialOverride* mat_over,
		GLint did, glm::dmat4 model) const
{
	for (const Mesh& mesh : meshes)
	{
		if (mesh.is_drawable())
		{
			const Material* def = mat;
			if(mat == nullptr)
			{
				mat = mesh.material.data;
			}

			mat->shader->use();
			int gl_tex;
			if(mat_over)
			{
				gl_tex = mat->set(mesh.textures, *mat_over);
			}
			else
			{
				gl_tex = mat->set(mesh.textures, mesh.mat_override);
			}
			mat->set_core(&gl_tex, uniforms, model, did);
			mesh.draw_command();

			mat = def;
		}
	}
}


void Node::draw(const CameraUniforms& uniforms, glm::dmat4 model, GLint did,
				bool ignore_our_subtform, bool increase_did) const
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

void Node::draw_override(const CameraUniforms& uniforms, const Material* mat, glm::dmat4 model, GLint did,
			const MaterialOverride* mat_override, bool ignore_our_subtform, bool increase_did) const
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


void Node::draw_shadow(const ShadowCamera& sh_cam, glm::dmat4 model, bool ignore_our_subtform) const
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

std::vector<Node*> Node::get_children_recursive() const
{
	std::vector<Node*> out;
	out.insert(out.end(), children.begin(), children.end());
	for(const auto& child : children)
	{
		std::vector<Node*> child_of_child = child->get_children_recursive();
		out.insert(out.end(), child_of_child.begin(), child_of_child.end());
	}

	return out;
}

std::pair<glm::dvec3, glm::dvec3> Node::get_bounds(const Node *n) const
{
	auto tform = get_tform(n);
	auto min = glm::dvec3(tform * glm::dvec4(n->min_bound, 1.0));
	auto max = glm::dvec3(tform * glm::dvec4(n->max_bound, 1.0));

	return std::make_pair(min, max);
}

Node* Node::get_child(const std::string& fname) const
{
	auto it = child_names.find(fname);
	logger->check(it != child_names.end(), "Could not find children named: {}", fname);
	return it->second;
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
