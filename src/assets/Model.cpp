#include "Model.h"

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>


void Model::process_node(aiNode* node, const aiScene* scene, Node* to)
{
	Node* n_node = new Node();

	n_node->name = node->mName.C_Str();

	logger->check(node_by_name.find(n_node->name) == node_by_name.end(), "We don't support non-unique node names");

	node_by_name[n_node->name] =n_node;

	for (unsigned int i = 0; i < node->mNumMeshes; i++)
	{
		// the node object only contains indices to index the actual objects in the scene. 
		// the scene contains all the data, node is just to keep stuff organized (like relations between nodes).
		aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
		process_mesh(mesh, scene, n_node);
	}

	// after we've processed all of the meshes (if any) we then recursively process each of the children nodes
	for (unsigned int i = 0; i < node->mNumChildren; i++)
	{
		process_node(node->mChildren[i], scene, n_node);
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

void Model::process_mesh(aiMesh* mesh, const aiScene* scene, Node* to)
{
	aiMaterial* ai_mat = scene->mMaterials[mesh->mMaterialIndex];


	AssetPointer mat_ptr = AssetPointer(ai_mat->GetName().C_Str());

	AssetHandle<Material> mat = AssetHandle<Material>(mat_ptr);


	to->meshes.push_back(Mesh(std::move(mat)));
	Mesh* m = &to->meshes[to->meshes.size() - 1];

	MeshConfig& config = m->material->cfg;

	size_t vert_size = config.get_vertex_size();
	size_t floats_per_vert = config.get_vertex_floats();

	// Work directly in the vector
	m->data = (float*)malloc(vert_size * mesh->mNumVertices);
	m->indices = (uint32_t*)malloc(sizeof(uint32_t) * mesh->mNumFaces * 3);

	m->data_size = vert_size * mesh->mNumVertices;

	bool log_once_tangents = false;
	bool log_once_colors = false;

	for (unsigned int i = 0; i < mesh->mNumFaces; i++)
	{
		aiFace face = mesh->mFaces[i];
		m->indices[i * 3 + 0] = face.mIndices[0];
		m->indices[i * 3 + 1] = face.mIndices[1];
		m->indices[i * 3 + 2] = face.mIndices[2];
	}

	m->index_count = mesh->mNumFaces * 3;

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



void Mesh::upload()
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

void Mesh::unload()
{
	glDeleteBuffers(1, &vbo);
	glDeleteBuffers(1, &ebo);
	glDeleteVertexArrays(1, &vao);

	vao = 0;
	ebo = 0;
	vbo = 0;

	material.unload();
}

void Mesh::bind_uniforms(const CameraUniforms& uniforms, glm::dmat4 model)
{
	// TODO
	material->shader->use();

	material->set();
	material->set_core(uniforms, model);
}

void Mesh::draw_command()
{
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

	process_node(scene->mRootNode, scene, nullptr);
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
	model->get_gpu();
}

GPUModelPointer::~GPUModelPointer()
{
	model->free_gpu();
}
