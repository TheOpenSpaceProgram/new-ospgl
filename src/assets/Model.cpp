#include "Model.h"

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <glm/gtx/quaternion.hpp>

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

	aiVector3D scaling, pos;
	aiQuaternion rot;
	node->mTransformation.Decompose(scaling, rot, pos);

	n_node->sub_transform = glm::dmat4(1.0);
	n_node->sub_transform = glm::translate(n_node->sub_transform, glm::dvec3(pos.x, pos.y, pos.z));
	n_node->sub_transform = glm::scale(n_node->sub_transform, glm::dvec3(scaling.x, scaling.y, scaling.z));
	n_node->sub_transform = n_node->sub_transform * glm::toMat4(glm::dquat(rot.w, rot.x, rot.y, rot.z));
	
	n_node->name = node->mName.C_Str();

	logger->check(node_by_name.find(n_node->name) == node_by_name.end(), "We don't support non-unique node names");

	node_by_name[n_node->name] =n_node;

	bool drawable = drawable_parent;

	if (n_node->name.rfind(Model::COLLIDER_PREFIX, 0) == 0) 
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

void Model::process_mesh(aiMesh* mesh, const aiScene* scene, Node* to, bool drawable)
{
	
	aiMaterial* ai_mat = scene->mMaterials[mesh->mMaterialIndex];
	
	AssetPointer mat_ptr;

	std::string ai_mat_name = ai_mat->GetName().C_Str();

	size_t pos = ai_mat_name.find_last_of('.');

	if (pos < ai_mat_name.size() && ai_mat_name.substr(pos) == "toml")
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

void Mesh::bind_uniforms(const CameraUniforms& uniforms, glm::dmat4 model)
{
	logger->check(drawable != false, "Cannot draw a non-drawable mesh!");

	// TODO
	material->shader->use();

	material->set(textures);
	material->set_core(uniforms, model);
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
	gpu_users = 0;
	uploaded = false;

	process_node(scene->mRootNode, scene, nullptr, true);
}

Model::~Model()
{
}

Model* loadModel(const std::string& path, const std::string& name, const std::string& pkg, const cpptoml::table& cfg)
{
	unsigned int flags = 
		aiProcess_Triangulate | aiProcess_JoinIdenticalVertices |
		aiProcess_GenSmoothNormals | aiProcess_GenUVCoords |
	    aiProcess_CalcTangentSpace | aiProcess_SortByPType;

	Assimp::Importer importer;
	const aiScene* scene = importer.ReadFile(path, flags);

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

void Node::draw_all_meshes(const CameraUniforms & uniforms, glm::dmat4 model)
{
	for (Mesh& mesh : meshes)
	{
		if (mesh.is_drawable())
		{
			mesh.bind_uniforms(uniforms, model);
			mesh.draw_command();
		}
	}

}

void Node::draw(const CameraUniforms& uniforms, glm::dmat4 model, bool ignore_our_subtform)
{
	glm::dmat4 n_model;
	if (ignore_our_subtform)
	{
		n_model = model;
	}
	else
	{
		n_model = sub_transform * model;
	}

	draw_all_meshes(uniforms, n_model);

	for (Node* node : children)
	{
		node->draw(uniforms, n_model, false);
	}
}
