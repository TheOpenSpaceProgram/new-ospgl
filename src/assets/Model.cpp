#include "Model.h"

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <glm/gtx/quaternion.hpp>

#pragma warning(push, 0)
#include "btBulletCollisionCommon.h"
#pragma warning(pop)

inline glm::dmat4 aiMatrix4x4ToGlm(const aiMatrix4x4* from)
{
    glm::dmat4 to;


    to[0][0] = (double)from->a1; to[0][1] = (double)from->b1;  to[0][2] = (double)from->c1; to[0][3] = (double)from->d1;
    to[1][0] = (double)from->a2; to[1][1] = (double)from->b2;  to[1][2] = (double)from->c2; to[1][3] = (double)from->d2;
    to[2][0] = (double)from->a3; to[2][1] = (double)from->b3;  to[2][2] = (double)from->c3; to[2][3] = (double)from->d3;
    to[3][0] = (double)from->a4; to[3][1] = (double)from->b4;  to[3][2] = (double)from->c4; to[3][3] = (double)from->d4;

    return to;
}


void Model::process_node(aiNode* node, const aiScene* scene, Node* to, bool drawable_parent)
{
	Node* n_node = new Node();

	aiMetadata* m = node->mMetaData;
	// Extract all properties:
	if (m)
	{
		for (size_t i = 0; i < m->mNumProperties; i++)
		{
			std::string key = std::string(m->mKeys[i].C_Str());
			aiMetadataEntry entry = m->mValues[i];

			// TODO: Maybe handle other property types, we don't need them yet!
			if (entry.mType == 5)
			{
				std::string cont = std::string(((aiString*)(entry.mData))->C_Str());
				n_node->properties[key] = cont;
			}
		}
	}

	n_node->sub_transform = aiMatrix4x4ToGlm(&node->mTransformation);

	n_node->name = node->mName.C_Str();

	logger->check(node_by_name.find(n_node->name) == node_by_name.end(), 
			"We don't support non-unique node names ('{}')", n_node->name);

	node_by_name[n_node->name] =n_node;

	bool drawable = drawable_parent;

	if (n_node->name.rfind(Model::COLLIDER_PREFIX, 0) == 0
		|| n_node->name.rfind(Model::MARK_PREFIX, 0) == 0) 
	{
		drawable = false;
	}

	for (unsigned int i = 0; i < node->mNumMeshes; i++)
	{
		// the node object only contains indices to index the actual objects in the scene. 
		// the scene contains all the data, node is just to keep stuff organized (like relations between nodes).
		aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];

		process_mesh(mesh, scene, n_node, drawable);
	}

	// after we've processed all of the meshes (if any) we then recursively process each of the children nodes
	for (unsigned int i = 0; i < node->mNumChildren; i++)
	{
		process_node(node->mChildren[i], scene, n_node, drawable);
	}

	if (to == nullptr)
	{
		root = n_node;
	}
	else
	{
		to->children.push_back(n_node);
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

void Model::process_mesh(aiMesh* mesh, const aiScene* scene, Node* to, bool drawable)
{
	
	aiMaterial* ai_mat = scene->mMaterials[mesh->mMaterialIndex];
	
	AssetPointer mat_ptr;

	std::string ai_mat_name = ai_mat->GetName().C_Str();

	size_t pos = ai_mat_name.find_last_of('.');

	if (pos < ai_mat_name.size() && ai_mat_name.substr(pos) == ".toml")
	{
		mat_ptr = AssetPointer(ai_mat->GetName().C_Str());
	}
	else
	{
		mat_ptr = AssetPointer("core:mat_default.toml");
	}

	
	AssetHandle<Material> mat = AssetHandle<Material>(mat_ptr);


	to->meshes.push_back(Mesh(std::move(mat)));
	Mesh* m = &to->meshes[to->meshes.size() - 1];
	m->drawable = drawable;

	// Load Assimp textures
	for (int type = 0; type < (int)aiTextureType::aiTextureType_UNKNOWN; type++)
	{
		aiTextureType type_t = (aiTextureType)type;
		unsigned int count = ai_mat->GetTextureCount(type_t);

		// TODO: Handle multiple textures of the same type
		if (count > 1)
		{
			logger->warn("Multiple textures of the same type are not yet supported.");
		}
		else if (count == 1)
		{
			aiString path;
			// TODO: Maybe handle other texture parameters? Atleast pass them to the shaders
			ai_mat->GetTexture(type_t, 0, &path);

			std::string path_s = std::string(path.C_Str());
			// Sanitize the path
			replace_all(path_s, "\\", "/");
			// We find 'res/', after it is the package name, and then the name, pretty easy
			size_t pos = path_s.find("res/");
			if (pos == std::string::npos)
			{
				logger->warn("Invalid path '{}' given in texture, ignoring!", path_s);
			}
			else
			{
				std::string sub_path = path_s.substr(pos + 4); //< +4 skips the res/
				size_t first_dash = sub_path.find_first_of('/');
				if (first_dash == std::string::npos)
				{
					logger->warn("Invalid path '{}' given in texture, ignoring!", path_s);
				}
				else
				{
					std::string pkg = sub_path.substr(0, first_dash);
					std::string name = sub_path.substr(first_dash + 1);

					AssimpTexture assimp_tex;
					assimp_tex.first = type_t;
					assimp_tex.second = std::move(AssetHandle<Image>(pkg, name, false));

					// Load the texture
					m->textures.push_back(assimp_tex);
				}
			}
		}

	}


	MeshConfig& config = m->material->cfg;

	size_t vert_size = config.get_vertex_size();
	size_t floats_per_vert = config.get_vertex_floats();

	m->indices = (uint32_t*)malloc(sizeof(uint32_t) * mesh->mNumFaces * 3);
	for (unsigned int i = 0; i < mesh->mNumFaces; i++)
	{
		aiFace face = mesh->mFaces[i];
		m->indices[i * 3 + 0] = face.mIndices[0];
		m->indices[i * 3 + 1] = face.mIndices[1];
		m->indices[i * 3 + 2] = face.mIndices[2];
	}
	m->index_count = mesh->mNumFaces * 3;


	if (drawable)
	{

		// Work directly in the vector
		m->data = (float*)malloc(vert_size * mesh->mNumVertices);
	
		m->data_size = vert_size * mesh->mNumVertices;

		bool log_once_tangents = false;
		bool log_once_colors = false;

		

	
		for (unsigned int i = 0; i < mesh->mNumVertices; i++)
		{
			size_t i0 = i * floats_per_vert;
			constexpr size_t o = 1; //< 1 Because we index using floats, could be removed ;)
			size_t j = 0;

			if (config.has_pos)
			{
				m->data[i0 + j * o] = mesh->mVertices[i].x; j++;
				m->data[i0 + j * o] = mesh->mVertices[i].y; j++;
				m->data[i0 + j * o] = mesh->mVertices[i].z; j++;
			}

			if (config.has_nrm)
			{
				m->data[i0 + j * o] = mesh->mNormals[i].x; j++;
				m->data[i0 + j * o] = mesh->mNormals[i].y; j++;
				m->data[i0 + j * o] = mesh->mNormals[i].z; j++;
			}

			if (config.has_tex)
			{
				if (mesh->mTextureCoords[0])
				{
					glm::vec2 uv;
					uv.x = mesh->mTextureCoords[0][i].x;
					uv.y = mesh->mTextureCoords[0][i].y;

					if (config.flip_uv)
					{
						uv.y = 1.0f - uv.y;
					}

					// We only support one UV map
					m->data[i0 + j * o] = uv.x; j++;
					m->data[i0 + j * o] = uv.y; j++;
				}
				else
				{
					m->data[i0 + j * o] = 0.0f; j++;
					m->data[i0 + j * o] = 0.0f; j++;
				}



			}

			if (config.has_tgt)
			{
				if (mesh->mTangents)
				{
					m->data[i0 + j * o] = mesh->mTangents[i].x; j++;
					m->data[i0 + j * o] = mesh->mTangents[i].y; j++;
					m->data[i0 + j * o] = mesh->mTangents[i].z; j++;
					m->data[i0 + j * o] = mesh->mBitangents[i].x; j++;
					m->data[i0 + j * o] = mesh->mBitangents[i].y; j++;
					m->data[i0 + j * o] = mesh->mBitangents[i].z; j++;
				}
				else
				{
					m->data[i0 + j * o] = 0.0f; j++;
					m->data[i0 + j * o] = 0.0f; j++;
					m->data[i0 + j * o] = 0.0f; j++;
					m->data[i0 + j * o] = 0.0f; j++;
					m->data[i0 + j * o] = 0.0f; j++;
					m->data[i0 + j * o] = 0.0f; j++;

					if (!log_once_tangents)
					{
						logger->warn("Could not find tangents. Make sure your model has texture coordinates and normals!");
						log_once_tangents = true;
					}
				}

			}

			// We only support one vertex color map
			if (config.has_cl3)
			{
				if (mesh->mColors[0])
				{
					m->data[i0 + j * o] = mesh->mColors[0][i].r; j++;
					m->data[i0 + j * o] = mesh->mColors[0][i].g; j++;
					m->data[i0 + j * o] = mesh->mColors[0][i].b; j++;
				}
				else
				{
					m->data[i0 + j * o] = 0.0f; j++;
					m->data[i0 + j * o] = 0.0f; j++;
					m->data[i0 + j * o] = 0.0f; j++;

					if (!log_once_colors)
					{
						logger->warn("Could not find vertex colors. Make sure your model has them!");
						log_once_colors = true;
					}
				}
			}

			if (config.has_cl4)
			{
				if (mesh->mColors[0])
				{
					m->data[i0 + j * o] = mesh->mColors[0][i].r; j++;
					m->data[i0 + j * o] = mesh->mColors[0][i].g; j++;
					m->data[i0 + j * o] = mesh->mColors[0][i].b; j++;
					m->data[i0 + j * o] = mesh->mColors[0][i].a; j++;
				}
				else
				{
					m->data[i0 + j * o] = 0.0f; j++;
					m->data[i0 + j * o] = 0.0f; j++;
					m->data[i0 + j * o] = 0.0f; j++;
					m->data[i0 + j * o] = 0.0f; j++;

					if (!log_once_colors)
					{
						logger->warn("Could not find vertex colors. Make sure your model has them!");
						log_once_colors = true;
					}
				}
			}
		}
	}
	else
	{
		for (unsigned int i = 0; i < mesh->mNumVertices; i++)
		{
			glm::vec3 p0 = glm::vec3(mesh->mVertices[i].x, mesh->mVertices[i].y, mesh->mVertices[i].z);
			m->verts.push_back(p0);
		}
	}
}



bool Mesh::is_drawable()
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

		glBindBuffer(GL_ARRAY_BUFFER, vbo);
		glBufferData(GL_ARRAY_BUFFER, data_size, data, GL_STATIC_DRAW);

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, index_count * sizeof(uint32_t), indices, GL_STATIC_DRAW);

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

		if (material->cfg.has_tex)
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
		glDeleteBuffers(1, &vbo);
		glDeleteBuffers(1, &ebo);
		glDeleteVertexArrays(1, &vao);

		vao = 0;
		ebo = 0;
		vbo = 0;

		material.unload();
	}
}

void Mesh::bind_uniforms(const CameraUniforms& uniforms, glm::dmat4 model, GLint did)
{
	logger->check(drawable != false, "Cannot draw a non-drawable mesh!");

	material->shader->use();

	material->set(textures, mat_override);
	material->set_core(uniforms, model, did);
}

void Mesh::draw_command()
{
	logger->check(drawable != false, "Cannot draw a non-drawable mesh!");

	logger->check(vao != 0, "Tried to render a non-loaded mesh");

	glBindVertexArray(vao);
	glDrawElements(GL_TRIANGLES, (GLsizei)index_count, GL_UNSIGNED_INT, 0);
	glBindVertexArray(0);

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



Model::Model(const aiScene* scene)
{
	logger->check(scene != nullptr, "Could not load model file, scene is nullptr");

	gpu_users = 0;
	uploaded = false;

	process_node(scene->mRootNode, scene, nullptr, true);
}

Model::~Model()
{
}

Model* load_model(const std::string& path, const std::string& name, const std::string& pkg, const cpptoml::table& cfg)
{
	unsigned int flags = 
		aiProcess_Triangulate | aiProcess_JoinIdenticalVertices |
		aiProcess_GenSmoothNormals | aiProcess_GenUVCoords |
	    aiProcess_CalcTangentSpace | aiProcess_SortByPType;

	Assimp::Importer importer;
	const aiScene* scene = importer.ReadFile(path, flags);
	if(scene == nullptr)
	{
		logger->warn("Error while loading model: {}", importer.GetErrorString());
	}
	
	Model* m = new Model(scene);

	return m;
}


Node* GPUModelPointer::get_node(std::string name)
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
	constexpr float BIG_NUMBER = 99999999999999.0f;
	glm::vec3 min = glm::vec3(BIG_NUMBER, BIG_NUMBER, BIG_NUMBER);
	glm::vec3 max = glm::vec3(-BIG_NUMBER, -BIG_NUMBER, -BIG_NUMBER);

	for (size_t i = 0; i < m->verts.size(); i++)
	{
		glm::vec3 v = m->verts[i];

		if (v.x < min.x) { min.x = v.x; }
		if (v.y < min.y) { min.y = v.y; }
		if (v.z < min.z) { min.z = v.z; }

		if (v.x > max.x) { max.x = v.x; }
		if (v.y > max.y) { max.y = v.y; }
		if (v.z > max.z) { max.z = v.z; }
	}

	return std::make_pair(min, max);

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

	for (size_t i = 0; i < n->meshes[0].verts.size(); i++)
	{
		target_c->addPoint(to_btVector3(n->meshes[0].verts[i]), false);
	}

	target_c->recalcLocalAabb();
	target_c->optimizeConvexHull();
}
